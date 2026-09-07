#pragma once
#include "Vector3.h"
#include "Vector4.h"

// =============================================================================
//  StageLayout ── ステージの配置・色・距離などの共通設定と小さなヘルパ。
//
//  StageScene / StageEnvironment / ItemField / BulletManager / CameraController
//  が共有する定数をここに集約する（各所へ数値が散らばらないように）。
// =============================================================================
namespace game::layout {

// 各陣フィールドの中心 |X|。自陣=-, 敵陣=+。中央に谷ができる距離にする。
constexpr float kFieldOffsetX = 30.0f;

// 陣営の床色(市松2色)。
constexpr TuboEngine::Math::Vector4 kSelfFloorA = {0.30f, 0.42f, 0.66f, 1.0f};
constexpr TuboEngine::Math::Vector4 kSelfFloorB = {0.22f, 0.32f, 0.54f, 1.0f};
constexpr TuboEngine::Math::Vector4 kEnemyFloorA = {0.62f, 0.32f, 0.34f, 1.0f};
constexpr TuboEngine::Math::Vector4 kEnemyFloorB = {0.50f, 0.24f, 0.26f, 1.0f};

// 城の紋章(旗代わり)色＝陣営色。
constexpr TuboEngine::Math::Vector4 kSelfColor = {0.35f, 0.55f, 1.0f, 1.0f};
constexpr TuboEngine::Math::Vector4 kEnemyColor = {1.0f, 0.35f, 0.32f, 1.0f};

constexpr float kHalfPi = 1.57079633f;

// 操作距離・基準値。
constexpr float kPickRange = 3.0f;       // 地面アイテムを拾える距離
constexpr float kBenchRange = 4.5f;      // 工作台に載せられる距離
constexpr float kCannonRange = 5.0f;     // 砲台に弾を装填できる距離
constexpr float kCannonZoomRange = 9.0f; // この距離まで砲台に近づくと自動ズームアウト
constexpr float kItemGroundY = 0.6f;     // 落ちているアイテムの基準高さ
constexpr float kEnemyCastleHP = 200.0f; // 敵の城のHP

} // namespace game::layout

namespace game {

// 線形補間（float）。
inline float Lerpf(float a, float b, float t) { return a + (b - a) * t; }

// XZ平面上の距離の2乗（高さは無視）。
inline float Dist2XZ(const TuboEngine::Math::Vector3& a, const TuboEngine::Math::Vector3& b) {
	float dx = a.x - b.x, dz = a.z - b.z;
	return dx * dx + dz * dz;
}

} // namespace game
