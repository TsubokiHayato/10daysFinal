#include "ItemDisplay.h"
#include "Item.h"
#include "TextManager.h"
#include "TextObject.h"

using namespace TuboEngine;

namespace game {

	// =============================================================================
	//  Initialize
	// =============================================================================
	void ItemDisplay::Initialize() {
		easetime_ = 0.0f;                 // イージング経過時間
		easeduration_ = 0.2f;   	        // 移動時間
		// スプライトのイージング座標
		startpos_ = { -250.0f, 120.0f }; 	// 開始位置
		endpos_ = { 0.0f, 120.0f }; 	// 終了位置

		// ============================================================
		// テキストの移動座標
		// ============================================================
		// ItemType
		textTypeStartPos_ = { -30.0f, 150.0f };
		textTypeEndPos_ = { 10.0f, 150.0f };

		// ItemParameter
		textParameterStartPos_ = { -30.0f, 140.0f };
		textParameterEndPos_ = { 110.0f, 140.0f };

		// 表示するスプライトの生成、初期化
		sprite_ = std::make_unique<TuboEngine::Sprite>();
		sprite_->Initialize("drill.png");
		sprite_->SetPosition(startpos_);
		sprite_->SetSize({ 250.0f, 120.0f });

		TextManager::GetInstance()->GetOrCreateFontSized(TextManager::PresetFontNames::Best10, 32.0f);
		// アイテム種類			
		TextManager::GetInstance()->CreateTextWithName(
			"ItemType",
			TextManager::PresetFontNames::Best10 + "_32",
			"",
			textTypeStartPos_,
			{ 1.0f, 1.0f, 1.0f, 1.0f },
			1.5f
		);
		// アイテムパラメータ
		TextManager::GetInstance()->CreateTextWithName(
			"ItemParameter",
			TextManager::PresetFontNames::Best10 + "_32",
			"",
			textParameterStartPos_,
			{ 1.0f, 1.0f, 1.0f, 1.0f },
			0.5f
		);
	}

