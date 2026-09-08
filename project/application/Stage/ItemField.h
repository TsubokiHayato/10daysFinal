#pragma once
#include "Vector3.h"
#include "Stage/Item.h"
#include "Stage/Workbench.h"
#include "Stage/PartCatalog.h"
#include <memory>
#include <vector>

namespace TuboEngine { class Camera; }

namespace game {

class Player;
class Field;

// =============================================================================
//  ItemField ── 地面のパーツ／砲弾アイテムと工作台をまとめる。
//
//  ・自陣にパーツを散布し、拾う/破棄/工作台への載せ降ろしを処理する。
//  ・工作台に胴体+頭がそろったら合成し、砲弾アイテムを出力位置に生成する。
//
//  StageScene は HandleInteraction(player) / Update() / Draw() を呼ぶだけ。
// =============================================================================
class ItemField {
public:
	// selfField : 散布範囲と工作台位置の基準に使う自陣フィールド（借用）。
	void Initialize(TuboEngine::Camera* camera, Field* selfField);
	void Update();
	void Draw();

	// E キー相当：手ぶら→拾う / 手持ち→工作台へ載せる or 破棄。
	void HandleInteraction(Player* player);

	// ImGui 表示用。
	Workbench* GetWorkbench() const { return workbench_.get(); }
	int CountGround() const;
	int CountShells() const;

private:
	Item* SpawnPart(const PartDef& def, const TuboEngine::Math::Vector3& pos);
	Item* SpawnShell(const ShellStats& stats, const TuboEngine::Math::Vector3& pos);
	void ScatterParts();

	TuboEngine::Camera* camera_ = nullptr;
	Field* selfField_ = nullptr; // 借用

	std::vector<std::unique_ptr<Item>> items_;
	std::unique_ptr<Workbench> workbench_;
};

} // namespace game
