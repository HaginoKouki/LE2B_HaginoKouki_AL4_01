#pragma once
/*====================================
 *
 * ゲームで使う音の一覧。読み込みと再生の窓口をここに集める。
 *
 * 【書き方】
 * 1. LoadAllSounds() に Cake::Sound::Load("名前", "パス") を足す.
 * 2. BGM / SE の名前空間に、その名前で鳴らす関数を足す.
 * 呼び出し側は SE::Hit() のように書くだけで、名前やファイルを知らずに済む.
 *
 * 例:
 *   inline void LoadAllSounds() {
 *       Cake::Sound::Load("BGM_Game", "Sound/BGM/game.wav");
 *       Cake::Sound::Load("SE_Decide", "Sound/SE/decide.wav");
 *   }
 *   namespace BGM { inline void Game() { Cake::Sound::PlayBGM("BGM_Game"); } }
 *   namespace SE { inline void Decide() { Cake::Sound::PlaySE("SE_Decide"); } }
 *
 * 【ファイル形式】
 * KamataEngine::Audio::LoadWave() は WAV を読む関数として用意されている.
 * mp3 などを使う場合は、読めるかどうかを先に確かめること.
 *
 * ====================================*/
#include "System/Platform/Sound/Sound.h"

inline void LoadAllSounds() {
}

namespace BGM {
} // namespace BGM

namespace SE {
} // namespace SE
