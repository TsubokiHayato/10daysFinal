#include "Player.h"

#include "Camera.h"
#include "Input.h"
#include "Item.h"

#include <cmath>

#ifdef USE_IMGUI
#include "ImGuiManager.h"
#include "externals/imgui/imgui.h"
#endif

using namespace TuboEngine;

namespace game {

// ※ Object3d.h がグローバルに class Camera; を前方宣言しているため、
//    ここでは曖昧さを避けるよう Camera は必ず TuboEngine:: を明示する。
void Player::Initialize(TuboEngine::Camera* camera) {
	model_ = std::make_unique<Object3d>();
	model_->Initialize("player/Player.obj");
	model_->SetCamera(camera);
	model_->SetPosition(position_);
}

void Player::SetCamera(TuboEngine::Camera* camera) {
	if (model_) model_->SetCamera(camera);
}

void Player::Update() {
	Input* input = Input::GetInstance();

	// --- 入力から移動ベクトルを作る（XZ 平面） ---
	Math::Vector3 move{0.0f, 0.0f, 0.0f};
	if (input->PushKey(DIK_W)) move.z += 1.0f; // 奥
	if (input->PushKey(DIK_S)) move.z -= 1.0f; // 手前
	if (input->PushKey(DIK_D)) move.x += 1.0f; // 右
	if (input->PushKey(DIK_A)) move.x -= 1.0f; // 左

	// 斜め移動が速くならないよう正規化してから速度を掛ける
	float len = std::sqrt(move.x * move.x + move.z * move.z);
	if (len > 0.0001f) {
		move.x = move.x / len * moveSpeed_;
		move.z = move.z / len * moveSpeed_;
		position_.x += move.x;
		position_.z += move.z;

		// 進行方向へ向く（Y軸回転）。モデルの正面が -Z 向きなので +π で反転させる。
		yaw_ = std::atan2(move.x, move.z) + 3.14159265f;
	}

	// --- フィールド範囲でクランプ（boundCenter_ を中心に ±half） ---
	if (boundHalfX_ > 0.0f) {
		float maxX = boundCenter_.x + boundHalfX_;
		float minX = boundCenter_.x - boundHalfX_;
		if (position_.x > maxX) position_.x = maxX;
		if (position_.x < minX) position_.x = minX;
	}
	if (boundHalfZ_ > 0.0f) {
		float maxZ = boundCenter_.z + boundHalfZ_;
		float minZ = boundCenter_.z - boundHalfZ_;
		if (position_.z > maxZ) position_.z = maxZ;
		if (position_.z < minZ) position_.z = minZ;
	}

	model_->SetPosition(position_);
	model_->SetRotation({0.0f, yaw_, 0.0f});
	model_->Update();

	// 手持ちアイテムは頭上に追従させる（実際の描画/Updateは Item 側が行う）。
	if (carried_) {
		carried_->SetPosition({position_.x, position_.y + 2.4f, position_.z});
	}
}

void Player::Draw() {
	model_->Draw();
}

#ifdef USE_IMGUI
void Player::DrawImGui() {
	if (ImGuiManager::GetInstance()->BeginPanel("Player")) {
		ImGui::DragFloat3("Position", &position_.x, 0.1f);
		ImGui::SliderFloat("Move Speed", &moveSpeed_, 0.05f, 1.0f);
		ImGui::Text("Yaw : %.2f rad", yaw_);
		ImGui::Text("Bounds : X=%.1f Z=%.1f", boundHalfX_, boundHalfZ_);
	}
	ImGuiManager::GetInstance()->EndPanel();
}
#endif

} // namespace game
