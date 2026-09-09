#include "Option.h"
#include "GameScenes.h"
#include "SceneManager.h"
#include "TextManager.h"
#include "Input.h"  
#include "audio/AudioManager.h" // BGM / SE

using namespace TuboEngine;

namespace game {

	// =============================================================================
	//  Initialize
	// =============================================================================
	void Option::Initialize() {
		background_ = std::make_unique<Sprite>();
		background_->Initialize("sceneScreen.png");
		background_->SetColor({ 1.0f,1.0f,1.0f,0.0f }); // 最初は明度は0で透明にする
		background_->SetPosition({ 0.0f, 0.0f });
		background_->SetSize({ 1280.0f, 720.0f });
		isOpen_ = false;
		selectedIndex_ = -1;

		TextManager::GetInstance()->GetOrCreateFontSized(TextManager::PresetFontNames::Best10, 32.0f);

		// ゲームへ戻る
		TextManager::GetInstance()->CreateTextWithName(
			"Option_Return",
			TextManager::PresetFontNames::Best10 + "_32",
			"ゲームへ戻る",
			{ 500.0f, 300.0f },
			{ 1.0f, 1.0f, 1.0f, 1.0f },
			1.5f
		);

		// タイトルへ
		TextManager::GetInstance()->CreateTextWithName(
			"Option_Title",
			TextManager::PresetFontNames::Best10 + "_32",
			"タイトルへ",
			{ 520.0f, 380.0f },
			{ 1.0f, 1.0f, 1.0f, 1.0f },
			1.5f
		);		
		// TextManagerから取得して、最初から透明にする。
		returnText_ = TextManager::GetInstance()->GetTextByName("Option_Return");
		titleText_ = TextManager::GetInstance()->GetTextByName("Option_Title");
		returnText_->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
		titleText_->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });

		// 初期状態		
		isOpen_ = false;
		selectedIndex_ = -1;
		titleSelected_ = false;
		// 入場フェードイン＋退場フェードアウト		
		fadeScreen_ = std::make_unique<FadeScreen>();
		fadeScreen_->Initialize();
		pendingScene_ = -1;
	}

	// =============================================================================
	// Update
	// =============================================================================
	void Option::Update() {

		Input* input = Input::GetInstance();

		// Qでオプションを開く	
		if (input->TriggerKey(DIK_ESCAPE)) {
			if (isOpen_) {
				Close();
			} else {
				Open();
			}
		}

		// オプションが開いている間の処理(フェード中は動かせない)		
		if (isOpen_ && pendingScene_ < 0) {
			background_->SetColor({1.0f,1.0f,1.0f,0.4f}); // 背景を薄く表示

			// 最初の選択
			if (selectedIndex_ == -1) {
				selectedIndex_ = 0;
			}

			// Wで上
			if (input->TriggerKey(DIK_W)) {
				selectedIndex_--;
				if (selectedIndex_ < 0) {
					selectedIndex_ = 1;
				}
				AudioManager::GetInstance()->PlaySe("cursor_move.mp3");  // カーソル移動音
			}

			// Sで下
			if (input->TriggerKey(DIK_S)) {
				selectedIndex_++;
				if (selectedIndex_ > 1) {
					selectedIndex_ = 0;
				}
				AudioManager::GetInstance()->PlaySe("cursor_move.mp3");  // カーソル移動音
			}

			// 選択中の文字を黄色		
			if (selectedIndex_ == 0) {
				returnText_->SetColor({ 1.0f, 0.85f, 0.2f, 1.0f });
				titleText_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
			} else {
				returnText_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
				titleText_->SetColor({ 1.0f, 0.85f, 0.2f, 1.0f });
			}

			// --------------------------------------------------------
			// SPACE または ENTERで決定
			// --------------------------------------------------------
			if (input->TriggerKey(DIK_SPACE) ||input->TriggerKey(DIK_RETURN)) {

				AudioManager::GetInstance()->PlaySe("decide.mp3");  // 決定音

				// ゲームへ戻る
				if (selectedIndex_ == 0) {
					Close();
				}
				// タイトルへ
				else if (selectedIndex_ == 1) {
					titleSelected_ = true;
				}
			}
		}

		if (titleSelected_) {
			pendingScene_ = TITLE;
			fadeScreen_->FadeOut();
		}

		background_->Update();

		fadeScreen_->Update();
		if (pendingScene_ >= 0 && fadeScreen_->IsFadeOuting()) {
			SceneManager::GetInstance()->ChangeScene(pendingScene_);
		}
	}


	// =============================================================================
	//  描画フェーズ
	// =============================================================================
	void Option::Draw() {
		if (!isOpen_) {
			return;
		}
		background_->Draw();
		fadeScreen_->Draw();
	}

	// =============================================================================
	// Open
	// =============================================================================
	void Option::Open() {
		isOpen_ = true;
		// 最初は「ゲームへ戻る」
		selectedIndex_ = 0;
		titleSelected_ = false;
		// オプションを開いたので、テキストを完全表示
		returnText_->SetColor({ 1.0f,1.0f,1.0f,1.0f });
		titleText_->SetColor({ 1.0f,1.0f,1.0f,1.0f });
		// 最初の選択状態は黄色にする
		returnText_->SetColor({ 1.0f,0.85f,0.2f,1.0f });
	}

	// =============================================================================
	// Close
	// =============================================================================
	void Option::Close() {
		isOpen_ = false;
		selectedIndex_ = -1;
		titleSelected_ = false;
		background_->SetColor({1.0f,1.0f,1.0f,0.0f});
		// テキストを完全に透明にする
		returnText_->SetColor({ 0.0f,0.0f,0.0f,0.0f });
		titleText_->SetColor({ 0.0f,0.0f,0.0f,0.0f });
	}
}