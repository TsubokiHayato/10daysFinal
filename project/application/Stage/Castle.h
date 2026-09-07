#pragma once
#include "Object3d.h"
#include "Vector3.h"
#include "Vector4.h"
#include "StatusEffect.h"
#include <cstdint>
#include <memory>
#include <vector>

namespace TuboEngine { class Camera; }

namespace game {

class Bullet;

// =============================================================================
//  Castle ── 城（基地）のロジック。見た目のブロックは StageScene 側が持ち、
//            こちらは「HPと状態異常」だけを管理する軽量エンティティ。
//
//  被弾の流れ:
//    ・弾が城に当たると OnHit(bullet) が呼ばれる。
//    ・城は弾のダメージを受け、弾の状態異常フラグ(GetStatus)を取得して
//      自分の状態異常フラグに立てる（例：毒フラグを取得→毒状態になる）。
//    ・毒などの継続効果は Update() で毎フレーム処理する。
// =============================================================================
class Castle {
public:
	// camera : 見た目ブロックの描画カメラ
	// center : 城の中心位置（当たり判定・弾の的に使う）
	// color  : 陣営色（紋章と石材の色味に使う）
	// hp     : 初期HP
	void Initialize(TuboEngine::Camera* camera,
	                const TuboEngine::Math::Vector3& center,
	                const TuboEngine::Math::Vector4& color, float hp);
	void Update();
	// 天守・塔・紋章を描画する。
	void Draw();

	// 弾が命中したときに呼ぶ：ダメージ適用＋状態異常フラグの取得。
	void OnHit(const Bullet& bullet);

	const TuboEngine::Math::Vector3& GetPosition() const { return position_; }
	float GetHP() const { return hp_; }
	bool IsAlive() const { return hp_ > 0.0f; }

	// 現在立っている状態異常フラグ（StatusFlagのOR）。
	uint32_t GetStatus() const { return status_; }
	bool IsPoisoned() const { return HasStatus(status_, Status_Poison); }

private:
	// 見た目ブロックを1つ生成して blocks_ に積む。
	void AddBlock(const std::string& model,
	              const TuboEngine::Math::Vector3& pos,
	              const TuboEngine::Math::Vector3& scale,
	              const TuboEngine::Math::Vector4& color);

	TuboEngine::Math::Vector3 position_{0.0f, 0.0f, 0.0f};
	float hp_ = 100.0f;
	float maxHp_ = 100.0f;

	uint32_t status_ = Status_None; // 城が受けた状態異常フラグ
	float poisonTimer_ = 0.0f;       // 毒の残り時間（フレーム）

	// 天守・四隅の塔・屋根の紋章など見た目ブロック。
	TuboEngine::Camera* camera_ = nullptr;
	std::vector<std::unique_ptr<TuboEngine::Object3d>> blocks_;
};

} // namespace game
