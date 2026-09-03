#pragma once
#include "Camera.h"
#include "FadeScreen.h" // シーン遷移フェード
#include "IScene.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>
class nagaiScene : public IScene {
public:
	void Initialize() override;
	void Update() override;
	void Finalize() override;
	void Object3DDraw() override;
	void SpriteDraw() override;
	void ImGuiDraw() override;
	void ParticleDraw() override;
	TuboEngine::Camera* GetMainCamera() const override { return camera_.get(); }

private:

	
	std::unique_ptr<TuboEngine::Camera> camera_;
};