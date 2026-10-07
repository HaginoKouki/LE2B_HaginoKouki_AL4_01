#pragma once
/*====================================
 *
 * クリップを再生し、結果を書き込み先へ流し込むコンポーネント。
 * CollisionBody と同じく、使う側がメンバとして持ち、Initialize() で明示的に繋ぐ。
 *
 * 【書き込み先の決め方】
 * C++ にはリフレクションが無いので、チャンネル番号 → 書き込み先ポインタ の表を先に作る。
 * BindTransform() を呼べば、localTransform_ の各成分が組み込みチャンネルへ繋がる。
 * Transform3（3Dオブジェクト）と Transform2（UI）のどちらも繋げる。
 * 繋がれていないチャンネルがクリップに含まれていても、黙って無視される。
 * クリップは共有物なので、使わないチャンネルが混ざっているのは正常な状態。
 *
 * 【KamataEngine に依存させない】
 * テクスチャの差し替えは setter を受け取る形にしている。
 * ここで ModelRenderer や Sprite を直接触ると、描画 API を知らないはずの Scene 層が汚れるため。
 *
 * 【呼ぶ順番】
 * オーナーの Update() の末尾で Update() を呼ぶ。
 * ロジックで姿勢を決めた後にアニメを重ねる形になり、適用順が呼び出し側から見える。
 *
 * ====================================*/
#include <array>
#include <functional>
#include <cstdint>

#include "System/Foundation/Animation/AnimationClip.h"
#include "System/Foundation/Math/Transform.h"

namespace Cake {
class Time;
}

class Animator {
private:
	// 所有しない。クリップは共有アセット.
	const Cake::AnimationClip* clip_ = nullptr;

	// チャンネル番号で引く書き込み先。nullptr のチャンネルは書き込まれない.
	std::array<float*, Cake::Channel::Count> floatTargets_{};
	std::function<void(uint32_t)> textureSetter_;

	// クリップ先頭からの経過時間.
	float time_ = 0.0f;
	float speed_ = 1.0f;

	bool isPlaying_ = false;
	// PingPong で折り返している最中か.
	bool isReversing_ = false;
	// UI など、ポーズ中でも動かしたいものは true にする.
	bool useUnscaledTime_ = false;

public:
	Animator() = default;

	// 他人の変数を指すポインタを抱えるため、コピーも移動も禁止する.
	// 複製すると、複製元のオーナーの Transform を書き換えてしまう.
	Animator(const Animator&) = delete;
	Animator& operator=(const Animator&) = delete;
	Animator(Animator&&) = delete;
	Animator& operator=(Animator&&) = delete;

	/* ===== 書き込み先の登録。Initialize() で、Play() より先に呼ぶ ===== */

	/// <summary>
	/// Transform3 の各成分を組み込みチャンネルへ繋ぐ.
	/// 参照先が動かないもの（メンバ変数）を渡すこと.
	/// </summary>
	void BindTransform(Cake::Transform3& transform);

	/// <summary>
	/// Transform2 の各成分を組み込みチャンネルへ繋ぐ。UI のスプライト向け.
	/// 回転は RotationZ、Z を持たない成分（PositionZ など）は繋がない.
	/// </summary>
	void BindTransform(Cake::Transform2& transform);

	/// <summary>
	/// 独自の値を動かす場合に足す。channel には Channel::UserBegin 以降を使う.
	/// </summary>
	void BindFloat(uint16_t channel, float* target);

	/// <summary>
	/// テクスチャ差し替えの受け口を登録する.
	/// 例: [this](uint32_t handle) { modelRenderer_.SetTexture(handle); }
	/// </summary>
	void SetTextureSetter(std::function<void(uint32_t)> setter);

	/* ===== 再生 ===== */

	/// <summary>
	/// クリップを再生する。開始時点の値がその場で書き込まれる.
	/// </summary>
	/// <param name="clip">再生するクリップ。所有しないので寿命は呼び出し側の責任</param>
	/// <param name="restart">再生中の同じクリップを頭から流し直すか</param>
	void Play(const Cake::AnimationClip* clip, bool restart = true);

	// 止めるだけ。最後に書き込んだ値はそのまま残る.
	void Stop() { isPlaying_ = false; }

	// オーナーの Update() の末尾で呼ぶ.
	void Update(Cake::Time* time);

	/// <summary>
	/// 時刻を直接指定して1回だけ適用する.
	/// ImGui でのプレビューや、停止状態で特定のポーズを取らせたい場合に使う.
	/// </summary>
	void Sample(float time);

	bool IsPlaying() const { return isPlaying_; }
	float GetTime() const { return time_; }
	const Cake::AnimationClip* GetClip() const { return clip_; }

	// 負の値を入れると逆再生になる.
	void SetSpeed(float speed) { speed_ = speed; }
	void SetUseUnscaledTime(bool use) { useUnscaledTime_ = use; }

private:
	// time_ をループ設定に合わせて折り返す。Once なら終端で止める.
	void AdvanceTime(float deltaTime);
	// 現在の time_ における値を、書き込み先へ流す.
	void Apply() const;
};
