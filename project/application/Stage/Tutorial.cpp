#include "Tutorial.h"

#include "TextManager.h"

using namespace TuboEngine;

void Tutorial::Initialize() {

	TextManager::GetInstance()->GetOrCreateFontSized(TextManager::PresetFontNames::YasashisaGothicBold, 32.0f);

	TextManager::GetInstance()->CreateTextWithName(
		"TutorialText01",
		TextManager::PresetFontNames::YasashisaGothicBold + "_32",
		"地面に置いてあるパーツを拾って工作台に設置しよう\n           Eキーでアイテムを持ち上げる・降ろす",
		{ 265.0f,575.0f },
		{ 1.0f,1.0f,1.0f,1.0f },
		1.0f
	);

	TextManager::GetInstance()->CreateTextWithName(
		"TutorialText02",
		TextManager::PresetFontNames::YasashisaGothicBold + "_32",
		" 胴体パーツと頭パーツを組み合わせて砲弾を作成しよう\nパーツを持っている状態で工作台にEキーでパーツを設置",
		{ 238.0f,575.0f },
		{ 1.0f,1.0f,1.0f,1.0f },
		1.0f
	);

	TextManager::GetInstance()->CreateTextWithName(
		"TutorialText03",
		TextManager::PresetFontNames::YasashisaGothicBold + "_32",
		"            砲弾を大砲に装填しよう\n砲弾を持っている状態で大砲にEキーで装填",
		{ 321.0f,575.0f },
		{ 1.0f,1.0f,1.0f,1.0f },
		1.0f
	);

	TextManager::GetInstance()->CreateTextWithName(
		"TutorialText04",
		TextManager::PresetFontNames::YasashisaGothicBold + "_32",
		"砲弾を発射して敵基地を攻撃しよう\n   Spaceキーを押して砲弾を発射",
		{ 412.0f,600.0f },
		{ 1.0f,1.0f,1.0f,1.0f },
		1.0f
	);

	tutorialTexts_.push_back(TextManager::GetInstance()->GetTextByName("TutorialText01"));
	tutorialTexts_.push_back(TextManager::GetInstance()->GetTextByName("TutorialText02"));
	tutorialTexts_.push_back(TextManager::GetInstance()->GetTextByName("TutorialText03"));
	tutorialTexts_.push_back(TextManager::GetInstance()->GetTextByName("TutorialText04"));
}

void Tutorial::Update(
	bool carriedFlag,
	bool createFlag,
	bool loadFlag,
	bool shotFlag
) {

	std::vector<bool> flags;
	int trueCount = 0;

	flags.push_back(carriedFlag);
	flags.push_back(createFlag);
	flags.push_back(loadFlag);
	flags.push_back(shotFlag);

	for (auto flag : flags) {
		if (flag) trueCount++;
	}

	for (int i = 0; i < tutorialTexts_.size(); i++) {

		if (trueCount == i) {

			tutorialTexts_[i]->SetColor({ 1.0f,1.0f,1.0f,1.0f });
		} else {

			tutorialTexts_[i]->SetColor({ 1.0f,1.0f,1.0f,0.0f });
		}
	}
}