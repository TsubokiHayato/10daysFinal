#pragma once
#include "IScene.h"
#include "Camera.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>
class OnoderaScene : public IScene {
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
