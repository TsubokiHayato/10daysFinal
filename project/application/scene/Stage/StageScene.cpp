#include "StageScene.h"
#include "GameScenes.h"
#include "SceneManager.h"

#include "Camera.h"
#include "Input.h"
#include "LineManager.h"
#include "TextManager.h"

#include <algorithm> // remove_if
#include <cstdlib>   // rand

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

using namespace TuboEngine;

namespace {
	// 各陣フィールドの中心 |X|。自陣=-, 敵陣=+。中央に谷ができる距離にする。
	constexpr float kFieldOffsetX = 30.0f;

	// 陣営の床色(市松2色)。
	constexpr Math::Vector4 kSelfFloorA = { 0.30f, 0.42f, 0.66f, 1.0f };
	constexpr Math::Vector4 kSelfFloorB = { 0.22f, 0.32f, 0.54f, 1.0f };
	constexpr Math::Vector4 kEnemyFloorA = { 0.62f, 0.32f, 0.34f, 1.0f };
	constexpr Math::Vector4 kEnemyFloorB = { 0.50f, 0.24f, 0.26f, 1.0f };

	// 城の紋章(旗代わり)色。
	constexpr Math::Vector4 kSelfColor = { 0.35f, 0.55f, 1.0f, 1.0f };
	constexpr Math::Vector4 kEnemyColor = { 1.0f, 0.35f, 0.32f, 1.0f };

	constexpr float kHalfPi = 1.57079633f;

	// アイテム操作の距離。
	constexpr float kPickRange = 3.0f;      // 地面アイテムを拾える距離
	constexpr float kBenchRange = 4.5f;     // 工作台に載せられる距離
	constexpr float kCannonRange = 5.0f;    // 砲台に弾を装填できる距離
	constexpr float kItemGroundY = 0.6f;    // 落ちているアイテムの基準高さ
	constexpr float kEnemyCastleHP = 200.0f; // 敵の城のHP

	float Lerp(float a, float b, float t) { return a + (b - a) * t; }

	// XZ平面上の距離の2乗（高さは無視）。
	float Dist2XZ(const Math::Vector3& a, const Math::Vector3& b) {
		float dx = a.x - b.x, dz = a.z - b.z;
		return dx * dx + dz * dz;
	}
} // namespace

