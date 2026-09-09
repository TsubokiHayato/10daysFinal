#include "StageCameraController.h"
#include "Camera.h"
#include "Input.h"

using namespace TuboEngine;

namespace game {

void StageCameraController::Initialize() {
	follow_ = std::make_unique<FollowCamera>();
	follow_->Initialize();
	debug_ = std::make_unique<DebugCamera>(); // F2 で乗っ取る確認用
}

void StageCameraController::Update(const Math::Vector3& playerPos, bool autoOverview) {
	// TAB長押し、または autoOverview(砲台接近など)で全体俯瞰へ滑らかに寄せる/戻す。
	const bool wantOverview = Input::GetInstance()->PushKey(DIK_TAB) || autoOverview;
	overview_ += ((wantOverview ? 1.0f : 0.0f) - overview_) * 0.12f;

	// 基本はプレイヤー追従。overview_ の分だけ戦場中央(原点)を見渡す。
	follow_->Update(playerPos, {0.0f, 0.0f, 0.0f}, overview_);

	// F2 デバッグカメラ（主カメラを乗っ取る）。追従更新の“後”に適用する。
	debug_->Update(follow_->GetCamera());
}

void StageCameraController::DrawImGui() {
#ifdef USE_IMGUI
	follow_->DrawImGui();
	debug_->DrawImGui();
#endif
}

} // namespace game
