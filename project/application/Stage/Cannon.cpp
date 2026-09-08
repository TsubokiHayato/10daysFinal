#include "Cannon.h"
#include "Input.h"            // キーボード / マウス / ゲームパッド入力（エンジンが毎フレーム自動 Update）
#include "Camera.h"

using namespace TuboEngine;

namespace game {

	void Cannon::Initialize(TuboEngine::Camera* camera, TuboEngine::Math::Vector3 propPos) {
		// 発射台の生成、初期化
		cannon_ = std::make_unique<Object3d>();
		cannon_->Initialize("square/square.obj");
		cannon_->SetCamera(camera);
		cannon_->SetPosition({ 0.0f, 0.0f, 0.0f });

		iconModel_ = std::make_unique<TuboEngine::Object3d>();
		iconModel_->Initialize("SpaceButton/SpaceButton.obj");
		iconModel_->SetCamera(camera);
		iconModel_->SetPosition({ 0.0f, 0.0f, 0.0f });

		propPos_ = propPos;
	}

	void Cannon::Update() {
		Input* input = Input::GetInstance();

		// ============================================================
		// スペースキーで弾を発射(弾が発射されていないとき)
		// ============================================================
		if (input->TriggerKey(DIK_SPACE) && !isBulletFired_) {
			// 弾を砲台の位置へ移動
			bullet_->SetPosition(cannon_->GetPosition());
			// 発射状態にする
			isBulletFired_ = true;
			isLoading_ = false;
		}

		if (isLoading_) {
			iconTimer_ += 1.0f / 60.0f;
		} else {
			iconTimer_ -= 1.0f / 60.0f;
		}

		iconTimer_ = std::clamp(iconTimer_, 0.0f, iconMaxTime_);

		iconModel_->SetModelColor({ 1.0f,1.0f,1.0f,iconTimer_ / iconMaxTime_ });
		iconModel_->SetPosition(propPos_ + TuboEngine::Math::Vector3(0.0f, 6.0f, 0.0f));
		iconModel_->SetRotation({ 0.2f,3.14f,0.0f });
		iconModel_->Update();

		cannon_->Update();
	}

	void Cannon::Draw() {
		//cannon_->Draw();
		if (iconTimer_ <= 0.0f) return;
		iconModel_->Draw();
	}

	//  ImGui（Debug ビルドのみ）
	void Cannon::ImGuiDraw() {
#ifdef USE_IMGUI
		cannon_->DrawImGui("3D Object : cannon");
#endif
	}
}