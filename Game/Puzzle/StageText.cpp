#include "StageText.h"

#include <algorithm>

namespace Puzzle {

std::optional<PuzzleState> StageText::Parse(const std::vector<std::string>& rows, int32_t rotationLimit) {
	if (rows.empty() || rows.front().empty()) {
		return std::nullopt;
	}
	const size_t width = rows.front().size();
	const bool isRectangle = std::ranges::all_of(rows, [width](const std::string& row) {
		return row.size() == width;
	});
	if (!isRectangle) {
		return std::nullopt;
	}

	const int32_t gridWidth = static_cast<int32_t>(width);
	const int32_t gridHeight = static_cast<int32_t>(rows.size());

	PuzzleState state;
	state.terrain = Cake::Grid<Terrain>(gridWidth, gridHeight, Terrain::Empty);
	state.rotationLimit = rotationLimit;

	int32_t coreCount = 0;
	for (int32_t row = 0; row < gridHeight; ++row) {
		// 1行目が一番上なので、盤面の Y は下から数え直す.
		const int32_t y = gridHeight - 1 - row;
		for (int32_t x = 0; x < gridWidth; ++x) {
			const Cake::GridCoord coord{x, y};
			const char symbol = rows[static_cast<size_t>(row)][static_cast<size_t>(x)];
			switch (symbol) {
			case '.':
				break;
			case '#':
				state.terrain.At(coord) = Terrain::Wall;
				break;
			case 'G':
				state.terrain.At(coord) = Terrain::Goal;
				break;
			case '@':
				state.player.push_back({coord, true});
				++coreCount;
				break;
			case 'o':
				state.player.push_back({coord, false});
				break;
			default: {
				if (symbol < '1' || '9' < symbol) {
					return std::nullopt;
				}
				// 同じ数字が初めて出てきたら、新しい図形として登録する.
				const int32_t id = symbol - '0';
				auto it = std::ranges::find(state.blocks, id, &ShapeBlock::id);
				ShapeBlock& block = (it != state.blocks.end()) ? *it : state.blocks.emplace_back(ShapeBlock{id, {}});
				block.cells.push_back(coord);
				break;
			}
			}
		}
	}

	if (coreCount != 1) {
		return std::nullopt;
	}
	return state;
}

std::vector<std::string> StageText::ToRows(const PuzzleState& state) {
	const int32_t width = state.terrain.GetWidth();
	const int32_t height = state.terrain.GetHeight();

	// まず盤面座標の向き（下の行が先頭）で埋め、最後に上下を反転する.
	std::vector<std::string> rows(static_cast<size_t>(height), std::string(static_cast<size_t>(width), '.'));
	auto at = [&rows](const Cake::GridCoord& coord) -> char& {
		return rows[static_cast<size_t>(coord.y)][static_cast<size_t>(coord.x)];
	};

	for (int32_t y = 0; y < height; ++y) {
		for (int32_t x = 0; x < width; ++x) {
			const Cake::GridCoord coord{x, y};
			switch (state.terrain.At(coord)) {
			case Terrain::Wall:
				at(coord) = '#';
				break;
			case Terrain::Goal:
				at(coord) = 'G';
				break;
			default:
				break;
			}
		}
	}
	for (const ShapeBlock& block : state.blocks) {
		const char symbol = static_cast<char>('0' + block.id % 10);
		for (const Cake::GridCoord& cell : block.cells) {
			if (state.terrain.IsInside(cell)) {
				at(cell) = symbol;
			}
		}
	}
	for (const PlayerCell& cell : state.player) {
		if (state.terrain.IsInside(cell.coord)) {
			at(cell.coord) = cell.isCore ? '@' : 'o';
		}
	}

	std::ranges::reverse(rows);
	return rows;
}

} // namespace Puzzle
