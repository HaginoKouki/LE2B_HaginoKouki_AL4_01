#pragma once
/*====================================
 *
 * エンジン内で唯一の「時間の権威」。
 * 前フレームからの経過時間をここ1箇所で測り、deltaTime として全ての Update へ配る。
 *
 * 【ここ以外で経過時間を測らないこと】
 * 測る場所が増えると値が食い違い、「今どの時間で動いているのか」が追えなくなる。
 * 経過時間が必要な処理は、自前で測らずここから受け取る。
 *
 * 【scaled と unscaled】
 * ゲームロジックは scaled（timeScale 適用済み）を使う。
 * フェードやポーズ画面のUIなど、ヒットストップやポーズ中も動かしたいものは unscaled を使う。
 *
 * 【timeScale とポーズは次のフレームから効く】
 * フレームの途中で deltaTime が変わると、先に更新したオブジェクトと後のオブジェクトで
 * 進み方が食い違う（ヒットストップを掛けた瞬間、半分だけ止まる）。
 * SetTimeScale() / SetPaused() は値を覚えるだけで、次の Tick() から反映する。
 *
 * 【クランプ】
 * ブレークポイントで止めた直後などに巨大な deltaTime が流れると、
 * オブジェクトが吹き飛び、当たり判定もすり抜ける。
 * 実測値は kMaxDeltaTime で頭打ちにする。捨てた分は取り戻さない
 * （遅れを取り戻そうとすると、重い環境で加速して余計に破綻するため）。
 *
 * std::chrono しか使わない。この層で最も依存が軽いファイル。
 *
 * ====================================*/
#include <cstdint>
#include <chrono>

namespace Cake {

class Time {
private:
	// 1フレームとして認める経過時間の上限（秒）。超えた分は捨てる.
	static constexpr float kMaxDeltaTime = 0.1f;

	std::chrono::steady_clock::time_point prevTime_{};
	bool hasPrevTime_ = false; // 初回 Tick を経過時間 0 で通すため.

	// 実経過時間（timeScaleが適用前の値）.
	float unscaledDeltaTime_ = 0.0f;
	// 経過時間（timeScaleが適用された値）.
	float deltaTime_ = 0.0f;

	// 起動からの累計時間.
	double unscaledTotalTime_ = 0.0;
	double totalTime_ = 0.0;

	uint64_t frameCount_ = 0;

	// 時間の流れ方。0で停止、1で等速。負の値は0に丸める. 次の Tick() から反映する.
	float timeScale_ = 1.0f;

	bool isPaused_ = false;

public:
	// フレームの先頭で1回だけ呼ぶ.
	// これ以降に走る処理は、そのフレーム中すべて同じ deltaTime を見る.
	void Tick();

	// 計測を打ち切り、次の Tick を初回として扱う.
	// シーン読み込みなど、止まっていた時間を deltaTime に混ぜたくない場面で呼ぶ.
	void Reset();

	/* ===== ゲームロジック用（timeScale 適用済み）===== */

	float GetDeltaTime() const { return deltaTime_; }
	double GetTotalTime() const { return totalTime_; }

	/* ===== 演出・UI用（実時間）===== */
	/* timeScale が 0 でも、ポーズ中でも進む. */

	float GetUnscaledDeltaTime() const { return unscaledDeltaTime_; }
	double GetUnscaledTotalTime() const { return unscaledTotalTime_; }

	// 起動からのフレーム数.
	uint64_t GetFrameCount() const { return frameCount_; }


	/* ===== 時間の流れ方 ===== */

	// 0 で停止、1 で等速。負の値は 0 に丸める。次の Tick() から反映する.
	float GetTimeScale() const { return timeScale_; }
	void SetTimeScale(float scale);

	// 次の Tick() から反映する.
	bool IsPaused() const { return isPaused_; }
	void SetPaused(bool paused);
};

} // namespace Cake