	// =============================================================================
	// Update
	// =============================================================================
	void ItemDisplay::Update(Item* item) {

		const bool hasItem = (item != nullptr);	

		// ============================================================
		// アイテム所持状態が変化したか
		// ============================================================
		if (hasItem != previousHasItem_) {
			previousHasItem_ = hasItem;
			// アイテムを持った
			// START → END
			if (hasItem) {
				moveforward_ = true;
				// 持ったアイテムの種類をテキストに反映 
				UpdateItemText(item);          //  アイテムの種類
				UpdateItemParameter(item);     //  アイテムのパラメータ
			}
			// アイテムを手放した
			// END → START
			else {
				moveforward_ = false;
				// テキストを消す 
				UpdateItemText(nullptr);       //  アイテムの種類
				UpdateItemParameter(nullptr);  //  アイテムのパラメータ
			}
			// イージング開始
			easetime_ = 0.0f;
			ismoving_ = true;
		}
		// 持っているアイテムそのものが変わった場合 
		else if (item != currentItem_ && hasItem) {
			UpdateItemText(item); 
		} 
		
		currentItem_ = item;

		// ============================================================
		// Spriteのイージング移動
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
			// START → END
			if (moveforward_) {
				pos.x = startpos_.x + (endpos_.x - startpos_.x) * easedT;
				pos.y = startpos_.y + (endpos_.y - startpos_.y) * easedT;
			}
			// END → START
			else {
				pos.x = endpos_.x + (startpos_.x - endpos_.x) * easedT;
				pos.y = endpos_.y + (startpos_.y - endpos_.y) * easedT;
			}
			sprite_->SetPosition(pos);

			// ============================================================
			// テキストのイージング移動
			// ============================================================
			auto* textManager = TextManager::GetInstance();
			// ------------------------------------------------------------
			// ItemType
			// ------------------------------------------------------------
			Math::Vector2 typePos;
			if (moveforward_) {
				// START → END
				typePos.x = textTypeStartPos_.x + (textTypeEndPos_.x - textTypeStartPos_.x) * easedT;
				typePos.y = textTypeStartPos_.y + (textTypeEndPos_.y - textTypeStartPos_.y) * easedT;
			} else {
				// END → START
				typePos.x = textTypeEndPos_.x + (textTypeStartPos_.x - textTypeEndPos_.x) * easedT;
				typePos.y = textTypeEndPos_.y + (textTypeStartPos_.y - textTypeEndPos_.y) * easedT;
			}
			// ------------------------------------------------------------
			// ItemParameter
			// ------------------------------------------------------------
			Math::Vector2 parameterPos;

			if (moveforward_) {
				// START → END
				parameterPos.x = textParameterStartPos_.x + (textParameterEndPos_.x - textParameterStartPos_.x) * easedT;
				parameterPos.y = textParameterStartPos_.y + (textParameterEndPos_.y - textParameterStartPos_.y) * easedT;
			} else {
				// END → START
				parameterPos.x = textParameterEndPos_.x + (textParameterStartPos_.x - textParameterEndPos_.x) * easedT;
				parameterPos.y = textParameterEndPos_.y + (textParameterStartPos_.y - textParameterEndPos_.y) * easedT;
			}

			TextObject* itemTypeText = textManager->GetTextByName("ItemType");
			TextObject* itemParameterText = textManager->GetTextByName("ItemParameter");
			// テキストの位置を更新
			if (itemTypeText) {
				itemTypeText->SetPosition(typePos);
			}
			if (itemParameterText) {
				itemParameterText->SetPosition(parameterPos);
			}
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

	std::string ItemDisplay::FormatFloat(float value) {
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(2) << value;
		return oss.str();
	}

	// =============================================================================
	// UpdateItemText(アイテムの種類を判定)
	// =============================================================================
	void ItemDisplay::UpdateItemText(Item* item) {
		auto* textManager = TextManager::GetInstance();
		const std::string textName = "ItemType";

		if (item == nullptr) {
			textManager->SetText(textName, "");
			return;
		}

		if (item->GetCategory() == Category::Body) {
			textManager->SetText(textName, "胴");
		} else if (item->GetCategory() == Category::Head) {
			textManager->SetText(textName, "頭");
		} else if (item->GetCategory() == Category::Shell) {
			textManager->SetText(textName, "砲弾");
		}
	}
	// =============================================================================	
	// UpdateItemText(アイテムのパラメータを変更)
	// =============================================================================
	void ItemDisplay::UpdateItemParameter(Item* item) {
		auto* textManager = TextManager::GetInstance();

		// 何も持っていない
		if (item == nullptr) {
			textManager->SetText("ItemParameter", "");
			return;
		}

		// アイテムのパラメータを取得
		const PartStats& stats = item->GetStats();
		float damage = stats.damage;
		float speed = stats.speed;
		float blast = stats.blast;
		float weight = stats.weight;
		uint32_t status = stats.status;

		std::string text;

		// 胴体
		if (item->GetCategory() == Category::Body) {
			text += "重量：" + FormatFloat(weight);
			text += "\n";
			text += "状態：" + std::to_string(status);		
		}
		// 頭
		else if (item->GetCategory() == Category::Head) {
			text += "ダメージ：" + FormatFloat(damage);
			text += "\n";
			text += "速度：" + FormatFloat(speed);
			text += "\n";
			text += "爆発：" + FormatFloat(blast);
			text += "\n";
			text += "重量：" + FormatFloat(weight);
			text += "\n";
			text += "状態：" + std::to_string(status);
		}
		// 砲弾
		else if (item->GetCategory() == Category::Shell) {
			text += "ダメージ：" + FormatFloat(damage);
			text += "\n";
			text += "速度：" + FormatFloat(speed);
			text += "\n";
			text += "爆発：" + FormatFloat(blast);
			text += "\n";
			text += "重量：" + FormatFloat(weight);
			text += "\n";
			text += "状態：" + std::to_string(status);
		}

		textManager->SetText("ItemParameter", text);
	}
}