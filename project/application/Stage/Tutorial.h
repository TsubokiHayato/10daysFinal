#pragma once

#include "Sprite.h"

#include <vector>
#include <memory>

namespace TuboEngine {
	class TextObject;
}

class Tutorial {

public:

	void Initialize();

	void Update(
		bool carriedFlag,
		bool createFlag,
		bool loadFlag,
		bool shotFlag
	);

	void Draw();

private:

	std::vector<TuboEngine::TextObject*> tutorialTexts_;

	TuboEngine::TextObject* completeText_;

	std::unique_ptr<TuboEngine::Sprite> textWindowSprite_;

	int preCount_ = 0;

	float maxTime_ = 0.75f;

	float timer_ = maxTime_ * 2.0f;

};

