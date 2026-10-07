#pragma once
/*====================================
 *
 * ゲーム用の3Dカメラ。KamataEngine::Camera を内部に持ち、
 * 注視点・回転・距離から View / Projection 行列を組み立てて転送する。
 * 追従(CameraFollow)・シェイク(CameraShake)・注視点の移動範囲の制限もここで掛ける。
 *
 * 【姿勢の決め方】
 * 注視点(target_)から、rotation_ が向く方向と逆へ distance_ だけ下がった位置に目を置く。
 *   eye = target - forward * distance
 * distance_ を 0 にすると、注視点に目を置く一人称視点になる。
 * 2D版の position_（画面中心）は target_ に、zoom_ は distance_ に相当する。
 * 既定値（target 原点・回転なし・距離50）は、KamataEngine::Camera の既定（Z=-50 から +Z を見る）と同じ絵になる.
 *
 * 【行列は Cake 側で組んで KamataEngine::Camera へ流し込む】
 * KamataEngine::Camera::UpdateMatrix() には任せず、ここで組んだ行列を
 * matView / matProjection へ書いて TransferMatrix() する。
 * ToSCS() やデバッグ描画が、描画に使った行列と必ず同じものを見るようにするため.
 *
 * 【描画に渡すカメラ】
 * KamataEngine::Model::Draw() には GetRenderCamera() を渡す（ModelRenderer が中で行う）.
 *
 * 【デバッグカメラ（Debug ビルドのみ）】
 * ESC で、Unity のシーンビューと同じ操作で飛び回れるカメラ（DebugFlyCamera）に切り替わる。
 * 切り替え中に差し替わるのは「描画に使う視点」だけで、GetForward() や ToCameraRelativeXZ() など
 * ゲームロジックが見る姿勢はゲーム側のカメラのまま。プレイヤーの操作感は変わらない.
 * 右ボタンで飛んでいる間は、Cake::Input がゲームへ入力を渡さない（プレイヤーまで WASD で動かないように）.
 *
 * 【Set〇〇() は Update の中で呼ぶ】
 * 変更はその場で定数バッファへ転送される。Draw() の途中で呼ぶと、
 * 既に積んだ描画コマンドまで新しい行列で描かれる.
 *
 * ====================================*/
#include "KamataEngine.h"

#include "System/Foundation/Math/Vector.h"
#include "System/Foundation/Math/Matrix.h"
#include "System/Foundation/Math/Geometry.h"
#include "System/Render/Camera/CameraFollow.h"
#include "System/Render/Camera/CameraShake.h"
#include "System/Render/Camera/DebugFlyCamera.h"

namespace Cake {
class Time;
}

class Camera {
public:
	// 視野角の範囲（ラジアン）。0 や π 以上では射影行列が作れない.
	static constexpr float kMinFovY = 0.01f;
	static constexpr float kMaxFovY = 3.0f;
	// ニアクリップの下限。0 だと射影行列が作れない.
	static constexpr float kMinNearZ = 0.001f;

	// 既定値。KamataEngine::Camera の初期値に揃えてある.
	static constexpr float kDefaultDistance = 50.0f;
	static constexpr float kDefaultFovY = 0.785398163f; // 45度.
	static constexpr float kDefaultNearZ = 0.1f;
	static constexpr float kDefaultFarZ = 1000.0f;

private:
	Cake::Vector3 target_;   // 注視点。追従中は CameraFollow が毎フレーム上書きする.
	// オイラー角(ラジアン)。x: 見下ろし角(+で下を向く), y: 水平方向の向き, z: ロール(視線の軸回りの傾き).
	// 回す順番は ロール → 見下ろし → 水平の向き。物体の Transform3 とは順番が違う.
	Cake::Vector3 rotation_;
	float distance_ = kDefaultDistance; // 注視点から目までの距離.

	float fovY_ = kDefaultFovY; // 垂直方向の視野角(ラジアン).
	float nearZ_ = kDefaultNearZ;
	float farZ_ = kDefaultFarZ;

	CameraShake cameraShake_;
	CameraFollow cameraFollow_;

	// シェイクを含めた、実際の目の位置.
	Cake::Vector3 eyePosition_;
	// 描画に使った目の位置。デバッグカメラ中はそちらの位置になる.
	Cake::Vector3 renderEyePosition_;

