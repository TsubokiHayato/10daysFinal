#include "Order.h"

#include "SceneManager.h"
#include "SceneRegistration.h"
#include "GameScenes.h"
#include "settings/Settings.h"
#include "WinApp.h"  // タイトルバー文字列の設定
#include <Windows.h> // SetWindowTextW

void TuboEngine::Order::Initialize() {
	// エンジン基盤の初期化（シーンには触れない）
	TuboEngine::Framework::Initialize();

	// タイトルバーの文字列を設定する（ウィンドウ生成後に上書き）。
	SetWindowTextW(TuboEngine::WinApp::GetInstance()->GetHWND(), L"4026_融合！迫撃砲");

	// 保存済みの設定を読み込む（未保存なら既定値のまま）
	Settings::GetInstance()->Load();

	// ゲームのシーンを登録してから開始シーンを指定（登録は Initialize より前）
	RegisterGameScenes();
	SceneManager::GetInstance()->Initialize(TITLE);
}

void TuboEngine::Order::Update() {
	TuboEngine::Framework::Update();
}

void TuboEngine::Order::Finalize() {
	TuboEngine::Framework::Finalize();
}

void TuboEngine::Order::Draw() {
	TuboEngine::Framework::FrameWorkRenderTargetPreDraw();
	TuboEngine::Framework::Object3dCommonDraw();
	TuboEngine::Framework::SpriteCommonDraw();
	LineManager::GetInstance()->Draw();
	TuboEngine::Framework::ParticleCommonDraw();
	TuboEngine::Framework::FrameworkSwapChainPreDraw();
	TuboEngine::Framework::OffScreenRenderingDraw();
#ifdef USE_IMGUI
	Framework::ImguiPreDraw();
	Framework::ImguiPostDraw();
#endif
	Framework::FrameworkSwapChainPostDraw();
}
