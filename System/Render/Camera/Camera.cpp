#include "Camera.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "System/Foundation/Math/Convert.h"
#include "System/Foundation/Math/Transform.h"
#include "System/Platform/Time/Time.h"

#ifdef CAKE_ENABLE_DEBUG_CAMERA
#include <imgui.h>

#include "System/Platform/Input/Input.h"
#endif // CAKE_ENABLE_DEBUG_CAMERA

namespace {
// 範囲が逆転している場合（マップが狭いなど）は、std::clamp に渡すと未定義動作になる.
// その軸は範囲の中央に固定する.
float ClampAxis(float value, float lower, float upper) {
	if (upper < lower) {
		return (lower + upper) * 0.5f;
	}
	return std::clamp(value, lower, upper);
}

// 回転行列の i 行目。行ベクトル形式なので、カメラのローカル i 軸がワールドでどちらを向くかになる.
Cake::Vector3 GetRow(const Cake::Matrix4x4& matrix, int row) {
	return Cake::Vector3{matrix.m[row][0], matrix.m[row][1], matrix.m[row][2]};
}

// 追従対象が画面中心からどのくらい端へずれてよいか。見える範囲の半分に掛ける係数.
constexpr float kFollowHardLimitRatio = 0.5f;

// カメラの回転行列。ロール(Z) → 見下ろし(X) → 水平の向き(Y) の順に回す.
// 物体と同じ X → Y → Z の順にすると、横を向いた状態でロールがワールドの Z 軸回りに掛かり、
// 傾くのではなく見上げてしまう。カメラはロールを自分の視線の軸回りに掛けたいので順番を変えている.
Cake::Matrix4x4 MakeCameraRotationMatrix(const Cake::Vector3& rotation) {
	return Cake::Matrix4x4::MakeZRotationMatrix(rotation.z) *
	       Cake::Matrix4x4::MakeXRotationMatrix(rotation.x) *
	       Cake::Matrix4x4::MakeYRotationMatrix(rotation.y);
}
} // namespace

Camera::Camera() {
	Initialize(KamataEngine::WinApp::kWindowWidth, KamataEngine::WinApp::kWindowHeight);
}
Camera::Camera(int windowWidth, int windowHeight) {
	Initialize(windowWidth, windowHeight);
}

Camera::~Camera() {
#ifdef CAKE_ENABLE_DEBUG_CAMERA
	// 飛んでいる最中にシーンが切り替わっても、ゲームの入力が止まったままにならないようにする.
	Cake::Input::SetGameInputEnabled(true);
#endif // CAKE_ENABLE_DEBUG_CAMERA
}

void Camera::Initialize(int windowWidth, int windowHeight) {
	width_ = windowWidth;
	height_ = windowHeight;

	target_ = Cake::Vector3::Zero;
	rotation_ = Cake::Vector3::Zero;
	distance_ = kDefaultDistance;

	fovY_ = kDefaultFovY;
	nearZ_ = kDefaultNearZ;
	farZ_ = kDefaultFarZ;

	cameraShake_.Initialize();

	// 定数バッファを作る。以降は UpdateMatrices() が書き込む.
	camera_.Initialize();
	UpdateMatrices();
}

void Camera::Update(Cake::Time* time) {
	UpdateInternal(time, nullptr);
}

void Camera::Update(Cake::Time* time, const Cake::AABB& clamp) {
	UpdateInternal(time, &clamp);
}

void Camera::UpdateInternal(Cake::Time* time, const Cake::AABB* clamp) {
#ifdef USE_IMGUI
	hasLastClamp_ = clamp != nullptr;
	if (clamp) {
		lastClamp_ = *clamp;
	}
#endif // USE_IMGUI

	const float deltaTime = time ? time->GetDeltaTime() : 0.0f;

	// 追従対象が画面中心からどのくらい端にズレられるか。
	// 右・上は見える範囲の半分に係数を掛けた量、奥行きは目が対象を追い越さないよう距離の半分まで.
	const Cake::Vector2 visibleHalfSize = GetVisibleSize() * 0.5f;
	const Cake::Vector3 hardLimitHalfSize{
		visibleHalfSize.x * kFollowHardLimitRatio,
		visibleHalfSize.y * kFollowHardLimitRatio,
		distance_ * 0.5f,
	};

	cameraFollow_.Update(MakeCameraRotationMatrix(rotation_), hardLimitHalfSize);
	if (cameraFollow_.IsActive()) {
		target_ = cameraFollow_.GetFollowedPosition();
	}
	cameraShake_.Update(deltaTime);

	// 端部処理.
	if (clamp) {
		target_.x = ClampAxis(target_.x, clamp->min.x, clamp->max.x);
		target_.y = ClampAxis(target_.y, clamp->min.y, clamp->max.y);
		target_.z = ClampAxis(target_.z, clamp->min.z, clamp->max.z);
	}

#ifdef CAKE_ENABLE_DEBUG_CAMERA
	UpdateDebugCamera(time);
#endif // CAKE_ENABLE_DEBUG_CAMERA

	UpdateMatrices();
}

