#pragma once
#include "Camera.h"
#include "Vector3.h"
#include <memory>

// =============================================================================
//  FollowCamera ── プレイヤーを追従する「見下ろし型」カメラ。
//
//  ・ターゲット（プレイヤー）の座標を追いかけ、真上やや後方から見下ろす。
//  ・height（高さ）と back（後方距離）から見下ろし角(ピッチ)を自動計算し、
//    常にターゲット付近を画面中央に捉える。
//  ・followLerp_ で追従の“ゆるさ”（0=即追従 / 小さいほどヌルっと遅れて付いていく）。
//
//  2D風の見下ろし3Dなので、back を小さく height を大きくするとほぼ真上、
//  back を大きくすると斜め見下ろし（奥行きが見える）になる。
// =============================================================================
namespace game {

class FollowCamera {
public:
	void Initialize();

	// 追従更新。targetPos はプレイヤーのワールド座標。
	void Update(const TuboEngine::Math::Vector3& targetPos);

	// 追従⇄全体俯瞰の補間つき更新。
	//  overview: 0=プレイヤー追従 / 1=全体（両陣）を見渡す俯瞰。
	//  overviewFocus: 俯瞰時に中心とする点（戦場の中央など）。
	void Update(const TuboEngine::Math::Vector3& targetPos,
	            const TuboEngine::Math::Vector3& overviewFocus,
	            float overview);

	// エンジンへ渡す主カメラ
	TuboEngine::Camera* GetCamera() const { return camera_.get(); }

	// 追従を待たずにその場へスナップ（シーン開始時など）
	void SnapTo(const TuboEngine::Math::Vector3& targetPos);

#ifdef USE_IMGUI
	void DrawImGui();
#endif

private:
	// 現在の注視点（ここから offset ぶんずらした所にカメラを置く）
	TuboEngine::Math::Vector3 focus_{0.0f, 0.0f, 0.0f};

	std::unique_ptr<TuboEngine::Camera> camera_;

	float height_ = 24.0f;   // ターゲットからの高さ
	float back_ = 12.0f;     // ターゲットから後方(-Z)への距離
	float followLerp_ = 0.15f; // 追従補間率(0〜1)。1で即追従

	// 全体俯瞰（TABなどで一時的に引く）用のカメラ距離。
	float overviewHeight_ = 90.0f; // 俯瞰時の高さ
	float overviewBack_ = 60.0f;   // 俯瞰時の後方距離
};

} // namespace game
