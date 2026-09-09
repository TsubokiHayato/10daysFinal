#pragma once
#include "Object3d.h"
#include "Vector3.h"
#include "PartCatalog.h"
#include <memory>
#include "Sprite.h"
#include "TextObject.h"
#include "FadeScreen.h"

// =============================================================================
//  Option ── オブションクラス。
//
//  ・オプション機能のクラス
//  ・メニューはゲームへ戻ると、titleへ戻る2種類がある
//  ・ESCでオプションに移る
//  ・WSで上下に動かせれる
//  ・SPACE または ENTERで決定
// =============================================================================
namespace game {

	class Option {
	public:
		// 初期化
		void Initialize();
		// 更新
		void Update();
		// 描画
		void Draw();
		// オプションを開く
		void Open();
		// オプションを閉じる
		void Close();
		// 表示中か
		bool IsOpen() const {
			return isOpen_;
		}
		// タイトルへ決定されたか
		bool IsTitleSelected() const {
			return titleSelected_;
		}
	private:
		// 背景
		std::unique_ptr<TuboEngine::Sprite> background_;
		TuboEngine::TextObject* returnText_ = nullptr;
		TuboEngine::TextObject* titleText_ = nullptr;
		// 表示状態
		bool isOpen_ = false;
		// 選択項目
		// -1 = 何もない
		// 0  = ゲームへ戻る
		// 1  = タイトルへ
		int selectedIndex_;
		// タイトルへ決定したか
		bool titleSelected_;
		// 入場フェードイン＋退場フェードアウト
		std::unique_ptr<FadeScreen> fadeScreen_;
		int pendingScene_;
	};

} // namespace game