void Camera::UpdateMatrices() {
	rotationMatrix_ = MakeCameraRotationMatrix(rotation_);

	// 注視点から視線と逆向きに下がり、シェイクの分だけ画面に平行にずらす.
	const Cake::Vector2& shake = cameraShake_.GetShakedOffset();
	eyePosition_ = target_ - GetForward() * distance_ + GetRight() * shake.x + GetUp() * shake.y;

	worldMatrix_ = rotationMatrix_ * Cake::Matrix4x4::MakeTranslateMatrix(eyePosition_);
	projectionMatrix_ = Cake::Matrix4x4::MakePerspectiveFovMatrix(fovY_, GetAspectRatio(), nearZ_, farZ_);
	viewportMatrix_ = Cake::Matrix4x4::MakeViewportMatrix(0.0f, 0.0f, static_cast<float>(width_), static_cast<float>(height_), 0.0f, 1.0f);

	// 描画に使う視点。通常はゲーム側のカメラ.
	Cake::Matrix4x4 renderWorldMatrix = worldMatrix_;
	Cake::Vector3 renderRotation = rotation_;
	renderEyePosition_ = eyePosition_;
#ifdef CAKE_ENABLE_DEBUG_CAMERA
	// デバッグカメラ中は描画の視点だけを差し替える。ToSCS() やデバッグ描画も画面に映る絵と揃う.
	if (isDebugCameraActive_) {
		renderWorldMatrix = debugCamera_.GetWorldMatrix();
		renderRotation = debugCamera_.GetRotation();
		renderEyePosition_ = debugCamera_.GetPosition();
	}
#endif // CAKE_ENABLE_DEBUG_CAMERA
	viewMatrix_ = Cake::Matrix4x4::Inverse(renderWorldMatrix);

	// KamataEngine::Camera へ流し込む.
	// TransferMatrix() は matView / matProjection に加えて translation_ を「視点の位置」として送る
	// （ライティングの鏡面反射に使われる）ので、目の位置を入れておく.
	camera_.rotation_ = ToKamata(renderRotation);
	camera_.translation_ = ToKamata(renderEyePosition_);
	camera_.fovAngleY = fovY_;
	camera_.aspectRatio = GetAspectRatio();
	camera_.nearZ = nearZ_;
	camera_.farZ = farZ_;
	camera_.matView = ToKamata(viewMatrix_);
	camera_.matProjection = ToKamata(projectionMatrix_);
	camera_.TransferMatrix();
}

/*
* 姿勢・画角
———————————————*/
void Camera::SetTarget(const Cake::Vector3& target) {
	target_ = target;
	// Update() の後に呼んだ同フレームでも、描画行列を最新位置へ合わせる.
	UpdateMatrices();
}

void Camera::SetRotation(const Cake::Vector3& rotation) {
	rotation_ = rotation;
	UpdateMatrices();
}

void Camera::SetDistance(float distance) {
	distance_ = (std::max)(distance, 0.0f);
	UpdateMatrices();
}

void Camera::SetFovY(float fovY) {
	fovY_ = std::clamp(fovY, kMinFovY, kMaxFovY);
	UpdateMatrices();
}

void Camera::SetClipRange(float nearZ, float farZ) {
	nearZ_ = (std::max)(nearZ, kMinNearZ);
	// near と far が一致すると射影行列が 0 除算になる.
	farZ_ = (std::max)(farZ, nearZ_ + kMinNearZ);
	UpdateMatrices();
}

float Camera::GetAspectRatio() const {
	if (height_ <= 0) {
		return 1.0f;
	}
	return static_cast<float>(width_) / static_cast<float>(height_);
}

