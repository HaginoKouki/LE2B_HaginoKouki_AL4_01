#include "Rigidbody.h"

#include <algorithm>

#include "System/Platform/Time/Time.h"
#include "System/Scene/GameObject/GameObject.h"

namespace {

// 質量 0 は AddForce() で 0 除算になる。物理的な意味は無いが、下限を置いて塞ぐ.
constexpr float kMinMass = 0.0001f;

} // namespace

void RigidBody::Update(Cake::Time* time) {
	if (bodyType_ == BodyType::Static) {
		// 動かない種別。積まれた力ごと捨てる.
		velocity_ = Cake::Vector3::Zero;
		acceleration_ = Cake::Vector3::Zero;
		return;
	}

	const float deltaTime = (time != nullptr) ? time->GetDeltaTime() : 0.0f;
	if (deltaTime <= 0.0f) {
		// 止まっている間に力をため込むと、再開した瞬間に一気に効いてしまう.
		acceleration_ = Cake::Vector3::Zero;
		return;
	}

	// 接地の鮮度を進める。ResolveContact() が呼ばれるのは全 Update() が終わった後の
	// 衝突フェーズなので、ここで進めておけば次のフレームが正しい値を読む.
	// 上限を設けないと、走り続けたときに float の精度が落ちていく.
	timeSinceGrounded_ = (std::min)(timeSinceGrounded_ + deltaTime, kNotGrounded);

	// Kinematic は速度をそのまま位置に流すだけで、重力も力も減衰も受けない.
	if (bodyType_ == BodyType::Dynamic) {
		velocity_ += (kGravityAccel * gravityScale_ + acceleration_) * deltaTime;

		// 減衰。v -= v * drag * dt と書くと、drag * dt が 1 を超えた瞬間に符号が反転して発散する。
		// 割り算の形なら倍率が必ず 0〜1 に収まるので、値をいくら大きくしても壊れない.
		if (linearDrag_ > 0.0f) {
			velocity_ /= (1.0f + linearDrag_ * deltaTime);
		}
	}
	acceleration_ = Cake::Vector3::Zero;

	// 位置へ反映する。ワールド基準で動かしたいので AddWorldTranslate() を通す.
	if (owner_ != nullptr) {
		owner_->AddWorldTranslate(velocity_ * deltaTime);
	}
}

void RigidBody::AddForce(const Cake::Vector3& force, ForceMode mode) {
	// 重力すら受けない種別に力だけ効くのは筋が通らないので、まとめて弾く.
	if (bodyType_ != BodyType::Dynamic) {
		return;
	}

	switch (mode) {
		case ForceMode::Force:
			acceleration_ += force / mass_;
			break;
		case ForceMode::Impulse:
			velocity_ += force / mass_;
			break;
		case ForceMode::VelocityChange:
			velocity_ += force;
			break;
	}
}

void RigidBody::ResolveContact(const Cake::Vector3& normal) {
	if (bodyType_ == BodyType::Static) {
		return;
	}

	// 長さ 0 の法線は Normalize() が Zero を返す。以降の判定がすべて空振りするので安全.
	const Cake::Vector3 axis = Cake::Vector3::Normalize(normal);

	// 法線は自分を相手から引き離す向き。内積が正なら、すでに離れる向きに進んでいる.
	const float along = Cake::Vector3::DotProduct(velocity_, axis);
	if (along > 0.0f) {
		// ジャンプした直後に地面と重なっているフレームがここに来る。
		// ここで速度を消すと飛べなくなるうえ、接地したままになって多段ジャンプの原因になる.
		return;
	}

	// 面へめり込む成分だけを抜き、面に沿う成分は残す。
	// 斜面を滑り降りる動きや、床を走る動きはこれで自然に出る.
	velocity_ -= axis * along;

	// 上を向いた面に乗っているなら床とみなす。壁や天井では立たない.
	if (axis.y >= groundNormalY_) {
		timeSinceGrounded_ = 0.0f;
		groundNormal_ = axis;
	}
}

void RigidBody::ClearGroundContact() {
	timeSinceGrounded_ = kNotGrounded;
	groundNormal_ = Cake::Vector3::Zero;
}

void RigidBody::SetMass(float mass) {
	mass_ = (std::max)(mass, kMinMass);
}

void RigidBody::SetLinearDrag(float drag) {
	// 負の減衰は加速になってしまう.
	linearDrag_ = (std::max)(drag, 0.0f);
}

void RigidBody::SetGroundNormalY(float y) {
	// 法線は正規化済みなので、Y成分は -1〜1 の範囲にしか入らない。
	// 0 未満を許すと天井を床と誤認する.
	groundNormalY_ = std::clamp(y, 0.0f, 1.0f);
}

void RigidBody::SetCoyoteTime(float seconds) {
	coyoteTime_ = (std::max)(seconds, 0.0f);
}
