#include "StageScene.h"
#include "GameScenes.h"
#include "SceneManager.h"

#include "Camera.h"
#include "Input.h"
#include "TextManager.h"
#include "OffScreenRendering.h" // 敗北演出のビネット
#include "Stage/StageLayout.h"
#include "Stage/Bullet.h"
#include <cstdio>
#include <vector>
#include <cmath>

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "ImGuiManager.h"
#endif

using namespace TuboEngine;

// =============================================================================
//  Initialize
// =============================================================================
void StageScene::Initialize() {
	// ① カメラ（このシーンの主カメラを内包）
	camera_ = std::make_unique<game::StageCameraController>();
	camera_->Initialize();
	TuboEngine::Camera* cam = camera_->GetCamera();

	// ② 地形・城・砲台プロップ
	environment_ = std::make_unique<game::StageEnvironment>();
	environment_->Initialize(cam);

	// ③ プレイヤー。自陣の中央あたりに配置し、自陣の範囲でクランプ。
	game::Field* selfField = environment_->GetSelfField();
	player_ = std::make_unique<game::Player>();
	player_->Initialize(cam);
	player_->SetPosition({-game::layout::kFieldOffsetX, 1.0f, -selfField->GetHalfZ() * 0.4f});
	player_->SetMoveBounds(selfField->GetCenter(),
	                       selfField->GetHalfX() - 1.5f, selfField->GetHalfZ() - 1.5f);

	// ④ アイテム・工作台（自陣に散布）
	itemField_ = std::make_unique<game::ItemField>();
	itemField_->Initialize(cam, selfField);

	// ⑤ 砲台・砲弾システム（的は敵陣中央の大砲。着弾で相手の床を削る）
	game::Field* enemyField = environment_->GetEnemyField();
	bulletManager_ = std::make_unique<game::BulletManager>();
	bulletManager_->Initialize(cam, environment_->GetMuzzle(), enemyField,
	                           environment_->GetEnemyCannonPos());

	// ⑥ 敵の攻撃：敵陣のベルトコンベアが球パーツを運び、大砲に届くと自陣を撃つ。
	enemyConveyor_ = std::make_unique<game::EnemyConveyor>();
	enemyConveyor_->Initialize(cam, enemyField, environment_->GetEnemyCannonPos(),
	                           environment_->GetSelfCannonPos(), selfField);

	// カメラをプレイヤー位置へスナップ（開始時にワープして見えないように）
	camera_->SnapTo(player_->GetPosition());

	TextManager::GetInstance()->LoadTextLayout("Resources/Text/StageUI.json");


	// HP UI(スプライト)の満タン幅を控える（以後は床HPの割合でこの幅を縮める）。
	if (Sprite* s = TextManager::GetInstance()->GetSpriteByName("PlayerHP")) {
		playerHpBaseW_ = s->GetSize().x;
	}
	if (Sprite* s = TextManager::GetInstance()->GetSpriteByName("EnemyHP")) {
		enemyHpBaseW_ = s->GetSize().x;
	}
	// アイテムのUI表示クラス
	itemdisplay_ = std::make_unique<game::ItemDisplay>();
	itemdisplay_->Initialize();

	tutorial_ = std::make_unique<Tutorial>();
	tutorial_->Initialize();

	visualManager_ = VisualManager::GetInstance();
	visualManager_->Initialize(cam);
	// オプションクラスの初期化
	option_ = std::make_unique<game::Option>();
	option_->Initialize();

	// 入場フェードイン（黒→ステージ）。クリア時はここから退場フェードアウトさせる。
	fadeScreen_ = std::make_unique<FadeScreen>();
	fadeScreen_->Initialize();
}

// 画面上のHP UI(スプライト)の幅を、各フィールドの床HP残量に合わせて縮める。
//  ・PlayerHP は自陣の床HP、EnemyHP は敵陣の床HPに連動。
//  ・アンカーが外側(左/右)に取ってあるので、幅を縮めると内側へ向かって減っていく。
void StageScene::UpdateHpUI() {
	TextManager* tm = TextManager::GetInstance();
	if (Sprite* s = tm->GetSpriteByName("PlayerHP")) {
		float ratio = environment_->GetSelfField()->GetHPRatio();
		s->SetSize({playerHpBaseW_ * ratio, s->GetSize().y});
	}
	if (Sprite* s = tm->GetSpriteByName("EnemyHP")) {
		float ratio = environment_->GetEnemyField()->GetHPRatio();
		s->SetSize({enemyHpBaseW_ * ratio, s->GetSize().y});
	}
}