// =============================================================================
//  Initialize
// =============================================================================
void StageScene::Initialize() {
	// ① 追従見下ろしカメラ（このシーンの主カメラ）
	followCamera_ = std::make_unique<game::FollowCamera>();
	followCamera_->Initialize();

	// F2 で乗っ取るデバッグカメラ
	debugCamera_ = std::make_unique<DebugCamera>();

	TuboEngine::Camera* cam = followCamera_->GetCamera();

	// ② フィールド（自陣=左/-X, 敵陣=右/+X）
	selfField_ = std::make_unique<game::Field>();
	selfField_->Initialize(cam, { -kFieldOffsetX, 0.0f, 0.0f }, kSelfFloorA, kSelfFloorB);
	enemyField_ = std::make_unique<game::Field>();
	enemyField_->Initialize(cam, { kFieldOffsetX, 0.0f, 0.0f }, kEnemyFloorA, kEnemyFloorB);

	// ③ 城(基地)：各陣の外側の端(谷と反対側)に建てる。
	const float hx = selfField_->GetHalfX();
	const Math::Vector3 enemyCastleCenter = { kFieldOffsetX + hx - 8.0f, 0.0f, 0.0f };
	BuildCastle({ -kFieldOffsetX - hx + 8.0f, 0.0f, 0.0f }, kSelfColor);  // 自陣の城(左端)
	BuildCastle(enemyCastleCenter, kEnemyColor);                        // 敵陣の城(右端)

	// 敵の城のロジック（HP・状態異常）を城の位置に用意。弾の的にもなる。
	enemyCastle_ = std::make_unique<game::Castle>();
	enemyCastle_->Initialize(enemyCastleCenter, kEnemyCastleHP);

	// ④ 砲台：各陣の内側の端(谷側)に、相手側を向けて置く。見た目は両陣ともプロップ。
	const Math::Vector3 selfCannonPos = { -kFieldOffsetX + hx - 6.0f, 0.0f, 0.0f };
	AddProp("artilleryBattery/artillery battery.obj", selfCannonPos, { 0.0f, -kHalfPi, 0.0f },
		{ 2.6f, 2.6f, 2.6f }, { 1.0f, 1.0f, 1.0f, 1.0f });
	enemyCannon_ = AddProp("artilleryBattery/artillery battery.obj",
		{ kFieldOffsetX - hx + 6.0f, 0.0f, 0.0f }, { 0.0f, kHalfPi, 0.0f },
		{ 2.6f, 2.6f, 2.6f }, { 1.0f, 1.0f, 1.0f, 1.0f });

	// 自陣の砲台は作者作の Cannon クラスを「発射フラグを持つ装置」としてロジックのみ使う。
	//  ・Cannon::Initialize は原点に square モデルを作るが、描画はしない（見た目は上のプロップ）。
	//  ・SPACEで isBulletFired_ が立つので、それを HandleBullets で読んで弾を発射する。
	selfCannon_ = std::make_unique<game::Cannon>();
	selfCannon_->Initialize(cam, selfCannonPos);
	// 砲口（弾の発射始点）は砲台プロップの少し上。
	muzzle_ = selfCannonPos + Math::Vector3{ 0.0f, 1.5f, 0.0f };
	// Cannon::Update が参照するダミー弾をセット（SPACEで null 参照しないため。描画しない）。
	cannonRound_ = std::make_unique<Object3d>();
	cannonRound_->Initialize("playerBullet/playerBullet.obj");
	cannonRound_->SetCamera(cam);
	selfCannon_->SetBullet(cannonRound_.get());

	// ⑤ プレイヤー。自陣の中央あたりに配置し、自陣の範囲でクランプ。
	player_ = std::make_unique<game::Player>();
	player_->Initialize(cam);
	player_->SetPosition({ -kFieldOffsetX, 1.0f, -selfField_->GetHalfZ() * 0.4f });
	player_->SetMoveBounds(selfField_->GetCenter(),
		selfField_->GetHalfX() - 1.5f, selfField_->GetHalfZ() - 1.5f);

	// ⑥ 工作台（製作台）。自陣の中央やや手前に置く。
	const Math::Vector3 selfCenter = selfField_->GetCenter();
	workbench_ = std::make_unique<game::Workbench>();
	workbench_->Initialize(cam, { selfCenter.x, 0.0f, selfCenter.z + 4.0f });

	// ⑦ パーツを自陣にランダムに散らばらせる。
	ScatterParts();

	// カメラをプレイヤー位置へスナップ（開始時にワープして見えないように）
	followCamera_->SnapTo(player_->GetPosition());

	TextManager::GetInstance()->LoadTextLayout("Resources/Text/StageTutorial.json");

	visualManager_ = VisualManager::GetInstance();

	visualManager_->Initialize(cam);
}

game::Item* StageScene::SpawnPart(const game::PartDef& def, const Math::Vector3& pos) {
	auto item = std::make_unique<game::Item>();
	item->InitializeFromDef(followCamera_->GetCamera(), def, pos);
	game::Item* raw = item.get();
	items_.push_back(std::move(item));
	return raw;
}

game::Item* StageScene::SpawnShell(const game::ShellStats& stats, const Math::Vector3& pos) {
	auto item = std::make_unique<game::Item>();
	item->InitializeAsShell(followCamera_->GetCamera(), stats, pos);
	game::Item* raw = item.get();
	items_.push_back(std::move(item));
	return raw;
}

// 自陣に各種パーツをランダム配置。図鑑の全種類を最低1つは撒く。
void StageScene::ScatterParts() {
	const Math::Vector3 c = selfField_->GetCenter();
	const float hx = selfField_->GetHalfX() - 3.0f;
	const float hz = selfField_->GetHalfZ() - 3.0f;

	auto randRange = [](float a, float b) {
		return a + (b - a) * (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX));
		};
	auto scatter = [&](const std::vector<game::PartDef>& defs, int perType) {
		for (const auto& def : defs) {
			for (int i = 0; i < perType; ++i) {
				Math::Vector3 p = { c.x + randRange(-hx, hx), kItemGroundY, c.z + randRange(-hz, hz) };
				SpawnPart(def, p);
			}
		}
		};
	scatter(game::BodyDefs(), 2); // 各胴体を2個ずつ
	scatter(game::HeadDefs(), 2); // 各頭を2個ずつ
}