Cake::Vector3 Camera::GetForward() const {
	return GetRow(rotationMatrix_, 2);
}
Cake::Vector3 Camera::GetRight() const {
	return GetRow(rotationMatrix_, 0);
}
Cake::Vector3 Camera::GetUp() const {
	return GetRow(rotationMatrix_, 1);
}

Cake::Vector3 Camera::ToCameraRelativeXZ(const Cake::Vector2& input) const {
	// Y軸回転だけを取り出した、水平な前方向と右方向.
	const float sinYaw = std::sin(rotation_.y);
	const float cosYaw = std::cos(rotation_.y);
	const Cake::Vector3 forward{sinYaw, 0.0f, cosYaw};
	const Cake::Vector3 right{cosYaw, 0.0f, -sinYaw};
	return right * input.x + forward * input.y;
}

Cake::Vector2 Camera::GetVisibleSize() const {
	const float height = 2.0f * distance_ * std::tan(fovY_ * 0.5f);
	return {height * GetAspectRatio(), height};
}

/*
* 座標変換
———————————————*/
Cake::Vector3 Camera::ToSCS(const Cake::Vector3& wcs) const {
	const Cake::Vector3 view = Cake::Vector3::Transform(wcs, viewMatrix_);
	if (view.z <= 0.0f) {
		// カメラの後ろ。射影すると上下左右が反転した位置に出てしまう.
		return {0.0f, 0.0f, view.z};
	}
	const Cake::Vector3 ndc = Cake::Vector3::Transform(view, projectionMatrix_);
	const Cake::Vector3 screen = Cake::Vector3::Transform(ndc, viewportMatrix_);
	return {screen.x, screen.y, view.z};
}

Cake::Ray Camera::ScreenPointToRay(const Cake::Vector2& scs) const {
	// view * projection * viewport をまとめて逆行列にすると、原点から離れた場所で float の誤差が膨らむ。
	// 射影だけを式で戻し、ビュー行列（回転と平行移動だけ）の逆行列でワールドへ移す.
	const float m00 = projectionMatrix_.m[0][0];
	const float m11 = projectionMatrix_.m[1][1];
	const float m22 = projectionMatrix_.m[2][2];
	const float m32 = projectionMatrix_.m[3][2];
	if (m00 == 0.0f || m11 == 0.0f || m22 == 0.0f || width_ <= 0 || height_ <= 0) {
		return Cake::Ray{renderEyePosition_, GetForward()};
	}

	// スクリーン座標 → 正規化デバイス座標(-1～1、Y上向き).
	const float ndcX = 2.0f * scs.x / static_cast<float>(width_) - 1.0f;
	const float ndcY = 1.0f - 2.0f * scs.y / static_cast<float>(height_);

	// ビュー空間で、奥行き 1 の位置にある点への向き.
	const Cake::Vector3 directionView{ndcX / m00, ndcY / m11, 1.0f};
	// m22 = f/(f-n), m32 = -nf/(f-n) からニアクリップの距離を戻す.
	const float nearZ = -m32 / m22;

	const Cake::Matrix4x4 inverseView = Cake::Matrix4x4::Inverse(viewMatrix_);
	const Cake::Vector3 origin = Cake::Vector3::Transform(directionView * nearZ, inverseView);
	const Cake::Vector3 direction = Cake::Vector3::Normalize(Cake::Vector3::TransformNormal(directionView, inverseView));
	return Cake::Ray{origin, direction};
}

