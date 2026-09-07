#pragma once
#include "Object3d.h"
#include "Vector3.h"
#include "PartCatalog.h"   // ShellStats
#include "StatusEffect.h"
#include <memory>

namespace TuboEngine { class Camera; }

// =============================================================================
//  Bullet ── 砲弾。「すべてのステータス」と「自分の動き」を持つ主役。
//
//  設計の要:
//    ・威力/弾速/爆発/状態異常フラグ(ShellStats) を弾自身が保持する。
//    ・的(target)も生成時に受け取り、発射後は弾が自分で的へ向かって飛ぶ。
//      → 砲台(Cannon)は Fire() で「発射フラグを立てるだけ」。動きは弾の責務。
//    ・城に当たったら、城が GetStatus() を読んで状態異常フラグを取得する。
//
//  弾の増やし方:
//    Bullet 自体は種類を持たない。ステータス(ShellStats)で挙動が変わるので、
//    PartCatalog にパーツを足して合成ステータスを変えるだけで新種の弾が作れる。
// =============================================================================
namespace game {

class Bullet {
public:
	// stats  : 弾の全ステータス（状態異常フラグ含む）
	// start  : 発射位置（砲台の砲口）
	// target : 的（相手の城など）の位置
	void Initialize(TuboEngine::Camera* camera, const ShellStats& stats,
	                const TuboEngine::Math::Vector3& start,
	                const TuboEngine::Math::Vector3& target);

	// 砲台から呼ばれる：発射フラグを立てて、的へ向かう挙動を開始する。
	void Fire() { fired_ = true; }

	void Update();
	void Draw();

	// ── 状態問い合わせ ─────────────────────────────
	bool IsFired() const { return fired_; }        // 発射済みか
	bool IsActive() const { return active_; }       // まだ生存しているか
	bool HasHitTarget() const { return hitTarget_; } // この更新で的に到達したか

	// ── ステータス（城が読み取る）─────────────────
	const ShellStats& GetStats() const { return stats_; }
	uint32_t GetStatus() const { return stats_.status; } // 状態異常フラグ

	const TuboEngine::Math::Vector3& GetPosition() const { return position_; }
	const TuboEngine::Math::Vector3& GetTarget() const { return target_; }

	void Deactivate() { active_ = false; }
	void SetCamera(TuboEngine::Camera* camera);

private:
	std::unique_ptr<TuboEngine::Object3d> model_;
	ShellStats stats_;

	TuboEngine::Math::Vector3 position_{0.0f, 0.0f, 0.0f};
	TuboEngine::Math::Vector3 start_{0.0f, 0.0f, 0.0f};  // 発射始点（弧の起点）
	TuboEngine::Math::Vector3 target_{0.0f, 0.0f, 0.0f}; // 的（弧の着弾点）

	// 放物線（弧）で飛ぶためのパラメータ。
	float progress_ = 0.0f;  // 経過フレーム
	float duration_ = 60.0f; // 着弾までのフレーム数（弾速と距離から決定）
	float arcHeight_ = 0.0f; // 弧の頂点の高さ（距離から決定）

	bool fired_ = false;     // 砲台に発射されたか（false の間は動かない）
	bool active_ = true;     // 生存フラグ（的到達 or 消滅で false）
	bool hitTarget_ = false; // 的に到達した瞬間 true（城側が拾う）
	float spin_ = 0.0f;      // 見た目の回転
};

} // namespace game
