#pragma once
#include "IScene.h"

#include "DebugCamera.h"
#include "FollowCamera.h"
#include "Player.h"
#include "Field.h"
#include <memory>

// =============================================================================
//  StageScene ── ゲーム本編の見下ろしステージ。
//
//  構成要素:
//    ・Field        : 床＋外周壁のフィールド
//    ・Player       : WASD で動くプレイヤー
//    ・FollowCamera : プレイヤーを追う見下ろしカメラ（このシーンの主カメラ）
//    ・DebugCamera  : F2 で切り替わる確認用フリーカメラ
//
//  砲弾の生成・発射・当たり判定などのゲームロジックは、この土台に
//  Player / Field と同じ流儀で足していく想定。
// =============================================================================
class StageScene : public IScene {
public:
	void Initialize() override;
	void Update() override;
	void Finalize() override;
	void Object3DDraw() override;
	void SpriteDraw() override;
	void ImGuiDraw() override;
	void ParticleDraw() override;

	TuboEngine::Camera* GetMainCamera() const override {
		return followCamera_->GetCamera();
	}

private:
	std::unique_ptr<game::FollowCamera> followCamera_;
	std::unique_ptr<TuboEngine::DebugCamera> debugCamera_;

	std::unique_ptr<game::Field> field_;
	std::unique_ptr<game::Player> player_;

	bool showGrid_ = false; // 追加のワールドグリッド表示（デバッグ）
};