// props_ に Object3d を1つ積む。
TuboEngine::Object3d* StageScene::AddProp(const std::string& model, const Math::Vector3& pos,
	const Math::Vector3& rot, const Math::Vector3& scale,
	const Math::Vector4& color) {
	auto obj = std::make_unique<Object3d>();
	obj->Initialize(model);
	obj->SetCamera(followCamera_->GetCamera());
	obj->SetPosition(pos);
	obj->SetRotation(rot);
	obj->SetScale(scale);
	obj->SetModelColor(color);
	Object3d* raw = obj.get();
	props_.push_back(std::move(obj));
	return raw;
}

// 城：天守(中央の大ブロック)+四隅の塔+屋根の紋章(crown)。
void StageScene::BuildCastle(const Math::Vector3& center, const Math::Vector4& color) {
	const float cx = center.x, cz = center.z;
	// 石材は白っぽく、陣営色をほんのり混ぜる。
	Math::Vector4 stone = { Lerp(0.85f, color.x, 0.3f), Lerp(0.85f, color.y, 0.3f),
						   Lerp(0.85f, color.z, 0.3f), 1.0f };

	// 天守(中央) 6x10x6, 底面を地面(y=0)に合わせる。
	AddProp("block/block.obj", { cx, 5.0f, cz }, { 0, 0, 0 }, { 3.0f, 5.0f, 3.0f }, stone);

	// 四隅の塔 2x14x2。
	const float t = 4.0f;
	AddProp("block/block.obj", { cx - t, 7.0f, cz - t }, { 0, 0, 0 }, { 1.0f, 7.0f, 1.0f }, stone);
	AddProp("block/block.obj", { cx - t, 7.0f, cz + t }, { 0, 0, 0 }, { 1.0f, 7.0f, 1.0f }, stone);
	AddProp("block/block.obj", { cx + t, 7.0f, cz - t }, { 0, 0, 0 }, { 1.0f, 7.0f, 1.0f }, stone);
	AddProp("block/block.obj", { cx + t, 7.0f, cz + t }, { 0, 0, 0 }, { 1.0f, 7.0f, 1.0f }, stone);

	// 屋根の紋章(旗代わり)。陣営色そのまま。
	AddProp("crown/crown.obj", { cx, 11.6f, cz }, { 0, 0, 0 }, { 2.2f, 2.2f, 2.2f }, color);
}

// =============================================================================
//  Update
// =============================================================================
void StageScene::Update() {
	// (1) プレイヤー入力・移動
	player_->Update();

	// (2) 全体俯瞰(TAB長押し)へ滑らかに寄せる/戻す。
	const bool wantOverview = Input::GetInstance()->PushKey(DIK_TAB);
	overview_ += ((wantOverview ? 1.0f : 0.0f) - overview_) * 0.12f;

	// (5) フィールド更新
	selfField_->Update();
	enemyField_->Update();

	// (5.5) アイテム操作（拾う/捨てる/工作台へ）と合成
	HandleItemInteraction();

	// (5.6) 砲弾の装填・発射・飛翔・命中
	HandleBullets();

	// (6) 城・砲台（砲台の Update は HandleBullets 内で処理済み）
	for (auto& p : props_) p->Update();
	enemyCastle_->Update();

	// (6.5) 工作台とアイテム
	workbench_->Update();
	for (auto& it : items_) it->Update();

	// (7) 任意：ワールドグリッド（デバッグ表示）
	if (showGrid_) {
		LineManager::GetInstance()->DrawGrid(
			(kFieldOffsetX + selfField_->GetHalfX()) * 2.0f, 24, { 0.0f, 0.01f, 0.0f },
			{ 0.4f, 0.4f, 0.4f, 1.0f });
	}

	TuboEngine::TextManager::GetInstance()->UpdateAll();

	// (3) カメラ：基本はプレイヤー追従。overview_ の分だけ戦場中央(原点)を見渡す。
	followCamera_->Update(player_->GetPosition(), { 0.0f, 0.0f, 0.0f }, overview_);

	// (4) F2 デバッグカメラ（主カメラを乗っ取る）。追従更新の“後”に適用する。
	debugCamera_->Update(followCamera_->GetCamera());

	visualManager_->Update();
}

