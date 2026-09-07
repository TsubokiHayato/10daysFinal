#pragma once
#include "Vector3.h"
#include "Vector4.h"
#include "StatusEffect.h"
#include <cstdint>
#include <string>
#include <vector>

// =============================================================================
//  PartCatalog ── パーツ「図鑑」とステータス定義。
//
//  ・アイテム(パーツ)を増やすときは BodyDefs()/HeadDefs() の配列に1行足すだけ。
//  ・各パーツは「砲弾への寄与(PartStats)」を持ち、胴体+頭を CombineStats() で
//    合成して砲弾のステータスを作る。
//
//  ステータスの意味:
//    damage : 威力（胴・頭の両方から加算）
//    speed  : 弾速（主に頭が決める。胴が重いほど下がる）
//    blast  : 爆発/誘爆の範囲（炸裂・榴弾系の頭で大きい）
//    weight : 重さ（主に胴。重いほど弾速ダウン、威力は高め）
//    status : 状態異常フラグ（StatusFlagのOR。毒などを弾に持たせる）
// =============================================================================
namespace game {

enum class Category { Body, Head, Shell };

struct PartStats {
	float damage = 0.0f;
	float speed = 1.0f;
	float blast = 0.0f;
	float weight = 1.0f;
	uint32_t status = Status_None; // 状態異常フラグ（StatusFlagのOR）
	int   count  = 1;    // 発射弾数（板野サーカス弾頭は複数）
	float swerve = 0.0f; // うねり量（0=まっすぐ / 大きいほど蛇行して的へ収束）
};
using ShellStats = PartStats;

// 1種類のパーツ定義。これを増やせばアイテムが増える。
struct PartDef {
	Category category;
	std::string name;  // 表示名
	std::string model; // モデルパス（Resources/Models/ から）
	TuboEngine::Math::Vector4 color;
	TuboEngine::Math::Vector3 scale;
	PartStats stats;
};

// パーツ図鑑（ここに追記するとアイテムが増える）。
const std::vector<PartDef>& BodyDefs();
const std::vector<PartDef>& HeadDefs();

// 胴体 + 頭 → 砲弾ステータス。
ShellStats CombineStats(const PartStats& body, const PartStats& head);

} // namespace game
