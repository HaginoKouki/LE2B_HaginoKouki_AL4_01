#pragma once
/*====================================
 *
 * 動かない地形ブロック。床・壁・足場に使う.
 *
 * 【大きさは Setup() で決める】
 * モデルは共有の cube（±1 の立方体）を拡縮して使う.
 * 当たり判定（AABB）も同じワールド行列で拡縮されるので、見た目と判定がずれない.
 *
 * 【奥行きはプレイヤーより厚くしておく】
 * 押し出しは「最も浅くめり込んでいる軸」で行われる.
 * ブロックの奥行きがプレイヤーと同じ薄さだと、Z 方向が最も浅いと判定されて
 * 奥へ押し出されることがある。GameScene では奥行きを 2 以上にして置いている.
 *
 * ====================================*/
#include "System/Foundation/Math/Vector.h"
#include "System/Render/Renderer/ModelRenderer.h"
#include "System/Scene/Collider/Collider.h"
#include "System/Scene/GameObject/GameObject.h"

class Block : public GameObject {
private:
	// cube.obj の頂点は ±1。全体の大きさを拡縮へ直すときに割る.
	static constexpr float kModelHalfSize = 1.0f;

	ModelRenderer modelRenderer_;
	CollisionBody collisionBody_;

public:
	Block();
	~Block() override = default;

	void Initialize() override;
	void Update(Cake::Time* time) override;
	void Draw() override;

	/// <summary>
	/// 中心位置と全体の大きさ（幅・高さ・奥行き）を設定する.
	/// AddGameObject() の後に呼ぶ（Initialize() はその中で呼ばれている）.
	/// </summary>
	void Setup(const Cake::Vector3& center, const Cake::Vector3& size);

	// 乗算色。床と足場を見分けたいときなどに使う.
	void SetColor(const Cake::Vector4& color) { modelRenderer_.SetColor(color); }
};
