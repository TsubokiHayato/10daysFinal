#pragma once
#include "Object3d.h"
#include "Vector3.h"
#include "PartCatalog.h"
#include <memory>

namespace TuboEngine { class Camera; }

// =============================================================================
//  Workbench ── 工作台。胴体と頭を1つずつ載せてドッキング（合成）する台。
//
//  ・スロットは「胴体用」「頭用」に分かれる（TryDeposit がカテゴリで振り分け）。
//  ・両方そろったら Combine() で砲弾ステータスを算出し、入力2つを消費する。
//    実際の砲弾アイテム生成は呼び出し側が GetOutputPosition() に対して行う。
// =============================================================================
namespace game {

class Item; // 前方宣言（スロットは Item* を借用して保持）

class Workbench {
public:
	void Initialize(TuboEngine::Camera* camera, const TuboEngine::Math::Vector3& pos);
	void Update();
	void Draw();

	const TuboEngine::Math::Vector3& GetPosition() const { return position_; }
	const TuboEngine::Math::Vector3& GetOutputPosition() const { return outputPos_; }

	// 手持ちアイテムを対応スロット(胴体/頭)へ載せる。成功で true。
	//  ・砲弾は載せられない。
	//  ・同じカテゴリのスロットが既に埋まっていれば false。
	bool TryDeposit(Item* item);
	bool IsReady() const { return bodySlot_ && headSlot_; } // 合成できる状態
	bool HasAny() const { return bodySlot_ || headSlot_; }

	// 胴体+頭がそろっているとき合成。入力2つを非アクティブ化しスロットを空にして、
	// 合成後の砲弾ステータスを返す。
	ShellStats Combine();

	Item* GetBodySlot() const { return bodySlot_; }
	Item* GetHeadSlot() const { return headSlot_; }

	// 台の見た目 Object3d（崩落演出などで外部から動かすとき用）。
	TuboEngine::Object3d* GetTable() const { return table_.get(); }

	void SetCamera(TuboEngine::Camera* camera);

private:
	std::unique_ptr<TuboEngine::Object3d> table_;

	TuboEngine::Math::Vector3 position_{0.0f, 0.0f, 0.0f};
	TuboEngine::Math::Vector3 bodySlotPos_{0.0f, 0.0f, 0.0f};
	TuboEngine::Math::Vector3 headSlotPos_{0.0f, 0.0f, 0.0f};
	TuboEngine::Math::Vector3 outputPos_{0.0f, 0.0f, 0.0f};

	Item* bodySlot_ = nullptr; // 借用（所有は StageScene 側）
	Item* headSlot_ = nullptr;
};

} // namespace game
