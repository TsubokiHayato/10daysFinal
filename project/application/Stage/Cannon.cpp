#include "Cannon.h"
#include "Input.h"            // キーボード / マウス / ゲームパッド入力（エンジンが毎フレーム自動 Update）
#include "Camera.h"

using namespace TuboEngine;

namespace game {

	void Cannon::Initialize(TuboEngine::Camera* camera) {
		// 発射台の生成、初期化
		cannon_ = std::make_unique<Object3d>();
		cannon_->Initialize("square/square.obj");
		cannon_->SetCamera(camera);
		cannon_->SetPosition({ 0.0f, 0.0f, 0.0f });
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
		}

		cannon_->Update();
	}

	void Cannon::Draw() {
		cannon_->Draw();
	}

	//  ImGui（Debug ビルドのみ）
	void Cannon::ImGuiDraw() {
#ifdef USE_IMGUI
		cannon_->DrawImGui("3D Object : cannon");
#endif
	}
}