#include "Bullet.h"
#include "Camera.h"
#include <cmath>

using namespace TuboEngine;

namespace game {

namespace {
constexpr float kMinDuration = 45.0f;  // 着弾までの最短フレーム
constexpr float kMaxDuration = 240.0f; // 着弾までの最長フレーム
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

	// 水平距離から「飛翔時間」と「弧の高さ」を決める。
	//  ・弾速(speed)が速いほど早く着弾（duration短い）。
	//  ・距離が遠いほど山なりに（arcHeight高い）。
	float dx = target.x - start.x;
	float dz = target.z - start.z;
	float dist = std::sqrt(dx * dx + dz * dz);
	float speed = stats.speed > 0.1f ? stats.speed : 0.1f;
	duration_ = Clampf(dist * 2.0f / speed, kMinDuration, kMaxDuration);
	arcHeight_ = Clampf(dist * 0.5f, 6.0f, 40.0f);

	// 見た目：威力が高いほど大きく、毒なら緑、爆発が大きいほど赤く。
	float sizet = 0.7f + stats.damage * 0.02f;
	if (sizet > 2.0f) sizet = 2.0f;
	Math::Vector4 color;
	if (HasStatus(stats.status, Status_Poison)) {
		color = {0.45f, 0.90f, 0.35f, 1.0f}; // 毒弾は緑
	} else {
		float redt = stats.blast / 5.0f;
		if (redt > 1.0f) redt = 1.0f;
		color = {0.95f, 0.85f - redt * 0.45f, 0.30f - redt * 0.25f, 1.0f};
	}

	model_ = std::make_unique<Object3d>();
	model_->Initialize("playerBullet/playerBullet.obj");
	model_->SetCamera(camera);
	model_->SetPosition(position_);
	model_->SetScale({sizet, sizet, sizet});
	model_->SetModelColor(color);
}

void Bullet::SetCamera(TuboEngine::Camera* camera) {
	if (model_) model_->SetCamera(camera);
}

void Bullet::Update() {
	if (!active_) return;
	hitTarget_ = false;

	// 発射されるまでは砲口で待機（動かない）。
	if (fired_) {
		progress_ += 1.0f;
		float t = progress_ / duration_;

		if (t >= 1.0f) {
			// 着弾：位置を的に合わせ、到達フラグを立てて消滅させる。
			position_ = target_;
			hitTarget_ = true;
			active_ = false;
		} else {
			// 始点→的を水平に補間し、放物線(4t(1-t))で高さを足して弧を描く。
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
