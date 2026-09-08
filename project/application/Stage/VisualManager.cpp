#include "VisualManager.h"
#include "Camera.h"
#include <cstdlib>

using namespace TuboEngine;

VisualManager* VisualManager::GetInstance() {

	static VisualManager* instance = new VisualManager();

	return instance;
}

void VisualManager::Initialize(TuboEngine::Camera* _camera) {

	camera_ = _camera;
}

void VisualManager::Shake(float duration, float intensity) {

	if (!camera_) {
		return;
	}

	// シェイク開始。本来の位置を保存しておく
	basePosition_ = camera_->GetTranslate();
	shakeDuration_ = duration;
	shakeIntensity_ = intensity;
	shakeTimer_ = 0.0f;
	isShaking_ = true;
}

void VisualManager::Update() {
	basePosition_ = camera_->GetTranslate();
	if (!isShaking_ || !camera_) {
		return;
	}

	shakeTimer_ += 1.0f / 60.0f;

	if (shakeTimer_ >= shakeDuration_) {
		// 終了 -> 元の位置に戻す
		camera_->SetTranslate(basePosition_);
		isShaking_ = false;
		return;
	}

	// 残り時間の割合で振幅を減衰
	float t = 1.0f - (shakeTimer_ / shakeDuration_);
	float amp = shakeIntensity_ * t;

	TuboEngine::Math::Vector3 offset{
		RandRange() * amp,
		RandRange() * amp,
		0.0f // 奥行きは揺らさない
	};

	TuboEngine::Math::Vector3 shaken{
		basePosition_.x + offset.x,
		basePosition_.y + offset.y,
		basePosition_.z + offset.z
	};

	camera_->SetTranslate(shaken);

	camera_->Update();
}

float VisualManager::RandRange() {
	return (static_cast<float>(std::rand()) / RAND_MAX) * 2.0f - 1.0f;
}