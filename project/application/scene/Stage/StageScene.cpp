#include "StageScene.h"
#include "GameScenes.h"
#include "SceneManager.h"

#include "Camera.h"
#include "Input.h"
#include "TextManager.h"
#include "Stage/StageLayout.h"
#include "Stage/Bullet.h"
#include <vector>

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

	// ⑤ 砲台・砲弾システム（的は敵陣中央の大砲。着弾で相手の床を削る）
	game::Field* enemyField = environment_->GetEnemyField();
	bulletManager_ = std::make_unique<game::BulletManager>();
	bulletManager_->Initialize(cam, environment_->GetMuzzle(), enemyField,
	                           environment_->GetEnemyCannonPos());

	// ⑥ 敵の攻撃：敵陣のベルトコンベアが球パーツを運び、大砲に届くと自陣を撃つ。
	enemyConveyor_ = std::make_unique<game::EnemyConveyor>();
	enemyConveyor_->Initialize(cam, enemyField, environment_->GetEnemyCannonPos(),
	                           environment_->GetSelfCannonPos(), selfField);

	// カメラをプレイヤー位置へスナップ（開始時にワープして見えないように）
	camera_->SnapTo(player_->GetPosition());

	//TextManager::GetInstance()->LoadTextLayout("Resources/Text/StageUI.json");

	// アイテムのUI表示クラス
	itemdisplay_ = std::make_unique<game::ItemDisplay>();
	itemdisplay_->Initialize();

	tutorial_ = std::make_unique<Tutorial>();
	tutorial_->Initialize();

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

	// (3.5) 敵の攻撃（ベルトコンベア→大砲→自陣の床）
	enemyConveyor_->Update();

	// (3.6) 空中での弾の相殺（自弾と敵弾が接触したら、どちらも打ち消す）
	ResolveBulletClashes();

	// (4) 地形・床(HP)・大砲プロップ・グリッド
	environment_->Update();

	// (5) カメラ：砲台に近づいたら自動ズームアウト（TAB長押しでも可）。
	const bool nearCannon =
		game::Dist2XZ(player_->GetPosition(), environment_->GetMuzzle()) <
		game::layout::kCannonZoomRange * game::layout::kCannonZoomRange;
	camera_->Update(player_->GetPosition(), nearCannon);

	TuboEngine::TextManager::GetInstance()->UpdateAll();

	// (6) アイテム情報のUI表示
	itemdisplay_->Update(player_->GetCarried());

	tutorial_->Update(
		itemField_->GetTutorialFlagCarried(),
		itemField_->GetTutorialFlagCreate(),
		bulletManager_->GetTutorialFlagLoad(),
		bulletManager_->GetTutorialFlagShot()
	);

	visualManager_->Update();
}

// =============================================================================
//  弾同士の空中相殺
//   ・飛翔中のプレイヤー弾と敵弾が kBulletCancelRadius 内で接触したら、
//     強さに関係なく双方を消す（着地点を固定しているので同じ線上で交差する）。
// =============================================================================
void StageScene::ResolveBulletClashes() {
	std::vector<game::Bullet*> mine = bulletManager_->GetFlyingBullets();
	std::vector<game::Bullet*> foe = enemyConveyor_->GetFlyingBullets();
	if (mine.empty() || foe.empty()) return;

	const float r2 = game::layout::kBulletCancelRadius * game::layout::kBulletCancelRadius;
	bool anyCancel = false;

	for (game::Bullet* a : mine) {
		if (!a->IsActive()) continue;
		for (game::Bullet* b : foe) {
			if (!b->IsActive()) continue;

			// 3D距離で接触判定。近ければ双方消滅。
			const TuboEngine::Math::Vector3& pa = a->GetPosition();
			const TuboEngine::Math::Vector3& pb = b->GetPosition();
			float dx = pa.x - pb.x, dy = pa.y - pb.y, dz = pa.z - pb.z;
			if (dx * dx + dy * dy + dz * dz > r2) continue;

			a->Deactivate();
			b->Deactivate();
			anyCancel = true;
			break; // a は消えたので次の a へ
		}
	}

	// 相殺が起きたら軽く画面を揺らして手応えを出す。
	if (anyCancel) visualManager_->Shake(0.25f, 1.2f);
}

// =============================================================================
//  描画フェーズ
// =============================================================================
void StageScene::Object3DDraw() {
	environment_->Draw();
	bulletManager_->Draw();
	enemyConveyor_->Draw();
	itemField_->Draw();
	player_->Draw();
}

void StageScene::SpriteDraw() {
	itemdisplay_->Draw();

	tutorial_->Draw();

	TuboEngine::TextManager::GetInstance()->DrawAll();
}

void StageScene::ParticleDraw() {}

// =============================================================================
//  ImGui（Debug ビルドのみ）
// =============================================================================
void StageScene::ImGuiDraw() {
#ifdef USE_IMGUI
	if (ImGuiManager::GetInstance()->BeginPanel("Stage")) {
		ImGui::TextWrapped("見下ろしステージ(自陣/敵陣)。WASD移動 / E=拾う(手ぶら)・大砲が近ければ装填・工作台へ載せる・その他は破棄(手持ち) / SPACE=発射 / 大砲に近づくと自動ズームアウト(TAB長押しでも可) / F2デバッグカメラ。");
		ImGui::Separator();

		// 大砲と床(HP)の状態。床＝HP。敵の攻撃はベルトコンベアから。
		ImGui::Text("大砲 : %s / 場の弾 : %d",
		            bulletManager_->HasPending() ? "装填済み(SPACEで発射)" : "空(Rで装填)",
		            bulletManager_->ActiveCount());
		game::Field* selfF = environment_->GetSelfField();
		game::Field* enemyF = environment_->GetEnemyField();
		ImGui::Text("自陣の床HP : %.0f / %.0f (%.0f%%)%s", selfF->GetHP(), selfF->GetMaxHP(),
		            selfF->GetHPRatio() * 100.0f, selfF->IsCollapsing() ? " 崩壊!" : "");
		ImGui::Text("敵陣の床HP : %.0f / %.0f (%.0f%%)%s", enemyF->GetHP(), enemyF->GetMaxHP(),
		            enemyF->GetHPRatio() * 100.0f, enemyF->IsCollapsing() ? " 崩壊!" : "");
		ImGui::Text("敵コンベア : 搬送中 %d / 大砲[胴%s 頭%s] / 敵弾 %d 発",
		            enemyConveyor_->PartCount(),
		            enemyConveyor_->HasBody() ? "○" : "×",
		            enemyConveyor_->HasHead() ? "○" : "×",
		            enemyConveyor_->BulletCount());
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

	}
	ImGuiManager::GetInstance()->EndPanel();

	TuboEngine::TextManager::GetInstance()->DrawImGui();
	player_->DrawImGui();
	camera_->DrawImGui();
	enemyConveyor_->DrawImGui();
#endif
}

// =============================================================================
//  Finalize
// =============================================================================
void StageScene::Finalize() {
	TuboEngine::TextManager::GetInstance()->ClearAllTexts();
	TuboEngine::TextManager::GetInstance()->ClearAllSprites();
}
