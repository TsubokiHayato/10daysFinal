#include "ItemField.h"
#include "StageLayout.h"
#include "Stage/Player.h"
#include "Stage/Field.h"
#include "Camera.h"
#include "Input.h"
#include <cstdlib> // rand

using namespace TuboEngine;

namespace game {

using namespace game::layout;

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

// 自陣に各種パーツをランダム配置。図鑑の全種類を最低1つは撒く。
void ItemField::ScatterParts() {
	const Math::Vector3 c = selfField_->GetCenter();
	const float hx = selfField_->GetHalfX() - 3.0f;
	const float hz = selfField_->GetHalfZ() - 3.0f;

	auto randRange = [](float a, float b) {
		return a + (b - a) * (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX));
	};
	auto scatter = [&](const std::vector<PartDef>& defs, int perType) {
		for (const auto& def : defs) {
			for (int i = 0; i < perType; ++i) {
				Math::Vector3 p = {c.x + randRange(-hx, hx), kItemGroundY, c.z + randRange(-hz, hz)};
				SpawnPart(def, p);
			}
		}
	};
	scatter(BodyDefs(), 2); // 各胴体を2個ずつ
	scatter(HeadDefs(), 2); // 各頭を2個ずつ
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
	// 胴体+頭がそろったら合成 → 砲弾を出力位置に生成。
	if (workbench_->IsReady()) {
		ShellStats stats = workbench_->Combine(); // 入力2つを消費
		SpawnShell(stats, workbench_->GetOutputPosition());
	}

	workbench_->Update();
	for (auto& it : items_) it->Update();
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
