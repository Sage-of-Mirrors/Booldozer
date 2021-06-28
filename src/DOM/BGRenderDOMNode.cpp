#include "DOM/BGRenderDOMNode.hpp"
#include "bigg.hpp"

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
}
