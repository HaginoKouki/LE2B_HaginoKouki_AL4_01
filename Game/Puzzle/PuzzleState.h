#pragma once
/*====================================
 *
 * 回転ブロックパズルの盤面の状態。ルールの計算はすべてこの値に対して行う.
 *
 * 【GameObject と分ける理由】
 * 自機が「どのマスで止まるか」はステージ設計の前提なので、毎回同じ結果になる必要がある.
 * float の位置と当たり判定の押し出しで動かすと、deltaTime や誤差で止まる位置がずれ、
 * T字のくぼみへ「ぴったり差し込む」判定も不安定になる.
 * ルールは整数のマス座標で計算し、GameObject は結果を補間して見せるだけにする.
 *
 * 【ステージのデータは回さない】
 * ステージを回しても、壁・ブロック・自機の盤面上の並びは変わらない.
 * 変わるのは「盤面から見てどちらが下か」だけなので、回転は gravity の1変数で表す.
 * 見た目の回転は、ステージの親 GameObject を回して表現する.
 *
 * 【コピーしてよい】
 * 値型なので、開始時の状態を取っておけば、やり直しはコピー1回で済む.
 *
 * ====================================*/
#include <cstdint>
#include <vector>

#include "System/Foundation/Grid/Grid.h"

namespace Puzzle {

// 動かない地形.
enum class Terrain : uint8_t {
	Empty,
	Wall, // 盤面の外も壁として扱う.
	Goal, // 自機が入れるマス。すべて埋めて、どれかに■が入ればクリア.
};

// 自機を構成する1マス。座標は盤面の絶対座標.
struct PlayerCell {
	Cake::GridCoord coord;
	bool isCore = false; // ■（本体）なら true、□（くっついたブロック）なら false.

	bool operator==(const PlayerCell&) const = default;
};

// ステージに置かれた、触れると自機にくっつく図形.
struct ShapeBlock {
	int32_t id = 0; // View との対応付けに使う。ステージ内で重複させない.
	std::vector<Cake::GridCoord> cells;

	bool operator==(const ShapeBlock&) const = default;
};

struct PuzzleState {
	Cake::Grid<Terrain> terrain;
	std::vector<PlayerCell> player;
	std::vector<ShapeBlock> blocks;

	// 盤面から見た「下」。ステージを回すたびに変わる.
	Cake::Direction gravity = Cake::Direction::Down;

	int32_t rotationCount = 0;
	int32_t rotationLimit = 0;

	bool isCleared = false;
	bool isMissed = false;
};

} // namespace Puzzle
