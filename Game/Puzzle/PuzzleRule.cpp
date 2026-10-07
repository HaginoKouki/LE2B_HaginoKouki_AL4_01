#include "PuzzleRule.h"

#include <algorithm>

namespace Puzzle {

namespace {
// 上下左右の4方向。「触れている」の判定に使う.
constexpr Cake::GridCoord kNeighborOffsets[] = {{0, 1}, {0, -1}, {-1, 0}, {1, 0}};
} // namespace

StepResult PuzzleRule::Step(PuzzleState& state) {
	StepResult result;
	if (state.isCleared || state.isMissed) {
		return result;
	}

	// 開始直後や回転直後に、すでに触れているブロックもここで拾う.
	AttachTouchingBlocks(state, result.pickedBlockIds);

	const Cake::GridCoord down = Cake::ToOffset(state.gravity);
	const Cake::GridCoord forward = Cake::ToOffset(GetForward(state));

	// (仮) 落ちられるなら落下を優先し、落ちられないときだけ前へ進む.
	// (仮) 落下も前進も1歩1マスで、同じ速さ.
	if (CanMove(state, down)) {
		Move(state, down);
		result.moved = down;
		result.isFalling = true;
	} else if (CanMove(state, forward)) {
		Move(state, forward);
		result.moved = forward;
	}

	AttachTouchingBlocks(state, result.pickedBlockIds);

	if (IsGoalFilled(state)) {
		state.isCleared = true;
		result.isCleared = true;
		return result;
	}

	// 回転を使い切った状態で動けなくなったらミス.
	// ブロックを拾うとマスが増えるだけなので、動けなかった歩のあとで動けるようになることはない.
	// 動く壁を入れたら、壁に押されて動ける場合があるので、この判定を見直すこと.
	const bool isStopped = (result.moved == Cake::GridCoord{});
	if (isStopped && state.rotationCount >= state.rotationLimit) {
		state.isMissed = true;
		result.isMissed = true;
	}
	return result;
}

bool PuzzleRule::Rotate(PuzzleState& state) {
	if (state.isCleared || state.isMissed) {
		return false;
	}
	// 上限を超えて回そうとしたらミス.
	if (state.rotationCount >= state.rotationLimit) {
		state.isMissed = true;
		return false;
	}
	++state.rotationCount;
	// ステージを画面上で時計回りに回すと、盤面から見た「下」は反時計回りに回る.
	// 例: 1回回すと、盤面の右の辺が床になる.
	state.gravity = Cake::RotateCounterClockwise(state.gravity);
	return true;
}

Cake::Direction PuzzleRule::GetForward(const PuzzleState& state) {
	// 画面上の「右」は、画面上の「下」を反時計回りに90°回した向き.
	// 盤面ごと回っても、この関係は変わらない.
	return Cake::RotateCounterClockwise(state.gravity);
}

bool PuzzleRule::CanMove(const PuzzleState& state, const Cake::GridCoord& offset) {
	for (const PlayerCell& cell : state.player) {
		const Cake::GridCoord target = cell.coord + offset;
		// 盤面の外は壁.
		if (state.terrain.GetOr(target, Terrain::Wall) == Terrain::Wall) {
			return false;
		}
		for (const ShapeBlock& block : state.blocks) {
			if (std::ranges::find(block.cells, target) != block.cells.end()) {
				return false;
			}
		}
	}
	return true;
}

bool PuzzleRule::IsGoalFilled(const PuzzleState& state) {
	bool hasGoal = false;
	for (int32_t y = 0; y < state.terrain.GetHeight(); ++y) {
		for (int32_t x = 0; x < state.terrain.GetWidth(); ++x) {
			const Cake::GridCoord coord{x, y};
			if (state.terrain.At(coord) != Terrain::Goal) {
				continue;
			}
			hasGoal = true;
			if (!IsPlayerAt(state, coord)) {
				return false;
			}
		}
	}
	if (!hasGoal) {
		return false;
	}
	return std::ranges::any_of(state.player, [&state](const PlayerCell& cell) {
		return cell.isCore && state.terrain.GetOr(cell.coord, Terrain::Wall) == Terrain::Goal;
	});
}

void PuzzleRule::Move(PuzzleState& state, const Cake::GridCoord& offset) {
	for (PlayerCell& cell : state.player) {
		cell.coord += offset;
	}
}

bool PuzzleRule::IsPlayerAt(const PuzzleState& state, const Cake::GridCoord& coord) {
	return std::ranges::any_of(state.player, [&coord](const PlayerCell& cell) {
		return cell.coord == coord;
	});
}

bool PuzzleRule::IsTouchingPlayer(const PuzzleState& state, const ShapeBlock& block) {
	// (仮) 「触れる」は、上下左右のどれかで隣り合ったこと。斜めは触れていない扱い.
	for (const Cake::GridCoord& blockCell : block.cells) {
		for (const Cake::GridCoord& offset : kNeighborOffsets) {
			if (IsPlayerAt(state, blockCell + offset)) {
				return true;
			}
		}
	}
	return false;
}

void PuzzleRule::AttachTouchingBlocks(PuzzleState& state, std::vector<int32_t>& pickedIds) {
	// (仮) くっついたブロックも自機の一部なので、それに触れているブロックも続けてくっつく.
	// 増えなくなるまで繰り返す.
	bool isAttached = true;
	while (isAttached) {
		isAttached = false;
		for (auto it = state.blocks.begin(); it != state.blocks.end();) {
			if (!IsTouchingPlayer(state, *it)) {
				++it;
				continue;
			}
			for (const Cake::GridCoord& cell : it->cells) {
				state.player.push_back({cell, false});
			}
			pickedIds.push_back(it->id);
			it = state.blocks.erase(it);
			isAttached = true;
		}
	}
}

} // namespace Puzzle
