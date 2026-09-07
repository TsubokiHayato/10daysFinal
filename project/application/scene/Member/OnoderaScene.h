#pragma once
#include "IScene.h"
#include "Camera.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "Object3d.h"     // 3D モデル描画
#include "DebugCamera.h"  // F2 で切り替わるデバッグ用フリーカメラ
#include "Stage/Cannon.h"       // 発射台
#include "Sprite.h"       // 2D スプライト描画

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

	void Itemdisplay();

	float EaseInOutBack(float t);

private:
	std::unique_ptr<TuboEngine::Camera> camera_;

	// ───────── 3D オブジェクト ─────────
	// 発射台
	std::unique_ptr <game::Cannon> cannon_;

	std::unique_ptr<TuboEngine::Object3d> player_;
	std::unique_ptr<TuboEngine::Object3d> bullet_;

	float moveSpeed_ = 0.1f;     // WASD 移動速度
	// 弾の移動速度
	float bulletSpeed_ = 0.1f;

	// ───────── 2D スプライト ─────────
	std::unique_ptr<TuboEngine::Sprite> sprite_; // UV チェッカー画像

	// アイテムを持っているか
	bool hasItem_ = false;
	// Spriteのイージング
	// Spriteが移動中か
	bool isSpriteMoving_ = false;
	float spriteEaseTime_ = 0.0f;
	float spriteEaseDuration_ = 0.3f; // 1秒かけて移動
	// trueなら start → end
	// falseなら end → start
	bool spriteMoveForward_ = false;

	// 開始・終了位置
	TuboEngine::Math::Vector2 spriteStartPos_;
	TuboEngine::Math::Vector2 spriteEndPos_;
	

};