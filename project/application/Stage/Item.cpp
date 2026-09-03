#include "Item.h"

#include "Camera.h"
#include <cmath>

using namespace TuboEngine;

namespace game {

void Item::Build(TuboEngine::Camera* camera, const std::string& model, const Math::Vector4& color,
                 const Math::Vector3& scale, const Math::Vector3& pos) {
	position_ = pos;
	state_ = State::Ground;
	active_ = true;

	model_ = std::make_unique<Object3d>();
	model_->Initialize(model);
	model_->SetCamera(camera);
	model_->SetPosition(position_);
	model_->SetScale(scale);
	model_->SetModelColor(color);
}

void Item::InitializeFromDef(TuboEngine::Camera* camera, const PartDef& def,
                             const Math::Vector3& pos) {
	category_ = def.category;
	name_ = def.name;
	stats_ = def.stats;
	Build(camera, def.model, def.color, def.scale, pos);
}

void Item::InitializeAsShell(TuboEngine::Camera* camera, const ShellStats& stats,
                             const Math::Vector3& pos) {
	category_ = Category::Shell;
	name_ = "砲弾";
	stats_ = stats;

	// ステータスから見た目を作る：威力が高いほど大きく、爆発が大きいほど赤く。
	float sizet = 0.7f + stats.damage * 0.02f; // 威力→サイズ
	if (sizet > 2.0f) sizet = 2.0f;
	float redt = stats.blast / 5.0f; // 爆発→赤み(0〜1目安)
	if (redt > 1.0f) redt = 1.0f;
	Math::Vector4 color = {0.95f, 0.85f - redt * 0.45f, 0.30f - redt * 0.25f, 1.0f};

	Build(camera, "playerBullet/playerBullet.obj", color, {sizet, sizet, sizet}, pos);
}

void Item::SetCamera(TuboEngine::Camera* camera) {
	if (model_) model_->SetCamera(camera);
}

void Item::Update() {
	if (!active_) return;

	Math::Vector3 drawPos = position_;
	float yaw = 0.0f;

	if (state_ == State::Ground) {
		// 地面のアイテムは、くるくる回転＋上下にふわっと。目立たせて拾いやすく。
		spin_ += 0.03f;
		yaw = spin_;
		drawPos.y += 0.2f + 0.15f * std::sin(spin_ * 2.0f);
	}

	model_->SetPosition(drawPos);
	model_->SetRotation({0.0f, yaw, 0.0f});
	model_->Update();
}

void Item::Draw() {
	if (!active_) return;
	model_->Draw();
}

} // namespace game
