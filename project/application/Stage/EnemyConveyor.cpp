#include "EnemyConveyor.h"
#include "StageLayout.h"
#include "Stage/Field.h"
#include "Camera.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <externals/nlohmann/json.hpp>

#ifdef USE_IMGUI
#include "ImGuiManager.h"
#include "externals/imgui/imgui.h"
#endif

using namespace TuboEngine;
using nlohmann::json;

namespace game {

using namespace game::layout;

namespace {
// ベルトの見た目に関する固定値（ジオメトリなので実行時変更の対象外）。
constexpr float kBeltY = 0.30f;   // ベルト表面の高さ
constexpr float kPartY = 1.15f;   // ベルト上を流れるパーツの中心高さ
constexpr float kBeltHalfW = 1.30f; // ベルト半幅(X)

Object3d* AddObj(std::vector<std::unique_ptr<Object3d>>& into, TuboEngine::Camera* cam,
                 const std::string& model, const Math::Vector3& pos,
                 const Math::Vector3& rot, const Math::Vector3& scale,
                 const Math::Vector4& color) {
	auto obj = std::make_unique<Object3d>();
	obj->Initialize(model);
	obj->SetCamera(cam);
	obj->SetPosition(pos);
	obj->SetRotation(rot);
	obj->SetScale(scale);
	obj->SetModelColor(color);
	Object3d* raw = obj.get();
	into.push_back(std::move(obj));
	return raw;
}

// PartDef から見た目の Object3d を1つ作る。
std::unique_ptr<Object3d> MakePartObj(TuboEngine::Camera* cam, const PartDef& def) {
	auto obj = std::make_unique<Object3d>();
	obj->Initialize(def.model);
	obj->SetCamera(cam);
	obj->SetScale(def.scale);
	obj->SetModelColor(def.color);
	return obj;
}
} // namespace

void EnemyConveyor::Initialize(TuboEngine::Camera* camera, Field* enemyField,
                               const Math::Vector3& cannonPos, const Math::Vector3& targetPos,
                               Field* playerField) {
	camera_ = camera;
	enemyField_ = enemyField;
	playerField_ = playerField;
	beltEnd_ = cannonPos;
	target_ = targetPos;

	// ベルトは敵フィールドの奥(+Z)から中央の大砲(終端)へ向かって走らせる。
	const float halfZ = enemyField_->GetHalfZ();
	beltStart_ = cannonPos + Math::Vector3{0.0f, 0.0f, halfZ * 0.78f};

	// 保存済みの調整パラメータがあれば読み込む（無ければ既定値のまま）。
	LoadParams(DefaultParamsPath());

	spawnTimer_ = params_.spawnInterval; // 開始直後に最初のパーツを流す
	nextIsBody_ = true;
	hasBody_ = false;
	hasHead_ = false;
	BuildBelt();
}

void EnemyConveyor::BuildBelt() {
	belt_.clear();
	const Math::Vector3 mid = (beltStart_ + beltEnd_) * 0.5f;
	const float length = beltStart_.z - beltEnd_.z; // +Z→中央 の長さ
	const float halfLen = length * 0.5f;
	const Math::Vector4 beltCol = {0.16f, 0.16f, 0.18f, 1.0f}; // ベルト面(暗色)
	const Math::Vector4 railCol = {0.42f, 0.42f, 0.46f, 1.0f}; // レール(金属)

	// ① ベルト面（1枚の細長い板）。block は基寸2 → 実寸 2*scale。
	AddObj(belt_, camera_, "block/block.obj", {mid.x, kBeltY, mid.z}, {0.0f, 0.0f, 0.0f},
	       {kBeltHalfW, 0.12f, halfLen}, beltCol);

	// ② 両脇のレール。
	AddObj(belt_, camera_, "block/block.obj", {mid.x - kBeltHalfW - 0.2f, kBeltY + 0.3f, mid.z},
	       {0.0f, 0.0f, 0.0f}, {0.18f, 0.42f, halfLen}, railCol);
	AddObj(belt_, camera_, "block/block.obj", {mid.x + kBeltHalfW + 0.2f, kBeltY + 0.3f, mid.z},
	       {0.0f, 0.0f, 0.0f}, {0.18f, 0.42f, halfLen}, railCol);

	// ③ ローラー風のリッジ（見た目のアクセント）。
	const int ridges = 10;
	for (int i = 0; i < ridges; ++i) {
		float t = (i + 0.5f) / ridges;
		float pz = Lerpf(beltStart_.z, beltEnd_.z, t);
		AddObj(belt_, camera_, "block/block.obj", {mid.x, kBeltY + 0.14f, pz}, {0.0f, 0.0f, 0.0f},
		       {kBeltHalfW * 0.9f, 0.06f, 0.18f}, {0.28f, 0.28f, 0.30f, 1.0f});
	}

	// ④ ゲート(生成口)：ベルト始点に門型の枠を建て、そこからパーツが出てくる。
	const Math::Vector4 gateCol = {0.34f, 0.36f, 0.42f, 1.0f};
	const Math::Vector4 gateDark = {0.08f, 0.09f, 0.12f, 1.0f};
	const float gx = beltStart_.x, gz = beltStart_.z;
	const float gw = kBeltHalfW + 0.35f;
	// 左右の柱。
	AddObj(belt_, camera_, "block/block.obj", {gx - gw, 1.6f, gz}, {0.0f, 0.0f, 0.0f},
	       {0.3f, 1.7f, 0.5f}, gateCol);
	AddObj(belt_, camera_, "block/block.obj", {gx + gw, 1.6f, gz}, {0.0f, 0.0f, 0.0f},
	       {0.3f, 1.7f, 0.5f}, gateCol);
	// 上の梁。
	AddObj(belt_, camera_, "block/block.obj", {gx, 3.2f, gz}, {0.0f, 0.0f, 0.0f},
	       {gw + 0.3f, 0.3f, 0.5f}, gateCol);
	// 奥の暗い開口部（ここから出てくる雰囲気）。
	AddObj(belt_, camera_, "block/block.obj", {gx, 1.5f, gz + 0.55f}, {0.0f, 0.0f, 0.0f},
	       {gw, 1.4f, 0.2f}, gateDark);
}

void EnemyConveyor::SpawnPart() {
	// 胴→頭を交互に出す（大砲で必ず1対そろうように）。
	// 敵はすべて同じ弾にするので、図鑑の先頭(標準の胴・頭)を固定で使う。
	Part p;
	if (nextIsBody_) {
		const PartDef& def = BodyDefs().front();
		p.model = MakePartObj(camera_, def);
		p.stats = def.stats;
		p.category = Category::Body;
	} else {
		const PartDef& def = HeadDefs().front();
		p.model = MakePartObj(camera_, def);
		p.stats = def.stats;
		p.category = Category::Head;
	}
	nextIsBody_ = !nextIsBody_;
	p.t = 0.0f;
	p.spin = 0.0f;
	parts_.push_back(std::move(p));
}

void EnemyConveyor::Deposit(Part& part) {
	// 到達したパーツを大砲のスロットへ。見た目はそのパーツのモデルを引き取って
	// 大砲の上に載せる。スロットが既に埋まっていれば無視（次の発射待ち）。
	if (part.category == Category::Body) {
		if (!hasBody_) {
			hasBody_ = true;
			bodyStats_ = part.stats;
			bodyMark_ = std::move(part.model);
		}
	} else {
		if (!hasHead_) {
			hasHead_ = true;
			headStats_ = part.stats;
			headMark_ = std::move(part.model);
		}
	}
	// 胴と頭がそろったら発射。
	if (hasBody_ && hasHead_) Fire();
}

void EnemyConveyor::Fire() {
	// 敵弾はすべて同じ。標準パーツ(標準胴+通常弾頭)を合成した“標準的なプレイヤー弾”と
	// 同じステータスにするので、弾速・弧＝軌道がプレイヤーの弾と同じになる。
	//  胴＋頭が大砲に揃ったことが発射条件（そろえる過程は工作と同じ）。
	ShellStats stats = CombineStats(BodyDefs().front().stats, HeadDefs().front().stats);

	// 自陣大砲(中央)そのものを狙う。着地点を固定して自弾と同じ線上を通す。
	// 発射高さもプレイヤー砲口(+1.5)に合わせ、弾道がプレイヤー弾の左右反転になるようにする。
	const Math::Vector3 target = target_;
	const Math::Vector3 start = beltEnd_ + Math::Vector3{0.0f, 1.5f, 0.0f};

	auto bullet = std::make_unique<Bullet>();
	bullet->Initialize(camera_, stats, start, target);
	bullet->Fire(); // 敵弾は即発射
	bullets_.push_back(std::move(bullet));

	// スロットを空にする（次の胴＋頭を待つ）。
	hasBody_ = false;
	hasHead_ = false;
	bodyMark_.reset();
	headMark_.reset();
}

void EnemyConveyor::Update() {
	// 敵陣の床が崩壊し始めたら、ベルト・ゲート・搬送中パーツ・装填パーツも一緒に落とす。
	if (enemyField_ && enemyField_->IsCollapsing()) {
		if (!beltFall_.Started()) {
			for (auto& obj : belt_) beltFall_.Add(obj.get());
			for (auto& p : parts_) beltFall_.Add(p.model.get());
			if (bodyMark_) beltFall_.Add(bodyMark_.get());
			if (headMark_) beltFall_.Add(headMark_.get());
		}
		beltFall_.Start();
		beltFall_.Update();
		// 飛翔中の弾は最後まで飛ばして掃除だけ続ける。
		for (auto& b : bullets_) b->Update();
		bullets_.erase(std::remove_if(bullets_.begin(), bullets_.end(),
		                              [](const std::unique_ptr<Bullet>& b) { return !b->IsActive(); }),
		               bullets_.end());
		return;
	}

	// 敵の攻撃が無効、または相手(自陣)の床が崩壊したら攻撃を止める。
	const bool active = params_.enabled &&
	                    (playerField_ == nullptr || !playerField_->IsCollapsing());

	// ── パーツの生成（ゲートから胴/頭が交互に出る） ──
	if (active) {
		spawnTimer_ += 1.0f;
		if (spawnTimer_ >= params_.spawnInterval) {
			spawnTimer_ = 0.0f;
			SpawnPart();
		}
	}

	// ── パーツの搬送：ベルトに沿って t を進め、終端でスロットへ ──
	const float travelSpeed = 1.0f / (params_.travelFrames > 1.0f ? params_.travelFrames : 1.0f);
	for (auto it = parts_.begin(); it != parts_.end();) {
		it->t += travelSpeed;
		if (it->t >= 1.0f) {
			Deposit(*it);        // 大砲に到達 → スロットへ（そろえば発射）
			it = parts_.erase(it);
			continue;
		}
		// ベルト上の位置へ配置（少し転がる回転を付ける）。
		Math::Vector3 base = Math::Vector3::Lerp(beltStart_, beltEnd_, it->t);
		it->spin += 0.12f;
		it->model->SetPosition({base.x, kPartY, base.z});
		it->model->SetRotation({it->spin, 0.0f, 0.0f});
		it->model->Update();
		++it;
	}

	// ── 大砲に載っているパーツ(胴・頭)の見た目を大砲の上に表示 ──
	//    「胴と頭がある時だけ撃つ」ことが見て分かるよう、装填状態を可視化する。
	if (bodyMark_) {
		bodyMark_->SetPosition(beltEnd_ + Math::Vector3{0.0f, 2.0f, 0.0f});
		bodyMark_->SetRotation({0.0f, 0.0f, 0.0f});
		bodyMark_->Update();
	}
	if (headMark_) {
		headMark_->SetPosition(beltEnd_ + Math::Vector3{0.0f, 2.9f, 0.0f});
		headMark_->SetRotation({0.0f, 0.0f, 0.0f});
		headMark_->Update();
	}

	// ── 砲弾の飛翔・命中：自陣の床を削る ──
	for (auto& b : bullets_) {
		b->Update();
		if (b->HasHitTarget() && playerField_) {
			const ShellStats& s = b->GetStats();
			float dmg = s.damage * params_.floorDamageMul;
			float radius = params_.hitBaseRadius + s.blast * params_.hitBlastRadius;
			playerField_->ApplyDamage(b->GetTarget(), dmg, radius);
		}
	}
	bullets_.erase(std::remove_if(bullets_.begin(), bullets_.end(),
	                              [](const std::unique_ptr<Bullet>& b) { return !b->IsActive(); }),
	               bullets_.end());

	for (auto& obj : belt_) obj->Update();
}

void EnemyConveyor::Draw() {
	for (auto& obj : belt_) obj->Draw();
	for (auto& p : parts_) p.model->Draw();
	if (bodyMark_) bodyMark_->Draw();
	if (headMark_) headMark_->Draw();
	for (auto& b : bullets_) b->Draw();
}

std::vector<Bullet*> EnemyConveyor::GetFlyingBullets() {
	std::vector<Bullet*> out;
	for (auto& b : bullets_) {
		if (b->IsActive() && b->IsFired()) out.push_back(b.get());
	}
	return out;
}

// ── JSON 保存 ──────────────────────────────────────────────
bool EnemyConveyor::SaveParams(const std::string& path) const {
	json root;
	root["enabled"] = params_.enabled;
	root["spawnInterval"] = params_.spawnInterval;
	root["travelFrames"] = params_.travelFrames;
	root["floorDamageMul"] = params_.floorDamageMul;
	root["hitBaseRadius"] = params_.hitBaseRadius;
	root["hitBlastRadius"] = params_.hitBlastRadius;

	try {
		std::filesystem::path p(path);
		if (p.has_parent_path()) std::filesystem::create_directories(p.parent_path());
	} catch (...) {
		return false;
	}
	std::ofstream ofs(path);
	if (!ofs) return false;
	ofs << root.dump(2);
	return true;
}

// ── JSON 読み込み ──────────────────────────────────────────
bool EnemyConveyor::LoadParams(const std::string& path) {
	std::ifstream ifs(path);
	if (!ifs) return false; // 無ければ既定値のまま

	json root;
	try {
		ifs >> root;
	} catch (...) {
		return false;
	}
	// 欠けている項目は現在値を既定にして読む。
	params_.enabled = root.value("enabled", params_.enabled);
	params_.spawnInterval = root.value("spawnInterval", params_.spawnInterval);
	params_.travelFrames = root.value("travelFrames", params_.travelFrames);
	params_.floorDamageMul = root.value("floorDamageMul", params_.floorDamageMul);
	params_.hitBaseRadius = root.value("hitBaseRadius", params_.hitBaseRadius);
	params_.hitBlastRadius = root.value("hitBlastRadius", params_.hitBlastRadius);
	return true;
}

#ifdef USE_IMGUI
void EnemyConveyor::DrawImGui() {
	if (ImGuiManager::GetInstance()->BeginPanel("EnemyAttack")) {
		ImGui::TextWrapped("敵の攻撃(ベルトコンベア)の調整。胴＋頭が大砲に揃うと合成して発射する。");
		ImGui::Separator();

		ImGui::Checkbox("敵の攻撃 有効", &params_.enabled);
		ImGui::DragFloat("生成間隔(フレーム)", &params_.spawnInterval, 1.0f, 10.0f, 600.0f, "%.0f");
		ImGui::DragFloat("搬送フレーム(小さいほど速い)", &params_.travelFrames, 1.0f, 20.0f, 600.0f, "%.0f");
		ImGui::DragFloat("床ダメージ倍率", &params_.floorDamageMul, 0.1f, 0.0f, 20.0f, "%.2f");
		ImGui::DragFloat("着弾半径", &params_.hitBaseRadius, 0.1f, 0.5f, 20.0f, "%.2f");
		ImGui::DragFloat("爆発半径係数", &params_.hitBlastRadius, 0.1f, 0.0f, 10.0f, "%.2f");

		ImGui::Separator();
		ImGui::Text("状態 : 搬送中 %d / 大砲[胴%s 頭%s] / 敵弾 %d 発",
		            PartCount(), hasBody_ ? "○" : "×", hasHead_ ? "○" : "×", BulletCount());

		ImGui::Separator();
		if (ImGui::Button("JSONへ保存")) SaveParams(DefaultParamsPath());
		ImGui::SameLine();
		if (ImGui::Button("JSONから読込")) LoadParams(DefaultParamsPath());
		ImGui::SameLine();
		if (ImGui::Button("既定値へ")) params_ = Params{};
		ImGui::TextDisabled("保存先: %s", DefaultParamsPath());
	}
	ImGuiManager::GetInstance()->EndPanel();
}
#endif

} // namespace game
