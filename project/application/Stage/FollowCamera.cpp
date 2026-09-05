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
	camera_->setFarClip(600.0f); // 引きの俯瞰でも奥まで映るように広げる
	camera_->setScale({1.0f, 1.0f, 1.0f});
}

void FollowCamera::SnapTo(const Math::Vector3& targetPos) {
	focus_ = targetPos;
	Update(targetPos); // すぐ反映
	focus_ = targetPos;
}

void FollowCamera::Update(const Math::Vector3& targetPos) {
	Update(targetPos, {0.0f, 0.0f, 0.0f}, 0.0f);
}

void FollowCamera::Update(const Math::Vector3& targetPos, const Math::Vector3& overviewFocus,
                          float overview) {
	// overview を 0〜1 にクランプ。
	if (overview < 0.0f) overview = 0.0f;
	if (overview > 1.0f) overview = 1.0f;

	// (1) 注視点：追従先(プレイヤー)と俯瞰の中心(戦場中央)を overview で混ぜる。
	Math::Vector3 target = {
		targetPos.x + (overviewFocus.x - targetPos.x) * overview,
		targetPos.y + (overviewFocus.y - targetPos.y) * overview,
		targetPos.z + (overviewFocus.z - targetPos.z) * overview,
	};
	focus_ += (target - focus_) * followLerp_;

	// (2) 高さ・後方距離も overview で引きの画へ寄せる。
	float height = height_ + (overviewHeight_ - height_) * overview;
	float back = back_ + (overviewBack_ - back_) * overview;

	// (3) 注視点の真上やや後方へカメラを配置
	Math::Vector3 eye = focus_;
	eye.y += height;
	eye.z -= back;
	camera_->SetTranslate(eye);

	// (4) 見下ろし角(ピッチ)。水平距離 back、垂直距離 height を見下ろす。
	float pitch = std::atan2(height, (back > 0.001f) ? back : 0.001f);
	camera_->setRotation({pitch, 0.0f, 0.0f});

	camera_->Update();
}

#ifdef USE_IMGUI
void FollowCamera::DrawImGui() {
	if (ImGuiManager::GetInstance()->BeginPanel("FollowCamera")) {
		ImGui::SliderFloat("Height", &height_, 5.0f, 200.0f);
		ImGui::SliderFloat("Back", &back_, 0.0f, 140.0f);
		ImGui::SliderFloat("Follow Lerp", &followLerp_, 0.02f, 1.0f);
		const Math::Vector3& e = camera_->GetTranslate();
		ImGui::Text("Eye  : (%.1f, %.1f, %.1f)", e.x, e.y, e.z);
		ImGui::Text("Focus: (%.1f, %.1f, %.1f)", focus_.x, focus_.y, focus_.z);
	}
	ImGuiManager::GetInstance()->EndPanel();
}
#endif

} // namespace game
