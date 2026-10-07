#include "Collider.h"

#include <algorithm>
#include <cmath>
#include <type_traits>

#include "System/Foundation/Math/Collision3D.h"
#include "System/Scene/Collider/CollisionManager.h"

namespace {

// これ以下の長さの軸は潰れている（拡縮0）とみなす.
constexpr float kEpsilon = 1e-6f;
// 各軸がワールド軸とこれ以上揃っていれば（内積の絶対値）、回転なしとみなして AABB のまま扱う.
constexpr float kAxisAlignedThreshold = 1.0f - 1e-5f;

Cake::Vector3 NormalizeOr(const Cake::Vector3& value, const Cake::Vector3& fallback) {
	const float length = Cake::Vector3::Length(value);
	return (length > kEpsilon) ? value / length : fallback;
}

// axis に直交する単位ベクトルを1本返す。向きは問わない.
Cake::Vector3 AnyPerpendicular(const Cake::Vector3& axis) {
	const Cake::Vector3 helper = (std::abs(axis.x) < 0.9f) ? Cake::Vector3::UnitX : Cake::Vector3::UnitY;
	return Cake::Vector3::Normalize(Cake::Vector3::CrossProduct(helper, axis));
}

// 3軸を直交する単位ベクトルに整える（グラム・シュミット）.
// 非一様スケールの親の下で子が回転すると、軸が直交しなくなる（せん断）。
// OBB は直交軸が前提なので、ずれは近似として受け入れて整え直す.
void Orthonormalize(Cake::Vector3 (&axes)[3]) {
	axes[0] = NormalizeOr(axes[0], Cake::Vector3::UnitX);
	axes[1] = NormalizeOr(axes[1] - axes[0] * Cake::Vector3::DotProduct(axes[1], axes[0]), AnyPerpendicular(axes[0]));
	// 箱は中心について対称なので、軸の符号（鏡像）は気にしなくてよい.
	axes[2] = Cake::Vector3::CrossProduct(axes[0], axes[1]);
}

// ワールド行列を、直交する3軸と各軸の拡縮に分ける.
struct Basis {
	Cake::Vector3 axes[3];
	Cake::Vector3 scale;
	bool isAxisAligned = true;
};

Basis Decompose(const Cake::Matrix4x4& matrix) {
	Basis basis;
	// 行ベクトル形式なので、i 行目がローカルの i 軸をワールドへ写したものになる.
	for (int i = 0; i < 3; ++i) {
		basis.axes[i] = {matrix.m[i][0], matrix.m[i][1], matrix.m[i][2]};
	}
	basis.scale = {
		Cake::Vector3::Length(basis.axes[0]),
		Cake::Vector3::Length(basis.axes[1]),
		Cake::Vector3::Length(basis.axes[2]),
	};
	Orthonormalize(basis.axes);

	basis.isAxisAligned =
		std::abs(basis.axes[0].x) >= kAxisAlignedThreshold &&
		std::abs(basis.axes[1].y) >= kAxisAlignedThreshold &&
		std::abs(basis.axes[2].z) >= kAxisAlignedThreshold;
	return basis;
}

// 球は非一様スケールを表現できないので、大きいほうの倍率を採用する.
float GetUniformScale(const Cake::Vector3& scale) {
	return (std::max)({std::abs(scale.x), std::abs(scale.y), std::abs(scale.z)});
}

// 負のスケールでも半サイズが反転しないように絶対値を取る.
Cake::Vector3 ScaleHalfSize(const Cake::Vector3& halfSize, const Cake::Vector3& scale) {
	return {
		std::abs(halfSize.x * scale.x),
		std::abs(halfSize.y * scale.y),
		std::abs(halfSize.z * scale.z),
	};
}

Cake::AABB MergeBounds(const Cake::AABB& a, const Cake::AABB& b) {
	return Cake::AABB{
		{(std::min)(a.min.x, b.min.x), (std::min)(a.min.y, b.min.y), (std::min)(a.min.z, b.min.z)},
		{(std::max)(a.max.x, b.max.x), (std::max)(a.max.y, b.max.y), (std::max)(a.max.z, b.max.z)}
	};
}

} // namespace

