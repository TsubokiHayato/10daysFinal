#include "ItemField.h"
#include "StageLayout.h"
#include "Stage/Player.h"
#include "Stage/Field.h"
#include "Camera.h"
#include "Input.h"
#include <algorithm> // remove_if
#include <cstdlib>   // rand

using namespace TuboEngine;

namespace game {

using namespace game::layout;

namespace {
// [0,1) の擬似乱数。
float Rand01() { return static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX) + 1.0f); }
float RandRange(float a, float b) { return a + (b - a) * Rand01(); }
} // namespace

void ItemField::Initialize(TuboEngine::Camera* camera, Field* selfField) {
	camera_ = camera;
	selfField_ = selfField;

	// 工作台（製作台）。中央の大砲(キャノン)のすぐ手前(下)＝足元に置く。
	const Math::Vector3 selfCenter = selfField_->GetCenter();
	workbench_ = std::make_unique<Workbench>();
	workbench_->Initialize(camera_, {selfCenter.x, 0.0f, selfCenter.z - 4.0f});

	// パーツを自陣にランダムに散らばらせる。
	ScatterParts();
}

Item* ItemField::SpawnPart(const PartDef& def, const Math::Vector3& pos) {
	auto item = std::make_unique<Item>();
	item->InitializeFromDef(camera_, def, pos);
	Item* raw = item.get();
	items_.push_back(std::move(item));
	return raw;
}

Item* ItemField::SpawnShell(const ShellStats& stats, const Math::Vector3& pos) {
	auto item = std::make_unique<Item>();
	item->InitializeAsShell(camera_, stats, pos);
	Item* raw = item.get();
	items_.push_back(std::move(item));
	return raw;
}

// 自陣に各種パーツを初期配置。図鑑の全種類を最低1つは撒く（砲台・作業台まわりは避ける）。
void ItemField::ScatterParts() {
	auto scatter = [&](const std::vector<PartDef>& defs, int perType) {
		for (const auto& def : defs) {
			for (int i = 0; i < perType; ++i) {
				if (static_cast<int>(items_.size()) >= kMaxGroundItems) return;
				Math::Vector3 p;
				if (FindRandomGroundPos(p)) SpawnPart(def, p);
			}
		}
	};
	scatter(BodyDefs(), 2); // 各胴体を2個ずつ
	scatter(HeadDefs(), 2); // 各頭を2個ずつ
}

// 砲台(自陣中央)・作業台の占有圏に入っていなければ true。
bool ItemField::IsSpawnAreaClear(const Math::Vector3& pos) const {
	// 砲台は自陣フィールドの中央に置かれている。
	if (Dist2XZ(pos, selfField_->GetCenter()) < kCannonKeepOut * kCannonKeepOut) return false;
	if (Dist2XZ(pos, workbench_->GetPosition()) < kBenchKeepOut * kBenchKeepOut) return false;
	return true;
}

// 自陣内のランダムな地面座標を探す。砲台・作業台の周辺は避ける。
bool ItemField::FindRandomGroundPos(Math::Vector3& out) const {
	const Math::Vector3 c = selfField_->GetCenter();
	const float hx = selfField_->GetHalfX() - 3.0f;
	const float hz = selfField_->GetHalfZ() - 3.0f;
	for (int attempt = 0; attempt < 16; ++attempt) {
		Math::Vector3 p = {c.x + RandRange(-hx, hx), kItemGroundY, c.z + RandRange(-hz, hz)};
		if (IsSpawnAreaClear(p)) { out = p; return true; }
	}
	return false; // 空き場所が見つからなかった
}

