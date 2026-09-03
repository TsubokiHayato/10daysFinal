				#pragma once
#include "Object3d.h"
#include "Vector3.h"
#include <memory>

namespace TuboEngine { class Camera; }

// =============================================================================
//  Player ── プレイヤー本体（見下ろし操作）。
//
//  ・WASD で XZ 平面上を移動（W=奥+Z / S=手前-Z / A=左-X / D=右+X）。
//  ・移動方向へ向きを回転（見下ろしでどちらを向いているか分かるように）。
//  ・Field の範囲内にクランプ（枠外へ出ないように）。
//
//  砲弾システムなど後段のゲームロジックはここに足していける下地。
// =============================================================================
namespace game {

class Player {
public:
	// camera : 描画に使う主カメラ（追従カメラの Camera を渡す）
	void Initialize(TuboEngine::Camera* camera);
	void Update();
	void Draw();

	void SetPosition(const TuboEngine::Math::Vector3& p) { position_ = p; }
	const TuboEngine::Math::Vector3& GetPosition() const { return position_; }

	// 移動可能な半径（フィールド半分のサイズ）を渡してクランプに使う。
	// 原点中心で使う簡易版。
	void SetMoveBounds(float halfX, float halfZ) {
		boundCenter_ = {0.0f, 0.0f, 0.0f};
		boundHalfX_ = halfX;
		boundHalfZ_ = halfZ;
	}
	// フィールド中心が原点でない場合（自陣が左寄り等）はこちらで中心も渡す。
	void SetMoveBounds(const TuboEngine::Math::Vector3& center, float halfX, float halfZ) {
		boundCenter_ = center;
		boundHalfX_ = halfX;
		boundHalfZ_ = halfZ;
	}

	void SetCamera(TuboEngine::Camera* camera);

#ifdef USE_IMGUI
	void DrawImGui();
#endif

private:
	std::unique_ptr<TuboEngine::Object3d> model_;

	TuboEngine::Math::Vector3 position_{0.0f, 1.0f, -20.0f};
	float moveSpeed_ = 0.25f; // 1フレームあたりの移動量
	float yaw_ = 0.0f;        // 向き（Y軸回転）

	// フィールド境界（中心からの半分の広さ）。0以下ならクランプ無効。
	TuboEngine::Math::Vector3 boundCenter_{0.0f, 0.0f, 0.0f}; // クランプの中心
	float boundHalfX_ = 0.0f;
	float boundHalfZ_ = 0.0f;
};

} // namespace game