// アイテムの拾う/捨てる/工作台への載せ降ろし・合成を処理する。
//  E : 手ぶら→最寄りの地面アイテムを拾う / 手持ち→工作台が近ければ載せる
//  Q : 手持ち→その場の地面に捨てる
void StageScene::HandleItemInteraction() {
	Input* in = Input::GetInstance();
	const bool pick = in->TriggerKey(DIK_E);
	const bool drop = in->TriggerKey(DIK_Q);
	const Math::Vector3 pp = player_->GetPosition();

	if (pick) {
		if (player_->IsCarrying()) {
			// 工作台が近ければ空きスロットへ載せる。
			if (Dist2XZ(pp, workbench_->GetPosition()) < kBenchRange * kBenchRange) {
				if (workbench_->TryDeposit(player_->GetCarried())) {
					player_->SetCarried(nullptr);
				}
			}
		} else {
			// 手ぶら：最寄りの地面アイテムを拾う。
			game::Item* best = nullptr;
			float bestD2 = kPickRange * kPickRange;
			for (auto& it : items_) {
				if (!it->IsActive() || it->GetState() != game::Item::State::Ground) continue;
				float d2 = Dist2XZ(pp, it->GetPosition());
				if (d2 < bestD2) { bestD2 = d2; best = it.get(); }
			}
			if (best) {
				best->SetState(game::Item::State::Held);
				player_->SetCarried(best);
			}
		}
	}

	if (drop && player_->IsCarrying()) {
		game::Item* it = player_->GetCarried();
		it->SetPosition({ pp.x, kItemGroundY, pp.z });
		it->SetState(game::Item::State::Ground);
		player_->SetCarried(nullptr);
	}

	// 胴体+頭がそろったら合成 → 砲弾を出力位置に生成。
	if (workbench_->IsReady()) {
		game::ShellStats stats = workbench_->Combine(); // 入力2つを消費
		SpawnShell(stats, workbench_->GetOutputPosition());
	}
}

// 砲弾の装填・発射・飛翔・命中を処理する。
//  R     : 砲弾を手持ちしていて砲台が近く、装填中の弾が無ければ弾を装填する（未発射）。
//  SPACE : Cannon::Update が発射フラグ(isBulletFired_)を立てる → 装填弾を Fire()。
//  飛翔・命中判定は弾自身と城が持ち、ここでは仲介するだけ。
void StageScene::HandleBullets() {
	Input* in = Input::GetInstance();
	const Math::Vector3 pp = player_->GetPosition();

	// ── 装填：手持ちの砲弾を Bullet 化して砲口に置く（まだ発射しない）──

	game::Item* carried = player_->GetCarried();
	if (carried && carried->GetCategory() == game::Category::Shell &&
		Dist2XZ(pp, muzzle_) < kCannonRange * kCannonRange) {

		player_->SetInIconRange(true);

		if (in->TriggerKey(DIK_R) && !pendingBullet_) {
			// 弾は「全ステータス＋的」を持って生まれる。的は敵の城の少し上。
			const Math::Vector3 target = enemyCastle_->GetPosition() + Math::Vector3{ 0.0f, 3.0f, 0.0f };
			auto bullet = std::make_unique<game::Bullet>();
			bullet->Initialize(followCamera_->GetCamera(), carried->GetStats(), muzzle_, target);
			pendingBullet_ = bullet.get();      // 発射待ち（Fireされるまで砲口で静止）
			bullets_.push_back(std::move(bullet));
			// 元の砲弾アイテムは消費する。
			carried->SetActive(false);
			player_->SetCarried(nullptr);
			// 砲台を再装填状態に戻す（前弾のフラグが残っていても撃てるように）。
			selfCannon_->SetIsBulletFired(false);
			selfCannon_->SetIsLoading(true);
		}
	}

	// ── 砲台の更新：SPACEで撃つ/撃たないフラグを立てる（作者作の Cannon をそのまま使う）──
	selfCannon_->Update();

	// ── 発射：砲台のフラグが立ったら、装填済みの弾に Fire() を伝える ──
	if (selfCannon_->GetIsBulletFired()) {
		if (pendingBullet_) {
			pendingBullet_->Fire(); // 以後は弾が自分で的へ飛ぶ
			pendingBullet_ = nullptr;
		}
		selfCannon_->SetIsBulletFired(false); // 次弾に備えてフラグを戻す
	}

	// ── 飛翔と命中：弾を更新し、的に到達したら城が受ける ──
	for (auto& b : bullets_) {
		b->Update();
		if (b->HasHitTarget()) {
			enemyCastle_->OnHit(*b); // 城が状態異常フラグを取得する
		}
	}

	// 消滅した弾を掃除する。
	bullets_.erase(std::remove_if(bullets_.begin(), bullets_.end(),
		[](const std::unique_ptr<game::Bullet>& b) { return !b->IsActive(); }),
		bullets_.end());
}