bool Camera::IsVisible(const Cake::Sphere& sphere) const {
	// 射影行列から視錐台の傾きと奥行きを読み取る。デバッグカメラの行列でも同じ式で通る.
	const float m00 = projectionMatrix_.m[0][0];
	const float m11 = projectionMatrix_.m[1][1];
	const float m22 = projectionMatrix_.m[2][2];
	const float m32 = projectionMatrix_.m[3][2];
	if (m00 == 0.0f || m11 == 0.0f || m22 == 0.0f) {
		// 行列が組めていない。消えるよりは描いたほうが原因を追いやすい.
		return true;
	}

	const Cake::Vector3 center = Cake::Vector3::Transform(sphere.center, viewMatrix_);
	const float radius = sphere.radius;

	// 奥行き。m22 = f/(f-n), m32 = -nf/(f-n) から n と f を戻す.
	const float nearZ = -m32 / m22;
	const float farZ = (m22 != 1.0f) ? m32 / (1.0f - m22) : (std::numeric_limits<float>::max)();
	if (center.z + radius < nearZ || center.z - radius > farZ) {
		return false;
	}

	// 側面。境界は x = z * tanX なので、外向き法線 (1, 0, -tanX) を正規化した平面までの距離で測る.
	const float tanX = 1.0f / m00;
	const float tanY = 1.0f / m11;
	const float inverseLengthX = 1.0f / std::sqrt(1.0f + tanX * tanX);
	const float inverseLengthY = 1.0f / std::sqrt(1.0f + tanY * tanY);
	if ((center.x - center.z * tanX) * inverseLengthX > radius || (-center.x - center.z * tanX) * inverseLengthX > radius) {
		return false;
	}
	if ((center.y - center.z * tanY) * inverseLengthY > radius || (-center.y - center.z * tanY) * inverseLengthY > radius) {
		return false;
	}
	return true;
}


#ifdef CAKE_ENABLE_DEBUG_CAMERA
/*
* デバッグカメラ
———————————————*/
void Camera::UpdateDebugCamera(Cake::Time* time) {
	// ESC で切り替える。ImGui の入力欄で文字を打っている間の ESC は、入力欄を抜けるためのものなので取らない.
	if (KamataEngine::Input::GetInstance()->TriggerKey(DIK_ESCAPE) && !ImGui::GetIO().WantTextInput) {
		SetDebugCameraActive(!isDebugCameraActive_);
	}

	if (isDebugCameraActive_) {
		// ポーズ中やヒットストップ中でも動かせるよう、実時間で進める.
		debugCamera_.Update(time ? time->GetUnscaledDeltaTime() : 0.0f);
	}

	// 右ボタンで飛んでいる間は、プレイヤーまで WASD で動かないようにする.
	// オブジェクトの Update() はカメラより先に走るので、反映されるのは次のフレームから.
	Cake::Input::SetGameInputEnabled(!(isDebugCameraActive_ && debugCamera_.IsFlying()));
}

void Camera::SetDebugCameraActive(bool isActive) {
	if (isActive && !hasDebugCameraPose_) {
		// 初めて切り替えたときは、今のゲームの視点から始める.
		// 2回目以降は前回の位置から続ける（Unity のシーンビューと同じ）.
		debugCamera_.SetPose(eyePosition_, rotation_);
		hasDebugCameraPose_ = true;
	}
	if (!isActive) {
		// 右ボタンを押したまま外しても、操作とゲーム入力の停止が残らないようにする.
		debugCamera_.CancelControl();
		Cake::Input::SetGameInputEnabled(true);
	}
	isDebugCameraActive_ = isActive;
	UpdateMatrices();
}
#endif // CAKE_ENABLE_DEBUG_CAMERA


#ifdef USE_IMGUI
#include <imgui.h>

#include "System/Render/Debug/DebugDraw.h"

namespace {
// 行列は TreeNode に畳んでおき、必要なときだけ開く.
void DrawMatrix4x4(const char* label, const Cake::Matrix4x4& matrix) {
	if (!ImGui::TreeNode(label)) {
		return;
	}
	for (int row = 0; row < 4; ++row) {
		ImGui::Text("%9.3f %9.3f %9.3f %9.3f", matrix.m[row][0], matrix.m[row][1], matrix.m[row][2], matrix.m[row][3]);
	}
	ImGui::TreePop();
}

// オイラー角を度数法で編集する。変更があれば true.
bool DragRotation(const char* label, Cake::Vector3& rotation) {
	float degrees[3] = {
		Cake::Math::ToDegree(rotation.x),
		Cake::Math::ToDegree(rotation.y),
		Cake::Math::ToDegree(rotation.z),
	};
	if (!ImGui::DragFloat3(label, degrees, 0.5f, -360.0f, 360.0f, "%.1f deg")) {
		return false;
	}
	rotation = {Cake::Math::ToRadian(degrees[0]), Cake::Math::ToRadian(degrees[1]), Cake::Math::ToRadian(degrees[2])};
	return true;
}
} // namespace

