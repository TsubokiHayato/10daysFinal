#include "PartCatalog.h"

using namespace TuboEngine;

namespace game {

// ─── 胴体パーツ図鑑（弾丸の「後ろ」3種）─────────────────────────
//  胴体は「威力の土台」と「重さ(=弾速への影響)」を決める。
//  3種はそれぞれ役割がはっきり分かれる:
//    標準胴 : バランス型。クセがなく扱いやすい。
//    重装胴 : 高威力だが重く、弾速が大きく落ちる（頭でリカバーする前提）。
//    軽量胴 : 威力は低いが軽く、弾速を伸ばせる速攻型。
//  { category, 名前, モデル, 色, スケール, {damage, speed, blast, weight} }
const std::vector<PartDef>& BodyDefs() {
	static const std::vector<PartDef> defs = {
		// 標準胴：中庸。威力・重さともに基準値。
		{Category::Body, "標準胴", "block/block.obj",   {0.75f, 0.80f, 0.90f, 1.0f}, {0.80f, 0.80f, 0.80f}, {10.0f, 1.0f, 0.0f, 1.0f}},
		// 重装胴：威力大／重い＝弾速ダウン。ひと回り大きい見た目。
		{Category::Body, "重装胴", "block/block.obj",   {0.45f, 0.50f, 0.62f, 1.0f}, {1.05f, 1.05f, 1.05f}, {24.0f, 1.0f, 0.0f, 2.4f}},
		// 軽量胴：威力小／軽い＝弾速アップ。ひと回り小さい見た目。
		{Category::Body, "軽量胴", "square/square.obj", {0.85f, 0.90f, 1.00f, 1.0f}, {0.65f, 0.65f, 0.65f}, {5.0f,  1.0f, 0.0f, 0.5f}},
	};
	return defs;
}

// ─── 頭パーツ図鑑（弾丸の「先端」3種）───────────────────────────
//  頭は「弾速」と「爆発/貫通/状態異常の性質」を決める。
//  3種はそれぞれ性質が分かれる:
//    通常弾頭 : バランス型。クセなく扱いやすい標準の弾頭。
//    貫通弾頭 : 速くて単体火力が高い。爆発はほぼ無し。
//    炸裂弾頭 : 遅いが爆発範囲が広い。範囲攻撃向き。
const std::vector<PartDef>& HeadDefs() {
	static const std::vector<PartDef> defs = {
		// 通常弾頭：中庸。標準的な威力・弾速・小さめの爆発。コーン型。
		{Category::Head, "通常弾頭", "cone/cone.obj",   {1.00f, 0.68f, 0.32f, 1.0f}, {0.90f, 0.90f, 0.90f}, {5.0f,  1.0f, 1.0f, 0.0f}},
		// 貫通弾頭：高速・高火力・爆発ほぼ無し。ドリル型。
		{Category::Head, "貫通弾頭", "drill/drill.obj", {0.40f, 0.85f, 0.90f, 1.0f}, {0.90f, 0.90f, 0.90f}, {12.0f, 1.4f, 0.2f, 0.0f}},
		// 炸裂弾頭：低速だが爆発範囲が広い。スター型。
		{Category::Head, "炸裂弾頭", "star.obj",        {0.95f, 0.35f, 0.30f, 1.0f}, {0.80f, 0.80f, 0.80f}, {3.0f,  0.8f, 3.5f, 0.0f}},
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
