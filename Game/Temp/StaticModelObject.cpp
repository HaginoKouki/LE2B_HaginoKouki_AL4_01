#include "StaticModelObject.h"

void StaticModelObject::Initialize() {
}
void StaticModelObject::Update(Cake::Time* time) {
	(void)time;
}
void StaticModelObject::Draw() {
	modelRenderer_->Draw(*camera_, GameObject::GetWorldMatrix());
}