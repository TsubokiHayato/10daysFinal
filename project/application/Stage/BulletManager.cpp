#include "BulletManager.h"
#include "StageLayout.h"
#include "Stage/Player.h"
#include "Stage/Item.h"
#include "Stage/Field.h"
#include "Object3d.h"
#include "Camera.h"
#include "Input.h"

using namespace TuboEngine;

namespace game {

using namespace game::layout;

void BulletManager::Initialize(TuboEngine::Camera* camera,
                               const Math::Vector3& muzzle, Field* enemyField,
                               const Math::Vector3& target) {
	camera_ = camera;
	muzzle_ = muzzle;
	enemyField_ = enemyField;
	target_ = target;

	// 砲台は「発射フラグを持つ装置」としてロジックのみ使う（見た目はプロップ側）。
	cannon_ = std::make_unique<Cannon>();
	cannon_->Initialize(camera_);
	// Cannon::Update が参照するダミー弾（SPACEで null 参照しないため。描画しない）。
	cannonRound_ = std::make_unique<Object3d>();
	cannonRound_->Initialize("playerBullet/playerBullet.obj");
	cannonRound_->SetCamera(camera_);
	cannon_->SetBullet(cannonRound_.get());
}

bool BulletManager::TryLoad(Player* player) {
	// E押下・装填中でない・砲弾を持っている、が前提。
	if (!Input::GetInstance()->TriggerKey(DIK_E) || pending_) return false;
	Item* carried = player->GetCarried();
	if (!carried || carried->GetCategory() != Category::Shell) return false;
	// 砲台が近くにあること。
	if (Dist2XZ(player->GetPosition(), muzzle_) >= kCannonRange * kCannonRange) return false;

	// 弾は「全ステータス＋的」を持って生まれる。的は相手大砲(中央)そのもの。
	//  着地点を固定することで自弾・敵弾が同じ線上を通り、空中で相殺できる。
	const Math::Vector3 target = target_;
	auto bullet = std::make_unique<Bullet>();
	bullet->Initialize(camera_, carried->GetStats(), muzzle_, target);
	pending_ = bullet.get();      // 発射待ち（Fireされるまで砲口で静止）
	bullets_.push_back(std::move(bullet));
	// 元の砲弾アイテムは消費する。
	carried->SetActive(false);
	player->SetCarried(nullptr);
	// 砲台を再装填状態に戻す（前弾のフラグが残っていても撃てるように）。
	cannon_->SetIsBulletFired(false);

	tutorialFlagLoad_ = true;

	return true;
}

void BulletManager::Update() {
	// ── 砲台の更新：SPACEで撃つ/撃たないフラグを立てる ──
	cannon_->Update();

	// ── 発射：砲台のフラグが立ったら、装填済みの弾に Fire() を伝える ──
	if (cannon_->GetIsBulletFired()) {
		if (pending_) {
			pending_->Fire(); // 以後は弾が自分で的へ飛ぶ
			pending_ = nullptr;
		}
		cannon_->SetIsBulletFired(false); // 次弾に備えてフラグを戻す

		tutorialFlagShot_ = true;
	}

	// ── 飛翔と命中：弾を更新し、的に到達したら相手の床タイルを削る ──
	for (auto& b : bullets_) {
		b->Update();
		if (b->HasHitTarget()) {
			const ShellStats& s = b->GetStats();
			float dmg = s.damage * kPlayerFloorDmgMul;
			float radius = kFloorHitBaseRadius + s.blast * kFloorHitBlastRadius;
			enemyField_->ApplyDamage(b->GetTarget(), dmg, radius);
		}
	}

	// 消滅した弾を掃除する。
	bullets_.erase(std::remove_if(bullets_.begin(), bullets_.end(),
	                              [](const std::unique_ptr<Bullet>& b) { return !b->IsActive(); }),
	               bullets_.end());
}

void BulletManager::Draw() {
	for (auto& b : bullets_) b->Draw();
}

std::vector<Bullet*> BulletManager::GetFlyingBullets() {
	std::vector<Bullet*> out;
	for (auto& b : bullets_) {
		if (b->IsActive() && b->IsFired()) out.push_back(b.get());
	}
	return out;
}

} // namespace game
