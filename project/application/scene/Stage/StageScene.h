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
#include "Stage/Collapser.h" // 敗北時にプレイヤーを落とす
#include "Stage/Tutorial.h"
#include "stage/Option.h"
#include "FadeScreen.h" // 入場フェードイン／クリア時の退場フェードアウト
#include <memory>
#include <string>

namespace TuboEngine { class TextObject; } // クリアテキストの参照保持用（前方宣言）

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

	// 敗北演出：ビネットを徐々に濃くしながら崩落を見せ、タイトルへ戻す。
	//  ・自陣(プレイヤーの陣地)の床が崩壊し始めたら開始する。
	void StartLoseSequence();
	void UpdateLoseSequence(float dt);

	// 勝利(クリア)演出：敵陣が崩れきってから開始。
	//  ・カメラが自陣を一周しながらプレイヤーへズーム→停止→CLEAR表示→数秒後タイトルへ。
	void StartWinSequence();
	void UpdateWinSequence(float dt);
	void ShowClearText(); // 画面中央に CLEAR テキストを出す。

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

	// 敗北演出の状態。
	bool losing_ = false;      // 敗北演出中（開始したら二度と戻さない）
	float loseTimer_ = 0.0f;   // 敗北演出の経過(秒)。ビネット強度とタイトル遷移に使う。
	game::Collapser playerFall_; // 敗北時にプレイヤーを床と一緒に落下させる。

	// 勝利(クリア)演出の状態。
	bool won_ = false;        // クリア演出中（開始したら二度と戻さない）
	float winTimer_ = 0.0f;   // クリア演出の経過(秒)。カメラ軌道・テキスト・遷移に使う。
	TuboEngine::Math::Vector3 winTarget_{}; // カメラのズーム先（開始時のプレイヤー位置を控える）。
	bool clearTextShown_ = false;           // CLEAR テキストを出したか。
	TuboEngine::TextObject* clearText_ = nullptr; // CLEAR テキスト（登場アニメ用に保持）。
	bool winFadeStarted_ = false;           // クリア演出後のフェードアウトを開始したか。

	// 入場フェードイン／クリア時の退場フェードアウト。
	std::unique_ptr<FadeScreen> fadeScreen_;

	// 弾の打ち消し演出用パーティクル（ParticleManager 所有。名前で参照）。
	std::string clashFxName_;
	// 指定ワールド座標で打ち消しバーストを出す。
	void EmitClashBurst(const TuboEngine::Math::Vector3& pos);
	// アイテムの情報を表示するUIクラス。プレイヤーがアイテムを持つと現れる
	std::unique_ptr <game::ItemDisplay> itemdisplay_;
	std::unique_ptr<Tutorial> tutorial_;

	// オプションクラス
	std::unique_ptr<game::Option> option_;
};
