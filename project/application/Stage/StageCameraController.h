#pragma once
#include "Vector3.h"
#include "DebugCamera.h"
#include "Stage/FollowCamera.h"
#include <memory>

namespace TuboEngine { class Camera; }

// =============================================================================
//  StageCameraController ── ステージのカメラ制御をまとめる。
//
//  ・FollowCamera : プレイヤーを追う見下ろしカメラ（このシーンの主カメラ）。
//  ・DebugCamera  : F2 で主カメラを乗っ取る確認用フリーカメラ。
//  ・overview_    : 0=追従 / 1=全体俯瞰。TAB長押しや砲台接近で 1 へ寄せる。
// =============================================================================
namespace game {

class StageCameraController {
public:
	void Initialize();

	// playerPos    : 追従先。
	// autoOverview : TAB以外の理由で俯瞰したいとき true（例：砲台に接近）。
	void Update(const TuboEngine::Math::Vector3& playerPos, bool autoOverview);

	void SnapTo(const TuboEngine::Math::Vector3& pos) { follow_->SnapTo(pos); }
	TuboEngine::Camera* GetCamera() const { return follow_->GetCamera(); }
	float GetOverview() const { return overview_; }

	void DrawImGui();

private:
	std::unique_ptr<FollowCamera> follow_;
	std::unique_ptr<TuboEngine::DebugCamera> debug_;
	float overview_ = 0.0f;
};

} // namespace game
