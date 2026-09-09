#pragma once
#include "Vector3.h"
#include "Stage/Item.h"
#include "Stage/Workbench.h"
#include "Stage/PartCatalog.h"
#include "Stage/Collapser.h"
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

	bool GetTutorialFlagCarried() { return tutorialFlagCarried_; }
	bool GetTutorialFlagCreate() { return tutorialFlagCreate_; }

private:
	Item* SpawnPart(const PartDef& def, const TuboEngine::Math::Vector3& pos);
	Item* SpawnShell(const ShellStats& stats, const TuboEngine::Math::Vector3& pos);
	void ScatterParts();

	// 図鑑からランダムに1個パーツを湧かせる（最大数未満のときだけ）。
	void TrySpawnRandomItem();
	// 自陣内のランダムな地面座標を返す。砲台・作業台の周辺は避ける。
	//  規定回数試して空き場所が見つからなければ false。
	bool FindRandomGroundPos(TuboEngine::Math::Vector3& out) const;
	// pos が砲台・作業台の占有圏に入っていないか（湧かせてよい場所か）。
	bool IsSpawnAreaClear(const TuboEngine::Math::Vector3& pos) const;

	TuboEngine::Camera* camera_ = nullptr;
	Field* selfField_ = nullptr; // 借用

	std::vector<std::unique_ptr<Item>> items_;
	std::unique_ptr<Workbench> workbench_;

	bool tutorialFlagCarried_ = false;

	bool tutorialFlagCreate_ = false;
	float spawnTimer_ = 0.0f; // 自動湧きのカウンタ
	Collapser itemFall_;      // 自陣の床崩壊時に作業台・アイテムも落とす
};

} // namespace game
