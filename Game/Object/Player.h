#pragma once
/*====================================
 *
 * 自機。2軸アクション（X: 左右移動、Y: ジャンプ・落下）の土台.
 *
 * 【動かし方】
 * 位置の積分は RigidBody に任せ、Update() では速度だけを決める.
 *   左右移動 : 入力の x をそのまま速度 X にする（加減速なし。欲しければここに足す）.
 *   ジャンプ : 接地中にボタンを押した瞬間、速度 Y を kJumpSpeed にする.
 *   可変ジャンプ : 上昇中にボタンを離したら、1回だけ速度 Y を kJumpCutRatio 倍に落とす.
 *   Z は毎フレーム 0 に固定する（押し出しで奥へずれても戻す）.
 *
 * 【地面との関係】
 * 押し出しは CollisionManager が行い、速度の停止と接地の記録は
 * OnCollisionEnter() / OnCollisionStay() から RigidBody::ResolveContact() を呼んで行う.
 * 接地判定は RigidBody::IsGrounded()（コヨーテタイム込み）を見る.
 *
 * 【向き】
 * facing_ が +1 で右、-1 で左。モデルの正面を +Z として、右向きは Y 軸回りに +90°回す.
 * （モデルを差し替えて向きが合わない場合は kRightRotationY / kLeftRotationY を直す）
 *
 * 【未実装】
 * 攻撃・回避は入力（Cake::Input::GetAttackButton() / GetDodgeButton()）だけ用意してある.
 * 攻撃判定を付ける場合は、CollisionLayer::PlayerAttack のトリガーの CollisionBody をもう1つ持たせる.
 *
 * ====================================*/
#include <numbers>

#include "System/Foundation/Math/Vector.h"
#include "System/Render/Renderer/ModelRenderer.h"
#include "System/Scene/Collider/Collider.h"
#include "System/Scene/GameObject/GameObject.h"
#include "System/Scene/Physics/Rigidbody.h"

class Player : public GameObject {
private:
	// cube.obj の頂点は ±1。当たり判定の大きさを拡縮へ直すときに割る.
	static constexpr float kModelHalfSize = 1.0f;
	// 自機の半径（当たり判定の箱の半分の大きさ）。1x1x1 の立方体になる.
	static constexpr float kHalfSize = 0.5f;

	// 左右の移動速度 [unit/s].
	static constexpr float kMoveSpeed = 6.0f;
	// ジャンプの初速 [unit/s].
	static constexpr float kJumpSpeed = 12.0f;
	// 上昇中にジャンプボタンを離したとき、速度 Y に掛ける倍率（可変ジャンプ）.
	static constexpr float kJumpCutRatio = 0.5f;
	// 落下速度の上限 [unit/s]。速すぎると薄い床をすり抜ける.
	static constexpr float kMaxFallSpeed = 25.0f;
	// 重力の倍率。1 だとふわっとするので強める.
	static constexpr float kGravityScale = 3.0f;

	// 右向き・左向きの Y 軸回転（ラジアン）。モデルの正面を +Z とした値.
	static constexpr float kRightRotationY = std::numbers::pi_v<float> / 2.0f;
	static constexpr float kLeftRotationY = -std::numbers::pi_v<float> / 2.0f;

	ModelRenderer modelRenderer_;
	CollisionBody collisionBody_;
	RigidBody rigidBody_;

	// 向いている方向。+1 で右、-1 で左.
	float facing_ = 1.0f;
	// ジャンプで上昇中か。可変ジャンプの減速を1回だけ掛けるために持つ.
	bool isJumping_ = false;

	void UpdateMove();
	void UpdateJump();
	// 地形との接触を速度へ反映する。Enter と Stay の両方から呼ぶ.
	void ResolveTerrainContact(const CollisionEvent3D& event);

public:
	Player();
	~Player() override = default;

	void Initialize() override;
	void Update(Cake::Time* time) override;
	void Draw() override;

	void OnCollisionEnter(const CollisionEvent3D& event) override;
	void OnCollisionStay(const CollisionEvent3D& event) override;

	// 初期位置。AddGameObject() の後に呼ぶ.
	void SetPosition(const Cake::Vector3& position) { localTransform_.translate = position; }

	bool IsGrounded() const { return rigidBody_.IsGrounded(); }
	float GetFacing() const { return facing_; }
	const RigidBody& GetRigidBody() const { return rigidBody_; }
};
