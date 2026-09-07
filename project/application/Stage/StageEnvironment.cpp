#include "StageEnvironment.h"
#include "StageLayout.h"
#include "Camera.h"
#include "LineManager.h"

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

	// ③ 城(基地)：各陣の外側の端(谷と反対側)に建てる。城が自分で見た目を組み立てる。
	const float hx = selfField_->GetHalfX();
	const Math::Vector3 selfCastleCenter = {-kFieldOffsetX - hx + 8.0f, 0.0f, 0.0f};
	const Math::Vector3 enemyCastleCenter = {kFieldOffsetX + hx - 8.0f, 0.0f, 0.0f};
	selfCastle_ = std::make_unique<Castle>();
	selfCastle_->Initialize(camera_, selfCastleCenter, kSelfColor, kEnemyCastleHP);
	enemyCastle_ = std::make_unique<Castle>();
	enemyCastle_->Initialize(camera_, enemyCastleCenter, kEnemyColor, kEnemyCastleHP);

	// ④ 砲台：各陣の内側の端(谷側)に、相手側を向けて置く（見た目のみ）。
	const Math::Vector3 selfCannonPos = {-kFieldOffsetX + hx - 6.0f, 0.0f, 0.0f};
	AddProp("artilleryBattery/artillery battery.obj", selfCannonPos, {0.0f, -kHalfPi, 0.0f},
	        {2.6f, 2.6f, 2.6f}, {1.0f, 1.0f, 1.0f, 1.0f});
	enemyCannon_ = AddProp("artilleryBattery/artillery battery.obj",
	                       {kFieldOffsetX - hx + 6.0f, 0.0f, 0.0f}, {0.0f, kHalfPi, 0.0f},
	                       {2.6f, 2.6f, 2.6f}, {1.0f, 1.0f, 1.0f, 1.0f});

	// 砲口（弾の発射始点）は砲台プロップの少し上。BulletManager に渡す。
	muzzle_ = selfCannonPos + Math::Vector3{0.0f, 1.5f, 0.0f};
}

void StageEnvironment::Update() {
	selfField_->Update();
	enemyField_->Update();
	for (auto& p : props_) p->Update();
	selfCastle_->Update();
	enemyCastle_->Update();

	// 任意：ワールドグリッド（デバッグ表示）。
	if (showGrid_) {
		LineManager::GetInstance()->DrawGrid(
			(kFieldOffsetX + selfField_->GetHalfX()) * 2.0f, 24, {0.0f, 0.01f, 0.0f},
			{0.4f, 0.4f, 0.4f, 1.0f});
	}
}

void StageEnvironment::Draw() {
	selfField_->Draw();
	enemyField_->Draw();
	for (auto& p : props_) p->Draw();
	selfCastle_->Draw();
	enemyCastle_->Draw();
}

} // namespace game
