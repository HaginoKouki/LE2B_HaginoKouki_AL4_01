#pragma once
/*====================================
 *
 * GameObject に持たせて 3D モデルを描くコンポーネント。
 * ワールド行列と Camera から KamataEngine::Model::Draw() の引数を組み立てて描画する。
 *
 * 【なぜ GameObject ごとに持つか】
 * Model（形と材質）は ModelManager から借りて共有するが、
 * 位置（WorldTransform）と色（ObjectColor）はそれぞれ定数バッファを1組しか持たない。
 * 描画コマンドが実行される頃には最後に書いた値で上書きされているので、
 * 描く物体の数だけ実体が要る。CollisionBody や Animator と同じく、オーナーがメンバとして持つ.
 *
 * 【1つの ModelRenderer を1フレームに複数回描いてはいけない】
 * 上記の理由で、2回描くと2回とも最後に渡した位置・色で描かれる.
 *
 * 【使い方】
 *   // メンバ
 *   ModelRenderer modelRenderer_;
 *   // Initialize()
 *   modelRenderer_.Initialize(ModelManager::Load("cube"));
 *   // Draw()
 *   modelRenderer_.Draw(*GetCamera(), GetWorldMatrix());
 *
 * ====================================*/
#include <cstdint>

#include "KamataEngine.h"

#include "System/Foundation/Math/Vector.h"
#include "System/Foundation/Math/Matrix.h"

class Camera;

class ModelRenderer {
private:
	// 共有のモデル。所有しない（ModelManager が持つ）.
	KamataEngine::Model* model_ = nullptr;

	// 個体ごとの定数バッファ。Initialize() で作る.
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::ObjectColor objectColor_;
	bool isInitialized_ = false;

	Cake::Vector4 color_ = Cake::Vector4::One;

	// テクスチャの差し替え。false ならモデルのマテリアルのテクスチャを使う.
	uint32_t textureHandle_ = 0;
	bool hasTextureOverride_ = false;

	bool isVisible_ = true;

	// 視錐台カリングに使う半径（モデルのローカル単位。ワールド行列の拡縮が掛かる）。0 以下ならカリングしない.
	float cullingRadius_ = 0.0f;

public:
	ModelRenderer() = default;

	// 定数バッファを抱えているので、複製すると同じバッファを2体で奪い合う.
	ModelRenderer(const ModelRenderer&) = delete;
	ModelRenderer& operator=(const ModelRenderer&) = delete;

	/// <summary>
	/// 定数バッファを作り、描くモデルを設定する。KamataEngine::Initialize() より後に呼ぶこと.
	/// </summary>
	/// <param name="model">ModelManager::Load() の戻り値。nullptr なら何も描かない</param>
	void Initialize(KamataEngine::Model* model);

	/// <summary>
	/// ワールド行列とカメラを使って描画する。Model::PreDraw()～PostDraw() の内側、つまり GameObject::Draw() から呼ぶ.
	/// </summary>
	/// <param name="camera">描画に使うカメラ。デバッグカメラ中はそちらで描かれる</param>
	/// <param name="worldMatrix">GameObject::GetWorldMatrix() の戻り値</param>
	void Draw(const Camera& camera, const Cake::Matrix4x4& worldMatrix);

	void SetModel(KamataEngine::Model* model) { model_ = model; }
	KamataEngine::Model* GetModel() const { return model_; }

	// 乗算色（RGBA）。α で半透明にできる.
	void SetColor(const Cake::Vector4& color) { color_ = color; }
	const Cake::Vector4& GetColor() const { return color_; }

	/// <summary>
	/// テクスチャを差し替える。Animator::SetTextureSetter() からも使える.
	/// </summary>
	void SetTexture(uint32_t textureHandle);
	// 差し替えをやめて、モデル本来のテクスチャに戻す.
	void ClearTexture() { hasTextureOverride_ = false; }

	void SetVisible(bool isVisible) { isVisible_ = isVisible; }
	bool IsVisible() const { return isVisible_; }

	/// <summary>
	/// 視錐台カリングの半径を設定する。モデルの原点からの、ローカル単位での大きさ.
	/// 0 以下ならカリングしない（既定）。大きめに取っておけば、見えているのに消えることはない.
	/// </summary>
	void SetCullingRadius(float radius) { cullingRadius_ = radius; }
};