// =============================================================================
//  Update
// =============================================================================
void StageScene::Update() {
	const float dt = 1.0f / 60.0f;

	// (0) ポーズ(オプション)は常に更新する。ESCで開閉し、開いている間はメニュー操作を受ける。
	option_->Update();
	// ポーズ中はゲーム進行を止める（閉じたらそのまま再開できる）。
	const bool paused = option_->IsOpen();

	// --- ワールドの進行（ポーズ中は丸ごと止める）---
	if (!paused) {
		// 敗北/勝利演出中は操作を受け付けない（崩落やカメラ演出だけを見せる）。
		if (!losing_ && !won_) {
			// (1) プレイヤー入力・移動
			player_->Update();

			// (2) E キー：砲台への装填(設置)を最優先。装填できなければアイテム操作(拾う/破棄/工作台へ)。
			if (!bulletManager_->TryLoad(player_.get())) {
				itemField_->HandleInteraction(player_.get());
			}

			// (3) 砲弾の発射・飛翔・命中
			bulletManager_->Update();

			// チュートリアル中(4項目すべて達成するまで)は敵に攻撃させない（生成/発射を止める）。
			//  ・4項目=拾う/合成/装填/発射。すべて達成してから敵が攻撃を始める。
			//  ※ Update 自体は毎フレーム呼ぶ必要がある（呼ばないとベルト等の Object3d が
			//    未初期化のまま Draw されて commandList が null になり落ちる）。
			const bool tutorialDone =
				itemField_->GetTutorialFlagCarried() && itemField_->GetTutorialFlagCreate() &&
				bulletManager_->GetTutorialFlagLoad() && bulletManager_->GetTutorialFlagShot();
			enemyConveyor_->SetAttackSuppressed(!tutorialDone);

			// (3.5) 敵の攻撃（ベルトコンベア→大砲→自陣の床）
			enemyConveyor_->Update();

			// (3.6) 空中での弾の相殺（有効時のみ）。無効なら相殺せず互いの床にダメージが入る。
			if (bulletCancelEnabled_) ResolveBulletClashes();
		}

		// アイテム（自陣崩落中は落下演出になる）。敗北演出中も進めて崩落を見せる。
		itemField_->Update();

		// (4) 地形・床(HP)・大砲プロップ・グリッド
		environment_->Update();

		// (5) カメラ：砲台に近づいたら自動ズームアウト（TAB長押しでも可）。
		//     さらに、どちらかの床HPが0になって崩壊し始めたら、強制的に全体を引く。
		const bool nearCannon =
			game::Dist2XZ(player_->GetPosition(), environment_->GetMuzzle()) <
			game::layout::kCannonZoomRange * game::layout::kCannonZoomRange;
		const bool anyCollapsing = environment_->GetSelfField()->IsCollapsing() ||
		                           environment_->GetEnemyField()->IsCollapsing();
		// 勝利演出中はカメラを自前で動かす（軌道＋ズーム）ので通常追従は止める。
		if (!won_) {
			camera_->Update(player_->GetPosition(), nearCannon || anyCollapsing);
		}

		tutorial_->Update(
			itemField_->GetTutorialFlagCarried(),
			itemField_->GetTutorialFlagCreate(),
			bulletManager_->GetTutorialFlagLoad(),
			bulletManager_->GetTutorialFlagShot()
		);

		visualManager_->Update();

		// (7) 勝敗判定・演出。
		//  ・勝利：敵陣が「崩れきってから」(IsCollapsed)開始。
		//  ・敗北：自陣の床が崩壊し始めたら(IsCollapsing)開始。
		if (!won_ && !losing_) {
			if (environment_->GetEnemyField()->IsCollapsed()) {
				StartWinSequence();
			} else if (environment_->GetSelfField()->IsCollapsing()) {
				StartLoseSequence();
			}
		}
		if (won_) {
			UpdateWinSequence(dt);
		} else if (losing_) {
			UpdateLoseSequence(dt);
		}
	}

	// --- 表示系は常に更新（ポーズメニューやUIが正しく描画されるように）---
	TuboEngine::TextManager::GetInstance()->UpdateAll();

	// フェード（入場フェードイン／クリア時の退場フェードアウト）を進める。
	fadeScreen_->Update();

	// (6) HP UI(スプライト)を床HPに合わせて更新（UpdateAllでジオメトリに反映される前に）
	UpdateHpUI();
	// (6) アイテム情報のUI表示
	itemdisplay_->Update(player_->GetCarried());
}

