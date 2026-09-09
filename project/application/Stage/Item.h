#pragma once
#include "Object3d.h"
#include "Vector3.h"
#include "PartCatalog.h"
#include <memory>
#include <string>

namespace TuboEngine { class Camera; }

// =============================================================================
//  Item ── フィールドに落ちているパーツ／合成結果の砲弾。
//
//  ・種類は Category（Body=胴体 / Head=頭 / Shell=砲弾）。
//  ・具体的な見た目とステータスは PartCatalog(PartDef) から受け取る（データ駆動）。
//    → パーツを増やす/性能を変えるのは PartCatalog 側の編集だけで済む。
//
//  状態(State):
//    ・Ground : 地面に落ちている（拾える／くるくる回って目立つ）
//    ・Held   : プレイヤーが手に持っている（頭上に追従）
//    ・Slot   : 工作台のスロットに載っている
// =============================================================================
namespace game {

class Item {
public:
	enum class State { Ground, Held, Slot };

	// 図鑑の定義(胴体/頭)からアイテムを作る。
	void InitializeFromDef(TuboEngine::Camera* camera, const PartDef& def,
	                       const TuboEngine::Math::Vector3& pos);
	// 合成結果の砲弾として作る（ステータスから見た目を決める）。
	void InitializeAsShell(TuboEngine::Camera* camera, const ShellStats& stats,
	                       const TuboEngine::Math::Vector3& pos);

	void Update();
	void Draw();

	Category GetCategory() const { return category_; }
	const std::string& GetName() const { return name_; }
	const PartStats& GetStats() const { return stats_; }

	State GetState() const { return state_; }
	void SetState(State s) { state_ = s; }

	const TuboEngine::Math::Vector3& GetPosition() const { return position_; }
	void SetPosition(const TuboEngine::Math::Vector3& p) { position_ = p; }

	bool IsActive() const { return active_; }
	void SetActive(bool a) { active_ = a; }

	// 見た目の Object3d（崩落演出などで外部から動かすとき用）。
	TuboEngine::Object3d* GetModel() const { return model_.get(); }

	void SetCamera(TuboEngine::Camera* camera);

private:
	// 共通の生成処理。
	void Build(TuboEngine::Camera* camera, const std::string& model,
	           const TuboEngine::Math::Vector4& color,
	           const TuboEngine::Math::Vector3& scale,
	           const TuboEngine::Math::Vector3& pos);

	std::unique_ptr<TuboEngine::Object3d> model_;
	Category category_ = Category::Body;
	std::string name_;
	PartStats stats_;
	State state_ = State::Ground;
	TuboEngine::Math::Vector3 position_{0.0f, 0.0f, 0.0f};
	bool active_ = true;
	float spin_ = 0.0f; // 地面にある間くるくる回す

	// 出現ポップ演出（スポーン/合成時に 0 から弾んで基準サイズへ）。
	TuboEngine::Math::Vector3 baseScale_{1.0f, 1.0f, 1.0f}; // 本来のスケール
	float spawnT_ = 0.0f;                                   // 0→1 の出現進捗（1で通常）
};

} // namespace game
