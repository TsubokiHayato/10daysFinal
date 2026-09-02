#include "FollowCamera.h"

#include <cmath>

#ifdef USE_IMGUI
#include "ImGuiManager.h"
#include "externals/imgui/imgui.h"
#endif

using namespace TuboEngine;

namespace game {

void FollowCamera::Initialize() {
	camera_ = std::make_unique<Camera>();
	// 見下ろし用の投影。near/far はコンストラクタ既定でも良いが明示しておく。
	camera_->setNearClip(0.1f);
	camera_->setFarClip(300.0f);
	camera_->setScale({1.0f, 1.0f, 1.0f});
}

void FollowCamera::SnapTo(const Math::Vector3& targetPos) {
	focus_ = targetPos;
	Update(targetPos); // すぐ反映
	focus_ = targetPos;
}

void FollowCamera::Update(const Math::Vector3& targetPos) {
	// (1) 注視点をターゲットへ向けて補間（ヌルっと追従）
	focus_ += (targetPos - focus_) * followLerp_;

	// (2) 注視点の真上やや後方へカメラを配置
	Math::Vector3 eye = focus_;
	eye.y += height_;
	eye.z -= back_;
	camera_->SetTranslate(eye);

	// (3) 見下ろし角(ピッチ)。カメラ→注視点ベクトルから求める。
	//     水平距離 back_、垂直距離 height_ を見下ろすので pitch = atan2(height, back)。
	//     back_ がほぼ 0 のときは真下（π/2）を向く。
	float pitch = std::atan2(height_, (back_ > 0.001f) ? back_ : 0.001f);
	camera_->setRotation({pitch, 0.0f, 0.0f});

	camera_->Update();
}

#ifdef USE_IMGUI
void FollowCamera::DrawImGui() {
	if (ImGuiManager::GetInstance()->BeginPanel("FollowCamera")) {
		ImGui::SliderFloat("Height", &height_, 5.0f, 60.0f);
		ImGui::SliderFloat("Back", &back_, 0.0f, 40.0f);
		ImGui::SliderFloat("Follow Lerp", &followLerp_, 0.02f, 1.0f);
		const Math::Vector3& e = camera_->GetTranslate();
		ImGui::Text("Eye  : (%.1f, %.1f, %.1f)", e.x, e.y, e.z);
		ImGui::Text("Focus: (%.1f, %.1f, %.1f)", focus_.x, focus_.y, focus_.z);
	}
	ImGuiManager::GetInstance()->EndPanel();
}
#endif

} // namespace game