// =============================================================================
//  敗北演出
//   ・自陣の崩落を見せつつ、画面周辺をビネットで徐々に暗くする。
//   ・一定時間後（崩落が終わるころ）にタイトルへ戻す。
// =============================================================================
void StageScene::StartLoseSequence() {
	losing_ = true;
	loseTimer_ = 0.0f;
	// ビネットを有効化（現在のポストエフェクトを保存して Vignette へ切り替わる）。
	OffScreenRendering::GetInstance()->SetLowHpVignetteEnabled(true);
	// プレイヤーも床と一緒に崩落させる（以後 player_->Update() は止めて Collapser に任せる）。
	playerFall_.Add(player_->GetModel());
	playerFall_.Start();
}

void StageScene::UpdateLoseSequence(float dt) {
	loseTimer_ += dt;

	// プレイヤーを落下＋回転させる（床の崩落と一緒に落ちていく）。
	playerFall_.Update();

	// 演出の長さ（秒）。崩落アニメ(約2.5秒)が終わるころにタイトルへ。
	constexpr float kLoseDuration = 3.0f;
	// ビネット強度の始点/終点（0.8=通常, 大きいほど周辺が暗い）。
	constexpr float kVignetteStart = 0.8f;
	constexpr float kVignetteEnd = 14.0f;
	// ビネットの広がり(scale)。16=通常。0 まで下げると画面中心まで暗転し、完全に真っ黒になる。
	constexpr float kScaleStart = 16.0f;
	constexpr float kScaleEnd = 0.0f;

	// 経過に応じてビネットを濃くする。t^3 のイーズインで、終盤ほど急激に暗くする。
	float t = loseTimer_ / kLoseDuration;
	if (t > 1.0f) t = 1.0f;
	float eased = t * t * t; // イーズイン（最初ゆっくり→最後に一気に暗転）
	float power = kVignetteStart + (kVignetteEnd - kVignetteStart) * eased;
	float scale = kScaleStart + (kScaleEnd - kScaleStart) * eased;
	OffScreenRendering::GetInstance()->SetLowHpVignettePower(power);
	OffScreenRendering::GetInstance()->SetLowHpVignetteScale(scale); // 最後は完全に真っ黒へ

	// 演出が終わったらタイトルへ戻す。
	if (loseTimer_ >= kLoseDuration) {
		SceneManager::GetInstance()->ChangeScene(TITLE);
	}
}

// =============================================================================
//  勝利(クリア)演出
//   ・敵陣が崩れきってから開始（呼び出し側で IsCollapsed 判定）。
//   ・カメラが自陣を一周しながらプレイヤーへズーム→停止→CLEAR表示→数秒後タイトルへ。
// =============================================================================
void StageScene::StartWinSequence() {
	won_ = true;
	winTimer_ = 0.0f;
	clearTextShown_ = false;
	// ズーム先＝開始時点のプレイヤー位置を控える（以後プレイヤーは動かないので固定でよい）。
	winTarget_ = player_->GetPosition();
}

