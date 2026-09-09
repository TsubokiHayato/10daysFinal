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

// 陣営の床色(市松2色)。床は「HP」を兼ねるので健康時は両陣とも青系にし、
// 被弾するほど赤(kFloorDamaged)へ寄せていく（陣の識別は青の色味差で残す）。
constexpr TuboEngine::Math::Vector4 kSelfFloorA = {0.30f, 0.42f, 0.66f, 1.0f};
constexpr TuboEngine::Math::Vector4 kSelfFloorB = {0.22f, 0.32f, 0.54f, 1.0f};
constexpr TuboEngine::Math::Vector4 kEnemyFloorA = {0.24f, 0.46f, 0.60f, 1.0f};
constexpr TuboEngine::Math::Vector4 kEnemyFloorB = {0.18f, 0.36f, 0.50f, 1.0f};

// 被弾で床が寄っていく色（HPゼロに近いほどこの赤になる）。
constexpr TuboEngine::Math::Vector4 kFloorDamaged = {0.88f, 0.16f, 0.14f, 1.0f};

// 陣営の識別色（大砲プロップの色味などに使う）。
constexpr TuboEngine::Math::Vector4 kSelfColor = {0.35f, 0.55f, 1.0f, 1.0f};
constexpr TuboEngine::Math::Vector4 kEnemyColor = {1.0f, 0.35f, 0.32f, 1.0f};

constexpr float kHalfPi = 1.57079633f;

// 操作距離・基準値。
constexpr float kPickRange = 3.0f;       // 地面アイテムを拾える距離
constexpr float kBenchRange = 4.5f;      // 工作台に載せられる距離
constexpr float kCannonRange = 5.0f;     // 砲台に弾を装填できる距離
constexpr float kCannonZoomRange = 9.0f; // この距離まで砲台に近づくと自動ズームアウト
constexpr float kItemGroundY = 0.6f;     // 落ちているアイテムの基準高さ

// プレイヤーのアイテム自動湧き。
constexpr int   kMaxGroundItems = 20;       // フィールド上に同時に存在できるアイテムの最大数
constexpr float kItemSpawnInterval = 75.0f; // 何フレームごとに1個湧かせるか
constexpr float kCannonKeepOut = 5.0f;      // 砲台まわり(この半径内)には湧かせない
constexpr float kBenchKeepOut = 4.0f;       // 作業台まわり(この半径内)には湧かせない

// 床(フィールド)をHP化するための設定。
constexpr float kFieldHP = 500.0f;            // フィールド(床)全体の総HP
constexpr float kTileMaxHP = 20.0f;           // 床タイル1枚の“見た目の赤み”基準(表示用)
constexpr float kFloorHitBaseRadius = 4.5f;   // 着弾ダメージが及ぶ基本半径
constexpr float kFloorHitBlastRadius = 1.4f;  // 弾のblast1あたりの追加半径
constexpr float kPlayerFloorDmgMul = 4.0f;    // プレイヤー弾→床ダメージ倍率

// 弾同士の空中相殺：この距離まで近づいたら、強さに関係なく双方を打ち消す。
// 球の見た目に対して当たりやすいよう、やや広めにとる。
constexpr float kBulletCancelRadius = 5.0f;

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
