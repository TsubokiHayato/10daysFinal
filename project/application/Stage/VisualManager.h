#pragma once

#include "Vector3.h"

namespace TuboEngine { class Camera; }

class VisualManager {

public:

	static VisualManager* GetInstance();

	void Initialize(TuboEngine::Camera* _camera);

	/// <summary>
	/// 毎フレーム呼ぶ。カメラのUpdate()より前に呼ぶこと
	/// </summary>
	void Update();

	/// <summary>
	/// シェイク開始
	/// </summary>
	/// <param name="duration">揺れる時間(秒)</param>
	/// <param name="intensity">揺れの強さ</param>
	void Shake(float duration = 0.5f, float intensity = 0.3f);

private:

	static float RandRange();

	TuboEngine::Camera* camera_ = nullptr;

	// シェイク用パラメータ
	TuboEngine::Math::Vector3 basePosition_{ 0.0f,0.0f,0.0f }; // シェイク開始時点の本来の位置
	float shakeDuration_ = 0.0f;               // 揺れる総時間
	float shakeTimer_ = 0.0f;                  // 経過時間
	float shakeIntensity_ = 0.0f;              // 初期振幅
	bool isShaking_ = false;

};