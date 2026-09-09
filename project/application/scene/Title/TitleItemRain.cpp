#include "TitleItemRain.h"

#include "Camera.h"
#include <cstdlib> // rand

using namespace TuboEngine;

namespace {

// [0,1) の擬似乱数と範囲版。
float Rand01() { return static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX) + 1.0f); }
float RandRange(float a, float b) { return a + (b - a) * Rand01(); }

// --- 降らせる範囲（タイトルの固定カメラに映る目安。実機で微調整可）---
constexpr float kRangeX   = 6.0f;   // X: [-kRangeX, +kRangeX]
constexpr float kRangeZmin = -2.0f; // Z: [kRangeZmin, kRangeZmax]
constexpr float kRangeZmax = 4.0f;
constexpr float kSpawnTop = 8.0f;   // 再投入する高さ（画面上端より上）
constexpr float kKillY    = -10.0f; // これより下へ落ちたら上へ戻す（画面下端より十分下まで落とす）

// ゲーム本編で使われている全オブジェクト。
//  ・model : Resources/Models/ からのパス（Object3d::Initialize に渡す）。
//  ・color : {1,1,1,1}=テクスチャそのまま。パーツ系は雰囲気に合わせて着色。
//  ・scale : 実寸がバラバラなので「降らせて見栄えする大きさ」に正規化（要微調整）。
struct TitleDropDef {
	const char* model;
	Math::Vector4 color;
	Math::Vector3 scale;
};
const TitleDropDef kTitleDrops[] = {
	{"block/block.obj",                        {1.0f, 1.0f, 1.0f, 1.0f}, {0.8f, 0.8f, 0.8f}},
	{"square/square.obj",                      {1.0f, 1.0f, 1.0f, 1.0f}, {0.8f, 0.8f, 0.8f}},
	{"cone/cone.obj",                          {1.0f, 1.0f, 1.0f, 1.0f}, {0.8f, 0.8f, 0.8f}},
	{"drill/drill.obj",                        {1.0f, 1.0f, 1.0f, 1.0f}, {0.8f, 0.8f, 0.8f}},
	{"star.obj",                               {1.0f, 1.0f, 0.3f, 1.0f}, {0.8f, 0.8f, 0.8f}},
	{"crown/crown.obj",                        {1.0f, 0.85f, 0.2f, 1.0f}, {0.9f, 0.9f, 0.9f}},
	{"playerBullet/playerBullet.obj",          {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
	{"player/Player.obj",                      {1.0f, 1.0f, 1.0f, 1.0f}, {0.5f, 0.5f, 0.5f}}, // 大きいので縮小
	{"artilleryBattery/artillery battery.obj", {1.0f, 1.0f, 1.0f, 1.0f}, {0.4f, 0.4f, 0.4f}}, // 巨大なので更に縮小
	{"wall/wall.obj",                          {1.0f, 1.0f, 1.0f, 1.0f}, {0.5f, 0.5f, 0.5f}}, // 平ら（板状）
	{"tile/tile.obj",                          {1.0f, 1.0f, 1.0f, 1.0f}, {0.5f, 0.5f, 0.5f}}, // 平ら（板状）
};
constexpr int kDropTypeCount = static_cast<int>(sizeof(kTitleDrops) / sizeof(kTitleDrops[0]));

} // namespace

void TitleItemRain::Initialize(TuboEngine::Camera* camera, int count) {
	camera_ = camera;
	drops_.clear();
	if (count < kDropTypeCount) count = kDropTypeCount; // 全種を最低1個は見せる

	drops_.reserve(static_cast<size_t>(count));
	for (int i = 0; i < count; ++i) {
		// モデルは種類を巡回して割り当て（先頭 kDropTypeCount 個で全種が出そろう）。
		// 各 Drop にモデルを固定し、以後は再Initializeしない（再ロードを避ける）。
		const TitleDropDef& def = kTitleDrops[i % kDropTypeCount];

		Drop d;
		d.obj = std::make_unique<Object3d>();
		d.obj->Initialize(def.model);
		d.obj->SetCamera(camera_);
		d.obj->SetModelColor(def.color);
		d.obj->SetScale(def.scale);

		Respawn(d, /*aboveTop=*/false); // 開幕は画面内のランダムな高さから降っている状態に
		drops_.push_back(std::move(d));
	}
}

void TitleItemRain::Respawn(Drop& d, bool aboveTop) {
	d.pos.x = RandRange(-kRangeX, kRangeX);
	d.pos.z = RandRange(kRangeZmin, kRangeZmax);
	d.pos.y = aboveTop ? RandRange(kSpawnTop, kSpawnTop + 4.0f)  // 上端の上から
	                   : RandRange(kKillY, kSpawnTop);            // 画面内のどこからでも

	d.rot = {RandRange(0.0f, 6.28318f), RandRange(0.0f, 6.28318f), RandRange(0.0f, 6.28318f)};
	d.spinVel = {RandRange(-1.5f, 1.5f), RandRange(-1.5f, 1.5f), RandRange(-1.5f, 1.5f)};
	d.fallSpeed = RandRange(1.5f, 3.5f);
}

void TitleItemRain::Update(float dt) {
	for (auto& d : drops_) {
		d.pos.y -= d.fallSpeed * dt; // 落とす
		d.rot += d.spinVel * dt;     // 回す
		if (d.pos.y < kKillY) Respawn(d, /*aboveTop=*/true); // 下端を割ったら上へ戻す（＝降り続ける）

		d.obj->SetPosition(d.pos);
		d.obj->SetRotation(d.rot);
		d.obj->SetCamera(camera_);
		d.obj->Update();
	}
}

void TitleItemRain::Draw() {
	for (auto& d : drops_) d.obj->Draw();
}
