#include "StageScene.h"
#include "GameScenes.h"
#include "SceneManager.h"

#include "Camera.h"
#include "Input.h"
#include "TextManager.h"
#include "Stage/StageLayout.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "ImGuiManager.h"
#endif

using namespace TuboEngine;

// =============================================================================
//  Initialize
// =============================================================================
void StageScene::Initialize() {
	// ① カメラ（このシーンの主カメラを内包）
	camera_ = std::make_unique<game::StageCameraController>();
	camera_->Initialize();
	TuboEngine::Camera* cam = camera_->GetCamera();

	// ② 地形・城・砲台プロップ
	environment_ = std::make_unique<game::StageEnvironment>();
	environment_->Initialize(cam);

	// ③ プレイヤー。自陣の中央あたりに配置し、自陣の範囲でクランプ。
	game::Field* selfField = environment_->GetSelfField();
	player_ = std::make_unique<game::Player>();
	player_->Initialize(cam);
	player_->SetPosition({-game::layout::kFieldOffsetX, 1.0f, -selfField->GetHalfZ() * 0.4f});
	player_->SetMoveBounds(selfField->GetCenter(),
	                       selfField->GetHalfX() - 1.5f, selfField->GetHalfZ() - 1.5f);

	// ④ アイテム・工作台（自陣に散布）
	itemField_ = std::make_unique<game::ItemField>();
	itemField_->Initialize(cam, selfField);

	// ⑤ 砲台・砲弾システム（的は敵の城）
	bulletManager_ = std::make_unique<game::BulletManager>();
	bulletManager_->Initialize(cam, environment_->GetMuzzle(), environment_->GetEnemyCastle());

	// カメラをプレイヤー位置へスナップ（開始時にワープして見えないように）
	camera_->SnapTo(player_->GetPosition());

	TextManager::GetInstance()->LoadTextLayout("Resources/Text/StageTutorial.json");

	visualManager_ = VisualManager::GetInstance();
	visualManager_->Initialize(cam);
}

// =============================================================================
//  Update
// =============================================================================
void StageScene::Update() {
	// (1) プレイヤー入力・移動
	player_->Update();

	// (2) E キー：砲台への装填(設置)を最優先。装填できなければアイテム操作(拾う/破棄/工作台へ)。
	if (!bulletManager_->TryLoad(player_.get())) {
		itemField_->HandleInteraction(player_.get());
	}
	itemField_->Update();

	// (3) 砲弾の発射・飛翔・命中
	bulletManager_->Update();

	// (4) 地形・城・砲台プロップ・グリッド
	environment_->Update();

	// (5) カメラ：砲台に近づいたら自動ズームアウト（TAB長押しでも可）。
	const bool nearCannon =
		game::Dist2XZ(player_->GetPosition(), environment_->GetMuzzle()) <
		game::layout::kCannonZoomRange * game::layout::kCannonZoomRange;
	camera_->Update(player_->GetPosition(), nearCannon);



	TuboEngine::TextManager::GetInstance()->UpdateAll();
	visualManager_->Update();
}

// =============================================================================
//  描画フェーズ
// =============================================================================
void StageScene::Object3DDraw() {
	environment_->Draw();
	bulletManager_->Draw();
	itemField_->Draw();
	player_->Draw();
}

void StageScene::SpriteDraw() {
	TuboEngine::TextManager::GetInstance()->DrawAll();
}

void StageScene::ParticleDraw() {}

// =============================================================================
//  ImGui（Debug ビルドのみ）
// =============================================================================
void StageScene::ImGuiDraw() {
#ifdef USE_IMGUI
	if (ImGuiManager::GetInstance()->BeginPanel("Stage")) {
		ImGui::TextWrapped("見下ろしステージ(自陣/敵陣)。WASD移動 / E=拾う(手ぶら)・砲台が近ければ装填・工作台へ載せる・その他は破棄(手持ち) / SPACE=発射 / 砲台に近づくと自動ズームアウト(TAB長押しでも可) / F2デバッグカメラ。");
		ImGui::Separator();

		// 砲台と敵の城の状態。
		game::Castle* enemyCastle = environment_->GetEnemyCastle();
		ImGui::Text("砲台 : %s / 場の弾 : %d",
		            bulletManager_->HasPending() ? "装填済み(SPACEで発射)" : "空(Rで装填)",
		            bulletManager_->ActiveCount());
		ImGui::Text("敵の城 : HP %.1f / %s", enemyCastle->GetHP(),
		            enemyCastle->IsPoisoned() ? "毒状態" : "正常");
		ImGui::Separator();

		// アイテム/クラフトの状態表示。
		game::Item* carried = player_->GetCarried();
		ImGui::Text("手持ち : %s", carried ? carried->GetName().c_str() : "なし");
		game::Workbench* bench = itemField_->GetWorkbench();
		game::Item* body = bench->GetBodySlot();
		game::Item* head = bench->GetHeadSlot();
		ImGui::Text("工作台 : 胴[%s] 頭[%s]",
		            body ? body->GetName().c_str() : "空",
		            head ? head->GetName().c_str() : "空");
		ImGui::Text("地面のアイテム : %d 個 / 砲弾 : %d 個",
		            itemField_->CountGround(), itemField_->CountShells());

		// 手持ちのステータスを見せる。
		if (carried) {
			const game::PartStats& s = carried->GetStats();
			ImGui::Text("  威力%.0f 弾速%.2f 爆発%.1f 重%.1f", s.damage, s.speed, s.blast, s.weight);
		}
		ImGui::Separator();

		ImGui::Text("Overview : %.2f", camera_->GetOverview());
		ImGui::Checkbox("World Grid", environment_->ShowGridRef());
		ImGui::Spacing();

		SceneManager* sm = SceneManager::GetInstance();
		if (ImGui::Button("Title へ")) sm->ChangeScene(TITLE);
		ImGui::SameLine();
		if (ImGui::Button("Sample へ")) sm->ChangeScene(SAMPLE);
		ImGui::SameLine();
		if (ImGui::Button("リロード")) sm->ChangeScene(STAGE);

		TuboEngine::TextManager::GetInstance()->DrawImGui();
	}
	ImGuiManager::GetInstance()->EndPanel();

	player_->DrawImGui();
	camera_->DrawImGui();
#endif
}

// =============================================================================
//  Finalize
// =============================================================================
void StageScene::Finalize() {
	TuboEngine::TextManager::GetInstance()->ClearAllTexts();
	TuboEngine::TextManager::GetInstance()->ClearAllSprites();
}
