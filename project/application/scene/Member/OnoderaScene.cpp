#include "OnoderaScene.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif
#include "Input.h"            // キーボード / マウス / ゲームパッド入力（エンジンが毎フレーム自動 Update）

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
	//  ③ 2D スプライト ── Sprite::Initialize(テクスチャパス)。
	//     パスは "Resources/Textures/" からの相対。座標は画面ピクセル(左上原点)。
	// ───────────────────────────────────────────────────────────

	spriteStartPos_ = { -50.0f, 100.0f };
	spriteEndPos_ = { 100.0f, 100.0f };

	sprite_ = std::make_unique<Sprite>();
	sprite_->Initialize("uvChecker.png");
	sprite_->SetPosition(spriteStartPos_);
	sprite_->SetSize({ 160.0f, 160.0f });

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

	//// ============================================================
	//// 弾の移動
	//// ============================================================
	//// 砲台クラスでisBulletFired_がtrueなら発射中の弾の処理を実行(弾クラスとかでやった方がいいかも？)
	//if (cannon_->GetIsBulletFired()) {
	//	Math::Vector3 bulletPos = bullet_->GetPosition();
	//	// X+方向へ移動
	//	bulletPos.x += bulletSpeed_;
	//	bullet_->SetPosition(bulletPos);
	//}


	// 取得したアイテムのパラメータをUIで表示する
	Itemdisplay();
		
	cannon_->Update();	
	player_->Update();
	bullet_->Update();

	sprite_->Update();
}


void OnoderaScene::Itemdisplay() {
	Input* input = Input::GetInstance();

	// ============================================================
	// Qキーでアイテム所持状態を切り替える
	// ============================================================
	if (input->TriggerKey(DIK_Q)) {
		hasItem_ = !hasItem_;

		// -----------------------------------------
		// アイテムを持った
		// start → end
		// -----------------------------------------
		if (hasItem_) {
			spriteMoveForward_ = true;
		}
		// -----------------------------------------
		// アイテムを手放した
		// end → start
		// -----------------------------------------
		else {
			spriteMoveForward_ = false;
		}

		// イージングを最初から開始
		spriteEaseTime_ = 0.0f;
		isSpriteMoving_ = true;
	}


	// ============================================================
	// Sprite イージング移動
	// ============================================================
	if (isSpriteMoving_) {
		// 経過時間
		spriteEaseTime_ += 1.0f / 60.0f;
		// 0.0 ～ 1.0
		float t = spriteEaseTime_ / spriteEaseDuration_;

		// 1を超えないようにする
		if (t >= 1.0f) {
			t = 1.0f;
			isSpriteMoving_ = false;
		}
		// EaseInOutBack
		float easedT = EaseInOutBack(t);

		// ========================================================
		// 移動方向によって座標を決める
		// ========================================================
		Math::Vector2 pos;
		if (spriteMoveForward_) {
			// start → end
			pos.x = spriteStartPos_.x + (spriteEndPos_.x - spriteStartPos_.x) * easedT;

			pos.y = spriteStartPos_.y + (spriteEndPos_.y - spriteStartPos_.y) * easedT;
		} else {
			// end → start
			pos.x = spriteEndPos_.x + (spriteStartPos_.x - spriteEndPos_.x) * easedT;

			pos.y = spriteEndPos_.y + (spriteStartPos_.y - spriteEndPos_.y) * easedT;
		}
		sprite_->SetPosition(pos);
	}
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
	sprite_->Draw();

}

void OnoderaScene::ParticleDraw() {}

// =============================================================================
//  ImGui（Debug ビルドのみ）
// =============================================================================
void OnoderaScene::ImGuiDraw() {
#ifdef USE_IMGUI

	player_->DrawImGui("3D Object : player");

#endif
}

// =============================================================================
//  Finalize
// =============================================================================
void OnoderaScene::Finalize() {}

float OnoderaScene::EaseInOutBack(float t) {
	const float c1 = 1.70158f;
	const float c2 = c1 * 1.525f;
	if (t < 0.5f) {
		return ((2.0f * t) * (2.0f * t) * ((c2 + 1.0f) * 2.0f * t - c2)) / 2.0f;
	} else {
		float x = 2.0f * t - 2.0f;
		return (x * x * ((c2 + 1.0f) * x + c2) + 2.0f) / 2.0f;
	}
}