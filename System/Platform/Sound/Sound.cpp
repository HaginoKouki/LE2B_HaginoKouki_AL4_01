#include "Sound.h"

#include <algorithm>
#include <cassert>

namespace Cake {

namespace {

// PlayWave() は 1 を超える値も受け取るが、そのまま鳴らすとクリップして歪む。
// 3つの音量を掛け合わせる都合で 1 を超えやすいので、入口で丸めておく.
float ClampVolume(float volume) {
	return std::clamp(volume, 0.0f, 1.0f);
}

} // namespace

void Sound::Initialize() {
	// KamataEngine::Initialize() の中で Audio の初期化まで済んでいる。
	// ここで Audio::Initialize() を呼び直すと XAudio2 を二重に握ることになるので呼ばない.
	//
	// 読み込み済みのデータは消さない。TextureManager::Load() と同じように
	// メンバ初期化子で Load() を書く使い方だと、この時点で既に登録が済んでいるため.
	StopAll();
}

void Sound::Load(const std::string& name, const std::string& path) {
	Sound* instance = GetInstance();

	const auto it = instance->soundDatas_.find(name);
	if (it != instance->soundDatas_.end()) {
		// 同じ名前・同じパスなら二重呼び出し。黙って無視してよい。
		// パスが違う場合は取り違えなので、気付けるように止める.
		assert(it->second.path == path && "同じ名前で別のファイルを登録しようとしている");
		return;
	}

	SoundData data{};
	data.name = name;
	data.path = path;
	data.handle = KamataEngine::Audio::GetInstance()->LoadWave(path);

	instance->soundDatas_.emplace(name, std::move(data));
}

bool Sound::IsLoaded(const std::string& name) {
	return Find(name) != nullptr;
}

const Sound::SoundData* Sound::Find(const std::string& name) {
	const Sound* instance = GetInstance();

	const auto it = instance->soundDatas_.find(name);
	if (it == instance->soundDatas_.end()) {
		return nullptr;
	}
	return &it->second;
}

/* ===== BGM ===== */

void Sound::PlayBGM(const std::string& name, bool loop, float volume) {
	const SoundData* data = Find(name);
	// 名前のタイプミスは音が鳴らないだけで済んでしまい、原因を探すのに時間がかかる.
	assert(data != nullptr && "Load() されていない名前でBGMを再生しようとしている");
	if (data == nullptr) {
		return;
	}

	// BGMは1本だけ。前の曲を止めてから鳴らす.
	StopBGM();

	Sound* instance = GetInstance();
	const float baseVolume = ClampVolume(volume);
	const float playVolume = ClampVolume(instance->masterVolume_ * instance->bgmVolume_ * baseVolume);

	instance->bgm_.voiceHandle = KamataEngine::Audio::GetInstance()->PlayWave(data->handle, loop, playVolume);
	instance->bgm_.volume = baseVolume;
	instance->hasBGM_ = true;
}

void Sound::StopBGM() {
	Sound* instance = GetInstance();
	if (!instance->hasBGM_) {
		return;
	}

	KamataEngine::Audio::GetInstance()->StopWave(instance->bgm_.voiceHandle);
	instance->bgm_ = PlayingVoice{};
	instance->hasBGM_ = false;
}

void Sound::PauseBGM() {
	const Sound* instance = GetInstance();
	if (!instance->hasBGM_) {
		return;
	}
	KamataEngine::Audio::GetInstance()->PauseWave(instance->bgm_.voiceHandle);
}

void Sound::ResumeBGM() {
	const Sound* instance = GetInstance();
	if (!instance->hasBGM_) {
		return;
	}
	KamataEngine::Audio::GetInstance()->ResumeWave(instance->bgm_.voiceHandle);
}

bool Sound::IsBGMPlaying() {
	const Sound* instance = GetInstance();
	if (!instance->hasBGM_) {
		return false;
	}
	return KamataEngine::Audio::GetInstance()->IsPlaying(instance->bgm_.voiceHandle);
}

/* ===== SE ===== */

uint32_t Sound::PlaySE(const std::string& name, float volume, bool loop) {
	const SoundData* data = Find(name);
	assert(data != nullptr && "Load() されていない名前でSEを再生しようとしている");
	if (data == nullptr) {
		return 0;
	}

	// 鳴らす前に、終わった分の席を空けておく.
	SweepFinishedSE();

	Sound* instance = GetInstance();
	const float baseVolume = ClampVolume(volume);
	const float playVolume = ClampVolume(instance->masterVolume_ * instance->seVolume_ * baseVolume);

	PlayingVoice voice{};
	voice.voiceHandle = KamataEngine::Audio::GetInstance()->PlayWave(data->handle, loop, playVolume);
	voice.volume = baseVolume;

	instance->seVoices_.push_back(voice);
	return voice.voiceHandle;
}

void Sound::StopSE(uint32_t voiceHandle) {
	Sound* instance = GetInstance();
	auto voiceIt = std::find_if(instance->seVoices_.begin(), instance->seVoices_.end(),
		[voiceHandle](const PlayingVoice& voice) { return voice.voiceHandle == voiceHandle; });
	if (voiceIt == instance->seVoices_.end()) {
		return;
	}
	KamataEngine::Audio::GetInstance()->StopWave(voiceIt->voiceHandle);
	instance->seVoices_.erase(voiceIt);
}

void Sound::StopAllSE() {
	Sound* instance = GetInstance();
	KamataEngine::Audio* audio = KamataEngine::Audio::GetInstance();

	for (const PlayingVoice& voice : instance->seVoices_) {
		audio->StopWave(voice.voiceHandle);
	}
	instance->seVoices_.clear();
}

void Sound::SweepFinishedSE() {
	Sound* instance = GetInstance();
	KamataEngine::Audio* audio = KamataEngine::Audio::GetInstance();

	// 鳴り終わってもエンジン側のボイスは残る。StopWave() を呼んで明示的に返す.
	std::erase_if(instance->seVoices_, [audio](const PlayingVoice& voice) {
		if (audio->IsPlaying(voice.voiceHandle)) {
			return false;
		}
		audio->StopWave(voice.voiceHandle);
		return true;
	});
}

/* ===== 音量 ===== */

void Sound::SetMasterVolume(float volume) {
	GetInstance()->masterVolume_ = ClampVolume(volume);
	ApplyVolumes();
}

void Sound::SetBGMVolume(float volume) {
	GetInstance()->bgmVolume_ = ClampVolume(volume);
	ApplyVolumes();
}

void Sound::SetSEVolume(float volume) {
	GetInstance()->seVolume_ = ClampVolume(volume);
	ApplyVolumes();
}

void Sound::ApplyVolumes() {
	// 終わったボイスへ SetVolume() を投げても意味が無いので、先に掃除する.
	SweepFinishedSE();

	Sound* instance = GetInstance();
	KamataEngine::Audio* audio = KamataEngine::Audio::GetInstance();

	if (instance->hasBGM_) {
		audio->SetVolume(instance->bgm_.voiceHandle, ClampVolume(instance->masterVolume_ * instance->bgmVolume_ * instance->bgm_.volume));
	}

	for (const PlayingVoice& voice : instance->seVoices_) {
		audio->SetVolume(voice.voiceHandle, ClampVolume(instance->masterVolume_ * instance->seVolume_ * voice.volume));
	}
}

void Sound::StopAll() {
	StopBGM();
	StopAllSE();
}

} // namespace Cake
