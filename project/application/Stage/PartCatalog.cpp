#include "PartCatalog.h"

using namespace TuboEngine;

namespace game {

// ─── 胴体パーツ図鑑 ─────────────────────────────────────────────
//  胴体は「威力の土台」と「重さ(=弾速への影響)」を決める。
//  { category, 名前, モデル, 色, スケール, {damage, speed, blast, weight} }
const std::vector<PartDef>& BodyDefs() {
	static const std::vector<PartDef> defs = {
		{Category::Body, "標準胴", "block/block.obj",   {0.75f, 0.80f, 0.90f, 1.0f}, {0.80f, 0.80f, 0.80f}, {10.0f, 1.0f, 0.0f, 1.0f}},
		{Category::Body, "重装胴", "block/block.obj",   {0.45f, 0.50f, 0.62f, 1.0f}, {0.95f, 0.95f, 0.95f}, {20.0f, 1.0f, 0.0f, 2.2f}},
		{Category::Body, "軽量胴", "square/square.obj", {0.85f, 0.90f, 1.00f, 1.0f}, {0.90f, 0.90f, 0.90f}, {6.0f, 1.0f, 0.0f, 0.5f}},
		{Category::Body, "爆薬胴", "sphere/sphere.obj", {0.90f, 0.55f, 0.45f, 1.0f}, {0.80f, 0.80f, 0.80f}, {8.0f, 1.0f, 1.5f, 1.2f}},
	};
	return defs;
}

// ─── 頭パーツ図鑑 ───────────────────────────────────────────────
//  頭は「弾速」と「爆発/貫通の性質」を決める。
const std::vector<PartDef>& HeadDefs() {
	static const std::vector<PartDef> defs = {
		{Category::Head, "通常弾頭", "cone/cone.obj",   {1.00f, 0.68f, 0.32f, 1.0f}, {0.90f, 0.90f, 0.90f}, {5.0f, 1.0f, 1.0f, 0.0f}},
		{Category::Head, "炸裂弾頭", "star.obj",        {0.95f, 0.35f, 0.30f, 1.0f}, {0.80f, 0.80f, 0.80f}, {3.0f, 0.8f, 3.0f, 0.0f}},
		{Category::Head, "貫通弾頭", "drill/drill.obj", {0.40f, 0.85f, 0.90f, 1.0f}, {0.90f, 0.90f, 0.90f}, {12.0f, 1.3f, 0.2f, 0.0f}},
		{Category::Head, "榴弾頭", "tip/tip.obj",       {0.70f, 0.50f, 0.90f, 1.0f}, {0.90f, 0.90f, 0.90f}, {6.0f, 0.9f, 2.0f, 0.0f}},
		// 状態異常パーツの追加例：statusにフラグを足すだけで「毒弾」になる（コード追加不要）。
		{Category::Head, "毒弾頭", "cone/cone.obj",     {0.45f, 0.85f, 0.35f, 1.0f}, {0.90f, 0.90f, 0.90f}, {3.0f, 1.0f, 0.5f, 0.0f, Status_Poison}},
	};
	return defs;
}

// 胴体 + 頭 → 砲弾ステータス。
//  ・威力/爆発は両者を加算。
//  ・弾速は頭の速度を基準に、胴が重いほど落ちる。
//  ・重さは胴のものを引き継ぐ。
ShellStats CombineStats(const PartStats& body, const PartStats& head) {
	ShellStats s;
	s.damage = body.damage + head.damage;
	s.blast = body.blast + head.blast;
	s.weight = body.weight;
	s.speed = head.speed - (body.weight - 1.0f) * 0.35f;
	if (s.speed < 0.3f) s.speed = 0.3f;
	// 状態異常は胴・頭のフラグを両方引き継ぐ（毒胴＋炸裂頭＝毒＋爆発、など）。
	s.status = body.status | head.status;
	return s;
}

} // namespace game
