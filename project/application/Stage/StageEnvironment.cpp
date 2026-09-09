#include "StageEnvironment.h"
#include "StageLayout.h"
#include "Camera.h"
#include "LineManager.h"
#include <cmath>

using namespace TuboEngine;

namespace game {

using namespace game::layout;

TuboEngine::Object3d* StageEnvironment::AddProp(const std::string& model, const Math::Vector3& pos,
                                                const Math::Vector3& rot, const Math::Vector3& scale,
                                                const Math::Vector4& color) {
	auto obj = std::make_unique<Object3d>();
	obj->Initialize(model);
	obj->SetCamera(camera_);
	obj->SetPosition(pos);
	obj->SetRotation(rot);
	obj->SetScale(scale);
	obj->SetModelColor(color);
	Object3d* raw = obj.get();
	props_.push_back(std::move(obj));
	return raw;
}

void StageEnvironment::Initialize(TuboEngine::Camera* camera) {
	camera_ = camera;

	// ② フィールド（自陣=左/-X, 敵陣=右/+X）
	selfField_ = std::make_unique<Field>();
	selfField_->Initialize(camera_, {-kFieldOffsetX, 0.0f, 0.0f}, kSelfFloorA, kSelfFloorB);
	enemyField_ = std::make_unique<Field>();
	enemyField_->Initialize(camera_, {kFieldOffsetX, 0.0f, 0.0f}, kEnemyFloorA, kEnemyFloorB);

	// ③ 大砲：各フィールドの真ん中に、相手側を向けて置く（見た目のみ）。
	//    弾はこの中央大砲を狙って飛び、相手フィールド中央の床を削る。
	selfCannonPos_ = selfField_->GetCenter();   // {-kFieldOffsetX, 0, 0}
	enemyCannonPos_ = enemyField_->GetCenter();  // {+kFieldOffsetX, 0, 0}
	selfCannonProp_ = AddProp("artilleryBattery/artillery battery.obj", selfCannonPos_,
	                          {0.0f, -kHalfPi, 0.0f}, {2.6f, 2.6f, 2.6f}, {1.0f, 1.0f, 1.0f, 1.0f});
	enemyCannonProp_ = AddProp("artilleryBattery/artillery battery.obj", enemyCannonPos_,
	                           {0.0f, kHalfPi, 0.0f}, {2.6f, 2.6f, 2.6f}, {1.0f, 1.0f, 1.0f, 1.0f});

	// 砲口（弾の発射始点）は大砲プロップの少し上。BulletManager に渡す。
	muzzle_ = selfCannonPos_ + Math::Vector3{0.0f, 1.5f, 0.0f};
}

void StageEnvironment::Update() {
	selfField_->Update();
	enemyField_->Update();

	// 大砲：自陣/敵陣の床が崩壊し始めたら、その陣の大砲も一緒に落とす。
	if (selfField_->IsCollapsing()) {
		if (!selfCannonFall_.Started()) selfCannonFall_.Add(selfCannonProp_);
		selfCannonFall_.Start();
		selfCannonFall_.Update();
	} else {
		// 発射ポップ：cannonPulse_ を 0→1 へ進め、sin で「ポンッ」と膨らんで戻す。
		if (cannonPulse_ >= 0.0f) {
			cannonPulse_ += 1.0f / 60.0f / 0.22f; // 約0.22秒で1回のポップ
			if (cannonPulse_ >= 1.0f) {
				cannonPulse_ = -1.0f; // 終了→通常スケールへ
				selfCannonProp_->SetScale({kCannonBaseScale_, kCannonBaseScale_, kCannonBaseScale_});
			} else {
				// 縦に伸びて横が縮む squash&stretch 風。sin(π t) で 0→1→0。
				float s = std::sin(3.14159265f * cannonPulse_);
				float up = kCannonBaseScale_ * (1.0f + 0.35f * s);   // 縦は伸びる
				float side = kCannonBaseScale_ * (1.0f - 0.15f * s); // 横は少し縮む
				selfCannonProp_->SetScale({side, up, side});
			}
		}
		selfCannonProp_->Update();
	}
	if (enemyField_->IsCollapsing()) {
		if (!enemyCannonFall_.Started()) enemyCannonFall_.Add(enemyCannonProp_);
		enemyCannonFall_.Start();
		enemyCannonFall_.Update();
	} else {
		enemyCannonProp_->Update();
	}

	// 任意：ワールドグリッド（デバッグ表示）。
	if (showGrid_) {
		LineManager::GetInstance()->DrawGrid(
			(kFieldOffsetX + selfField_->GetHalfX()) * 2.0f, 24, {0.0f, 0.01f, 0.0f},
			{0.4f, 0.4f, 0.4f, 1.0f});
	}
}

// 発射時に呼ぶ：ポップ演出を最初から再生する。
void StageEnvironment::PulseSelfCannon() {
	cannonPulse_ = 0.0f;
}

void StageEnvironment::Draw() {
	selfField_->Draw();
	enemyField_->Draw();
	for (auto& p : props_) p->Draw();
}

} // namespace game
