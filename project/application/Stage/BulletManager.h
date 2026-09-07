#pragma once
#include "Vector3.h"
#include "Stage/Cannon.h"
#include "Stage/Bullet.h"
#include <memory>
#include <vector>

namespace TuboEngine { class Camera; class Object3d; }

namespace game {

class Player;
class Castle;

// =============================================================================
//  BulletManager ── 砲台と砲弾の運用をまとめる。
//
//  ・E     : 手持ちが砲弾で砲台が近ければ、砲口に弾を装填(設置)する（未発射）。
//  ・SPACE : Cannon が発射フラグを立てる → 装填弾に Fire() を伝える。
//  ・弾の飛翔・命中(敵の城への通知)・消滅した弾の掃除もここで行う。
//
//  E はアイテム操作(拾う/破棄)とも共有するため、装填は TryLoad() に分離し、
//  StageScene が「砲台への装填を最優先」で呼べるようにしている。
// =============================================================================
class BulletManager {
public:
	// muzzle      : 弾の発射始点（砲口）
	// enemyCastle : 弾の的。命中時に OnHit を呼ぶ（借用）。
	void Initialize(TuboEngine::Camera* camera,
	                const TuboEngine::Math::Vector3& muzzle, Castle* enemyCastle);

	// E相当：手持ちが砲弾で砲台が近ければ砲口に装填する。装填できたら true。
	//  ・true のときは E を消費済みなので、呼び出し側はアイテム操作を行わない。
	bool TryLoad(Player* player);

	// 砲台の発射(SPACE)・弾の飛翔・命中・掃除を1フレーム分処理する。
	void Update();
	void Draw();

	// ImGui 表示用。
	bool HasPending() const { return pending_ != nullptr; }
	int ActiveCount() const { return static_cast<int>(bullets_.size()); }

private:
	TuboEngine::Camera* camera_ = nullptr;
	TuboEngine::Math::Vector3 muzzle_{0.0f, 0.0f, 0.0f};
	Castle* enemyCastle_ = nullptr; // 借用（所有は StageEnvironment）

	std::unique_ptr<Cannon> cannon_;                     // 撃つ/撃たないフラグ装置
	std::unique_ptr<TuboEngine::Object3d> cannonRound_;  // Cannon::Update 参照用ダミー弾

	std::vector<std::unique_ptr<Bullet>> bullets_; // 発射済み・飛翔中の弾
	Bullet* pending_ = nullptr;                    // 装填済み・発射待ち（bullets_内を借用）
};

} // namespace game
