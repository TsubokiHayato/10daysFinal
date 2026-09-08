#pragma once
#include "IScene.h"

#include "Stage/Player.h"
#include "Stage/StageEnvironment.h"
#include "Stage/ItemField.h"
#include "Stage/BulletManager.h"
#include "Stage/EnemyConveyor.h"
#include "Stage/StageCameraController.h"
#include "Stage/VisualManager.h"
#include "Stage/ItemDisplay.h"
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

	std::unique_ptr<game::StageCameraController> camera_;
	std::unique_ptr<game::StageEnvironment> environment_;
	std::unique_ptr<game::Player> player_;
	std::unique_ptr<game::ItemField> itemField_;
	std::unique_ptr<game::BulletManager> bulletManager_;
	std::unique_ptr<game::EnemyConveyor> enemyConveyor_;

	VisualManager* visualManager_ = nullptr;
	// アイテムの情報を表示するUIクラス。プレイヤーがアイテムを持つと現れる
	std::unique_ptr <game::ItemDisplay> itemdisplay_;
};
