#include "StageScene.h"
#include "GameScenes.h"
#include "SceneManager.h"

#include "Camera.h"
#include "Input.h"
#include "LineManager.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

using namespace TuboEngine;

namespace {
// 各陣フィールドの中心 |X|。自陣=-, 敵陣=+。中央に谷ができる距離にする。
constexpr float kFieldOffsetX = 30.0f;

// 陣営の床色(市松2色)。
constexpr Math::Vector4 kSelfFloorA = {0.30f, 0.42f, 0.66f, 1.0f};
constexpr Math::Vector4 kSelfFloorB = {0.22f, 0.32f, 0.54f, 1.0f};
constexpr Math::Vector4 kEnemyFloorA = {0.62f, 0.32f, 0.34f, 1.0f};
constexpr Math::Vector4 kEnemyFloorB = {0.50f, 0.24f, 0.26f, 1.0f};

// 城の紋章(旗代わり)色。
constexpr Math::Vector4 kSelfColor = {0.35f, 0.55f, 1.0f, 1.0f};
constexpr Math::Vector4 kEnemyColor = {1.0f, 0.35f, 0.32f, 1.0f};

constexpr float kHalfPi = 1.57079633f;

float Lerp(float a, float b, float t) { return a + (b - a) * t; }
} // namespace

// =============================================================================
//  Initialize
// =============================================================================
void StageScene::Initialize() {
	// ① 追従見下ろしカメラ（このシーンの主カメラ）
	followCamera_ = std::make_unique<game::FollowCamera>();
	followCamera_->Initialize();

	// F2 で乗っ取るデバッグカメラ
	debugCamera_ = std::make_unique<DebugCamera>();

	TuboEngine::Camera* cam = followCamera_->GetCamera();

	// ② フィールド（自陣=左/-X, 敵陣=右/+X）
	selfField_ = std::make_unique<game::Field>();
	selfField_->Initialize(cam, {-kFieldOffsetX, 0.0f, 0.0f}, kSelfFloorA, kSelfFloorB);
	enemyField_ = std::make_unique<game::Field>();
	enemyField_->Initialize(cam, {kFieldOffsetX, 0.0f, 0.0f}, kEnemyFloorA, kEnemyFloorB);

	// ③ 城(基地)：各陣の外側の端(谷と反対側)に建てる。
	const float hx = selfField_->GetHalfX();
	BuildCastle({-kFieldOffsetX - hx + 8.0f, 0.0f, 0.0f}, kSelfColor);  // 自陣の城(左端)
	BuildCastle({kFieldOffsetX + hx - 8.0f, 0.0f, 0.0f}, kEnemyColor);  // 敵陣の城(右端)

	// ④ 砲台：各陣の内側の端(谷側)に、相手側を向けて置く。※モデルのみ。
	selfCannon_ = AddProp("artilleryBattery/artillery battery.obj",
	                      {-kFieldOffsetX + hx - 6.0f, 0.0f, 0.0f}, {0.0f, -kHalfPi, 0.0f},
	                      {2.6f, 2.6f, 2.6f}, {1.0f, 1.0f, 1.0f, 1.0f});
	enemyCannon_ = AddProp("artilleryBattery/artillery battery.obj",
	                       {kFieldOffsetX - hx + 6.0f, 0.0f, 0.0f}, {0.0f, kHalfPi, 0.0f},
	                       {2.6f, 2.6f, 2.6f}, {1.0f, 1.0f, 1.0f, 1.0f});

	// ⑤ プレイヤー。自陣の中央あたりに配置し、自陣の範囲でクランプ。
	player_ = std::make_unique<game::Player>();
	player_->Initialize(cam);
	player_->SetPosition({-kFieldOffsetX, 1.0f, -selfField_->GetHalfZ() * 0.4f});
	player_->SetMoveBounds(selfField_->GetCenter(),
	                       selfField_->GetHalfX() - 1.5f, selfField_->GetHalfZ() - 1.5f);

	// カメラをプレイヤー位置へスナップ（開始時にワープして見えないように）
	followCamera_->SnapTo(player_->GetPosition());
}

// props_ に Object3d を1つ積む。
TuboEngine::Object3d* StageScene::AddProp(const std::string& model, const Math::Vector3& pos,
                                          const Math::Vector3& rot, const Math::Vector3& scale,
                                          const Math::Vector4& color) {
	auto obj = std::make_unique<Object3d>();
	obj->Initialize(model);
	obj->SetCamera(followCamera_->GetCamera());
	obj->SetPosition(pos);
	obj->SetRotation(rot);
	obj->SetScale(scale);
	obj->SetModelColor(color);
	Object3d* raw = obj.get();
	props_.push_back(std::move(obj));
	return raw;
}

