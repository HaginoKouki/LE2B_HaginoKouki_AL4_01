#include "Geometry.h"

#include <numbers>
#include <cmath>

namespace Cake {

Vector3 GetNormal(const Triangle& triangle) {
	Vector3 v01 = triangle.vertices[1] - triangle.vertices[0];	// 頂点0から頂点1へのベクトル.
	Vector3 v12 = triangle.vertices[2] - triangle.vertices[1];	// 頂点1から頂点2へのベクトル.
	return Vector3::Normalize(Vector3::CrossProduct(v01, v12));
}



Plane GetPlane(const Vector3& normal, const Vector3& point) {
	Plane plane;
	plane.normal = Vector3::Normalize(normal);
	plane.distance = Vector3::DotProduct(plane.normal, point);
	return plane;
}
Plane GetPlane(const Triangle& triangle) {
	return GetPlane(GetNormal(triangle), triangle.vertices[0]);
}

}	// namespace Cake