// 図鑑(胴/頭)からランダムに1個、最大数未満のときだけ湧かせる。
void ItemField::TrySpawnRandomItem() {
	if (CountGround() >= kMaxGroundItems) return;

	Math::Vector3 pos;
	if (!FindRandomGroundPos(pos)) return; // 空きが無ければ今回は見送り

	// 胴 or 頭をランダムに選び、さらにその中の種類をランダムに決める。
	const std::vector<PartDef>& defs = (Rand01() < 0.5f) ? BodyDefs() : HeadDefs();
	if (defs.empty()) return;
	const PartDef& def = defs[static_cast<size_t>(Rand01() * defs.size()) % defs.size()];
	SpawnPart(def, pos);
}

void ItemField::HandleInteraction(Player* player) {
	const Math::Vector3 pp = player->GetPosition();

	if (Input::GetInstance()->TriggerKey(DIK_E)) {
		if (player->IsCarrying()) {
			// 工作台が近ければ空きスロットへ載せる。載せられなければ破棄する。
			bool deposited = false;
			if (Dist2XZ(pp, workbench_->GetPosition()) < kBenchRange * kBenchRange) {
				deposited = workbench_->TryDeposit(player->GetCarried());
				if (deposited) player->SetCarried(nullptr);
				tutorialFlagCarried_ = true;
			}
			if (!deposited) {
				// 手持ち→その場の地面に破棄する。
				Item* it = player->GetCarried();
				it->SetPosition({pp.x, kItemGroundY, pp.z});
				it->SetState(Item::State::Ground);
				player->SetCarried(nullptr);
			}
		} else {
			// 手ぶら：最寄りの地面アイテムを拾う。
			Item* best = nullptr;
			float bestD2 = kPickRange * kPickRange;
			for (auto& it : items_) {
				if (!it->IsActive() || it->GetState() != Item::State::Ground) continue;
				float d2 = Dist2XZ(pp, it->GetPosition());
				if (d2 < bestD2) { bestD2 = d2; best = it.get(); }
			}
			if (best) {
				best->SetState(Item::State::Held);
				player->SetCarried(best);
			}
		}
	}
}

void ItemField::Update() {
	// 自陣の床が崩壊し始めたら、作業台とアイテムも一緒に落とす（以後は通常処理を止める）。
	if (selfField_ && selfField_->IsCollapsing()) {
		if (!itemFall_.Started()) {
			itemFall_.Add(workbench_->GetTable());
			for (auto& it : items_) {
				if (it->IsActive()) itemFall_.Add(it->GetModel());
			}
		}
		itemFall_.Start();
		itemFall_.Update();
		return;
	}

	// 胴体+頭がそろったら合成 → 砲弾を出力位置に生成。
	if (workbench_->IsReady()) {
		ShellStats stats = workbench_->Combine(); // 入力2つを消費
		SpawnShell(stats, workbench_->GetOutputPosition());
		tutorialFlagCreate_ = true;
	}

	// アイテムの自動湧き：一定間隔で、最大数に達していなければ1個追加する。
	spawnTimer_ += 1.0f;
	if (spawnTimer_ >= kItemSpawnInterval) {
		spawnTimer_ = 0.0f;
		TrySpawnRandomItem();
	}

	workbench_->Update();
	for (auto& it : items_) it->Update();

	// 消費・破棄されて非アクティブになったアイテムを掃除する（items_の肥大化防止）。
	//  ・工作台スロット中/手持ちのアイテムはアクティブなので消えない。
	items_.erase(std::remove_if(items_.begin(), items_.end(),
	                            [](const std::unique_ptr<Item>& it) { return !it->IsActive(); }),
	             items_.end());
}

void ItemField::Draw() {
	workbench_->Draw();
	for (auto& it : items_) it->Draw();
}

int ItemField::CountGround() const {
	int n = 0;
	for (auto& it : items_) {
		if (it->IsActive() && it->GetState() == Item::State::Ground) ++n;
	}
	return n;
}

int ItemField::CountShells() const {
	int n = 0;
	for (auto& it : items_) {
		if (it->IsActive() && it->GetCategory() == Category::Shell) ++n;
	}
	return n;
}

} // namespace game
