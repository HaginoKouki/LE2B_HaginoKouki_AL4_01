#pragma once
/*====================================
 *
 * 画面全体を1色で覆う演出。フェードイン・アウトとフラッシュをここに集める。
 *
 * 【レンダーターゲットを使うポストプロセスではない】
 * KamataEngine::Sprite はブレンドモードもシェーダも差し替えられないので、
 * 「描き終えた絵を読み直して加工する」処理は作れない。
 * ここでやるのは、全ての描画の上に全画面の板を重ねること。
 * フェードとフラッシュはそれで足りるので、必要になるまで踏み込まない。
 *
 * 【フェードとフラッシュを別の層に分けている理由】
 * 1つの色を共有すると、暗転の途中で被弾フラッシュが入った瞬間に暗転が消える。
 * 「終点で止まったまま残る層(fade)」と「必ず透明へ戻る層(flash)」を独立に持ち、
 * 描画時に重ねることで、互いを打ち消さないようにしている。
 *
 * 【時間は unscaled を使う】
 * ヒットストップやポーズで timeScale が 0 になっても暗転は進んでほしい。
 * scaled にすると、ポーズ中に暗転させた瞬間、暗いまま何も進まなくなる。
 *
 * 【シーンをまたいで生き残る】
 * SceneManager::ChangeScene() はシーンごと作り直すので、シーンが持つと
 * 「暗転 → シーン切り替え → 明転」が切り替えの瞬間に途切れて一瞬絵が出る。
 * 静的な実体にして、シーンの寿命から切り離してある。
 *
 * 【オーバーレイのスプライトを2枚持つ理由】
 * KamataEngine::Sprite が持つ定数バッファは1組しかない。1枚を2回描くと、
 * コマンドリストが実行される頃には最後に書いた色になっていて、
 * fade と flash が同じ色で2回描かれる。層ごとに実体を分ける。
 *
 * 【呼ぶ場所】
 * Initialize()  : KamataEngine::Initialize() の後、最初のシーンを作る前に1回.
 * Finalize()    : KamataEngine::Finalize() の前に1回.
 * Update()      : SceneManager::Update() の先頭.
 * Draw()        : SceneManager::DrawUI() の末尾（Sprite::PreDraw()～PostDraw() の内側）.
 *                 3D の描画より後なので、モデルも UI もまとめて覆う.
 *
 * ====================================*/
#include <cstdint>

#include "KamataEngine.h"

#include "System/Foundation/Math/Vector.h"
#include "System/Foundation/Math/Easing.h"

namespace Cake {
class Time;
}

class ScreenEffect {
public:
	// よく使う色。RGB だけを持ち、濃さ(α)は各関数の引数で決める.
	static inline const Cake::Vector3 kBlack{0.0f, 0.0f, 0.0f};
	static inline const Cake::Vector3 kWhite{1.0f, 1.0f, 1.0f};
	static inline const Cake::Vector3 kRed{1.0f, 0.0f, 0.0f};

	// 状態は全て静的に持つので、実体は作らせない.
	ScreenEffect() = delete;

	/// <summary>
	/// テクスチャとスプライトを用意する。TextureManager を使うので
	/// KamataEngine::Initialize() より後に呼ぶこと.
	/// </summary>
	static void Initialize();
	/// <summary>
	/// Sprite の実体を畳む。KamataEngine::Finalize() より前に呼ぶこと.
	/// </summary>
	static void Finalize();

	/// <param name="time">nullptr を渡した場合は時間が進まないだけで、落ちはしない</param>
	static void Update(const Cake::Time* time);

	/// <summary>
	/// 全ての描画の後に呼ぶ。UI も前景フレームも含めて覆う.
	/// </summary>
	static void Draw();

	/* ===== フェード（終点で止まったまま残る層）===== */

	/// <summary>
	/// 今の濃さから color の不透明まで濃くする。終わると画面は color で覆われたまま残る.
	/// 途中で呼び直しても、今の濃さから続くので飛ばない.
	/// </summary>
	/// <param name="duration">秒。0 を渡すと即座に覆う</param>
	static void FadeOut(const Cake::Vector3& color, float duration, Cake::Easing::EaseType ease = Cake::Easing::EaseType::Linear);