	// 変換行列。Update() と Set〇〇() のたびに作り直す.
	Cake::Matrix4x4 rotationMatrix_;   // rotation_ だけの回転行列.
	Cake::Matrix4x4 worldMatrix_;      // カメラ自身の姿勢（目の位置と向き）.
	Cake::Matrix4x4 viewMatrix_;       // 描画に使ったビュー行列（デバッグカメラ中はそちら）.
	Cake::Matrix4x4 projectionMatrix_; // 描画に使った射影行列（同上）.
	Cake::Matrix4x4 viewportMatrix_;

	// 描画に渡す実体。定数バッファを持つので、エンジンの初期化後に作ること.
	KamataEngine::Camera camera_;

	int width_ = 0;
	int height_ = 0;

public:
	// ウィンドウサイズは KamataEngine::WinApp の既定値を使う.
	Camera();
	Camera(int windowWidth, int windowHeight);

	// KamataEngine::Camera が定数バッファを握っているので、複製できない.
	Camera(const Camera&) = delete;
	Camera& operator=(const Camera&) = delete;

	~Camera();

	/// <summary>
	/// 姿勢と画角を既定値へ戻し、行列を作り直す.
	/// </summary>
	void Initialize(int windowWidth, int windowHeight);

	/// <summary>
	/// 追従・シェイクを進め、描画用の行列を作り直して転送する。移動範囲の制限は掛けない.
	/// シーンの Update() で、オブジェクトの更新（UpdateGameObjects()）の後に1回呼ぶ.
	/// </summary>
	void Update(Cake::Time* time);

	/// <summary>
	/// Update(time) に加えて、注視点を clamp の範囲へ収める.
	/// 範囲が逆転している軸（マップが狭いなど）は、範囲の中央に固定する.
	/// </summary>
	/// <param name="clamp">注視点が動ける範囲（ワールド座標）</param>
	void Update(Cake::Time* time, const Cake::AABB& clamp);

	/* ===== 姿勢 ===== */

	const Cake::Vector3& GetTarget() const { return target_; }
	// 演出時など、追従を止めたカメラを任意の位置へ即時配置する.
	void SetTarget(const Cake::Vector3& target);

	const Cake::Vector3& GetRotation() const { return rotation_; }
	void SetRotation(const Cake::Vector3& rotation);

	float GetDistance() const { return distance_; }
	// 負の値は 0 に丸める.
	void SetDistance(float distance);

	/* ===== 画角 ===== */

	float GetFovY() const { return fovY_; }
	// kMinFovY ～ kMaxFovY に丸める.
	void SetFovY(float fovY);
	float GetNearZ() const { return nearZ_; }
	float GetFarZ() const { return farZ_; }
	// nearZ は kMinNearZ 以上、farZ は nearZ より奥に丸める.
	void SetClipRange(float nearZ, float farZ);
	float GetAspectRatio() const;

	/* ===== 向き ===== */

	// シェイクを含めた、実際の目の位置.
	const Cake::Vector3& GetEyePosition() const { return eyePosition_; }
	// 描画に使った目の位置。デバッグカメラ中はそちら。半透明の並べ替えなど、見た目に合わせる処理に使う.
	const Cake::Vector3& GetRenderEyePosition() const { return renderEyePosition_; }
	// カメラから見た 前・右・上 のワールドでの向き（単位ベクトル）.
	Cake::Vector3 GetForward() const;
	Cake::Vector3 GetRight() const;
	Cake::Vector3 GetUp() const;

	/// <summary>
	/// 入力の2D軸（x: 右, y: 奥。Cake::Input::GetMoveAxis() の戻り値）を、
	/// カメラの水平方向の向きを基準にした XZ 平面の向きへ直す。長さはそのまま保つ.
	/// 見下ろし角とロールは無視し、ヨー（rotation_.y）だけを見る.
	/// </summary>
	Cake::Vector3 ToCameraRelativeXZ(const Cake::Vector2& input) const;

	/// <summary>注視点の位置で、画面に映る範囲の大きさ（ワールド単位）.</summary>
	Cake::Vector2 GetVisibleSize() const;

	/* ===== 行列 ===== */

