#pragma once
#include "Object3d.h"
#include "Vector3.h"
#include "Stage/Field.h"
#include "Stage/Castle.h"
#include <memory>
#include <string>
#include <vector>

namespace TuboEngine { class Camera; }

// =============================================================================
//  StageEnvironment ── ステージの静的な地形・構造物をまとめる箱。
//
//  所有物:
//    ・Field  ×2 : 自陣(左/-X)・敵陣(右/+X)の床＋外周壁
//    ・Castle ×2 : 各陣の城（見た目＋HP/状態異常）。敵の城は弾の的になる。
//    ・砲台プロップ ×2 : 見た目だけの砲台モデル（撃つロジックは BulletManager）
//    ・デバッグ用ワールドグリッド
//
//  StageScene からは Update/Draw と、他システムが必要とする位置情報を渡すだけ。
// =============================================================================
namespace game {

class StageEnvironment {
public:
	void Initialize(TuboEngine::Camera* camera);
	void Update();
	void Draw();

	// 他システムへ渡す情報。
	Field* GetSelfField() const { return selfField_.get(); }
	Field* GetEnemyField() const { return enemyField_.get(); }
	Castle* GetEnemyCastle() const { return enemyCastle_.get(); }
	const TuboEngine::Math::Vector3& GetMuzzle() const { return muzzle_; }

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

	std::unique_ptr<Castle> selfCastle_;  // 自陣の城（現状は見た目のみ）
	std::unique_ptr<Castle> enemyCastle_; // 敵陣の城（弾の的・被弾する）

	// 砲台などの飾りプロップ。
	std::vector<std::unique_ptr<TuboEngine::Object3d>> props_;
	TuboEngine::Object3d* enemyCannon_ = nullptr; // props_ が所有（見た目のみ）

	TuboEngine::Math::Vector3 muzzle_{0.0f, 0.0f, 0.0f}; // 自陣砲台の砲口（弾の発射始点）
	bool showGrid_ = false;
};

} // namespace game
