#include "SceneRegistration.h"

#include "GameScenes.h"
#include "Option/OptionScene.h"
#include "Sample/SampleScene.h"
#include "SceneManager.h"
#include "Stage/StageScene.h"
#include "Title/TitleScene.h"
#include "application/scene/Member/OnoderaScene.h"
#include "application/scene/Member/nagaiScene.h"

#include <memory>

void RegisterGameScenes() {
	SceneManager* sm = SceneManager::GetInstance();
	// シーン番号 → 生成関数 を登録。シーンを増やすときはここに足す。
	sm->RegisterScene(SAMPLE, [] { return std::make_unique<SampleScene>(); }, "Sample");
	sm->RegisterScene(TITLE, [] { return std::make_unique<TitleScene>(); }, "Title");
	sm->RegisterScene(OPTION, [] { return std::make_unique<OptionScene>(); }, "Option");
	sm->RegisterScene(STAGE, [] { return std::make_unique<StageScene>(); }, "Stage");
	sm->RegisterScene(NAGAI, [] { return std::make_unique<nagaiScene>(); }, "Nagai");
	sm->RegisterScene(ONODERA, [] { return std::make_unique<OnoderaScene>(); }, "Onodera");
}