void Camera::DrawDebugUI(const char* windowName) {
	// ウィンドウを畳んでいても、グリッドと注視点の目印は出す.
	if (debugDrawGrid_) {
		Cake::DebugDraw::Grid(*this, debugGridHalfSize_, debugGridSpacing_);
	}
	if (debugDrawTarget_) {
		Cake::DebugDraw::Axis(*this, target_, 1.0f);
	}

#ifdef CAKE_ENABLE_DEBUG_CAMERA
	if (isDebugCameraActive_) {
		if (debugDrawGameCamera_) {
			DrawGameCameraGizmo();
		}
		// 今どちらのカメラで見ているのか取り違えないよう、画面の左上に出しておく.
		static constexpr const char* kGuide = "DEBUG CAMERA  [ESC] game view  [RMB]+WASD/QE fly  [Wheel] zoom  [MMB] pan";
		const ImVec2 origin = ImGui::GetMainViewport()->Pos;
		ImDrawList* drawList = ImGui::GetBackgroundDrawList(ImGui::GetMainViewport());
		drawList->AddText(ImVec2(origin.x + 11.0f, origin.y + 11.0f), 0xFF000000, kGuide); // 影.
		drawList->AddText(ImVec2(origin.x + 10.0f, origin.y + 10.0f), Cake::DebugDraw::kColorYellow, kGuide);
	}
#endif // CAKE_ENABLE_DEBUG_CAMERA

	// 折りたたまれている間は中身を組み立てない.
	if (!ImGui::Begin(windowName)) {
		ImGui::End();
		return;
	}

	bool isDirty = false;

	/* ===== Transform ===== */
	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (cameraFollow_.IsActive()) {
			// 追従中は Update() が毎フレーム target_ を上書きするので、編集させると混乱の元になる.
			ImGui::Text("Target: (%.2f, %.2f, %.2f)", target_.x, target_.y, target_.z);
			ImGui::TextDisabled("Read only while Follow is active.");
		} else {
			if (ImGui::DragFloat3("Target", &target_.x, 0.1f, -10000.0f, 10000.0f, "%.2f")) {
				isDirty = true;
			}
		}
		if (DragRotation("Rotation", rotation_)) {
			isDirty = true;
		}
		if (ImGui::DragFloat("Distance", &distance_, 0.1f, 0.0f, 10000.0f, "%.2f")) {
			distance_ = (std::max)(distance_, 0.0f);
			isDirty = true;
		}

		// シェイクを含めた、実際に描画で使われる目の位置.
		ImGui::Text("Eye: (%.2f, %.2f, %.2f)", eyePosition_.x, eyePosition_.y, eyePosition_.z);

		if (ImGui::Button("Reset")) {
			rotation_ = Cake::Vector3::Zero;
			distance_ = kDefaultDistance;
			isDirty = true;
		}
	}

	/* ===== Projection ===== */
	if (ImGui::CollapsingHeader("Projection")) {
		ImGui::Text("Window: %d x %d (aspect %.3f)", width_, height_, GetAspectRatio());
		float fovDegree = Cake::Math::ToDegree(fovY_);
		if (ImGui::SliderFloat("FovY", &fovDegree, 1.0f, 170.0f, "%.1f deg")) {
			fovY_ = std::clamp(Cake::Math::ToRadian(fovDegree), kMinFovY, kMaxFovY);
			isDirty = true;
		}
		float clip[2] = {nearZ_, farZ_};
		if (ImGui::DragFloat2("Near / Far", clip, 0.1f, kMinNearZ, 100000.0f, "%.3f")) {
			nearZ_ = (std::max)(clip[0], kMinNearZ);
			farZ_ = (std::max)(clip[1], nearZ_ + kMinNearZ);
			isDirty = true;
		}

		const Cake::Vector2 visibleSize = GetVisibleSize();
		ImGui::Text("Visible Size at Target: %.2f x %.2f", visibleSize.x, visibleSize.y);

		ImGui::Separator();
		if (hasLastClamp_) {
			ImGui::Text("Clamp Min: (%.2f, %.2f, %.2f)", lastClamp_.min.x, lastClamp_.min.y, lastClamp_.min.z);
			ImGui::Text("Clamp Max: (%.2f, %.2f, %.2f)", lastClamp_.max.x, lastClamp_.max.y, lastClamp_.max.z);
			// 追従で止まっているのか、マップ端で止まっているのかを切り分ける.
			const bool atEdgeX = target_.x <= lastClamp_.min.x || target_.x >= lastClamp_.max.x;
			const bool atEdgeY = target_.y <= lastClamp_.min.y || target_.y >= lastClamp_.max.y;
			const bool atEdgeZ = target_.z <= lastClamp_.min.z || target_.z >= lastClamp_.max.z;
			ImGui::Text("At Clamp Edge: X %s / Y %s / Z %s", atEdgeX ? "YES" : "no", atEdgeY ? "YES" : "no", atEdgeZ ? "YES" : "no");
		} else {
			ImGui::TextDisabled("Clamp: none");
		}
	}

