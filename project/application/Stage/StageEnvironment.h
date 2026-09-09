#pragma once
#include "Object3d.h"
#include "Vector3.h"
#include "Stage/Field.h"
#include <memory>
#include <string>
#include <vector>

namespace TuboEngine { class Camera; }

// =============================================================================
//  StageEnvironment ── ステージの静的な地形・構造物をまとめる箱。
//
//  所有物:
//    ・Field  ×2 : 自陣(左/-X)・敵陣(右/+X)の床＋外周壁（床自体がHP）
//    ・大砲プロップ ×2 : 各フィールドの真ん中に置く砲台モデル（見た目のみ）
//    ・デバッグ用ワールドグリッド
//
//  大砲は互いのフィールド中央に置き、弾は相手の大砲(=相手フィールド中央)へ
//  着弾して床タイルを削る。城は廃止した。
//
//  StageScene からは Update/Draw と、他システムが必要とする位置情報を渡すだけ。
// =============================================================================
namespace game {

class StageEnvironment {
public:
	void Initialize(TuboEngine::Camera* camera);
	void Update();
	void Draw();

	// 発射時に自陣大砲をカートゥーン風にポンッと拡縮させる（1発ごとに呼ぶ）。
	void PulseSelfCannon();

	// 他システムへ渡す情報。
	Field* GetSelfField() const { return selfField_.get(); }
	Field* GetEnemyField() const { return enemyField_.get(); }
	const TuboEngine::Math::Vector3& GetMuzzle() const { return muzzle_; } // 自陣大砲の砲口
	const TuboEngine::Math::Vector3& GetSelfCannonPos() const { return selfCannonPos_; }
	const TuboEngine::Math::Vector3& GetEnemyCannonPos() const { return enemyCannonPos_; }

	// デバッグ用グリッド表示のフラグ（ImGui チェックボックスから触れるよう参照を返す）。
	bool* ShowGridRef() { return &showGrid_; }

private:
	// props_ に見た目だけの Object3d を1つ積む。
	TuboEngine::Object3d* AddProp(const std::string& model,
	                              const TuboEngine::Math::Vector3& pos,
	                              const TuboEngine::Math::Vector3& rot,
	                              const TuboEngine::Math::Vector3& scale,
	                              const TuboEngine::Math::Vector4& color);

	TuboEngine::Camera* camera_ = nullptr;

	std::unique_ptr<Field> selfField_;  // 自陣(左)
	std::unique_ptr<Field> enemyField_; // 敵陣(右)

	// 大砲プロップ（各フィールド中央・見た目のみ。撃つロジックは別システム）。
	std::vector<std::unique_ptr<TuboEngine::Object3d>> props_;
	TuboEngine::Object3d* selfCannonProp_ = nullptr;  // props_ が所有（自陣中央）
	TuboEngine::Object3d* enemyCannonProp_ = nullptr; // props_ が所有（敵陣中央）
	Collapser selfCannonFall_;  // 自陣の床崩壊時に自陣大砲も落とす
	Collapser enemyCannonFall_; // 敵陣の床崩壊時に敵陣大砲も落とす

	TuboEngine::Math::Vector3 selfCannonPos_{0.0f, 0.0f, 0.0f};  // 自陣大砲(中央)
	TuboEngine::Math::Vector3 enemyCannonPos_{0.0f, 0.0f, 0.0f}; // 敵陣大砲(中央)
	TuboEngine::Math::Vector3 muzzle_{0.0f, 0.0f, 0.0f};         // 自陣大砲の砲口
	bool showGrid_ = false;

	// 自陣大砲の発射ポップ演出。cannonPulse_ が [0,1) の間だけ拡縮する（<0 で無効）。
	static constexpr float kCannonBaseScale_ = 2.6f; // 通常時のスケール（AddProp と一致）
	float cannonPulse_ = -1.0f;
};

} // namespace game
