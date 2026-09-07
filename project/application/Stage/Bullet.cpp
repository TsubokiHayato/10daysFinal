#include "Bullet.h"
#include "Camera.h"
#include "VisualManager.h"
#include "ParticleManager.h" // 板野サーカス弾の演出
#include <algorithm>
#include <cmath>

using namespace TuboEngine;

namespace game {

namespace {
constexpr float kMinDuration = 45.0f;  // 着弾までの最短フレーム
constexpr float kMaxDuration = 240.0f; // 着弾までの最長フレーム

constexpr float kDt = 1.0f / 60.0f;    // 固定フレーム時間（サーカス弾の速度積分に使用）
constexpr float kCircusSpeed = 66.0f;  // ホーミング時の基準速度[units/sec]（stats.speed 倍）
constexpr float kHitRadius = 2.0f;     // 的への到達とみなす距離
constexpr float kTwoPi = 6.283185f;
} // namespace

// clamp（<algorithm>を増やさない軽量版）。
static float Clampf(float v, float lo, float hi) {
	return v < lo ? lo : (v > hi ? hi : v);
}

void Bullet::Initialize(TuboEngine::Camera* camera, const ShellStats& stats,
                        const Math::Vector3& start, const Math::Vector3& target) {
	stats_ = stats;
	position_ = start;
	start_ = start;
	target_ = target;
	fired_ = false;
	active_ = true;
	hitTarget_ = false;
	progress_ = 0.0f;

	// swerve か多弾ならサーカス弾（速度ベースのホーミング＋演出）として扱う。
	circus_ = (stats.swerve > 0.0f) || (stats.count > 1);
	swerveAmp_ = stats.swerve;
	launched_ = false;
	dispersed_ = false;
	elapsedTime_ = 0.0f;
	lastTrailPos_ = start;

	// 水平距離から「飛翔時間」と「弧の高さ」を決める（通常弾の放物線用）。
	//  ・弾速(speed)が速いほど早く着弾（duration短い）。
	//  ・距離が遠いほど山なりに（arcHeight高い）。
	float dx = target.x - start.x;
	float dz = target.z - start.z;
	float dist = std::sqrt(dx * dx + dz * dz);
	float speed = stats.speed > 0.1f ? stats.speed : 0.1f;
	duration_ = Clampf(dist * 2.0f / speed, kMinDuration, kMaxDuration);
	arcHeight_ = Clampf(dist * 0.5f, 6.0f, 40.0f);

	// 見た目：サーカス弾はシアンの小型弾。通常弾は威力/爆発で色とサイズが変わる。
	float sizet;
	Math::Vector4 color;
	if (circus_) {
		sizet = 0.5f;
		color = {0.0f, 0.6f, 1.0f, 1.0f};
	} else {
		sizet = 0.7f + stats.damage * 0.02f;
		if (sizet > 2.0f) sizet = 2.0f;
		if (HasStatus(stats.status, Status_Poison)) {
			color = {0.45f, 0.90f, 0.35f, 1.0f}; // 毒弾は緑
		} else {
			float redt = stats.blast / 5.0f;
			if (redt > 1.0f) redt = 1.0f;
			color = {0.95f, 0.85f - redt * 0.45f, 0.30f - redt * 0.25f, 1.0f};
		}
	}

	model_ = std::make_unique<Object3d>();
	model_->Initialize("playerBullet/playerBullet.obj");
	model_->SetCamera(camera);
	model_->SetPosition(position_);
	model_->SetScale({sizet, sizet, sizet});
	model_->SetModelColor(color);

	if (circus_) SetupCircusEmitters();
}

void Bullet::SetCamera(TuboEngine::Camera* camera) {
	if (model_) model_->SetCamera(camera);
}

// 全サーカス弾で使い回す共有エミッタを用意する（無ければ生成）。
//  ・元 PlayerCircusBullet と同じく Find で共有し、Emit のたびに center を動かす。
void Bullet::SetupCircusEmitters() {
	auto* pm = ParticleManager::GetInstance();

	trailEmitter_ = pm->Find("CircusTrail");
	if (!trailEmitter_) {
		ParticlePreset p{};
		p.name = "CircusTrail";
		p.texture = "particle.png";
		p.maxInstances = 12000; // 補間エミットで増えるので多めに
		p.autoEmit = false;
		p.lifeMin = 0.5f;
		p.lifeMax = 0.9f;
		p.scaleStart = {0.55f, 0.55f, 0.55f};
		p.scaleEnd = {0.0f, 0.0f, 0.0f};
		p.colorStart = {0.9f, 0.9f, 1.0f, 0.45f};
		p.colorEnd = {0.4f, 0.4f, 1.0f, 0.0f};
		trailEmitter_ = pm->CreateEmitterByType("Default", p);
	}

	burnerEmitter_ = pm->Find("CircusBurner");
	if (!burnerEmitter_) {
		ParticlePreset p{};
		p.name = "CircusBurner";
		p.texture = "circle.png";
		p.maxInstances = 2000;
		p.autoEmit = false;
		p.lifeMin = 0.05f;
		p.lifeMax = 0.12f;
		p.scaleStart = {0.4f, 0.4f, 0.4f};
		p.scaleEnd = {0.0f, 0.0f, 0.0f};
		p.colorStart = {0.2f, 0.8f, 1.0f, 1.0f};
		p.colorEnd = {0.0f, 0.2f, 1.0f, 0.0f};
		p.velMin = {-0.1f, -0.1f, -0.1f};
		p.velMax = {0.1f, 0.1f, 0.1f};
		burnerEmitter_ = pm->CreateEmitterByType("Default", p);
	}

	explosionEmitter_ = pm->Find("CircusExplosion");
	if (!explosionEmitter_) {
		ParticlePreset p{};
		p.name = "CircusExplosion";
		p.texture = "circle.png";
		p.maxInstances = 1000;
		p.autoEmit = false;
		p.lifeMin = 0.2f;
		p.lifeMax = 0.4f;
		p.scaleStart = {0.8f, 0.8f, 0.8f};
		p.scaleEnd = {0.0f, 0.0f, 0.0f};
		p.colorStart = {0.0f, 0.4f, 1.0f, 1.0f};
		p.colorEnd = {0.0f, 0.0f, 1.0f, 0.0f};
		p.velMin = {-1.8f, -1.8f, -1.8f};
		p.velMax = {1.8f, 1.8f, 1.8f};
		explosionEmitter_ = pm->CreateEmitterByType("Primitive", p);
	}
}

// 板野サーカス：①まとまって前進 → ②拡散 → ③うねって的へ収束。
void Bullet::UpdateCircus() {
	// ── 初速：発射された最初のフレームで一度だけ与える ──
	//  ・①は砲台の弾のように放物線(山なり)で打ち上げる。全弾が同じ弧を通るので塊に見える。
	//  ・弧の終点(dispersePoint_)は的の手前(距離の75%)。ここで拡散する。
	if (!launched_) {
		Math::Vector3 fwd = target_ - start_;
		float dist = fwd.Length();
		forwardDir_ = (dist > 0.0001f) ? (fwd / dist) : Math::Vector3{0.0f, 0.0f, 1.0f};
		dispersePoint_ = Math::Vector3::Lerp(start_, target_, 0.75f);
		groupArcHeight_ = Clampf(dist * 0.5f, 8.0f, 40.0f); // 距離が遠いほど高く打ち上げる
		launched_ = true;
		dispersed_ = false;
		elapsedTime_ = 0.0f;
		lastTrailPos_ = position_;
	}

	elapsedTime_ += kDt;
	Math::Vector3 pos = position_;
	Math::Vector3 vel = velocity_;

	if (elapsedTime_ < groupedDuration_) {
		// ① まとまって前進：始点→拡散点へ砲台のような弧を描いて上昇し、頂点で②へ移る。
		//  ・高さは sin(tg·π/2) で単調上昇し、tg=1 で最頂点(鉛直速度0)になる。
		//    → 拡散は「弧が一番高くなった瞬間」に起きる。
		float tg = elapsedTime_ / groupedDuration_;
		Math::Vector3 base = Math::Vector3::Lerp(start_, dispersePoint_, tg);
		base.y += groupArcHeight_ * std::sin(tg * 1.5707963f);
		vel = (base - pos) * (1.0f / kDt); // 見た目の噴射方向・次段の初速用に速度を推定
		pos = base;
		position_ = pos;
		velocity_ = vel;
		// この段では以降の的到達判定・ホーミングを行わず、演出だけ出して抜ける。
		if (trailEmitter_) {
			Math::Vector3 segment = pos - lastTrailPos_;
			float segLen = segment.Length();
			const float kSpacing = 0.45f;
			int steps = std::max(1, static_cast<int>(segLen / kSpacing));
			steps = std::min(steps, 32);
			for (int i = 1; i <= steps; ++i) {
				float t = static_cast<float>(i) / static_cast<float>(steps);
				trailEmitter_->GetPreset().center = lastTrailPos_ + segment * t;
				trailEmitter_->Emit(1);
			}
			lastTrailPos_ = pos;
		}
		if (burnerEmitter_ && vel.LengthSquared() > 0.001f) {
			Math::Vector3 back = Math::Vector3::Normalize(vel) * -0.5f;
			burnerEmitter_->GetPreset().center = pos + back;
			burnerEmitter_->Emit(1);
		}
		model_->SetPosition(pos);
		if (vel.LengthSquared() > 0.0001f) {
			Math::Vector3 d = Math::Vector3::Normalize(vel);
			float yaw = std::atan2(d.x, d.z);
			float pitch = -std::asin(Clampf(d.y, -1.0f, 1.0f));
			model_->SetRotation({pitch, yaw, 0.0f});
		}
		return;
	} else {
		// ②拡散：まとまり終了の瞬間に、弾ごとの位相で外向きへ一気に散開させる。
		if (!dispersed_) {
			Math::Vector3 ref = (std::fabs(forwardDir_.y) > 0.99f) ? Math::Vector3{0.0f, 0.0f, 1.0f}
			                                                      : Math::Vector3{0.0f, 1.0f, 0.0f};
			Math::Vector3 r = Math::Vector3::Normalize(Math::Vector3::Cross(forwardDir_, ref));
			Math::Vector3 u = Math::Vector3::Cross(r, forwardDir_);
			Math::Vector3 lateral = r * std::cos(swervePhase_) + u * std::sin(swervePhase_);
			// 前へ少し＋外へ大きく。この向きが③ホーミングの初期進行方向になり、
			// 旋回力が0から徐々に強まるので「一度ばらけてから的へ吸い込まれる」動きになる。
			Math::Vector3 dir = forwardDir_ * 0.35f + lateral * 1.0f;
			velocity_ = Math::Vector3::Normalize(dir) * (kCircusSpeed * (stats_.speed > 0.1f ? stats_.speed : 0.1f));
			vel = velocity_;
			dispersed_ = true;
		}

		// ③的へ吸い込むホーミング＋進行軸まわりのコークスクリューうねり。
		Math::Vector3 toTarget = target_ - pos;
		float distToTarget = toTarget.Length();
		Math::Vector3 targetDir = (distToTarget > 0.0001f) ? Math::Vector3::Normalize(toTarget)
		                                                   : Math::Vector3{0.0f, 0.0f, 1.0f};
		Math::Vector3 curDir = (vel.LengthSquared() > 0.0001f) ? Math::Vector3::Normalize(vel)
		                                                       : targetDir;

		// 拡散直後は旋回力ほぼ0（外へ広がる）→ 時間とともに強まり的へ収束する。
		float turn = turnSpeed_ * std::min(2.5f, (elapsedTime_ - groupedDuration_) * 3.0f);
		float swerveScale = 1.0f;
		if (distToTarget < 8.0f) {
			// 的に近いほど旋回を強め・うねりを弱めて確実に当てる。
			float t = distToTarget / 8.0f;
			turn *= (2.0f - t);
			swerveScale = t;
		}

		// 芯となる進行方向（直進＋的への補正）。
		Math::Vector3 core = curDir + targetDir * turn;
		core = Math::Vector3::Normalize(core);

		// 進行軸まわりの直交基底を作り、位相を回してうねらせる。
		Math::Vector3 ref = (std::fabs(core.z) > 0.99f) ? Math::Vector3{0.0f, 1.0f, 0.0f}
		                                               : Math::Vector3{0.0f, 0.0f, 1.0f};
		Math::Vector3 right = Math::Vector3::Normalize(Math::Vector3::Cross(core, ref));
		Math::Vector3 up = Math::Vector3::Cross(right, core);
		float phase = elapsedTime_ * swerveFreq_ + swervePhase_;
		Math::Vector3 sw = right * std::cos(phase) + up * std::sin(phase);

		Math::Vector3 finalDir = core + sw * (swerveAmp_ * swerveScale * 0.1f);
		finalDir = Math::Vector3::Normalize(finalDir);

		float base = stats_.speed > 0.1f ? stats_.speed : 0.1f;
		vel = finalDir * (kCircusSpeed * base);
	}

	pos += vel * kDt;
	position_ = pos;
	velocity_ = vel;

	// ── 航跡トレイル：前回位置から現在位置まで細かく刻んで途切れない尾を引く ──
	if (trailEmitter_) {
		Math::Vector3 segment = pos - lastTrailPos_;
		float segLen = segment.Length();
		const float kSpacing = 0.45f;
		int steps = std::max(1, static_cast<int>(segLen / kSpacing));
		steps = std::min(steps, 32);
		for (int i = 1; i <= steps; ++i) {
			float t = static_cast<float>(i) / static_cast<float>(steps);
			trailEmitter_->GetPreset().center = lastTrailPos_ + segment * t;
			trailEmitter_->Emit(1);
		}
		lastTrailPos_ = pos;
	}
	// ── バーナー：進行方向の後ろに噴射炎 ──
	if (burnerEmitter_ && vel.LengthSquared() > 0.001f) {
		Math::Vector3 back = Math::Vector3::Normalize(vel) * -0.5f;
		burnerEmitter_->GetPreset().center = pos + back;
		burnerEmitter_->Emit(1);
	}

	// ── 着弾：的に十分近づいたら命中扱いで爆発 ──
	if (Math::Vector3::DistanceSquared(pos, target_) < kHitRadius * kHitRadius) {
		position_ = target_;
		hitTarget_ = true;
		active_ = false;
		if (explosionEmitter_) {
			explosionEmitter_->GetPreset().center = target_;
			explosionEmitter_->Emit(15);
		}
		VisualManager::GetInstance()->Shake(0.5f, 2.0f);
	}
	// 保険：長生きしすぎたら消す。
	if (elapsedTime_ > 6.0f) active_ = false;

	// ── 見た目：進行方向へ機首を向ける（見下ろしXZ平面なので yaw=atan2(x,z)） ──
	model_->SetPosition(pos);
	if (vel.LengthSquared() > 0.0001f) {
		Math::Vector3 d = Math::Vector3::Normalize(vel);
		float yaw = std::atan2(d.x, d.z);
		float pitch = -std::asin(Clampf(d.y, -1.0f, 1.0f));
		model_->SetRotation({pitch, yaw, 0.0f});
	}
}

void Bullet::Update() {
	if (!active_) return;
	hitTarget_ = false;

	if (fired_) {
		if (circus_) {
			// 板野サーカス：速度ベースのホーミング＋演出。位置と姿勢はこの中で更新。
			UpdateCircus();
			model_->Update();
			return;
		}

		// 通常弾：始点→的を放物線で飛ぶ。
		progress_ += 1.0f;
		float t = progress_ / duration_;
		if (t >= 1.0f) {
			position_ = target_;
			hitTarget_ = true;
			active_ = false;
			VisualManager::GetInstance()->Shake(0.5f, 2.0f);
		} else {
			Math::Vector3 base = Math::Vector3::Lerp(start_, target_, t);
			base.y += arcHeight_ * 4.0f * t * (1.0f - t);
			position_ = base;
		}
	}

	spin_ += 0.25f;
	model_->SetPosition(position_);
	model_->SetRotation({0.0f, spin_, 0.0f});
	model_->Update();
}

void Bullet::Draw() {
	if (!active_) return;
	model_->Draw();
}

} // namespace game
