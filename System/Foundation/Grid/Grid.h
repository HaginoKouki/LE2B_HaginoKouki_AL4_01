#pragma once
/*====================================
 *
 * マス目の座標と、マス目に値を並べる入れ物。パズルの盤面データの土台。
 *
 * 【float の Transform3 と分ける理由】
 * 盤面の判定は整数のマス座標で行い、ワールド座標への変換は表示側（View）でだけ行う.
 * float で「同じマスか」を比べると、補間移動の途中や計算誤差で判定が揺れるため.
 *
 * 【エンジンに依存させない】
 * KamataEngine も GameObject も include しない。値型なのでコピーでき、
 * やり直し用に開始時の状態を取っておける。描画なしでルールだけを確かめることもできる.
 *
 * 【Y の向き】
 * Y+ を上とする。Cake::Input::GetMoveAxis() と同じ向き.
 * テキストのステージデータは上の行から読むので、読み込み時に上下を反転すること.
 *
 * ====================================*/
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <vector>

namespace Cake {

// マス目の座標。Pos ではなく Coord にしているのは、.editorconfig のスペルチェックで
// エラー扱いにならないようにするため（coord は exclusion.dic に登録済み）.
struct GridCoord {
	int32_t x = 0;
	int32_t y = 0;

	constexpr GridCoord operator+(const GridCoord& other) const { return {x + other.x, y + other.y}; }
	constexpr GridCoord operator-(const GridCoord& other) const { return {x - other.x, y - other.y}; }
	constexpr GridCoord& operator+=(const GridCoord& other) {
		x += other.x;
		y += other.y;
		return *this;
	}
	constexpr bool operator==(const GridCoord&) const = default;
};

enum class Direction : uint8_t {
	None,
	Up,
	Down,
	Left,
	Right,
};

// 方向を1マスぶんのずれに直す.
constexpr GridCoord ToOffset(Direction direction) {
	switch (direction) {
	case Direction::Up:
		return {0, 1};
	case Direction::Down:
		return {0, -1};
	case Direction::Left:
		return {-1, 0};
	case Direction::Right:
		return {1, 0};
	default:
		return {0, 0};
	}
}

// 反時計回りに90°回した向き（Y+ が上として）.
constexpr Direction RotateCounterClockwise(Direction direction) {
	switch (direction) {
	case Direction::Up:
		return Direction::Left;
	case Direction::Left:
		return Direction::Down;
	case Direction::Down:
		return Direction::Right;
	case Direction::Right:
		return Direction::Up;
	default:
		return Direction::None;
	}
}

// 時計回りに90°回した向き（Y+ が上として）.
constexpr Direction RotateClockwise(Direction direction) {
	switch (direction) {
	case Direction::Up:
		return Direction::Right;
	case Direction::Right:
		return Direction::Down;
	case Direction::Down:
		return Direction::Left;
	case Direction::Left:
		return Direction::Up;
	default:
		return Direction::None;
	}
}

template <typename T>
class Grid {
	// std::vector<bool> は特殊化されていて At() が bool& を返せない。uint8_t などを使うこと.
	static_assert(!std::is_same_v<T, bool>, "Grid<bool> は使えない。Grid<uint8_t> などを使う.");

private:
	int32_t width_ = 0;
	int32_t height_ = 0;
	std::vector<T> cells_; // y * width_ + x の順に並べる.

public:
	Grid() = default;
	Grid(int32_t width, int32_t height, const T& initial = T{})
	    : width_(std::max<int32_t>(width, 0)), height_(std::max<int32_t>(height, 0)),
	      cells_(static_cast<size_t>(width_) * static_cast<size_t>(height_), initial) {
		assert(width >= 0 && height >= 0 && "盤面の大きさが負");
	}

	int32_t GetWidth() const { return width_; }
	int32_t GetHeight() const { return height_; }

	bool IsInside(const GridCoord& coord) const {
		return 0 <= coord.x && coord.x < width_ && 0 <= coord.y && coord.y < height_;
	}

	// 範囲外を渡すと assert で止まる。範囲外があり得る場面では GetOr() を使う.
	T& At(const GridCoord& coord) {
		assert(IsInside(coord));
		return cells_[ToIndex(coord)];
	}
	const T& At(const GridCoord& coord) const {
		assert(IsInside(coord));
		return cells_[ToIndex(coord)];
	}

	// 範囲外なら fallback を返す。「盤面の外は壁」のような判定を短く書くため.
	T GetOr(const GridCoord& coord, const T& fallback) const {
		return IsInside(coord) ? cells_[ToIndex(coord)] : fallback;
	}

	void Fill(const T& value) { std::fill(cells_.begin(), cells_.end(), value); }

	// 「盤面が変わったか」の比較などに使う.
	bool operator==(const Grid&) const = default;

private:
	size_t ToIndex(const GridCoord& coord) const {
		return static_cast<size_t>(coord.y) * static_cast<size_t>(width_) + static_cast<size_t>(coord.x);
	}
};

} // namespace Cake
