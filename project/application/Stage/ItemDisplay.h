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
class ItemDisplay {
public:
	// 初期化処理
	void Initialize();
	// 更新処理
	// アイテムを持っているかどうかでUIの挙動が変わる
	void Update(bool hasItem);
	// 描画
	void Draw();
private:
	// EaseInOutBack
	float EaseInOutBack(float t);
private:
	// スプライト本体
	std::unique_ptr<TuboEngine::Sprite> sprite_;
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
	// 開始位置
	TuboEngine::Math::Vector2 startpos_;
	// 終了位置
	TuboEngine::Math::Vector2 endpos_;
};