#include "Castle.h"
#include "Bullet.h"

using namespace TuboEngine;

namespace game {

namespace {
// 毒の効果時間（フレーム）と1フレームあたりのダメージ。
constexpr float kPoisonDuration = 300.0f; // 約5秒(60fps)
constexpr float kPoisonDps = 0.05f;       // 継続ダメージ
} // namespace

void Castle::Initialize(const Math::Vector3& center, float hp) {
	position_ = center;
	hp_ = hp;
	maxHp_ = hp;
	status_ = Status_None;
	poisonTimer_ = 0.0f;
}

void Castle::OnHit(const Bullet& bullet) {
	if (!IsAlive()) return;

	// ① 弾のダメージを受ける。
	hp_ -= bullet.GetStats().damage;
	if (hp_ < 0.0f) hp_ = 0.0f;

	// ② 弾の状態異常フラグを取得し、自分の状態異常フラグに立てる。
	//    （例：弾が毒フラグを持っていれば、城が毒状態になる）
	uint32_t incoming = bullet.GetStatus();
	status_ = AddStatus(status_, static_cast<StatusFlag>(incoming));

	// 毒を受けたら効果時間をリフレッシュ。
	if (HasStatus(incoming, Status_Poison)) {
		poisonTimer_ = kPoisonDuration;
	}
}

void Castle::Update() {
	if (!IsAlive()) return;

	// 毒：残り時間の間、継続ダメージ。切れたらフラグを下ろす。
	if (HasStatus(status_, Status_Poison)) {
		hp_ -= kPoisonDps;
		if (hp_ < 0.0f) hp_ = 0.0f;
		poisonTimer_ -= 1.0f;
		if (poisonTimer_ <= 0.0f) {
			status_ &= ~static_cast<uint32_t>(Status_Poison);
		}
	}
}

} // namespace game
