#include "Workbench.h"

#include "Camera.h"
#include "Item.h"

using namespace TuboEngine;

namespace game {

void Workbench::Initialize(TuboEngine::Camera* camera, const Math::Vector3& pos) {
	position_ = pos;

	// 台：横長のブロックを台にする。天板の高さ ≒ y=1。
	table_ = std::make_unique<Object3d>();
	table_->Initialize("block/block.obj");
	table_->SetCamera(camera);
	table_->SetPosition({pos.x, 0.5f, pos.z});
	table_->SetScale({2.6f, 0.5f, 1.4f});
	table_->SetModelColor({0.52f, 0.38f, 0.24f, 1.0f}); // 木の台っぽい茶色

	// 胴体=左、頭=右のスロット。合成結果は手前(-Z)に出す。
	bodySlotPos_ = {pos.x - 1.1f, 1.4f, pos.z};
	headSlotPos_ = {pos.x + 1.1f, 1.4f, pos.z};
	outputPos_ = {pos.x, 0.6f, pos.z - 2.6f};
}

void Workbench::SetCamera(TuboEngine::Camera* camera) {
	if (table_) table_->SetCamera(camera);
}

bool Workbench::TryDeposit(Item* item) {
	if (!item) return false;

	if (item->GetCategory() == Category::Body) {
		if (bodySlot_) return false; // 胴体スロットは埋まっている
		bodySlot_ = item;
		item->SetState(Item::State::Slot);
		item->SetPosition(bodySlotPos_);
		return true;
	}
	if (item->GetCategory() == Category::Head) {
		if (headSlot_) return false; // 頭スロットは埋まっている
		headSlot_ = item;
		item->SetState(Item::State::Slot);
		item->SetPosition(headSlotPos_);
		return true;
	}
	return false; // 砲弾などは載せられない
}

ShellStats Workbench::Combine() {
	ShellStats result;
	if (!IsReady()) return result;

	result = CombineStats(bodySlot_->GetStats(), headSlot_->GetStats());

	// 入力2つを消費（非アクティブ化）してスロットを空ける。
	bodySlot_->SetActive(false);
	headSlot_->SetActive(false);
	bodySlot_ = nullptr;
	headSlot_ = nullptr;
	return result;
}

void Workbench::Update() {
	table_->Update();
}

void Workbench::Draw() {
	table_->Draw();
}

} // namespace game
