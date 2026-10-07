#pragma once
/*====================================
 *
 * カーブをチャンネルごとに束ねた再生単位。Unity の AnimationClip 相当。
 *
 * 【共有アセットとして扱う】
 * 再生位置は Animator が持つため、実体は1つを全オブジェクトで使い回してよい。
 * 個体ごとにコピーを配ると、同じ敵を10体出したとき全員が同じ姿勢で固まる。
 *
 * 【組み立ては初期化時に済ませる】
 * Float() は無ければカーブを新しく作る。毎フレーム呼ぶ想定ではない。
 *
 * ====================================*/
#include <string>
#include <vector>
#include <utility>
#include <cstdint>

#include "System/Foundation/Animation/AnimationCurve.h"

namespace Cake {

// 書き込み先を表す番号。Animator はこの番号でバインド表を引く.
// 文字列で引くと毎フレームのハッシュ計算が乗るため、番号による配列引きにしている.
// Transform2（UI）を繋いだ場合、2D の回転は RotationZ に割り当たる.
namespace Channel {
inline constexpr uint16_t PositionX = 0;
inline constexpr uint16_t PositionY = 1;
inline constexpr uint16_t PositionZ = 2;
inline constexpr uint16_t RotationX = 3;
inline constexpr uint16_t RotationY = 4;
inline constexpr uint16_t RotationZ = 5;
inline constexpr uint16_t ScaleX = 6;
inline constexpr uint16_t ScaleY = 7;
inline constexpr uint16_t ScaleZ = 8;

// ゲーム固有の値を動かしたい場合はここから使う.
inline constexpr uint16_t UserBegin = 9;
// バインド表の大きさ。足りなくなったらここを増やす.
inline constexpr uint16_t Count = 24;
} // namespace Channel

enum class LoopMode : uint8_t {
	Once,     // 最後まで再生したら止まる。値は最終キーのまま残る.
	Loop,     // 先頭へ戻って繰り返す.
	PingPong, // 端まで行ったら逆再生する.
};

class AnimationClip {
private:
	std::string name_;
	LoopMode loop_ = LoopMode::Once;

	// 使うチャンネルの分だけ持つ。数が少ないので線形探索で足りる.
	std::vector<std::pair<uint16_t, AnimationCurve<float>>> floatCurves_;
	// テクスチャの差し替え。1クリップに1本だけ持つ.
	StepCurve<uint32_t> textureCurve_;

public:
	explicit AnimationClip(std::string name) : name_(std::move(name)) {}

	/* ===== 組み立て。初期化時にだけ呼ぶ ===== */

	/// <summary>
	/// 指定チャンネルのカーブを取り出す。無ければ作る.
	/// 戻り値に AddKey() を繋げて書ける.
	/// </summary>
	/// <remarks>
	/// 返す参照は次に Float() を呼ぶまでの間だけ有効。
	/// 内部の vector が伸びると参照先が移動するため、跨いで持ち回らないこと.
	/// </remarks>
	AnimationCurve<float>& Float(uint16_t channel);

	/// <summary>
	/// テクスチャを等間隔で切り替える。コマアニメはほぼこの形になる.
	/// 呼び直すと前の内容は捨てられる.
	/// </summary>
	/// <param name="handles">表示する順に並べたテクスチャハンドル</param>
	/// <param name="fps">1秒あたりの表示枚数</param>
	void SetTextureFrames(const std::vector<uint32_t>& handles, float fps);

	// 不等間隔で切り替えたい場合は、こちらへ直接キーを積む.
	StepCurve<uint32_t>& Texture() { return textureCurve_; }

	void SetLoop(LoopMode loop) { loop_ = loop; }

	/* ===== 再生。Animator から呼ぶ ===== */

	const std::string& GetName() const { return name_; }
	LoopMode GetLoop() const { return loop_; }

	// 全チャンネル中で最も遅いキーの時刻.
	float GetDuration() const;

	const std::vector<std::pair<uint16_t, AnimationCurve<float>>>& GetFloatCurves() const { return floatCurves_; }
	const StepCurve<uint32_t>& GetTextureCurve() const { return textureCurve_; }
};

} // namespace Cake
