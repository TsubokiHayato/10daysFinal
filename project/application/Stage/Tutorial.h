#pragma once

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

private:

	std::vector<TuboEngine::TextObject*> tutorialTexts_;

};