void StageScene::UpdateWinSequence(float dt) {
	winTimer_ += dt;

	// 各フェーズの長さ（秒）。
	constexpr float kOrbitDuration = 4.0f; // 一周しながらズームインする時間
	constexpr float kHoldDuration = 3.0f;  // 停止して CLEAR を見せる時間
	// カメラ軌道パラメータ（球面座標: pos=target+radius*(cosP sinY, sinP, cosP cosY), rot={P,Y+π,0}）。
	constexpr float kRadiusStart = 45.0f;  // 最初は引き（自陣全体が入る）
	constexpr float kRadiusEnd = 9.0f;     // 最後はプレイヤーに寄る
	constexpr float kPitchStart = 0.35f;   // 見下ろし角(開始)
	constexpr float kPitchEnd = 0.55f;     // 見下ろし角(終了・やや見下ろす)
	constexpr float kPi = 3.14159265f;
	constexpr float kTwoPi = 6.28318530f;

	TuboEngine::Camera* cam = camera_->GetCamera();
	const Math::Vector3 look = winTarget_ + Math::Vector3{0.0f, 1.0f, 0.0f}; // プレイヤーの少し上を見る

	// 必ず「プレイヤーの正面」でカメラが止まるよう終端角を決める。
	//  ・プレイヤーの visual forward は (-sin(yaw), 0, -cos(yaw))。
	//    球面座標のオフセット水平成分 (sinY, cosY) をこの前方向へ向ける → Y = yaw + π。
	//    そこにカメラを置き、プレイヤーを見る（rot={P,Y+π,0}）ので正面（顔）が見える。
	constexpr float kTotalSweep = kTwoPi * 1.5f;                    // 一周半
	const float endYaw = player_->GetYaw() + kPi;                  // 正面に立つ最終角
	const float startYaw = endYaw - kTotalSweep;                    // ここから回し始める

	if (winTimer_ < kOrbitDuration) {
		// --- フェーズ1：一周半しながらズームイン。必ず正面で終わる ---
		float t = winTimer_ / kOrbitDuration;
		float ez = 1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t); // ズームはイーズアウト（最後ゆっくり寄る）
		float yaw = startYaw + kTotalSweep * t;                 // 終端で endYaw（＝正面）に到達
		float radius = kRadiusStart + (kRadiusEnd - kRadiusStart) * ez;
		float pitch = kPitchStart + (kPitchEnd - kPitchStart) * ez;

		float cosP = std::cos(pitch), sinP = std::sin(pitch);
		Math::Vector3 offset = {radius * cosP * std::sin(yaw), radius * sinP, radius * cosP * std::cos(yaw)};
		cam->SetTranslate(look + offset);
		cam->setRotation({pitch, yaw + kPi, 0.0f});
		cam->setScale({1.0f, 1.0f, 1.0f});
		cam->Update();
		// カメラを動かした後にプレイヤーを再投影（その場に固定したまま画面内で正しく回る）。
		player_->UpdateVisualOnly();
	} else {
		// --- フェーズ2：停止して CLEAR を表示し、数秒後タイトルへ ---
		if (!clearTextShown_) {
			ShowClearText();
			clearTextShown_ = true;
		}
		// CLEAR テキストを弾むように登場させる（最初の 0.4 秒だけスケールアニメ）。
		if (clearText_) {
			float ht = (winTimer_ - kOrbitDuration) / 0.4f;
			if (ht > 1.0f) ht = 1.0f;
			// EaseOutBack（少し行き過ぎて戻る）でポップ感を出す。
			const float c1 = 1.70158f, c3 = c1 + 1.0f;
			float back = 1.0f + c3 * (ht - 1.0f) * (ht - 1.0f) * (ht - 1.0f) + c1 * (ht - 1.0f) * (ht - 1.0f);
			clearText_->SetScale(1.5f * back);
		}
		cam->Update(); // 最終姿勢のまま固定
		player_->UpdateVisualOnly(); // 固定カメラでもプレイヤーを再投影して描画を保つ
		if (winTimer_ >= kOrbitDuration + kHoldDuration) {
			// 退場フェードアウト（黒へ）を始め、真っ黒になったらタイトルへ。
			if (!winFadeStarted_) {
				fadeScreen_->FadeOut();
				winFadeStarted_ = true;
			}
			if (fadeScreen_->IsFadeOuting()) {
				SceneManager::GetInstance()->ChangeScene(TITLE);
			}
		}
	}
}

