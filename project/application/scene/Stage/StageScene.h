#pragma once
#include "IScene.h"

#include "DebugCamera.h"
#include "FollowCamera.h"
#include "Player.h"
#include "Field.h"
#include <memory>

#include "Object3d.h"
#include <vector>

// =============================================================================
//  StageScene ── ゲーム本編の見下ろしステージ。
//
//  フィールドは「自陣(左/-X)」「敵陣(右/+X)」の対称構造。中央は谷(no man's land)。
//
//  構成要素:
//    ・Field  ×2   : 自陣(青系)・敵陣(赤系)の床＋外周壁
//    ・Castle ×2   : 各陣の奥に建つ城(基地)。block+crown で構築、陣営色で色分け
//    ・Cannon ×2   : 砲台モデル。※挙動クラスは別担当が実装するため“モデルのみ”設置
//    ・Player       : WASD で自陣内を動くプレイヤー
//    ・FollowCamera : プレイヤーを追う見下ろしカメラ。TAB長押しで全体俯瞰へ
//    ・DebugCamera  : F2 で切り替わる確認用フリーカメラ
//
//  砲弾の生成・発射・当たり判定などのゲームロジックは、この土台に足していく想定。
// =============================================================================
class StageScene : public IScene {
public:
	void Initialize() override;
	void Update() override;
	void Finalize() override;
	void Object3DDraw() override;
	void SpriteDraw() override;
	void ImGuiDraw() override;
	void ParticleDraw() override;

	TuboEngine::Camera* GetMainCamera() const override {
		return followCamera_->GetCamera();
	}

private:
	// props_ に静的モデルを1つ生成して積む（城・砲台などフィールドの飾り）。
	TuboEngine::Object3d* AddProp(const std::string& model,
	                              const TuboEngine::Math::Vector3& pos,
	                              const TuboEngine::Math::Vector3& rot,
	                              const TuboEngine::Math::Vector3& scale,
	                              const TuboEngine::Math::Vector4& color);
	// 城(天守+四隅の塔+屋根の紋章)を center に組み立てる。color は陣営色。
	void BuildCastle(const TuboEngine::Math::Vector3& center,
	                 const TuboEngine::Math::Vector4& color);

private:
	std::unique_ptr<game::FollowCamera> followCamera_;
	std::unique_ptr<TuboEngine::DebugCamera> debugCamera_;

	std::unique_ptr<game::Field> selfField_;  // 自陣(左)
	std::unique_ptr<game::Field> enemyField_; // 敵陣(右)
	std::unique_ptr<game::Player> player_;

	// 城・砲台などの静的モデルをまとめて所有。
	std::vector<std::unique_ptr<TuboEngine::Object3d>> props_;
	TuboEngine::Object3d* selfCannon_ = nullptr;  // props_ が所有（挙動は別担当）
	TuboEngine::Object3d* enemyCannon_ = nullptr; // props_ が所有

	float overview_ = 0.0f; // 0=追従 / 1=全体俯瞰。TAB長押しで寄せる。
	bool showGrid_ = false; // 追加のワールドグリッド表示（デバッグ）
};
