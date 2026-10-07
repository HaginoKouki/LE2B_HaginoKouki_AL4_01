// System/Scene/Physics/Rigidbody.h
#pragma once
/*====================================
 *
 * 速度を持ち、毎フレーム位置へ積分するコンポーネント。
 * ノックバックや加減速を各オブジェクトで書き直さずに済ませるために置く。
 *
 * 【誰が呼ぶか】
 * オーナーの Update() の中から Update() を呼ぶ。Animator と同じ扱い。
 * 位置の反映は GameObject::AddWorldTranslate() を通すので、親子関係があっても崩れない。
 *
 * 【衝突との関係】
 * CollisionManager がやるのは位置の押し出しだけで、速度には触れない。
 * オーナーの OnCollisionEnter() / OnCollisionStay() から ResolveContact() を呼んで、
 * 初めて壁や地面で速度が止まり、接地が記録される。
 *
 * 【接地が1フレーム遅れる件】
 * SceneBase は「全オブジェクトの Update() → CollisionManager::Update()」の順に回す。
 * つまり ResolveContact() が呼ばれるのは、その結果を読みたい Update() より後になる。
 * そのため接地は bool ではなく「最後に床へ触れてからの経過時間」で持ち、
 * coyoteTime_ の猶予でこのズレを吸収している。
 * 崖際で少しの間ジャンプできる、いわゆるコヨーテタイムを兼ねる。
 *
 * ====================================*/
#include "System/Foundation/Math/Vector.h"

class GameObject;

namespace Cake {
class Time;
}

// 重力加速度 [unit/s^2]。Y+ が上なので、下向きは負.
// 1unit = 1m とみなした地球の重力。ふわっとして感じる場合は SetGravityScale() で強める
// （アクションゲームでは 2～3 倍にすることが多い）.
// constexpr にしていないのは Cake::Vector3 のコンストラクタが constexpr ではないため.
inline const Cake::Vector3 kGravityAccel{0.0f, -9.8f, 0.0f};

// 物体の種別。Unity の Rigidbody2D.bodyType と同じ分け方にしてある.
enum class BodyType {
	Dynamic,   //!< 重力と力を受ける。プレイヤーや敵など.
	Kinematic, //!< 速度は持つが、重力も力も受けない。演出で動かす物や、押されたくない物.
	Static,    //!< 動かない。地形など.
};

// AddForce() に渡した値をどう解釈するか.
enum class ForceMode {
	Force,          //!< 継続的な力。質量で割ったうえ、さらに時間で積分される.
	Impulse,        //!< 瞬間的な力積。質量で割って速度へ直接足す。ノックバック向け.
	VelocityChange, //!< 質量を無視して速度へ直接足す。重さで差を付けたくない場合.
};

class RigidBody {
private:
	// 「接地していない」を表す、十分に大きな経過時間 [s].
	static constexpr float kNotGrounded = 1000.0f;

	GameObject* owner_ = nullptr;

	BodyType bodyType_ = BodyType::Dynamic;

	// ワールド基準の速度 [unit/s].
	Cake::Vector3 velocity_ = Cake::Vector3::Zero;
	// このフレームに積まれた加速度。Update() で使い切って 0 に戻す.
	Cake::Vector3 acceleration_ = Cake::Vector3::Zero;

	// 質量 [kg]。AddForce() の割り算にしか使わないので、0 以下は許さない.
	float mass_ = 1.0f;
	// 速度の減衰の強さ。0 で減衰なし、大きいほど早く止まる.
	float linearDrag_ = 0.0f;
	// 重力の倍率。0 で無重力、1 で kGravityAccel そのまま.
	float gravityScale_ = 1.0f;

	// 床とみなす法線のY成分の下限。cos(45度) ≒ 0.707 なので、既定では45度より急な面は壁扱い.
	float groundNormalY_ = 0.7f;
	// 最後に床へ触れてからの経過時間 [s].
	float timeSinceGrounded_ = kNotGrounded;
	// 接地を認める猶予時間 [s]。フレームのズレの吸収とコヨーテタイムを兼ねる.
	float coyoteTime_ = 0.05f;
	// 最後に触れた床の法線。斜面の扱いや、着地エフェクトの向きに使える.
	Cake::Vector3 groundNormal_ = Cake::Vector3::Zero;

public:
	explicit RigidBody(GameObject* owner) : owner_(owner) {}

	// owner_ を握っているので、コピーすると別のオブジェクトを指したまま複製されてしまう.
	RigidBody(const RigidBody&) = delete;
	RigidBody& operator=(const RigidBody&) = delete;

	/// <summary>
	/// 速度を更新し、位置へ反映する。オーナーの Update() から1フレームに1回だけ呼ぶ.
	/// </summary>
	void Update(Cake::Time* time);

	/// <summary>
	/// 力を加える。Dynamic 以外では何も起こらない.
	/// </summary>
	void AddForce(const Cake::Vector3& force, ForceMode mode = ForceMode::Force);

	/// <summary>
	/// 衝突1件を速度へ反映する。面へめり込む向きの成分だけを消し、
	/// 上を向いた面なら接地として記録する。
	/// 引数には CollisionEvent3D::info.normal をそのまま渡す。
	/// トリガー相手のイベントを渡してはいけない。押し出されないのに速度だけ死ぬ.
	/// </summary>
	void ResolveContact(const Cake::Vector3& normal);

	/// <summary>
	/// 接地の記録を消す。ジャンプした直後に呼ぶ。
	/// 呼ばないと、coyoteTime_ の猶予のあいだ何度でも飛べてしまう.
	/// </summary>
	void ClearGroundContact();

	bool IsGrounded() const { return timeSinceGrounded_ <= coyoteTime_; }
	const Cake::Vector3& GetGroundNormal() const { return groundNormal_; }

	void SetBodyType(BodyType type) { bodyType_ = type; }
	void SetVelocity(const Cake::Vector3& velocity) { velocity_ = velocity; }
	// 水平移動とジャンプのように、一部の軸だけ差し替えたい場面が多いので個別にも用意する.
	void SetVelocityX(float x) { velocity_.x = x; }
	void SetVelocityY(float y) { velocity_.y = y; }
	void SetVelocityZ(float z) { velocity_.z = z; }
	// 水平方向（X と Z）だけ差し替え、落下中の Y は残す。入力による移動に使う.
	void SetVelocityXZ(const Cake::Vector3& horizontal) {
		velocity_.x = horizontal.x;
		velocity_.z = horizontal.z;
	}
	void SetMass(float mass);
	void SetLinearDrag(float drag);
	void SetGravityScale(float scale) { gravityScale_ = scale; }
	void SetGroundNormalY(float y);
	void SetCoyoteTime(float seconds);

	BodyType GetBodyType() const { return bodyType_; }
	const Cake::Vector3& GetVelocity() const { return velocity_; }
	float GetMass() const { return mass_; }
	float GetLinearDrag() const { return linearDrag_; }
	float GetGravityScale() const { return gravityScale_; }
};
