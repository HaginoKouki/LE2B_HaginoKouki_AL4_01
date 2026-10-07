#pragma once
/*====================================
 *
 * 文字で書いたステージと PuzzleState を相互に変換する.
 * ステージ制作とデバッグ表示（ImGui）の両方で使う.
 *
 * 【記号】
 *   #  壁
 *   .  空き
 *   G  ゴール
 *   @  自機の■（本体）。ちょうど1つ置く
 *   o  自機の□（最初からくっついているブロック）
 *   1～9  図形ブロック。同じ数字のマスが1つの図形になる
 * 盤面の外は壁として扱うので、外周の # は書かなくてよい.
 *
 * 【上下の向き】
 * 1行目が画面の一番上。盤面座標は Y+ が上なので、読み込み時に反転する.
 *
 * 【例】1マス拾って3マスにし、1回回すと■がゴールに届く（回転上限1）
 *   ..G..
 *   .....
 *   .....
 *   @o1..
 *
 * ====================================*/
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "Game/Puzzle/PuzzleState.h"

namespace Puzzle {

class StageText {
public:
	// 静的関数だけで使うので実体は作らせない.
	StageText() = delete;

	/// <summary>
	/// 文字のステージを読み込む。行の長さが揃っていない、■が1つでない、
	/// 知らない記号がある、のいずれかなら std::nullopt を返す.
	/// </summary>
	static std::optional<PuzzleState> Parse(const std::vector<std::string>& rows, int32_t rotationLimit);

	/// <summary>
	/// 現在の状態を文字にする。1要素目が画面の一番上の行.
	/// ImGui::TextUnformatted() に1行ずつ渡せば、描画を作る前にルールを確かめられる.
	/// </summary>
	static std::vector<std::string> ToRows(const PuzzleState& state);
};

} // namespace Puzzle
