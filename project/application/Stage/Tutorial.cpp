#include "Tutorial.h"

#include "TextManager.h"

using namespace TuboEngine;

float EaseOut(float t) {
	return 1.0f - std::powf(1.0f - t, 4.0f);
}

float EaseOutBack(float t) {

	const float c1 = 2.0f;
	const float c3 = c1 + 1.0f;

	return 1.0f + c3 * std::powf(t - 1.0f, 3.0f) + c1 * std::powf(t - 1.0f, 2.0f);
}

void Tutorial::Initialize() {

	TextManager::GetInstance()->GetOrCreateFontSized(TextManager::PresetFontNames::YasashisaGothicBold, 32.0f);

	TextManager::GetInstance()->GetOrCreateFontSized(TextManager::PresetFontNames::YasashisaGothicBold, 128.0f);

	TextManager::GetInstance()->CreateTextWithName(
		"TutorialText01",
		TextManager::PresetFontNames::YasashisaGothicBold + "_32",
		"地面に置いてあるパーツを拾って工作台に設置しよう\n           Eキーでアイテムを持ち上げる・降ろす",
		{ 265.0f,575.0f },
		{ 1.0f,1.0f,1.0f,0.0f },
		1.0f
	);

	TextManager::GetInstance()->CreateTextWithName(
		"TutorialText02",
		TextManager::PresetFontNames::YasashisaGothicBold + "_32",
		" 胴体パーツと頭パーツを組み合わせて砲弾を作成しよう\nパーツを持っている状態で工作台にEキーでパーツを設置",
		{ 238.0f,575.0f },
		{ 1.0f,1.0f,1.0f,0.0f },
		1.0f
	);

	TextManager::GetInstance()->CreateTextWithName(
		"TutorialText03",
		TextManager::PresetFontNames::YasashisaGothicBold + "_32",
		"            砲弾を大砲に装填しよう\n砲弾を持っている状態で大砲にEキーで装填",
		{ 321.0f,575.0f },
		{ 1.0f,1.0f,1.0f,0.0f },
		1.0f
	);

	TextManager::GetInstance()->CreateTextWithName(
		"TutorialText04",
		TextManager::PresetFontNames::YasashisaGothicBold + "_32",
		"砲弾を発射して敵基地を攻撃しよう\n   Spaceキーを押して砲弾を発射",
		{ 378.0f,575.0f },
		{ 1.0f,1.0f,1.0f,0.0f },
		1.0f
	);

	TextManager::GetInstance()->CreateTextWithName(
		"CompleateText",
		TextManager::PresetFontNames::YasashisaGothicBold + "_128",
		"OK",
		{ 635.0f,645.0f },
		{ 1.0f,0.0f,0.0f,0.0f },
		1.0f
	);

	tutorialTexts_.push_back(TextManager::GetInstance()->GetTextByName("TutorialText01"));
	tutorialTexts_.push_back(TextManager::GetInstance()->GetTextByName("TutorialText02"));
	tutorialTexts_.push_back(TextManager::GetInstance()->GetTextByName("TutorialText03"));
	tutorialTexts_.push_back(TextManager::GetInstance()->GetTextByName("TutorialText04"));

	completeText_ = TextManager::GetInstance()->GetTextByName("CompleateText");

	completeText_->SetHorizontalAlign(1);
	completeText_->SetVerticalAlign(1);

	textWindowSprite_ = std::make_unique<TuboEngine::Sprite>();

	textWindowSprite_->Initialize("Hp.png");
	textWindowSprite_->SetColor({ 0.06f,0.06f,0.06f,1.0f });
	textWindowSprite_->SetPosition({ 200.0f,535.0f });
	textWindowSprite_->SetSize({ 880.0f,160.0f });
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

	if (trueCount != preCount_) {
		timer_ = 0.0f;
	}

	timer_ += 1.0f / 60.0f;

	timer_ = std::clamp(timer_, 0.0f, maxTime_ * 2.0f);

	for (int i = 0; i < tutorialTexts_.size(); i++) {

		if (trueCount == i) {

			tutorialTexts_[i]->SetColor({ 1.0f,1.0f,1.0f,1.0f });
		} else {

			tutorialTexts_[i]->SetColor({ 1.0f,1.0f,1.0f,0.0f });
		}

		if (trueCount == static_cast<int>(tutorialTexts_.size())) {

			tutorialTexts_[trueCount - 1]->SetColor({ 1.0f,1.0f,1.0f,1.0f * (1.0f - EaseOut(timer_ - maxTime_) / maxTime_) });

			textWindowSprite_->SetColor({ 0.06f,0.06f,0.06f,1.0f * (1.0f - EaseOut(timer_ - maxTime_) / maxTime_) });
		}
	}

	if (timer_ <= maxTime_) {

		completeText_->SetScale(1.25f * EaseOutBack(timer_ / maxTime_));

		completeText_->SetColor({ 1.0f,0.0f,0.0f,1.0f * EaseOut(timer_ / maxTime_) });
	} else {

		completeText_->SetColor({ 1.0f,0.0f,0.0f, 1.0f * (1.0f - EaseOut(timer_ - maxTime_) / maxTime_) });
	}

	textWindowSprite_->Update();

	preCount_ = trueCount;
}

void Tutorial::Draw() {
	textWindowSprite_->Draw();
}