void Collider::UpdateWorldShape(const Cake::Matrix4x4& worldMatrix) {
	const Basis basis = Decompose(worldMatrix);

	worldShape_ = std::visit([&worldMatrix, &basis](const auto& shape) -> Shape3D {
		using ShapeType = std::decay_t<decltype(shape)>;

		if constexpr (std::is_same_v<ShapeType, Cake::Sphere>) {
			return Cake::Sphere{
				Cake::Vector3::Transform(shape.center, worldMatrix),
				shape.radius * GetUniformScale(basis.scale)
			};
		} else if constexpr (std::is_same_v<ShapeType, Cake::AABB>) {
			const Cake::Vector3 center = Cake::Vector3::Transform((shape.min + shape.max) * 0.5f, worldMatrix);
			const Cake::Vector3 halfSize = ScaleHalfSize((shape.max - shape.min) * 0.5f, basis.scale);

			if (basis.isAxisAligned) {
				// 回転がなければ軸平行のまま扱えるので、高速な判定を通せる.
				return Cake::AABB{center - halfSize, center + halfSize};
			}
			// 回転が入ると軸平行では表現できないので OBB へ格上げする.
			Cake::OBB obb;
			obb.center = center;
			for (int i = 0; i < 3; ++i) {
				obb.orientations[i] = basis.axes[i];
			}
			obb.size = halfSize;
			return obb;
		} else {
			// OBB 自身の軸をワールドへ写す。写した軸の長さが、その向きの拡縮になる.
			Cake::OBB obb;
			obb.center = Cake::Vector3::Transform(shape.center, worldMatrix);
			const float localSize[3] = {std::abs(shape.size.x), std::abs(shape.size.y), std::abs(shape.size.z)};
			float worldSize[3] = {};
			for (int i = 0; i < 3; ++i) {
				const Cake::Vector3 axis = Cake::Vector3::TransformNormal(shape.orientations[i], worldMatrix);
				worldSize[i] = localSize[i] * Cake::Vector3::Length(axis);
				obb.orientations[i] = axis;
			}
			Orthonormalize(obb.orientations);
			obb.size = {worldSize[0], worldSize[1], worldSize[2]};
			return obb;
		}
	},
	                         localShape_);
}

Cake::AABB Collider::GetWorldBounds() const {
	return std::visit([](const auto& shape) -> Cake::AABB {
		return Cake::GetBounds(shape);
	},
	                  worldShape_);
}

void CollisionBody::AttachCollisionManager(CollisionManager* manager) {
	if (manager_ == manager) {
		return;
	}
	DetachManager();
	manager_ = manager;
	if (manager_) {
		manager_->Register(this);
	}
}

void CollisionBody::DetachManager() {
	// 解除を忘れると、マネージャ側のポインタが宙に浮く.
	if (manager_) {
		manager_->Unregister(this);
		manager_ = nullptr;
	}
}

void CollisionBody::UpdateWorldShapes(const Cake::Matrix4x4& ownerWorldMatrix) {
	// 判定固有のずらしを先に、オーナーの姿勢を後に掛ける.
	const Cake::Matrix4x4 worldMatrix = Cake::Math::MakeAffineMatrix(localTransform_) * ownerWorldMatrix;

	if (colliders_.empty()) {
		const Cake::Vector3 position = Cake::Math::GetTranslate(worldMatrix);
		bounds_ = Cake::AABB{position, position};
		return;
	}

	for (Collider& collider : colliders_) {
		collider.UpdateWorldShape(worldMatrix);
	}

	bounds_ = colliders_[0].GetWorldBounds();
	for (size_t i = 1; i < colliders_.size(); ++i) {
		bounds_ = MergeBounds(bounds_, colliders_[i].GetWorldBounds());
	}
}

bool CollisionBody::CanCollideWith(const CollisionBody& other) const {
	if (!isEnabled_ || !other.isEnabled_) {
		return false;
	}
	// 同一オブジェクトが複数のボディを持つ場合に、自分同士で当たらないようにする.
	if (owner_ == other.owner_) {
		return false;
	}
	// どちらも動かないなら、判定しても解決すべきことがない.
	if (isStatic_ && other.isStatic_) {
		return false;
	}
	// 双方が相手を見ている場合だけ成立させる。片方向にすると検出が非対称になる.
	if ((ToMask(layer_) & other.mask_) == 0) {
		return false;
	}
	if ((ToMask(other.layer_) & mask_) == 0) {
		return false;
	}
	return true;
}