	// カメラ自身の姿勢。デバッグカメラ中もゲーム側のカメラのまま.
	const Cake::Matrix4x4& GetWorldMatrix() const { return worldMatrix_; }
	// 以下の3つは描画に使った行列。デバッグカメラ中はそちらの行列になる.
	const Cake::Matrix4x4& GetViewMatrix() const { return viewMatrix_; }
	const Cake::Matrix4x4& GetProjectionMatrix() const { return projectionMatrix_; }
	const Cake::Matrix4x4& GetViewportMatrix() const { return viewportMatrix_; }

	/// <summary>
	/// KamataEngine::Model::Draw() などに渡すカメラ。デバッグカメラ中は、その視点の行列が入っている.
	/// </summary>
	const KamataEngine::Camera& GetRenderCamera() const { return camera_; }

	CameraShake& GetCameraShake() { return cameraShake_; }
	CameraFollow& GetCameraFollow() { return cameraFollow_; }

	/* ===== 座標変換 ===== */

	/// <summary>
	/// ワールド座標をスクリーン座標（左上原点・ピクセル）へ変換する.
	/// 戻り値の z はカメラからの奥行き（ビュー空間の Z）。0 以下ならカメラの後ろにあり、x, y は意味を持たない.
	/// </summary>
	Cake::Vector3 ToSCS(const Cake::Vector3& wcs) const;

	/// <summary>
	/// スクリーン座標からワールドへ伸びるレイを求める。マウスで3D空間の物を指す場合に使う.
	/// 戻り値の diff は正規化済みの向き。CollisionManager::Raycast() へそのまま渡せる.
	/// </summary>
	Cake::Ray ScreenPointToRay(const Cake::Vector2& scs) const;

	/// <summary>
	/// 球が視錐台に少しでも入っているか。描画のカリングに使う.
	/// </summary>
	bool IsVisible(const Cake::Sphere& sphere) const;

	int GetWindowWidth() const { return width_; }
	int GetWindowHeight() const { return height_; }

private:
	// clamp が nullptr なら範囲の制限を掛けない.
	void UpdateInternal(Cake::Time* time, const Cake::AABB* clamp);
	// 現在の姿勢と画角から行列を作り直し、KamataEngine::Camera へ転送する.
	void UpdateMatrices();

#pragma region デバッグ用
#ifdef CAKE_ENABLE_DEBUG_CAMERA
	// Unity のシーンビューと同じ操作で動かせるカメラ。有効な間は描画だけがこちらの視点になる.
	DebugFlyCamera debugCamera_;
	bool isDebugCameraActive_ = false;
	// 一度でも有効にしたか。初回だけゲームの視点から始め、2回目以降は前回の位置を覚えておく.
	bool hasDebugCameraPose_ = false;
	// デバッグカメラ中に、ゲーム側のカメラの視錐台を描くか.
	bool debugDrawGameCamera_ = true;

	// ESC での切り替えと、デバッグカメラの操作.
	void UpdateDebugCamera(Cake::Time* time);
	// ゲーム側のカメラの位置と映す範囲を、線で描く.
	void DrawGameCameraGizmo() const;

public:
	bool IsDebugCameraActive() const { return isDebugCameraActive_; }
	/// <summary>
	/// デバッグカメラの有効・無効を切り替える。ESC キーと同じ.
	/// </summary>
	void SetDebugCameraActive(bool isActive);

private:
#endif // CAKE_ENABLE_DEBUG_CAMERA

#ifdef USE_IMGUI
	Cake::AABB lastClamp_{};     // 直近の Update() に渡された移動範囲.
	bool hasLastClamp_ = false;  // 直近の Update() が範囲を受け取ったか.

	bool debugDrawGrid_ = true;
	bool debugDrawTarget_ = true;
	float debugGridHalfSize_ = 50.0f;
	float debugGridSpacing_ = 1.0f;

public:
	/// <summary>
	/// カメラのデバッグウィンドウと、地面のグリッド・注視点の目印を描画する.
	/// デバッグカメラ中は、ゲーム側のカメラの視錐台と、画面左上の案内も描く.
	/// ImGuiManager::Begin()～End() の内側、つまりシーンの Update() から、Update() の後に呼ぶこと.
	/// </summary>
	/// <param name="windowName">カメラが複数ある場合に区別するための名前</param>
	void DrawDebugUI(const char* windowName = "Camera");
#endif // USE_IMGUI
#pragma endregion
};
