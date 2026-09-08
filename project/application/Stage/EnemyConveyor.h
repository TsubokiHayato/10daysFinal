#pragma once
#include "Object3d.h"
#include "Vector3.h"
#include "Stage/Bullet.h"
#include "Stage/PartCatalog.h"
#include <memory>
#include <string>
#include <vector>

namespace TuboEngine { class Camera; }

// =============================================================================
//  EnemyConveyor ── 敵陣の「ベルトコンベア式」自動攻撃システム。
//
//  プレイヤーの砲弾づくりと同じ「胴＋頭を合成」する流れを敵も踏む:
//    ① ベルト始点のゲートから、胴パーツ・頭パーツが1つずつ出てくる。
//    ② パーツはベルトに乗って敵フィールド中央の大砲へ運ばれる。
//    ③ 大砲に胴と頭の両方がそろった時だけ、CombineStats() で砲弾を合成し発射。
//    ④ 砲弾は自陣大砲(中央)付近へ着弾し、自陣フィールドの床タイルを削る。
//
//  「無からパーツが湧く」のを避けるため、始点にはゲート(生成口)の見た目を置く。
//  砲弾は Bullet クラスを流用し、命中で自陣の床(=HP)へダメージを与える。
// =============================================================================
namespace game {

class Field;

class EnemyConveyor {
public:
	// 敵の攻撃の調整パラメータ（ImGuiで変更・JSONで保存/読込できる）。
	struct Params {
		bool enabled = true;          // 敵の攻撃 有効/無効
		float spawnInterval = 110.0f; // パーツ生成間隔(フレーム)
		float travelFrames = 170.0f;  // 始点→大砲の搬送フレーム数(小さいほど速い)
		float floorDamageMul = 4.0f;  // 合成威力→床ダメージ倍率
		float hitBaseRadius = 4.5f;   // 着弾ダメージの基本半径
		float hitBlastRadius = 1.4f;  // 弾のblast1あたりの追加半径
	};

	// camera     : 描画に使う主カメラ
	// enemyField : コンベアを敷く敵フィールド（広さ参照・借用）
	// cannonPos  : ベルト終端＝敵フィールド中央の大砲位置
	// targetPos  : 弾の着弾点＝自陣大砲(中央)の位置
	// playerField: 着弾ダメージを与える自陣フィールド（床＝HP・借用）
	void Initialize(TuboEngine::Camera* camera, Field* enemyField,
	                const TuboEngine::Math::Vector3& cannonPos,
	                const TuboEngine::Math::Vector3& targetPos, Field* playerField);
	void Update();
	void Draw();

	// ── 調整（ImGui / JSON） ─────────────────────────────
	Params& GetParams() { return params_; }
	const Params& GetParams() const { return params_; }
	// 既定の保存先パス。
	static const char* DefaultParamsPath() { return "Resources/Settings/EnemyAttack.json"; }
	bool SaveParams(const std::string& path) const; // JSONへ書き出し
	bool LoadParams(const std::string& path);        // JSONから読み込み
#ifdef USE_IMGUI
	void DrawImGui(); // 敵攻撃パラメータの編集パネル
#endif

	// ImGui 表示用。
	int PartCount() const { return static_cast<int>(parts_.size()); }
	int BulletCount() const { return static_cast<int>(bullets_.size()); }
	bool HasBody() const { return hasBody_; }
	bool HasHead() const { return hasHead_; }

	// 飛翔中の敵弾を集める（弾同士の空中相殺に使う。借用ポインタ）。
	std::vector<Bullet*> GetFlyingBullets();

private:
	// ベルト上を流れるパーツ1つ（胴 or 頭）。見た目は図鑑(PartDef)から作る。
	struct Part {
		std::unique_ptr<TuboEngine::Object3d> model;
		PartStats stats;
		Category category = Category::Body; // Body or Head
		float t = 0.0f; // ベルト上の進行度 0(始点)→1(終端=大砲)
		float spin = 0.0f;
	};

	void BuildBelt();  // ベルト＋ゲートの見た目を組む
	void SpawnPart();  // ゲートから胴 or 頭を1つ生成（胴→頭を交互に）
	void Deposit(Part& part); // 到達したパーツを大砲のスロットへ載せる（見た目を引き取る）
	void Fire();       // 胴と頭がそろったら合成して砲弾を1発発射

	TuboEngine::Camera* camera_ = nullptr;
	Field* enemyField_ = nullptr;  // 借用（広さ参照）
	Field* playerField_ = nullptr; // 借用（床にダメージ）

	TuboEngine::Math::Vector3 beltStart_{0.0f, 0.0f, 0.0f}; // ベルト始点(ゲート)
	TuboEngine::Math::Vector3 beltEnd_{0.0f, 0.0f, 0.0f};   // ベルト終端(=大砲)
	TuboEngine::Math::Vector3 target_{0.0f, 0.0f, 0.0f};    // 着弾点(=自陣大砲)

	std::vector<std::unique_ptr<TuboEngine::Object3d>> belt_; // ベルト＋ゲートの見た目
	std::vector<Part> parts_;                                 // 流れているパーツ
	std::vector<std::unique_ptr<Bullet>> bullets_;            // 発射済みの砲弾

	// 大砲のスロット（工作台と同じく胴・頭を1つずつ）。両方そろうと発射。
	bool hasBody_ = false;
	bool hasHead_ = false;
	PartStats bodyStats_;
	PartStats headStats_;
	std::unique_ptr<TuboEngine::Object3d> bodyMark_; // 大砲に載った胴の見た目
	std::unique_ptr<TuboEngine::Object3d> headMark_; // 大砲に載った頭の見た目

	Params params_;           // 調整パラメータ
	float spawnTimer_ = 0.0f; // 次のパーツ生成までのカウンタ
	bool nextIsBody_ = true;  // 次に出すのが胴か頭か（交互）
};

} // namespace game
