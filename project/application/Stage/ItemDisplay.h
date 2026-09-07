#pragma once
#include <memory>
#include "Sprite.h"

// =============================================================================
//  ItemDisplay ── アイテムの情報を表示するクラス
//
//  プレイヤーがアイテムを持っている間そのアイテムの情報をUIで表示する。
//  イージング(EaseInOutBack)で持てる
//  プレイヤーがアイテムを離すと元の位置に戻る
//
// =============================================================================
namespace game {

	class Item;

	class ItemDisplay {
	public:
		// 初期化処理
		void Initialize();
		// 更新処理
		// 現在持っているアイテムを渡す 
		// nullptr = 何も持っていない 
		void Update(Item* item);
		// 描画
		void Draw();
	private:
		// EaseInOutBack
		float EaseInOutBack(float t);
		// アイテムの種類に応じてテキストを変更 
		void UpdateItemText(Item* item);
		// アイテムのパラメータを取得
		void UpdateItemParameter(Item* item);
		std::string FormatFloat(float value);
	private:
		// スプライト本体
		std::unique_ptr<TuboEngine::Sprite> sprite_;
		// 現在表示しているアイテム 
		Item* currentItem_ = nullptr;
		// 前回のアイテム所持状態
		bool previousHasItem_ = false;
		// Sprite移動中か
		bool ismoving_ = false;
		// true  : START → END
		// false : END → START
		bool moveforward_ = false;
		// イージング経過時間
		float easetime_;
		// 移動時間
		float easeduration_;
		// spriteの開始位置
		TuboEngine::Math::Vector2 startpos_;
		// spriteの終了位置
		TuboEngine::Math::Vector2 endpos_;

		// テキストの開始位置
		TuboEngine::Math::Vector2 textTypeStartPos_;
		TuboEngine::Math::Vector2 textParameterStartPos_;
		// テキストの終了位置
		TuboEngine::Math::Vector2 textTypeEndPos_;
		TuboEngine::Math::Vector2 textParameterEndPos_;
	};
}