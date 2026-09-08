#pragma once
#include "IScene.h"

#include "Stage/Player.h"
#include "Stage/StageEnvironment.h"
#include "Stage/ItemField.h"
#include "Stage/BulletManager.h"
#include "Stage/EnemyConveyor.h"
#include "Stage/StageCameraController.h"
#include "Stage/VisualManager.h"
#include <memory>

// =============================================================================
//  StageScene ── ゲーム本編の見下ろしステージ（進行の統括役）。
//
//  実処理は各システムに委譲し、このクラスは所有と更新順序の調整だけを持つ:
//    ・StageCameraController : 追従/デバッグカメラ＋ズーム
//    ・StageEnvironment      : フィールド・城・砲台プロップ・グリッド
//    ・Player                : プレイヤー操作
//    ・ItemField             : 地面アイテム・工作台・拾う/破棄/合成
//    ・BulletManager         : 砲台・装填・発射・飛翔・命中
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
		return camera_->GetCamera();
	}

private:
	// 飛翔中のプレイヤー弾と敵弾が接触したら、強さに関係なく双方を打ち消す。
	void ResolveBulletClashes();

	// 画面上のHP UI(PlayerHP/EnemyHPスプライト)の幅を、床HPの残り割合に合わせて更新する。
	void UpdateHpUI();

	std::unique_ptr<game::StageCameraController> camera_;
	std::unique_ptr<game::StageEnvironment> environment_;
	std::unique_ptr<game::Player> player_;
	std::unique_ptr<game::ItemField> itemField_;
	std::unique_ptr<game::BulletManager> bulletManager_;
	std::unique_ptr<game::EnemyConveyor> enemyConveyor_;

	VisualManager* visualManager_ = nullptr;

	// 弾同士の空中相殺(打ち消し)を有効にするか。
	//  ・true（既定）: 自弾と敵弾が空中で接触したら双方消滅（＝打ち消し）。
	//                  すれ違わなかった弾はそのまま相手の床に着弾してダメージを与える。
	//  ・false         : 相殺しない（デバッグ用。着弾ダメージだけを確認したいとき）。
	//  デバッグビルドでは ImGui のチェックボックスから切り替えられる。
	bool bulletCancelEnabled_ = true;

	// HP UI(PlayerHP/EnemyHP スプライト)の満タン時の幅。Initialize で控える。
	float playerHpBaseW_ = 0.0f;
	float enemyHpBaseW_ = 0.0f;
};