	/// <summary>
	/// 今の色のまま透明へ戻す。直前の FadeOut() の色をそのまま引き継ぐ.
	/// </summary>
	static void FadeIn(float duration, Cake::Easing::EaseType ease = Cake::Easing::EaseType::Linear);

	/// <summary>
	/// 色を差し替えてから透明へ戻す。覆いの色が分からない状態
	/// （シーンの Initialize() など）から明ける場合に使う.
	/// </summary>
	static void FadeIn(const Cake::Vector3& color, float duration, Cake::Easing::EaseType ease = Cake::Easing::EaseType::Linear);

	/// <summary>
	/// 補間せず即座にその状態にする。シーンを「暗転済み」から始めたい場合に使う.
	/// </summary>
	static void SetFade(const Cake::Vector3& color, float alpha);

	/// <summary>覆いを即座に外す.</summary>
	static void ClearFade();

	/// <summary>フェードが補間中の間 true。シーン切り替えの合図に使う.</summary>
	static bool IsFading();

	/// <summary>画面が完全に覆われているか。暗転しきってから処理を進めたい場合に見る.</summary>
	static bool IsCovered();

	/* ===== フラッシュ（必ず透明へ戻る層）===== */

	/// <summary>
	/// 呼んだ瞬間に peakAlpha まで光り、duration 秒かけて透明へ戻る.
	/// 減衰中に呼び直された場合は、今の濃さと peakAlpha の濃い方から引き直す。
	/// 弱いフラッシュが強いフラッシュを途中で暗くしてしまうのを避けるため.
	///
	/// 【濃さと長さは控えめに】
	/// 全画面が真っ白に 1.0 で 0.5 秒光るような設定は、見づらいだけでなく
	/// 光過敏の人に負担をかける。被弾表現なら 0.2～0.4 程度、0.2 秒前後で足りる.
	/// </summary>
	static void Flash(const Cake::Vector3& color, float duration, float peakAlpha = 0.5f, Cake::Easing::EaseType ease = Cake::Easing::EaseType::EaseOutQuad);

	static bool IsFlashing();

private:
	// 全画面を覆う板1枚ぶんの、色と濃さの補間状態.
	struct Layer {
		Cake::Vector3 color = Cake::Vector3::Zero;
		float alpha = 0.0f;       // 現在の濃さ.
		float startAlpha = 0.0f;  // 補間の始点.
		float targetAlpha = 0.0f; // 補間の終点.
		float duration = 0.0f;    // 0 なら補間せず即座に targetAlpha へ.
		float elapsed = 0.0f;
		Cake::Easing::EaseType ease = Cake::Easing::EaseType::Linear;

		bool IsPlaying() const { return elapsed < duration; }

		void Start(const Cake::Vector3& newColor, float from, float to, float newDuration, Cake::Easing::EaseType newEase);
		void Update(float deltaTime);

		// 描画に渡す RGBA へ畳む.
		Cake::Vector4 ToColor() const { return {color.x, color.y, color.z, alpha}; }
	};

	static Layer fade_;
	static Layer flash_;

	// 白1x1。色を掛けて全画面へ伸ばすので、テクスチャは1枚で全ての色を賄える.
	static uint32_t textureHandle_;
	static KamataEngine::Sprite* fadeSprite_;
	static KamataEngine::Sprite* flashSprite_;

#pragma region デバッグ用
#ifdef USE_IMGUI
public:
	/// <summary>
	/// 手元で暗転や発光を試すためのウィンドウ。
	/// ImGuiManager::Begin()～End() の内側、つまりシーンの Update() から呼ぶ.
	/// </summary>
	static void DrawDebugUI(const char* windowName = "ScreenEffect");

private:
	static inline float debugColor_[3] = {0.0f, 0.0f, 0.0f};
	static inline float debugDuration_ = 0.5f;
	static inline float debugPeakAlpha_ = 0.5f;
#endif // USE_IMGUI
#pragma endregion
};
