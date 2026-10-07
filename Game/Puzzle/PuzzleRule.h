#pragma once
/*====================================
 *
 * PuzzleState を1歩進める・回転させる、といったルールの計算.
 * 描画もエンジンも知らない。GameScene が一定間隔で Step() を呼び、
 * 戻り値の StepResult を見て View に演出をさせる.
 *
 * 【一定間隔で呼ぶ理由】
 * deltaTime ぶん連続で動かすと、フレームレートによって「SPACE を押した瞬間に
 * 自機がどのマスにいるか」が変わり、ステージ設計どおりに止まらなくなる.
 * Step() は1回で必ず1マス（または停止）だけ進める.
 *
 * 【(仮) と書いた箇所】
 * 企画書で未確定のルールを、仮の解釈で実装している。決まったらそこだけ直す.
 *
 * 【未対応のギミック】
 * 落とし穴・マスコット・動く壁・はじく壁・ワープは、まだ入れていない.
 * 企画書のステージ1～3（壁・図形ブロック・ゴール・回転回数）だけで遊べる範囲にしてある.
 *
 * ====================================*/
#include <cstdint>
#include <vector>

#include "Game/Puzzle/PuzzleState.h"

namespace Puzzle {

// 1歩で起きたこと。View が演出を選ぶのに使う.
struct StepResult {
	Cake::GridCoord moved{};             // 今回動いた量。止まっていれば {0, 0}.
	bool isFalling = false;              // 落下による移動なら true.
	std::vector<int32_t> pickedBlockIds; // 今回くっついたブロックの id.
	bool isCleared = false;              // この歩でクリアしたら true.
	bool isMissed = false;               // この歩でミスしたら true.
};

class PuzzleRule {
public:
	// 静的関数だけで使うので実体は作らせない.
	PuzzleRule() = delete;

	/// <summary>
	/// 1歩ぶん進める。クリア済み・ミス済みなら何もしない.
	/// </summary>
	static StepResult Step(PuzzleState& state);

	/// <summary>
	/// ステージを画面上で時計回りに90°回す.
	/// 回転回数の上限を超えて回そうとするとミスになり、false を返す.
	/// </summary>
	static bool Rotate(PuzzleState& state);

	/// <summary>
	/// 画面上の「右」（自機が進む向き）を、盤面から見た向きで返す.
	/// </summary>
	static Cake::Direction GetForward(const PuzzleState& state);

	/// <summary>
	/// 自機全体を offset だけずらせるか。壁・盤面の外・図形ブロックに当たるなら false.
	/// </summary>
	static bool CanMove(const PuzzleState& state, const Cake::GridCoord& offset);

	/// <summary>
	/// ゴールのマスがすべて自機で埋まり、そのどれかに■が入っているか.
	/// </summary>
	static bool IsGoalFilled(const PuzzleState& state);

private:
	static void Move(PuzzleState& state, const Cake::GridCoord& offset);
	static bool IsPlayerAt(const PuzzleState& state, const Cake::GridCoord& coord);
	static bool IsTouchingPlayer(const PuzzleState& state, const ShapeBlock& block);
	// 自機に触れている図形ブロックをすべてくっつける。くっつけた id を pickedIds に足す.
	static void AttachTouchingBlocks(PuzzleState& state, std::vector<int32_t>& pickedIds);
};

} // namespace Puzzle
