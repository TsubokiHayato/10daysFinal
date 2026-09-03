#pragma once
#include "Object3d.h"
#include "Vector3.h"
#include <memory>
#include <vector>

namespace TuboEngine { class Camera; }

// =============================================================================
//  Field ── 見下ろしステージの地形。
//
//  ・床：フラットなタイル(wall.obj)を格子状に敷き詰め、市松模様で見やすくする。
//  ・壁：外周を囲む境界壁(tile.obj を立てて使用)。プレイヤーの移動範囲の目印。
//
//  GetHalfX()/GetHalfZ() が「中心からの半分の広さ」。Player のクランプに渡す。
// =============================================================================
namespace game {

class Field {
public:
	// camera : 描画に使う主カメラ。原点中心・既定色で敷く簡易版。
	void Initialize(TuboEngine::Camera* camera);
	// フィールドを center を中心に敷く。floorA/floorB は市松模様の2色。
	// 自陣=青系・敵陣=赤系のように陣営で色を変えられる。
	void Initialize(TuboEngine::Camera* camera,
	                const TuboEngine::Math::Vector3& center,
	                const TuboEngine::Math::Vector4& floorA,
	                const TuboEngine::Math::Vector4& floorB);
	void Update();
	void Draw();

	void SetCamera(TuboEngine::Camera* camera);

	float GetHalfX() const { return halfX_; }
	float GetHalfZ() const { return halfZ_; }
	const TuboEngine::Math::Vector3& GetCenter() const { return center_; }

private:
	// 壁セグメントを1枚追加するヘルパ
	void AddWall(TuboEngine::Camera* camera,
	             const TuboEngine::Math::Vector3& pos,
	             const TuboEngine::Math::Vector3& rot,
	             const TuboEngine::Math::Vector3& scale,
	             const TuboEngine::Math::Vector4& color);

private:
	std::vector<std::unique_ptr<TuboEngine::Object3d>> floor_; // 床タイル
	std::vector<std::unique_ptr<TuboEngine::Object3d>> walls_; // 外周壁

	// フィールドの広さ設定
	int cols_ = 8;          // X方向のタイル数
	int rows_ = 12;         // Z方向のタイル数
	float tileScale_ = 3.0f; // タイル1枚のスケール（wall.obj は 2x2 → 実寸 2*scale）

	TuboEngine::Math::Vector3 center_{0.0f, 0.0f, 0.0f}; // フィールド中心
	float halfX_ = 0.0f; // 中心からの X 半径（実寸）
	float halfZ_ = 0.0f; // 中心からの Z 半径（実寸）
};

} // namespace game
