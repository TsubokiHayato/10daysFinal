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

	// ── 床＝HP ───────────────────────────────────────────────
	//  worldPos を中心に radius 内の床タイルへ damage を与える（距離で減衰）。
	//  被弾したタイルは色が青→赤へ寄り、フィールド総HPが0になると崩壊し始める。
	void ApplyDamage(const TuboEngine::Math::Vector3& worldPos, float damage, float radius);

	float GetHP() const { return hp_; }
	float GetMaxHP() const { return maxHp_; }
	float GetHPRatio() const { return maxHp_ > 0.0f ? hp_ / maxHp_ : 0.0f; }
	bool IsCollapsing() const { return collapsing_; } // 崩壊アニメ中
	bool IsCollapsed() const { return collapsed_; }   // 崩壊完了

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

	// 床タイルと並列に持つHP・見た目・崩壊アニメ用データ（index が floor_ と対応）。
	std::vector<TuboEngine::Math::Vector4> tileBase_; // 健康時の元色（市松）
	std::vector<float> tileHp_;                        // タイルごとの残HP
	std::vector<TuboEngine::Math::Vector3> tileHome_;  // 元位置（被弾判定・崩壊起点）
	std::vector<TuboEngine::Math::Vector3> tileVel_;   // 崩壊落下速度
	std::vector<TuboEngine::Math::Vector3> tileSpin_;  // 崩壊回転速度

	float tileMaxHp_ = 0.0f; // タイル1枚のHP上限
	float hp_ = 0.0f;        // フィールド総HP（= 全タイルHPの合計）
	float maxHp_ = 0.0f;     // フィールド総HP上限
	bool collapsing_ = false; // 崩壊アニメ中
	bool collapsed_ = false;  // 崩壊完了
	float collapseTimer_ = 0.0f;

	// フィールドの広さ設定
	int cols_ = 8;          // X方向のタイル数
	int rows_ = 12;         // Z方向のタイル数
	float tileScale_ = 3.0f; // タイル1枚のスケール（wall.obj は 2x2 → 実寸 2*scale）

	TuboEngine::Math::Vector3 center_{0.0f, 0.0f, 0.0f}; // フィールド中心
	float halfX_ = 0.0f; // 中心からの X 半径（実寸）
	float halfZ_ = 0.0f; // 中心からの Z 半径（実寸）
};

} // namespace game
