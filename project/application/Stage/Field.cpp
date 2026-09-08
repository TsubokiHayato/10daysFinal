#include "Field.h"

#include "Camera.h"
#include "StageLayout.h"
#include <cmath>
#include <cstdlib>

using namespace TuboEngine;

namespace game {

namespace {
// [-1,1] の擬似乱数（崩壊アニメのばらつき用）。
float Rand11() { return (static_cast<float>(std::rand()) / RAND_MAX) * 2.0f - 1.0f; }
} // namespace

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
	//  各タイルはHPを持ち、被弾で色が赤へ寄る。並列配列に元色/元位置も控える。
	floor_.clear();
	tileBase_.clear();
	tileHp_.clear();
	tileHome_.clear();
	tileVel_.clear();
	tileSpin_.clear();
	tileMaxHp_ = layout::kTileMaxHP;
	for (int z = 0; z < rows_; ++z) {
		for (int x = 0; x < cols_; ++x) {
			auto tile = std::make_unique<Object3d>();
			tile->Initialize("wall/wall.obj"); // 平らな床タイル(2x2, 薄い)
			tile->SetCamera(camera);

			// 格子の中心が center_ に来るよう配置
			float px = center_.x - halfX_ + step * 0.5f + x * step;
			float pz = center_.z - halfZ_ + step * 0.5f + z * step;
			const Math::Vector3 home = {px, center_.y, pz};
			tile->SetPosition(home);
			tile->SetScale({tileScale_, 1.0f, tileScale_});

			// 市松模様：見下ろしでマス目が読めるよう陣営2色を交互に
			const Math::Vector4 base = (((x + z) & 1) == 0) ? floorA : floorB;
			tile->SetModelColor(base);
			floor_.push_back(std::move(tile));

			tileBase_.push_back(base);
			tileHp_.push_back(tileMaxHp_);
			tileHome_.push_back(home);
			tileVel_.push_back({0.0f, 0.0f, 0.0f});
			tileSpin_.push_back({0.0f, 0.0f, 0.0f});
		}
	}
	// フィールド総HP = 全タイルHPの合計。
	maxHp_ = tileMaxHp_ * static_cast<float>(floor_.size());
	hp_ = maxHp_;
	collapsing_ = false;
	collapsed_ = false;
	collapseTimer_ = 0.0f;

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

void Field::ApplyDamage(const Math::Vector3& worldPos, float damage, float radius) {
	if (collapsing_ || radius <= 0.0f) return;

	for (size_t i = 0; i < floor_.size(); ++i) {
		if (tileHp_[i] <= 0.0f) continue;
		// XZ距離で減衰（中心ほど大ダメージ）。
		float dx = tileHome_[i].x - worldPos.x;
		float dz = tileHome_[i].z - worldPos.z;
		float dist = std::sqrt(dx * dx + dz * dz);
		if (dist >= radius) continue;

		float falloff = 1.0f - dist / radius;
		float dmg = damage * falloff;
		float before = tileHp_[i];
		tileHp_[i] = before - dmg;
		if (tileHp_[i] < 0.0f) tileHp_[i] = 0.0f;
		hp_ -= (before - tileHp_[i]);
	}
	if (hp_ < 0.0f) hp_ = 0.0f;

	// 総HPが尽きたら崩壊開始：各タイルに落下速度と回転を割り当てる。
	if (hp_ <= 0.0f && !collapsing_) {
		collapsing_ = true;
		collapseTimer_ = 0.0f;
		for (size_t i = 0; i < floor_.size(); ++i) {
			tileVel_[i] = {Rand11() * 0.06f, -0.05f - (static_cast<float>(std::rand()) / RAND_MAX) * 0.15f,
			               Rand11() * 0.06f};
			tileSpin_[i] = {Rand11() * 0.08f, Rand11() * 0.05f, Rand11() * 0.08f};
		}
	}
}

void Field::Update() {
	if (collapsing_) {
		// 崩壊：各タイルを重力で落下＋回転させる。一定時間で崩壊完了とする。
		collapseTimer_ += 1.0f;
		for (size_t i = 0; i < floor_.size(); ++i) {
			tileVel_[i].y -= 0.012f; // 重力
			tileHome_[i] += tileVel_[i];
			Math::Vector3 rot = tileSpin_[i] * collapseTimer_;
			floor_[i]->SetPosition(tileHome_[i]);
			floor_[i]->SetRotation(rot);
			floor_[i]->SetModelColor(layout::kFloorDamaged);
			floor_[i]->Update();
		}
		for (auto& w : walls_) w->Update();
		if (collapseTimer_ >= 150.0f) collapsed_ = true;
		return;
	}

	// 通常時：残HP比率で色を 元色(青)→赤 へ補間する。
	for (size_t i = 0; i < floor_.size(); ++i) {
		float ratio = tileMaxHp_ > 0.0f ? tileHp_[i] / tileMaxHp_ : 0.0f; // 1=健康,0=瀕死
		const Math::Vector4& base = tileBase_[i];
		const Math::Vector4& dmg = layout::kFloorDamaged;
		Math::Vector4 c = {Lerpf(dmg.x, base.x, ratio), Lerpf(dmg.y, base.y, ratio),
		                   Lerpf(dmg.z, base.z, ratio), 1.0f};
		floor_[i]->SetModelColor(c);
		floor_[i]->Update();
	}
	for (auto& w : walls_) w->Update();
}

void Field::Draw() {
	for (auto& t : floor_) t->Draw();
	for (auto& w : walls_) w->Draw();
}

} // namespace game
