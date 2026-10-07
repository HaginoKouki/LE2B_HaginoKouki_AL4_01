#pragma once
/*====================================
 *
 * 衝突の絞り込みに使う属性とマスク。
 * Collider（形状を持つ側）と CollisionManager（束ねる側）の
 * どちらもが必要とする語彙だけを切り出したもの。
 * 他のどのヘッダにも依存しないため、循環参照の起点にならない.
 *
 * ====================================*/
#include <cstdint>

// 衝突の絞り込みに使う属性。描画順・更新順とは別軸で管理する.
// マスクとの論理積で判定するため、0 を割り当ててはいけない.
enum class CollisionLayer : uint32_t {
	Default = 1 << 0,
	Terrain = 1 << 1,
	Player = 1 << 2,
	Enemy = 1 << 3,
	PlayerAttack = 1 << 4,
	EnemyAttack = 1 << 5,
	Item = 1 << 6,
};

inline constexpr uint32_t ToMask(CollisionLayer layer) {
	return static_cast<uint32_t>(layer);
}
// マスクを組み立てるための演算子.
inline constexpr uint32_t operator|(CollisionLayer a, CollisionLayer b) {
	return ToMask(a) | ToMask(b);
}
inline constexpr uint32_t operator|(uint32_t a, CollisionLayer b) {
	return a | ToMask(b);
}

// すべての属性とぶつかるマスク.
inline constexpr uint32_t kCollisionMaskAll = 0xFFFFFFFF;
