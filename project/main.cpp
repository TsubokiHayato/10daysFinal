#include"engine/Framework/Framework.h"
#include"application/Order.h"
#include"D3DResourceLeakChecker.h"
#include <memory>

// TD4-2 エントリーポイント（共有エンジン EngineCore を使う最小ゲーム）
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#ifdef _DEBUG
	TuboEngine::D3DResourceLeakChecker leakChecker;
#endif
	std::unique_ptr<TuboEngine::Framework> framework = std::make_unique<TuboEngine::Order>();
	framework->Run();
	return 0;
}
