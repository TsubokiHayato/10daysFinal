#include "Castle.h"
#include "Bullet.h"
#include "StageLayout.h" // Lerpf
#include "Camera.h"

using namespace TuboEngine;

namespace game {

namespace {
// 毒の効果時間（フレーム）と1フレームあたりのダメージ。
constexpr float kPoisonDuration = 300.0f; // 約5秒(60fps)
constexpr float kPoisonDps = 0.05f;       // 継続ダメージ
} // namespace

void Castle::AddBlock(const std::string& model, const Math::Vector3& pos,
                      const Math::Vector3& scale, const Math::Vector4& color) {
	auto obj = std::make_unique<Object3d>();
	obj->Initialize(model);
	obj->SetCamera(camera_);
	obj->SetPosition(pos);
	obj->SetScale(scale);
	obj->SetModelColor(color);
	blocks_.push_back(std::move(obj));
}

void Castle::Initialize(TuboEngine::Camera* camera, const Math::Vector3& center,
                        const Math::Vector4& color, float hp) {
	camera_ = camera;
	position_ = center;
	hp_ = hp;
	maxHp_ = hp;
	status_ = Status_None;
	poisonTimer_ = 0.0f;

	// ── 見た目：天守(中央の大ブロック)+四隅の塔+屋根の紋章(crown) ──
	blocks_.clear();
	const float cx = center.x, cz = center.z;
	// 石材は白っぽく、陣営色をほんのり混ぜる。
	Math::Vector4 stone = {Lerpf(0.85f, color.x, 0.3f), Lerpf(0.85f, color.y, 0.3f),
	                       Lerpf(0.85f, color.z, 0.3f), 1.0f};

	// 天守(中央) 6x10x6、底面を地面(y=0)に合わせる。
	AddBlock("block/block.obj", {cx, 5.0f, cz}, {3.0f, 5.0f, 3.0f}, stone);

	// 四隅の塔 2x14x2。
	const float t = 4.0f;
	AddBlock("block/block.obj", {cx - t, 7.0f, cz - t}, {1.0f, 7.0f, 1.0f}, stone);
	AddBlock("block/block.obj", {cx - t, 7.0f, cz + t}, {1.0f, 7.0f, 1.0f}, stone);
	AddBlock("block/block.obj", {cx + t, 7.0f, cz - t}, {1.0f, 7.0f, 1.0f}, stone);
	AddBlock("block/block.obj", {cx + t, 7.0f, cz + t}, {1.0f, 7.0f, 1.0f}, stone);

	// 屋根の紋章(旗代わり)。陣営色そのまま。
	AddBlock("crown/crown.obj", {cx, 11.6f, cz}, {2.2f, 2.2f, 2.2f}, color);
}

void Castle::OnHit(const Bullet& bullet) {
	if (!IsAlive()) return;

	// ① 弾のダメージを受ける。
	hp_ -= bullet.GetStats().damage;
	if (hp_ < 0.0f) hp_ = 0.0f;

	// ② 弾の状態異常フラグを取得し、自分の状態異常フラグに立てる。
	//    （例：弾が毒フラグを持っていれば、城が毒状態になる）
	uint32_t incoming = bullet.GetStatus();
	status_ = AddStatus(status_, static_cast<StatusFlag>(incoming));

	// 毒を受けたら効果時間をリフレッシュ。
	if (HasStatus(incoming, Status_Poison)) {
		poisonTimer_ = kPoisonDuration;
	}
}

void Castle::Update() {
	// 毒：残り時間の間、継続ダメージ。切れたらフラグを下ろす。
	if (IsAlive() && HasStatus(status_, Status_Poison)) {
		hp_ -= kPoisonDps;
		if (hp_ < 0.0f) hp_ = 0.0f;
		poisonTimer_ -= 1.0f;
		if (poisonTimer_ <= 0.0f) {
			status_ &= ~static_cast<uint32_t>(Status_Poison);
		}
	}

	for (auto& b : blocks_) b->Update();
}

void Castle::Draw() {
	for (auto& b : blocks_) b->Draw();
}

} // namespace game
