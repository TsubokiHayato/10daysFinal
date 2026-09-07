#include "ItemDisplay.h"

using namespace TuboEngine;

// =============================================================================
//  Initialize
// =============================================================================
void ItemDisplay::Initialize() {
	easetime_ = 0.0f;                 // イージング経過時間
	easeduration_ = 0.2f;   	        // 移動時間
	startpos_ = { 0.0f, 100.0f }; 	// 開始位置
	endpos_ = { 100.0f, 100.0f }; 	// 終了位置

	// 表示するスプライトの生成、初期化
	sprite_ = std::make_unique<TuboEngine::Sprite>();
	sprite_->Initialize("uvChecker.png");
	sprite_->SetPosition(startpos_);
	sprite_->SetSize({ 160.0f, 160.0f });
}

// =============================================================================
// Update
// =============================================================================
void ItemDisplay::Update(bool hasItem) {
	// ============================================================
	// アイテム所持状態が変化したか
	// ============================================================
	if (hasItem != previousHasItem_) {
		previousHasItem_ = hasItem;
		// --------------------------------------------------------
		// アイテムを持った
		// START → END
		// --------------------------------------------------------
		if (hasItem) {
			moveforward_ = true;
		}
		// --------------------------------------------------------
		// アイテムを手放した
		// END → START
		// --------------------------------------------------------
		else {
			moveforward_ = false;
		}
		// イージング開始
		easetime_ = 0.0f;
		ismoving_ = true;
	}

	// ============================================================
	// Sprite のイージング移動
	// ============================================================
	if (ismoving_) {
		// 経過時間
		easetime_ += 1.0f / 60.0f;
		// 0.0 ～ 1.0
		float t = easetime_ / easeduration_;
		// 1を超えないようにする
		if (t >= 1.0f) {
			t = 1.0f;
			ismoving_ = false;
		}

		// EaseInOutBack
		float easedT = EaseInOutBack(t);
		Math::Vector2 pos;
		// ========================================================
		// START → END
		// ========================================================
		if (moveforward_) {
			pos.x = startpos_.x + (endpos_.x - startpos_.x) * easedT;
			pos.y = startpos_.y + (endpos_.y - startpos_.y) * easedT;
		}
		// ========================================================
		// END → START
		// ========================================================
		else {
			pos.x = endpos_.x + (startpos_.x - endpos_.x) * easedT;
			pos.y = endpos_.y + (startpos_.y - endpos_.y) * easedT;
		}
		sprite_->SetPosition(pos);
	}
	// Spriteの更新
	sprite_->Update();
}

// =============================================================================
// Draw
// =============================================================================
void ItemDisplay::Draw() {
	sprite_->Draw();
}

// =============================================================================
// EaseInOutBack
// =============================================================================
float ItemDisplay::EaseInOutBack(float t) {
	const float c1 = 1.70158f;
	const float c2 = c1 * 1.525f;
	if (t < 0.5f) {
		return ((2.0f * t) * (2.0f * t) * ((c2 + 1.0f) * 2.0f * t - c2)) / 2.0f;
	} else {
		float x = 2.0f * t - 2.0f;
		return (x * x * ((c2 + 1.0f) * x + c2) + 2.0f) / 2.0f;
	}
}