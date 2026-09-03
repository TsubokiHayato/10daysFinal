#pragma once
#include "IScene.h"
#include "Camera.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "Object3d.h"     // 3D モデル描画
#include "DebugCamera.h"  // F2 で切り替わるデバッグ用フリーカメラ

class OnoderaScene : public IScene {
public:
	void Initialize() override;
	void Update() override;
	void Finalize() override;
	void Object3DDraw() override;
	void SpriteDraw() override;
	void ImGuiDraw() override;
	void ParticleDraw() override;
	TuboEngine::Camera* GetMainCamera() const override { return camera_.get(); }

private:
	
	std::unique_ptr<TuboEngine::Camera> camera_;

	// ───────── 3D オブジェクト ─────────
	std::unique_ptr<TuboEngine::Object3d> cannon_;
	std::unique_ptr<TuboEngine::Object3d> player_;
	std::unique_ptr<TuboEngine::Object3d> bullet_;

	float moveSpeed_ = 0.1f;     // WASD 移動速度
	// 弾が発射中か
	bool isBulletFired_ = false;
	// 弾の移動速度
	float bulletSpeed_ = 0.1f;
};