#ifdef CAKE_ENABLE_DEBUG_CAMERA
	/* ===== Debug Camera ===== */
	if (ImGui::CollapsingHeader("Debug Camera")) {
		bool isActive = isDebugCameraActive_;
		if (ImGui::Checkbox("Active (ESC)", &isActive)) {
			SetDebugCameraActive(isActive);
		}
		ImGui::SameLine();
		if (ImGui::Button("Move to Game Camera")) {
			debugCamera_.SetPose(eyePosition_, rotation_);
			hasDebugCameraPose_ = true;
			isDirty = true;
		}
		ImGui::Checkbox("Draw Game Camera", &debugDrawGameCamera_);
		ImGui::TextDisabled("Game logic still uses the game camera's pose.");
		debugCamera_.DrawDebugUI();
	}
#endif // CAKE_ENABLE_DEBUG_CAMERA

	/* ===== Gizmos ===== */
	if (ImGui::CollapsingHeader("Gizmos")) {
		ImGui::Checkbox("Draw Grid", &debugDrawGrid_);
		ImGui::DragFloat("Grid Half Size", &debugGridHalfSize_, 1.0f, 1.0f, 1000.0f, "%.0f");
		ImGui::DragFloat("Grid Spacing", &debugGridSpacing_, 0.1f, 0.1f, 100.0f, "%.1f");
		ImGui::Checkbox("Draw Target", &debugDrawTarget_);
	}

	/* ===== 各機能は自分のセクションを自分で描く ===== */
	cameraFollow_.DrawDebugUI();
	cameraShake_.DrawDebugUI();

	/* ===== Matrix ===== */
	if (ImGui::CollapsingHeader("Matrix")) {
		DrawMatrix4x4("World", worldMatrix_);
		DrawMatrix4x4("View", viewMatrix_);
		DrawMatrix4x4("Projection", projectionMatrix_);
		DrawMatrix4x4("Viewport", viewportMatrix_);
	}

	ImGui::End();

	// 編集した値を、このフレームの描画から反映させる.
	if (isDirty) {
		UpdateMatrices();
	}
}

#ifdef CAKE_ENABLE_DEBUG_CAMERA
void Camera::DrawGameCameraGizmo() const {
	// 注視点までの距離で区切った視錐台を描く。一人称（距離0）のときは見やすい長さで区切る.
	constexpr float kFirstPersonGizmoDepth = 5.0f;
	const float depth = (distance_ > 0.0f) ? distance_ : kFirstPersonGizmoDepth;
	const float halfHeight = depth * std::tan(fovY_ * 0.5f);
	const float halfWidth = halfHeight * GetAspectRatio();

	const Cake::Vector3 center = eyePosition_ + GetForward() * depth;
	const Cake::Vector3 right = GetRight() * halfWidth;
	const Cake::Vector3 up = GetUp() * halfHeight;
	// 左上から時計回り.
	const Cake::Vector3 corners[4] = {
		center - right + up,
		center + right + up,
		center + right - up,
		center - right - up,
	};

	const uint32_t color = Cake::DebugDraw::kColorYellow;
	for (int i = 0; i < 4; ++i) {
		Cake::DebugDraw::Line(*this, eyePosition_, corners[i], color);
		Cake::DebugDraw::Line(*this, corners[i], corners[(i + 1) % 4], color);
	}
	// 画面の上側がどちらか分かるよう、上辺の中央から上へ短い線を出す.
	Cake::DebugDraw::Line(*this, center + up, center + up * 1.2f, color);
	Cake::DebugDraw::Cross(*this, eyePosition_, color);
}
#endif // CAKE_ENABLE_DEBUG_CAMERA
#endif // USE_IMGUI