// =============================================================================
//  描画フェーズ
// =============================================================================
void StageScene::Object3DDraw() {
	selfField_->Draw();
	enemyField_->Draw();
	for (auto& p : props_) p->Draw();
	// selfCannon_ は発射フラグ用のロジックのみ（見た目はプロップ）なので描画しない。
	for (auto& b : bullets_) b->Draw();
	workbench_->Draw();
	for (auto& it : items_) it->Draw();
	player_->Draw();
	selfCannon_->Draw();
}

void StageScene::SpriteDraw() {
	TuboEngine::TextManager::GetInstance()->DrawAll();
}

void StageScene::ParticleDraw() {}

// =============================================================================
//  ImGui（Debug ビルドのみ）
// =============================================================================
void StageScene::ImGuiDraw() {
#ifdef USE_IMGUI
	if (ImGuiManager::GetInstance()->BeginPanel("Stage")) {
		ImGui::TextWrapped("見下ろしステージ(自陣/敵陣)。WASD移動 / E=拾う・工作台へ載せる / Q=捨てる / R=砲台に装填 / SPACE=発射 / TAB長押しで全体表示 / F2デバッグカメラ。");
		ImGui::Separator();

		// 砲台と敵の城の状態。
		ImGui::Text("砲台 : %s / 場の弾 : %d",
			pendingBullet_ ? "装填済み(SPACEで発射)" : "空(Rで装填)",
			static_cast<int>(bullets_.size()));
		ImGui::Text("敵の城 : HP %.1f / %s", enemyCastle_->GetHP(),
			enemyCastle_->IsPoisoned() ? "毒状態" : "正常");
		ImGui::Separator();

		// アイテム/クラフトの状態表示。
		int ground = 0, shells = 0;
		for (auto& it : items_) {
			if (!it->IsActive()) continue;
			if (it->GetState() == game::Item::State::Ground) ++ground;
			if (it->GetCategory() == game::Category::Shell) ++shells;
		}
		game::Item* carried = player_->GetCarried();
		ImGui::Text("手持ち : %s", carried ? carried->GetName().c_str() : "なし");
		game::Item* body = workbench_->GetBodySlot();
		game::Item* head = workbench_->GetHeadSlot();
		ImGui::Text("工作台 : 胴[%s] 頭[%s]",
			body ? body->GetName().c_str() : "空",
			head ? head->GetName().c_str() : "空");
		ImGui::Text("地面のアイテム : %d 個 / 砲弾 : %d 個", ground, shells);

		// 手持ち/直近の砲弾のステータスを見せる。
		if (carried) {
			const game::PartStats& s = carried->GetStats();
			ImGui::Text("  威力%.0f 弾速%.2f 爆発%.1f 重%.1f", s.damage, s.speed, s.blast, s.weight);
		}
		ImGui::Separator();

		ImGui::Text("Overview : %.2f", overview_);
		ImGui::Checkbox("World Grid", &showGrid_);
		ImGui::Spacing();

		SceneManager* sm = SceneManager::GetInstance();
		if (ImGui::Button("Title へ")) sm->ChangeScene(TITLE);
		ImGui::SameLine();
		if (ImGui::Button("Sample へ")) sm->ChangeScene(SAMPLE);
		ImGui::SameLine();
		if (ImGui::Button("リロード")) sm->ChangeScene(STAGE);

		TuboEngine::TextManager::GetInstance()->DrawImGui();
	}
	ImGuiManager::GetInstance()->EndPanel();

	player_->DrawImGui();
	followCamera_->DrawImGui();
	debugCamera_->DrawImGui();
#endif
}

// =============================================================================
//  Finalize
// =============================================================================
void StageScene::Finalize() {
	TuboEngine::TextManager::GetInstance()->ClearAllTexts();
	TuboEngine::TextManager::GetInstance()->ClearAllSprites();
}
