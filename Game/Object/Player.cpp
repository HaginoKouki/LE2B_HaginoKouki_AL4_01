#include "Player.h"

#include "System/Platform/Input/Input.h"
#include "System/Platform/Time/Time.h"
#include "System/Render/Camera/Camera.h"
#include "System/Render/Renderer/ModelManager.h"
#include "System/Scene/Collider/CollisionLayer.h"

Player::Player() : collisionBody_(this), rigidBody_(this) {
}

void Player::Initialize() {
	layer_ = GameObjectLayer::Player;

	modelRenderer_.Initialize(ModelManager::Load("animal-cat"));
	// 共有の cube（±1）を kHalfSize の箱に縮める.
	localTransform_.scale = Cake::Vector3::One * (kHalfSize / kModelHalfSize);
	localTransform_.rotate.y = kRightRotationY;

	// 当たり判定。モデルと同じ ±1 の箱を置けば、拡縮後に kHalfSize の箱になる.
	collisionBody_.AddCollider(Cake::AABB{
		{-kModelHalfSize, -kModelHalfSize, -kModelHalfSize},
		{kModelHalfSize, kModelHalfSize, kModelHalfSize},
	});
	collisionBody_.SetLayer(CollisionLayer::Player);
	// 自分の攻撃判定（PlayerAttack）とは当たらない.
	collisionBody_.SetMask(CollisionLayer::Terrain | CollisionLayer::Enemy | CollisionLayer::EnemyAttack);
	collisionBody_.AttachCollisionManager(GetCollisionManager());

	rigidBody_.SetBodyType(BodyType::Dynamic);
	rigidBody_.SetGravityScale(kGravityScale);
}

void Player::Update(Cake::Time* time) {
	UpdateMove();
	UpdateJump();

	// 落下速度に上限を掛けてから積分する.
	const Cake::Vector3 velocity = rigidBody_.GetVelocity();
	if (velocity.y < -kMaxFallSpeed) {
		rigidBody_.SetVelocityY(-kMaxFallSpeed);
	}
	rigidBody_.Update(time);

	// 2軸なので奥行きは常に 0。押し出しで奥へずれても、次のフレームには戻る.
	localTransform_.translate.z = 0.0f;
	rigidBody_.SetVelocityZ(0.0f);
}

void Player::UpdateMove() {
	const Cake::Vector2 axis = Cake::Input::GetMoveAxis();

	// 入力をそのまま速度にする。慣性を付けたい場合は、ここを目標速度への補間にする.
	rigidBody_.SetVelocityX(axis.x * kMoveSpeed);

	// 入力がある間だけ向きを更新する。離しても最後の向きを保つ.
	if (axis.x > 0.0f) {
		facing_ = 1.0f;
	} else if (axis.x < 0.0f) {
		facing_ = -1.0f;
	}
	localTransform_.rotate.y = (facing_ > 0.0f) ? kRightRotationY : kLeftRotationY;
}

void Player::UpdateJump() {
	if (rigidBody_.IsGrounded()) {
		isJumping_ = false;
		if (Cake::Input::GetJumpButton()) {
			rigidBody_.SetVelocityY(kJumpSpeed);
			// 消さないと、コヨーテタイムの猶予のあいだ何度でも飛べてしまう.
			rigidBody_.ClearGroundContact();
			isJumping_ = true;
		}
		return;
	}

	// 上昇中にボタンを離したら減速する。1回だけ掛けるので、押し直しても再加速はしない.
	const float velocityY = rigidBody_.GetVelocity().y;
	if (isJumping_ && velocityY > 0.0f && !Cake::Input::IsJumpButtonHeld()) {
		rigidBody_.SetVelocityY(velocityY * kJumpCutRatio);
		isJumping_ = false;
	}
}

void Player::Draw() {
	modelRenderer_.Draw(*GetCamera(), GetWorldMatrix());
}

void Player::OnCollisionEnter(const CollisionEvent3D& event) {
	ResolveTerrainContact(event);
}

void Player::OnCollisionStay(const CollisionEvent3D& event) {
	ResolveTerrainContact(event);
}

void Player::ResolveTerrainContact(const CollisionEvent3D& event) {
	// 相手のボディが先に破棄された場合は nullptr が来る.
	if (event.otherBody == nullptr) {
		return;
	}
	// トリガーは押し出されないので、速度まで止めると空中で引っかかる.
	if (event.otherBody->IsTrigger()) {
		return;
	}
	if (event.otherBody->GetLayer() != CollisionLayer::Terrain) {
		return;
	}
	// normal は自分を相手から引き離す向き。床なら上向きになり、接地が記録される.
	rigidBody_.ResolveContact(event.info.normal);
}