// 画面中央に大きく "CLEAR!!" を出す。
void StageScene::ShowClearText() {
	TextManager* tm = TextManager::GetInstance();
	tm->GetOrCreateFontSized(TextManager::PresetFontNames::Best10, 128.0f);
	tm->CreateTextWithName(
		"WinClear",
		TextManager::PresetFontNames::Best10 + "_128",
		"CLEAR!!",
		{640.0f, 300.0f},              // 画面中央（1280x720 の中央付近）
		{1.0f, 0.9f, 0.2f, 1.0f},      // 金色
		1.0f);
	clearText_ = tm->GetTextByName("WinClear");
	if (clearText_) {
		clearText_->SetHorizontalAlign(1); // 中央揃え
		clearText_->SetVerticalAlign(1);
		clearText_->SetScale(0.0f);        // 0 から弾んで出る
	}
}

// =============================================================================
//  弾同士の空中相殺
//   ・飛翔中のプレイヤー弾と敵弾が kBulletCancelRadius 内で接触したら、
//     強さに関係なく双方を消す（着地点を固定しているので同じ線上で交差する）。
// =============================================================================
void StageScene::ResolveBulletClashes() {
	std::vector<game::Bullet*> mine = bulletManager_->GetFlyingBullets();
	std::vector<game::Bullet*> foe = enemyConveyor_->GetFlyingBullets();
	if (mine.empty() || foe.empty()) return;

	const float r2 = game::layout::kBulletCancelRadius * game::layout::kBulletCancelRadius;
	bool anyCancel = false;

	for (game::Bullet* a : mine) {
		if (!a->IsActive()) continue;
		for (game::Bullet* b : foe) {
			if (!b->IsActive()) continue;

			// 3D距離で接触判定。近ければ双方消滅。
			const TuboEngine::Math::Vector3& pa = a->GetPosition();
			const TuboEngine::Math::Vector3& pb = b->GetPosition();
			float dx = pa.x - pb.x, dy = pa.y - pb.y, dz = pa.z - pb.z;
			if (dx * dx + dy * dy + dz * dz > r2) continue;

			a->Deactivate();
			b->Deactivate();
			anyCancel = true;
			break; // a は消えたので次の a へ
		}
	}

	// 相殺が起きたら軽く画面を揺らして手応えを出す。
	if (anyCancel) visualManager_->Shake(0.25f, 1.2f);
}

// =============================================================================
//  描画フェーズ
// =============================================================================
void StageScene::Object3DDraw() {
	environment_->Draw();
	bulletManager_->Draw();
	enemyConveyor_->Draw();
	itemField_->Draw();
	player_->Draw();
}

void StageScene::SpriteDraw() {
	itemdisplay_->Draw();

	tutorial_->Draw();

	TuboEngine::TextManager::GetInstance()->DrawAll();
	option_->Draw();
	fadeScreen_->Draw(); // 最前面（入場フェードイン／退場フェードアウト）
}

void StageScene::ParticleDraw() {}

