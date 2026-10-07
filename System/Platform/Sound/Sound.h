#pragma once
/*====================================
 *
 * KamataEngineのAudioをゲーム用にラップするクラス。
 * 名前を付けて読み込み、以降は名前だけで鳴らす。BGMは常に1本、SEは重ねて鳴らせる。
 *
 * 【ハンドルを外へ出さない】
 * Load() で名前を付けて登録し、呼び出し側は Sound::PlaySE("Hit") とだけ書く。
 * ハンドルを配ると「どこで読み込んだどのハンドルか」を各所が覚える羽目になり、
 * 音を差し替えるたびに呼び出し側まで直すことになる。
 *
 * 【音量は3階建て】
 * master × bgm(またはse) × 個別音量 が実際の音量になる。
 * オプション画面が master と bgm/se を触り、演出の都合で個別音量を触る想定。
 * SetMasterVolume() などは再生中の音にもその場で反映される。
 *
 * 【BGMは1本だけ】
 * PlayBGM() は前の曲を止めてから鳴らす。
 * クロスフェードや2曲重ねが要るようになったら、この方針から見直すこと。
 *
 * 【鳴り終わったSEの後始末】
 * KamataEngine の Audio は再生が終わってもボイス実体を抱えたままで、
 * StopWave() を呼ぶまで解放されない。PlaySE() の先頭で終了済みの分を返しているので、
 * 呼び出し側で毎フレーム Update() を回す必要はない。
 *
 * ====================================*/
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "KamataEngine.h"

namespace Cake {
class Sound final {
public:
	// Load() で登録した音声データ1件分.
	struct SoundData {
		std::string name;
		std::string path;
		uint32_t handle = 0;
	};

private:
	// 再生中の音1本分。後から音量を変えられるよう、再生ハンドルと個別音量を持つ.
	struct PlayingVoice {
		uint32_t voiceHandle = 0;
		// master/bgm/se を掛ける前の、この1本だけの音量.
		float volume = 1.0f;
	};

	// 読み込み済みの音声データ。キーは Load() で付けた名前.
	std::unordered_map<std::string, SoundData> soundDatas_;

	// 再生中のBGM。同時に鳴らせるのは1本だけ.
	PlayingVoice bgm_;
	bool hasBGM_ = false;

	// 再生中のSE。鳴り終わった分は PlaySE() の先頭で取り除く.
	std::vector<PlayingVoice> seVoices_;

	float masterVolume_ = 1.0f;
	float bgmVolume_ = 0.2f;
	float seVolume_ = 1.0f;

private:
	static Sound* GetInstance() {
		static Sound instance;
		return &instance;
	}
	Sound() = default;

	// 名前に対応するデータを探す。未登録なら nullptr.
	static const SoundData* Find(const std::string& name);
	// 鳴り終わったSEのボイスをエンジンへ返す.
	static void SweepFinishedSE();
	// 現在の音量設定を、再生中の全ての音へ流し込む.
	static void ApplyVolumes();

public:
	static void Initialize();

	/// <summary>
	/// 音声データをロードする.
	/// </summary>
	/// <param name="name">データ呼び出し用の名前</param>
	/// <param name="path">音声ファイルのパス（Resources/ からの相対）</param>
	static void Load(const std::string& name, const std::string& path);

	static bool IsLoaded(const std::string& name);

	/* ===== BGM ===== */

	/// <summary>
	/// BGMを再生する。既に鳴っているBGMは止まる.
	/// </summary>
	/// <param name="name">Load() で付けた名前</param>
	/// <param name="loop">ループするか</param>
	/// <param name="volume">この曲だけの音量。master と bgm に掛かる</param>
	static void PlayBGM(const std::string& name, bool loop = true, float volume = 1.0f);
	static void StopBGM();
	static void PauseBGM();
	static void ResumeBGM();
	// 一時停止中も true を返す。エンジン側がバッファの有無で判定しているため.
	static bool IsBGMPlaying();

	/* ===== SE ===== */

	/// <summary>
	/// SEを再生する。重ねて鳴らせる.
	/// </summary>
	/// <param name="name">Load() で付けた名前</param>
	/// <param name="volume">この1回だけの音量。master と se に掛かる</param>
	/// <param name="loop">true の場合は StopSE() されるまで繰り返す。</param>
	/// <returns>個別停止に使える再生ハンドル。</returns>
	static uint32_t PlaySE(const std::string& name, float volume = 1.0f, bool loop = false);
	static void StopSE(uint32_t voiceHandle);
	static void StopAllSE();

	/* ===== 音量。0〜1に丸められる ===== */

	static void SetMasterVolume(float volume);
	static void SetBGMVolume(float volume);
	static void SetSEVolume(float volume);

	static float GetMasterVolume() { return GetInstance()->masterVolume_; }
	static float GetBGMVolume() { return GetInstance()->bgmVolume_; }
	static float GetSEVolume() { return GetInstance()->seVolume_; }

	// 鳴っている音を全て止める。シーン遷移や終了時に呼ぶ。読み込み済みのデータは残る.
	static void StopAll();
};
} // namespace Cake
