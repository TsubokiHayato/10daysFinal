#include "StageScene.h"
#include "GameScenes.h"
#include "SceneManager.h"

#include "LineManager.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

using namespace TuboEngine;

// =============================================================================
//  Initialize
// =============================================================================
void StageScene::Initialize() {
	// ① 追従見下ろしカメラ（このシーンの主カメラ）
	followCamera_ = std::make_unique<game::FollowCamera>();
	followCamera_->Initialize();

	// F2 で乗っ取るデバッグカメラ
	debugCamera_ = std::make_unique<DebugCamera>();

	TuboEngine::Camera* cam = followCamera_->GetCamera();

	// ② フィールド（床＋壁）
	field_ = std::make_unique<game::Field>();
	field_->Initialize(cam);

	// ③ プレイヤー。手前側(-Z)に配置し、フィールド範囲でクランプ。
	player_ = std::make_unique<game::Player>();
	player_->Initialize(cam);
	player_->SetPosition({0.0f, 1.0f, -field_->GetHalfZ() * 0.6f});
	// 壁の内側で止まるよう、半径から少し内側をクランプ境界にする
	player_->SetMoveBounds(field_->GetHalfX() - 1.5f, field_->GetHalfZ() - 1.5f);

	// カメラをプレイヤー位置へスナップ（開始時にワープして見えないように）
	followCamera_->SnapTo(player_->GetPosition());
}

// =============================================================================
//  Update
// =============================================================================
void StageScene::Update() {
	// (1) プレイヤー入力・移動
	player_->Update();

	// (2) カメラをプレイヤーへ追従
	followCamera_->Update(player_->GetPosition());

	// (3) F2 デバッグカメラ（主カメラを乗っ取る）。追従更新の“後”に適用する。
	debugCamera_->Update(followCamera_->GetCamera());

	// (4) フィールド更新
	field_->Update();

	// (5) 任意：ワールドグリッド（デバッグ表示）
	if (showGrid_) {
		LineManager::GetInstance()->DrawGrid(
			field_->GetHalfX() * 2.0f, 20, {0.0f, 0.01f, 0.0f}, {0.4f, 0.4f, 0.4f, 1.0f});
	}
}

// =============================================================================
//  描画フェーズ
// =============================================================================
void StageScene::Object3DDraw() {
	field_->Draw();
	player_->Draw();
}

void StageScene::SpriteDraw() {}

void StageScene::ParticleDraw() {}

// =============================================================================
//  ImGui（Debug ビルドのみ）
// =============================================================================
void StageScene::ImGuiDraw() {
#ifdef USE_IMGUI
	if (ImGuiManager::GetInstance()->BeginPanel("Stage")) {
		ImGui::TextWrapped("見下ろしステージ。WASDで移動 / F2でデバッグカメラ。");
		ImGui::Separator();
		ImGui::Checkbox("World Grid", &showGrid_);
		ImGui::Spacing();

		SceneManager* sm = SceneManager::GetInstance();
		if (ImGui::Button("Title へ")) sm->ChangeScene(TITLE);
		ImGui::SameLine();
		if (ImGui::Button("Sample へ")) sm->ChangeScene(SAMPLE);
		ImGui::SameLine();
		if (ImGui::Button("リロード")) sm->ChangeScene(STAGE);
	}
	ImGuiManager::GetInstance()->EndPanel();

	player_->DrawImGui();
	followCamera_->DrawImGui();
	debugCamera_->DrawImGui();
#endif
}

// =============================================================================
//  Finalize
// =============================================================================
void StageScene::Finalize() {}
