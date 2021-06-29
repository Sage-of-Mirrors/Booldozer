#include "DOM/BGRenderDOMNode.hpp"
#include "bigg.hpp"
#include <glm/gtx/matrix_decompose.hpp>

LBGRenderDOMNode::LBGRenderDOMNode(std::string name) : LUIRenderDOMNode(name)
{
	mType = EDOMNodeType::BGRender;
}

void LBGRenderDOMNode::RenderBG(float dt, LEditorScene* scene)
{
	if(!GetIsInitialized()){
		glm::mat4 m = glm::identity<glm::mat4>();
		m = glm::translate(m, mPosition);
		mTransform = scene->InstanceModel(mName, m);
		SetIsInitialized(true);
	}

	if (GetIsInitialized() && GetIsSelected())
	{
		// We don't care about skew or perspective, but we need some place for them to go
		// to use glm:decompose().
		glm::quat rotQuat;
		glm::vec3 skew;
		glm::vec4 persp;
		glm::decompose(*mTransform.get(), mScale, rotQuat, mPosition, skew, persp);

		mRotation = glm::eulerAngles(rotQuat);
	}
}
