#pragma once
/*====================================
 *
 * 複数の対象を比重付きで追従するカメラ補助クラス。
 * baseObject は必ず可視範囲内に収まることを保証しつつ、
 * followTargets の比重に応じてカメラの注視点をそちらへ引き寄せる。
 *
 * 【3Dでの「画面内に収める」】
 * 引っ張られ量を、カメラから見た 右(x)・上(y)・奥(z) の成分に分けて、それぞれ上限で止める。
 * ワールドの X/Y/Z で止めると、カメラが回り込んだときに画面上の制限の形が崩れるため.
 *
 * ====================================*/
#include <vector>
#include "System/Foundation/Math/Vector.h"
#include "System/Foundation/Math/Matrix.h"

class GameObject;

class CameraFollow {
private:
	struct FollowTarget {
		GameObject* object = nullptr;
		float weight = 1.0f;
	};

	GameObject* baseObject_ = nullptr; // 必ず画面内に収める基準（プレイヤー等）.
	float baseWeight_ = 1.0f;          // 重心計算における baseObject 自身の重み.
	std::vector<FollowTarget> followTargets_;

	Cake::Vector3 followedPosition_ = Cake::Vector3::Zero;

public:
	/// <param name="cameraRotation">カメラの回転行列。行0が右、行1が上、行2が視線の向き</param>
	/// <param name="baseObjectHardLimitHalfSize">
	/// baseObject からの引っ張られ量の上限（カメラのローカル x: 右, y: 上, z: 奥。ワールド単位）。
	/// 通常は Camera 側の「注視点の位置で見える範囲の半分」に、余白を持たせる係数を掛けたもの.
	/// </param>
	void Update(const Cake::Matrix4x4& cameraRotation, const Cake::Vector3& baseObjectHardLimitHalfSize);

	bool IsActive() const { return baseObject_ != nullptr; }

	void SetBaseObject(GameObject* baseObject) { baseObject_ = baseObject; }
	void SetBaseWeight(float weight) { baseWeight_ = weight; }

	void AddFollowTarget(GameObject* target, float weight = 1.0f);
	void RemoveFollowTarget(const GameObject* target);

	/// <summary>
	/// object を基準・追従対象の両方から外す.
	/// ~GameObject() から自動で呼ばれるので、通常は手で呼ぶ必要はない.
	/// </summary>
	void Forget(const GameObject* object);

	Cake::Vector3 GetFollowedPosition() const { return followedPosition_; }

#pragma region デバッグ用
private:
	Cake::Vector3 desiredPosition_ = Cake::Vector3::Zero;   // クランプ前の重心.
	Cake::Vector3 hardLimitHalfSize_ = Cake::Vector3::Zero; // 直近に渡された上限.
	Cake::Vector3 localOffset_ = Cake::Vector3::Zero;       // クランプ前の引っ張られ量（カメラのローカル）.

public:
#ifdef USE_IMGUI
	/// <summary>
	/// 「Follow」セクションを描画する。ImGui::Begin()/End() は呼び出し側の責任.
	/// </summary>
	void DrawDebugUI();
#endif // USE_IMGUI
#pragma endregion
};
