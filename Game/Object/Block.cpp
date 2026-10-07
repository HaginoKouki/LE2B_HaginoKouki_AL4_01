#include "Block.h"

#include "System/Render/Camera/Camera.h"
#include "System/Render/Renderer/ModelManager.h"

Block::Block() : collisionBody_(this) {
}

void Block::Initialize() {
	modelRenderer_.Initialize(ModelManager::Load("cube"));

	// 判定はモデルと同じ ±1 の箱。Setup() の拡縮がそのまま掛かる.
	collisionBody_.AddCollider(Cake::AABB{
		{-kModelHalfSize, -kModelHalfSize, -kModelHalfSize},
		{kModelHalfSize, kModelHalfSize, kModelHalfSize},
	});
	collisionBody_.SetLayer(CollisionLayer::Terrain);
	// 地形は押し出されない側。動く物同士（Player と Enemy など）だけが押し合う.
	collisionBody_.SetStatic(true);
	collisionBody_.AttachCollisionManager(GetCollisionManager());
}

void Block::Update(Cake::Time* time) {
	// 動かないので何もしない。動く足場にする場合はここで translate を変える.
	(void)time;
}

void Block::Draw() {
	modelRenderer_.Draw(*GetCamera(), GetWorldMatrix());
}

void Block::Setup(const Cake::Vector3& center, const Cake::Vector3& size) {
	localTransform_.translate = center;
	localTransform_.scale = size / (kModelHalfSize * 2.0f);
}