// =============================================================================
//  ImGui（Debug ビルドのみ）
// =============================================================================
void StageScene::ImGuiDraw() {
#ifdef USE_IMGUI
	if (ImGuiManager::GetInstance()->BeginPanel("Stage")) {
		ImGui::TextWrapped("見下ろしステージ(自陣/敵陣)。WASD移動 / E=拾う(手ぶら)・大砲が近ければ装填・工作台へ載せる・その他は破棄(手持ち) / SPACE=発射 / 大砲に近づくと自動ズームアウト(TAB長押しでも可) / F2デバッグカメラ。");
		ImGui::Separator();

		// 大砲と床(HP)の状態。床＝HP。敵の攻撃はベルトコンベアから。
		ImGui::Text("大砲 : %s / 場の弾 : %d",
		            bulletManager_->HasPending() ? "装填済み(SPACEで発射)" : "空(Rで装填)",
		            bulletManager_->ActiveCount());
		game::Field* selfF = environment_->GetSelfField();
		game::Field* enemyF = environment_->GetEnemyField();

		// 床HP＝バーで表示（減りが視覚的に分かるように）。
		char hpBuf[64];
		// 自陣(青系バー)。
		ImGui::Text("自陣の床HP"); ImGui::SameLine();
		std::snprintf(hpBuf, sizeof(hpBuf), "%.0f / %.0f%s", selfF->GetHP(), selfF->GetMaxHP(),
		              selfF->IsCollapsing() ? " 崩壊!" : "");
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.30f, 0.55f, 1.0f, 1.0f));
		ImGui::ProgressBar(selfF->GetHPRatio(), ImVec2(-1.0f, 0.0f), hpBuf);
		ImGui::PopStyleColor();
		// 敵陣(赤系バー)。
		ImGui::Text("敵陣の床HP"); ImGui::SameLine();
		std::snprintf(hpBuf, sizeof(hpBuf), "%.0f / %.0f%s", enemyF->GetHP(), enemyF->GetMaxHP(),
		              enemyF->IsCollapsing() ? " 崩壊!" : "");
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.95f, 0.30f, 0.28f, 1.0f));
		ImGui::ProgressBar(enemyF->GetHPRatio(), ImVec2(-1.0f, 0.0f), hpBuf);
		ImGui::PopStyleColor();
		ImGui::Text("敵コンベア : 搬送中 %d / 大砲[胴%s 頭%s] / 敵弾 %d 発",
		            enemyConveyor_->PartCount(),
		            enemyConveyor_->HasBody() ? "○" : "×",
		            enemyConveyor_->HasHead() ? "○" : "×",
		            enemyConveyor_->BulletCount());

		// 弾の打ち消し(相殺)の切り替え。OFFなら互いの床にダメージを与え合える。
		ImGui::Checkbox("弾の打ち消し(相殺)を有効化", &bulletCancelEnabled_);
		ImGui::SameLine();
		ImGui::TextDisabled(bulletCancelEnabled_ ? "(空中で相殺・ダメージ入らず)" : "(相殺なし・ダメージあり)");
		ImGui::Separator();

		// アイテム/クラフトの状態表示。
		game::Item* carried = player_->GetCarried();
		ImGui::Text("手持ち : %s", carried ? carried->GetName().c_str() : "なし");
		game::Workbench* bench = itemField_->GetWorkbench();
		game::Item* body = bench->GetBodySlot();
		game::Item* head = bench->GetHeadSlot();
		ImGui::Text("工作台 : 胴[%s] 頭[%s]",
		            body ? body->GetName().c_str() : "空",
		            head ? head->GetName().c_str() : "空");
		ImGui::Text("地面のアイテム : %d 個 / 砲弾 : %d 個",
		            itemField_->CountGround(), itemField_->CountShells());

		// 手持ちのステータスを見せる。
		if (carried) {
			const game::PartStats& s = carried->GetStats();
			ImGui::Text("  威力%.0f 弾速%.2f 爆発%.1f 重%.1f", s.damage, s.speed, s.blast, s.weight);
		}
		ImGui::Separator();

		ImGui::Text("Overview : %.2f", camera_->GetOverview());
		ImGui::Checkbox("World Grid", environment_->ShowGridRef());
		ImGui::Spacing();

		SceneManager* sm = SceneManager::GetInstance();
		if (ImGui::Button("Title へ")) sm->ChangeScene(TITLE);
		ImGui::SameLine();
		if (ImGui::Button("Sample へ")) sm->ChangeScene(SAMPLE);
		ImGui::SameLine();
		if (ImGui::Button("リロード")) sm->ChangeScene(STAGE);

	}
	ImGuiManager::GetInstance()->EndPanel();

	TuboEngine::TextManager::GetInstance()->DrawImGui();
	player_->DrawImGui();
	camera_->DrawImGui();
	enemyConveyor_->DrawImGui();
#endif
}

// =============================================================================
//  Finalize
// =============================================================================
void StageScene::Finalize() {
	TuboEngine::TextManager::GetInstance()->ClearAllTexts();
	TuboEngine::TextManager::GetInstance()->ClearAllSprites();

	// 敗北時のビネットはあえて無効化しない。暗いままタイトルへ引き継ぎ、
	// タイトル側で一気に明るくする（リビール）演出につなげる。
}
