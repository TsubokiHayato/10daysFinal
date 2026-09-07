#include "OnoderaScene.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif
#include "Input.h"            // キーボード / マウス / ゲームパッド入力（エンジンが毎フレーム自動 Update）
#include "TextManager.h"

using namespace TuboEngine;

// =============================================================================
//  Initialize
// =============================================================================
void OnoderaScene::Initialize() {
	// ※ Camera はグローバルにも前方宣言があるため TuboEngine:: を明示する。
	camera_ = std::make_unique<TuboEngine::Camera>();
	camera_->SetTranslate({ 0.0f, 4.0f, -35.0f });
	camera_->setRotation({ 0.0f, 0.0f, 0.0f });
	camera_->setScale({ 1.0f, 1.0f, 1.0f });
	camera_->Update();

	// 発射台の初期化
	cannon_ = std::make_unique<game::Cannon>();
	cannon_->Initialize(camera_.get());

	// プレイヤーの生成、初期化
	player_ = std::make_unique<Object3d>();
	player_->Initialize("player/player.obj");
	player_->SetCamera(camera_.get());
	player_->SetPosition({ -2.0f, 0.0f, 0.0f });
	// 弾の生成、初期化
	bullet_ = std::make_unique<Object3d>();
	bullet_->Initialize("block/block.obj");
	bullet_->SetCamera(camera_.get());
	bullet_->SetPosition({ 2.0f, 0.0f, 0.0f });
	// 最初は砲台の位置に置いておく
	cannon_->SetBullet(bullet_.get());

	// ───────────────────────────────────────────────────────────
	//  ⑥ テキスト ── TextManager（シングルトン）。
	//     CreateText で生成したテキストはエンジン終了まで TextManager 側が所有する。
	//     Update/Draw はシーンが駆動する（ParticleManager と同じ作法）。
	// ───────────────────────────────────────────────────────────
	TextManager::GetInstance()->GetOrCreateFontSized(TextManager::PresetFontNames::Best10, 32.0f);
	TextManager::GetInstance()->CreateText(
		TextManager::PresetFontNames::Best10 + "_32",
		"Sample",
		{ 40.0f, 220.0f },
		{1.0f,1.0f,1.0f,1.0f},
		1.0f
	);
}

// =============================================================================
//  Update
// =============================================================================
void OnoderaScene::Update() {
	camera_->Update();

	Input* input = Input::GetInstance();
	Math::Vector3 pos = player_->GetPosition();

	// PushKey(DIK_*) は「押されている間 true」。DIK_* は dinput.h のキーコード。
	if (input->PushKey(DIK_D)) pos.x += moveSpeed_;
	if (input->PushKey(DIK_A)) pos.x -= moveSpeed_;
	if (input->PushKey(DIK_S)) pos.z -= moveSpeed_;
	if (input->PushKey(DIK_W)) pos.z += moveSpeed_;

	player_->SetPosition(pos);

	// ============================================================
	// 弾の移動
	// ============================================================
	// 砲台クラスでisBulletFired_がtrueなら発射中の弾の処理を実行(弾クラスとかでやった方がいいかも？)
	if (cannon_->GetIsBulletFired()) {
		Math::Vector3 bulletPos = bullet_->GetPosition();
		// X+方向へ移動
		bulletPos.x += bulletSpeed_;
		bullet_->SetPosition(bulletPos);
	}
	cannon_->Update();
	player_->Update();
	bullet_->Update();

	// (7) TextManager 更新。Particle と同様にシーンが駆動する。
	TextManager::GetInstance()->UpdateAll();
}

// =============================================================================
//  描画フェーズ
// =============================================================================
void OnoderaScene::Object3DDraw() {
	cannon_->Draw();
	player_->Draw();
	bullet_->Draw();
}

void OnoderaScene::SpriteDraw() {
	// TextManager が持つテキストの描画。Sprite と同じパイプライン状態で描く。
	TextManager::GetInstance()->DrawAll();
}

void OnoderaScene::ParticleDraw() {}

// =============================================================================
//  ImGui（Debug ビルドのみ）
// =============================================================================
void OnoderaScene::ImGuiDraw() {
#ifdef USE_IMGUI

	player_->DrawImGui("3D Object : player");

	TextManager::GetInstance()->DrawImGui();     // テキストの本格エディタ（生成/JSONレイアウト保存読込）
#endif
}

// =============================================================================
//  Finalize
// =============================================================================
void OnoderaScene::Finalize() {

	// TextManager はシングルトンでシーンを越えて生存するため、
	// このシーンで作ったテキストは退場時に必ず片付ける。
	TextManager::GetInstance()->ClearAllTexts();
}