#include "DOM/BGRenderDOMNode.hpp"
//#include "bigg.hpp"
#include <glm/gtx/matrix_decompose.hpp>

LBGRenderDOMNode::LBGRenderDOMNode(std::string name) : LUIRenderDOMNode(name),
	mPosition(glm::vec3(0, 0, 0)), mScale(glm::vec3(1, 1, 1)), mRotation(glm::vec3(0, 0, 0))
{
	mType = EDOMNodeType::BGRender;
}

void LBGRenderDOMNode::Manipulate(){
	
}

void LBGRenderDOMNode::RenderBG(float dt)
{
	if(!GetIsInitialized()){
		glm::mat4 m = glm::identity<glm::mat4>();

		m = glm::translate(m, mPosition);
		m = glm::rotate(m, glm::radians(mRotation.x), glm::vec3(1, 0, 0));
		m = glm::rotate(m, glm::radians(-mRotation.y), glm::vec3(0, 1, 0));
		m = glm::rotate(m, glm::radians(mRotation.z), glm::vec3(0, 0, 1));
		m = glm::scale(m, mScale);
		mTransform = std::make_shared<glm::mat4>(m);
		SetIsInitialized(true);
	}

	if (GetIsInitialized() && GetIsSelected())
	{
	}
}