// 城：天守(中央の大ブロック)+四隅の塔+屋根の紋章(crown)。
void StageScene::BuildCastle(const Math::Vector3& center, const Math::Vector4& color) {
	const float cx = center.x, cz = center.z;
	// 石材は白っぽく、陣営色をほんのり混ぜる。
	Math::Vector4 stone = {Lerp(0.85f, color.x, 0.3f), Lerp(0.85f, color.y, 0.3f),
	                       Lerp(0.85f, color.z, 0.3f), 1.0f};

	// 天守(中央) 6x10x6, 底面を地面(y=0)に合わせる。
	AddProp("block/block.obj", {cx, 5.0f, cz}, {0, 0, 0}, {3.0f, 5.0f, 3.0f}, stone);

	// 四隅の塔 2x14x2。
	const float t = 4.0f;
	AddProp("block/block.obj", {cx - t, 7.0f, cz - t}, {0, 0, 0}, {1.0f, 7.0f, 1.0f}, stone);
	AddProp("block/block.obj", {cx - t, 7.0f, cz + t}, {0, 0, 0}, {1.0f, 7.0f, 1.0f}, stone);
	AddProp("block/block.obj", {cx + t, 7.0f, cz - t}, {0, 0, 0}, {1.0f, 7.0f, 1.0f}, stone);
	AddProp("block/block.obj", {cx + t, 7.0f, cz + t}, {0, 0, 0}, {1.0f, 7.0f, 1.0f}, stone);

	// 屋根の紋章(旗代わり)。陣営色そのまま。
	AddProp("crown/crown.obj", {cx, 11.6f, cz}, {0, 0, 0}, {2.2f, 2.2f, 2.2f}, color);
}

// =============================================================================
//  Update
// =============================================================================
void StageScene::Update() {
	// (1) プレイヤー入力・移動
	player_->Update();

	// (2) 全体俯瞰(TAB長押し)へ滑らかに寄せる/戻す。
	const bool wantOverview = Input::GetInstance()->PushKey(DIK_TAB);
	overview_ += ((wantOverview ? 1.0f : 0.0f) - overview_) * 0.12f;

	// (3) カメラ：基本はプレイヤー追従。overview_ の分だけ戦場中央(原点)を見渡す。
	followCamera_->Update(player_->GetPosition(), {0.0f, 0.0f, 0.0f}, overview_);

	// (4) F2 デバッグカメラ（主カメラを乗っ取る）。追従更新の“後”に適用する。
	debugCamera_->Update(followCamera_->GetCamera());

	// (5) フィールド更新
	selfField_->Update();
	enemyField_->Update();

	// (6) 城・砲台
	for (auto& p : props_) p->Update();

	// (7) 任意：ワールドグリッド（デバッグ表示）
	if (showGrid_) {
		LineManager::GetInstance()->DrawGrid(
			(kFieldOffsetX + selfField_->GetHalfX()) * 2.0f, 24, {0.0f, 0.01f, 0.0f},
			{0.4f, 0.4f, 0.4f, 1.0f});
	}
}

// =============================================================================
//  描画フェーズ
// =============================================================================
void StageScene::Object3DDraw() {
	selfField_->Draw();
	enemyField_->Draw();
	for (auto& p : props_) p->Draw();
	player_->Draw();
}

void StageScene::SpriteDraw() {}

void StageScene::ParticleDraw() {}

// =============================================================================
//  ImGui（Debug ビルドのみ）
// =============================================================================
void StageScene::ImGuiDraw() {
#ifdef USE_IMGUI
	if (ImGuiManager::GetInstance()->BeginPanel("Stage")) {
		ImGui::TextWrapped("見下ろしステージ(自陣/敵陣)。WASDで移動 / TAB長押しで全体表示 / F2でデバッグカメラ。");
		ImGui::Separator();
		ImGui::Text("Overview : %.2f", overview_);
		ImGui::Checkbox("World Grid", &showGrid_);
		ImGui::Spacing();

		SceneManager* sm = SceneManager::GetInstance();
		if (ImGui::Button("Title へ")) sm->ChangeScene(TITLE);
		ImGui::SameLine();
		if (ImGui::Button("Sample へ")) sm->ChangeScene(SAMPLE);
		ImGui::SameLine();
		if (ImGui::Button("リロード")) sm->ChangeScene(STAGE);
	}
	ImGuiManager::GetInstance()->EndPanel();

	player_->DrawImGui();
	followCamera_->DrawImGui();
	debugCamera_->DrawImGui();
#endif
}

// =============================================================================
//  Finalize
// =============================================================================
void StageScene::Finalize() {}
