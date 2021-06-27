#pragma once

#include "UIRenderDOMNode.hpp"
#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"

// DOM node with the ability to render models to the main window's background.
class LBGRenderDOMNode : public LUIRenderDOMNode
{
protected:
	// 3D location of this node in the world.
	glm::vec3 mPosition;
	// Euler rotation of this node.
	glm::vec3 mRotation;
	// 3D scale of this node.
	glm::vec3 mScale;

public:
	typedef LUIRenderDOMNode Super;

	LBGRenderDOMNode(std::string name);

	// Renders this node to the main window's background.
	virtual void RenderBG(float dt);

	// Returns this node's position.
	glm::vec3 GetPosition() { return mPosition; }
	// Returns this node's rotation.
	glm::vec3 GetRotation() { return mRotation; }
	// Returns this node's scale.
	glm::vec3 GetScale() { return mScale; }

	// Sets this node's position to the given value.
	void SetPosition(glm::vec3 newPos) { mPosition = newPos; }
	// Sets this node's rotation to the given value.
	void SetRotation(glm::vec3 newRot) { mRotation = newRot; }
	// Sets this node's scale to the given value.
	void SetScale(glm::vec3 newScale) { mScale = newScale; }

/*=== Type operations ===*/
	// Returns whether this node is of the given type, or derives from a node of that type.
	virtual bool IsNodeType(EDOMNodeType type) const override
	{
		if (type == EDOMNodeType::BGRender)
			return true;

		return Super::IsNodeType(type);
	}
};
