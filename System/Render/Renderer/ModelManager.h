#pragma once
/*====================================
 *
 * KamataEngine::Model を名前で読み込み、使い回すための置き場。
 *
 * 【なぜ置き場が要るか】
 * Model::CreateFromOBJ() は呼ぶたびにファイルを読み直し、頂点バッファを作り直す。
 * 同じ敵を10体出すたびに読むと遅いうえ、メモリも10倍になる。
 * Model は「形と材質」だけを持ち、位置は描画時に WorldTransform で渡すので、
 * 1つの Model を何体ぶんの描画にでも使い回せる。
 * （Sprite と違い、1つの Model を1フレームに何度描いてもよい）
 *
 * 【所有権】
 * 読み込んだ Model はここが持ち続ける。呼び出し側は返ってきたポインタを delete してはいけない.
 * シーンを跨いでも残るので、タイトルとゲームで同じモデルを読み直さずに済む.
 *
 * 【呼ぶ場所】
 * Load() : KamataEngine::Initialize() の後ならどこでも（通常は GameObject の Initialize()）.
 * Finalize() : KamataEngine::Finalize() の前に1回。Game が呼ぶ.
 *
 * ====================================*/
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

#include "KamataEngine.h"

class ModelManager {
public:
	// 状態は全て静的に持つので、実体は作らせない.
	ModelManager() = delete;

	/// <summary>
	/// OBJ モデルを読み込む。2回目以降は読み込み済みのものを返す.
	/// ファイルは "Resources/{name}/{name}.obj"（KamataEngine::Model::CreateFromOBJ() の規約）.
	/// </summary>
	/// <param name="name">モデル名。例: "cube" なら Resources/cube/cube.obj</param>
	/// <param name="smoothing">法線を平滑化するか。同じ名前でも別のモデルとして扱う</param>
	/// <returns>読み込んだモデル。所有権は ModelManager が持つ</returns>
	static KamataEngine::Model* Load(const std::string& name, bool smoothing = false);

	/// <summary>
	/// 球のモデルを作る。分割数ごとに使い回す。仮置きや当たり判定の見た目の確認に使う.
	/// </summary>
	static KamataEngine::Model* LoadSphere(uint32_t division = 16);

	/// <summary>
	/// 読み込んだ全てのモデルを破棄する。以降、それまでに返したポインタは使えない.
	/// </summary>
	static void Finalize();

private:
	// 静的メンバの初期化順に巻き込まれないよう、関数内の static で持つ.
	static std::unordered_map<std::string, std::unique_ptr<KamataEngine::Model>>& GetModels();
};
