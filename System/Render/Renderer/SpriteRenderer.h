#pragma once
/*====================================
 *
 * KamataEngine::Sprite の描画パラメータを組み立てて描画する。UI と全画面演出の担当。
 * 3D の物体は ModelRenderer が描く。スプライトは 3D の後に、画面へ重ねて描かれる.
 *
 * 【なぜ切り出すか】
 * 回転の符号反転や、3D 座標からスクリーン座標への投影といった「座標系の都合」を
 * 1箇所に閉じ込めるため。各 GameObject の DrawUI() に散らすと、直すときに全部を触ることになる。
 *
 * 【DrawAtWorld() について】
 * 敵の頭上の HP バーのように、3D 空間の位置に貼り付く UI を描く。
 * 位置だけを投影し、大きさは距離に関係なくピクセル指定のまま描く（ビルボードではない）.
 *
 * 【DrawFullScreen() について】
 * フェードやフラッシュのような全画面演出のための、色付き板1枚を置く関数。
 * レンダーターゲットを読み直すポストプロセスではない。KamataEngine::Sprite は
 * ブレンドモードもシェーダも差し替えられないので、ぼかしや色収差はここでは扱えない。
 * 演出の状態（今どのくらい濃いか）は持たず、渡された色で塗るだけに徹する。
 * 状態は ScreenEffect が持つ。
 *
 * 【1つの Sprite を1フレームに複数回描いてはいけない】
 * KamataEngine::Sprite は定数バッファを1組しか持たず、描画コマンドが実行される頃には
 * 最後に書いた位置・色で上書きされている。同じ見た目の敵を10体描く場合でも、
 * Sprite の実体は1体につき1つずつ用意すること（テクスチャハンドルは共有してよい）。
 *
 * ====================================*/
#include "KamataEngine.h"

#include "System/Foundation/Math/Vector.h"
#include "System/Foundation/Math/Transform.h"

class Camera;

class SpriteRenderer {
public:
	// 状態を持たないので実体は作らせない.
	SpriteRenderer() = delete;

	/// <summary>
	/// カメラを通さず、スクリーン座標にそのまま描画する。UI に使う.
	/// 引数の translate はスクリーン座標(左上原点・Y下向き)としてそのまま扱う.
	/// </summary>
	static void DrawScreen(KamataEngine::Sprite* sprite, const Cake::Transform2& scs, const Cake::Vector2& baseSize);

	/// <summary>
	/// 3D 空間の位置を画面へ投影し、その位置にスプライトを描画する。頭上の HP バーなどに使う.
	/// 位置がカメラの後ろにある場合は描画コマンドを積まずに抜ける.
	/// </summary>
	/// <param name="camera">投影に使うカメラ</param>
	/// <param name="sprite">描画するスプライト。アンカーは中心({0.5f, 0.5f})を想定</param>
	/// <param name="worldPosition">貼り付けるワールド座標</param>
	/// <param name="screenOffset">投影した位置からのずらし量(ピクセル。Y下向き)</param>
	/// <param name="size">表示サイズ(ピクセル)。距離では変わらない</param>
	static void DrawAtWorld(const Camera& camera, KamataEngine::Sprite* sprite, const Cake::Vector3& worldPosition, const Cake::Vector2& screenOffset, const Cake::Vector2& size);

	/// <summary>
	/// バックバッファ全体を1色で塗りつぶす。フェード・フラッシュ用.
	/// α が 0 以下なら、描いても結果が変わらないので描画コマンドを積まずに抜ける.
	///
	/// 【渡すスプライトは専用のものを用意すること】
	/// この関数はアンカーポイントと色を書き換える。他の用途と共有すると、
	/// そちらの描画まで全画面の白板になる。
	/// また、同じスプライトを1フレームに2回渡してはいけない。
	/// Sprite が持つ定数バッファは1組しかなく、コマンドリストが実行される頃には
	/// 最後に書いた色で上書きされているため、2回とも同じ色で描かれる。
	/// </summary>
	/// <param name="sprite">塗りつぶしに使うスプライト。white1x1.png を想定</param>
	/// <param name="color">RGBA。α が覆いの濃さになる</param>
	static void DrawFullScreen(KamataEngine::Sprite* sprite, const Cake::Vector4& color);

private:
	static void DrawSprite(KamataEngine::Sprite* sprite, const Cake::Vector2& position, float rotation, const Cake::Vector2& size);
};
