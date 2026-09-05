#pragma once
#include <cstdint>

// =============================================================================
//  StatusEffect ── 弾／城が持つ「状態異常」フラグ（ビットフラグ）。
//
//  ・弾がステータスの一部としてこのフラグを持ち、城に当たると城がフラグを取得して
//    自分の状態異常フラグに立てる（例：毒）。
//  ・状態異常を増やすときは、このenumに `1u << n` を1行足すだけ。
//    さらに PartCatalog のパーツ定義(PartDef.stats.status)にフラグを付ければ、
//    そのパーツから合成した弾が状態異常を持つようになる（コード追加不要）。
// =============================================================================
namespace game {

// 状態異常フラグ。複数同時に立てられるようビット割り当てにする。
enum StatusFlag : uint32_t {
	Status_None   = 0,
	Status_Poison = 1u << 0, // 毒：継続ダメージ
	// ── 追加例（必要になったら解禁）─────────────
	// Status_Burn  = 1u << 1, // 炎上
	// Status_Slow  = 1u << 2, // 鈍足
	// Status_Freeze = 1u << 3, // 凍結
};

// フラグ操作の小さなヘルパ（可読性のため）。
inline bool HasStatus(uint32_t flags, StatusFlag f) { return (flags & f) != 0u; }
inline uint32_t AddStatus(uint32_t flags, StatusFlag f) { return flags | f; }

} // namespace game
