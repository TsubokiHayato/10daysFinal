#include "Field.h"

#include "Camera.h"

using namespace TuboEngine;

namespace game {

// ※ Object3d.h がグローバルに class Camera; を前方宣言しているため、
//    ここでは曖昧さを避けるよう Camera は必ず TuboEngine:: を明示する。
void Field::Initialize(TuboEngine::Camera* camera) {
	// 既定色（原点中心の単一フィールド）。
	Initialize(camera, {0.0f, 0.0f, 0.0f},
	           {0.32f, 0.36f, 0.42f, 1.0f}, {0.24f, 0.28f, 0.34f, 1.0f});
}

void Field::Initialize(TuboEngine::Camera* camera, const Math::Vector3& center,
                       const Math::Vector4& floorA, const Math::Vector4& floorB) {
	center_ = center;

	// wall.obj のフラット床は XZ 平面で 2x2。スケール s なら実寸 2s、間隔も 2s。
	const float step = 2.0f * tileScale_;
	halfX_ = cols_ * step * 0.5f;
	halfZ_ = rows_ * step * 0.5f;

	// --- 床タイルを格子状に敷く（市松模様） ---
	floor_.clear();
	for (int z = 0; z < rows_; ++z) {
		for (int x = 0; x < cols_; ++x) {
			auto tile = std::make_unique<Object3d>();
			tile->Initialize("wall/wall.obj"); // 平らな床タイル(2x2, 薄い)
			tile->SetCamera(camera);

			// 格子の中心が center_ に来るよう配置
			float px = center_.x - halfX_ + step * 0.5f + x * step;
			float pz = center_.z - halfZ_ + step * 0.5f + z * step;
			tile->SetPosition({px, center_.y, pz});
			tile->SetScale({tileScale_, 1.0f, tileScale_});

			// 市松模様：見下ろしでマス目が読めるよう陣営2色を交互に
			tile->SetModelColor((((x + z) & 1) == 0) ? floorA : floorB);
			floor_.push_back(std::move(tile));
		}
	}

	// --- 外周の境界壁 ---
	//  tile.obj は XY 平面(2x2, Z=0)の板。立てて壁に使う。
	//  ・±Z の辺（X方向に走る壁）：板は既定で ±Z を向くのでそのまま。
	//  ・±X の辺（Z方向に走る壁）：Y軸に 90° 回して向きを変える。
	const float wallH = 1.5f;             // 壁の高さ(スケール)
	const float halfPi = 1.57079633f;     // 90°
	const Math::Vector4 wallColor{0.5f, 0.42f, 0.30f, 1.0f};
	const float cx = center_.x, cz = center_.z;

	// 奥(+Z)・手前(-Z) の壁
	AddWall(camera, {cx, wallH, cz + halfZ_}, {0.0f, 0.0f, 0.0f},
	        {halfX_, wallH, 1.0f}, wallColor);
	AddWall(camera, {cx, wallH, cz - halfZ_}, {0.0f, 0.0f, 0.0f},
	        {halfX_, wallH, 1.0f}, wallColor);
	// 右(+X)・左(-X) の壁
	AddWall(camera, {cx + halfX_, wallH, cz}, {0.0f, halfPi, 0.0f},
	        {halfZ_, wallH, 1.0f}, wallColor);
	AddWall(camera, {cx - halfX_, wallH, cz}, {0.0f, halfPi, 0.0f},
	        {halfZ_, wallH, 1.0f}, wallColor);
}

void Field::AddWall(TuboEngine::Camera* camera, const Math::Vector3& pos, const Math::Vector3& rot,
                    const Math::Vector3& scale, const Math::Vector4& color) {
	auto wall = std::make_unique<Object3d>();
	wall->Initialize("tile/tile.obj"); // 縦板
	wall->SetCamera(camera);
	wall->SetPosition(pos);
	wall->SetRotation(rot);
	wall->SetScale(scale);
	wall->SetModelColor(color);
	walls_.push_back(std::move(wall));
}

void Field::SetCamera(TuboEngine::Camera* camera) {
	for (auto& t : floor_) t->SetCamera(camera);
	for (auto& w : walls_) w->SetCamera(camera);
}

void Field::Update() {
	for (auto& t : floor_) t->Update();
	for (auto& w : walls_) w->Update();
}

void Field::Draw() {
	for (auto& t : floor_) t->Draw();
	for (auto& w : walls_) w->Draw();
}

} // namespace game
