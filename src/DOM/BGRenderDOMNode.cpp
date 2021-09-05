#include "DOM/BGRenderDOMNode.hpp"
#include "bigg.hpp"
#include <glm/gtx/matrix_decompose.hpp>

LBGRenderDOMNode::LBGRenderDOMNode(std::string name) : LUIRenderDOMNode(name)
{
	mType = EDOMNodeType::BGRender;
}

void LBGRenderDOMNode::RenderBG(float dt)
{
	if(!GetIsInitialized()){
		glm::mat4 m = glm::identity<glm::mat4>();
		m = glm::translate(m, mPosition);
		m = glm::rotate(m, glm::radians(mRotation.x), glm::vec3(1, 0, 0));
		m = glm::rotate(m, glm::radians(mRotation.y), glm::vec3(0, 1, 0));
		m = glm::rotate(m, glm::radians(mRotation.z), glm::vec3(0, 0, 1));
		m = glm::scale(m, mScale);
		mTransform = std::make_shared<glm::mat4>(m);
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
