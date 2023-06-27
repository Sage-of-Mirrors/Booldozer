#pragma once

//#include "io/CmnIo.hpp"
#include "BGRenderDOMNode.hpp"
#include "TextEditor.h"

class LEventDataDOMNode : public LBGRenderDOMNode {
private:
    //TODO: Load camera animations here
    //std::vector<LCamAnim> mCameraAnimations;

	std::string mEventArchivePath;
public:
	typedef LBGRenderDOMNode Super;

    //These should be private with getters and setters.

	//Data from event's txt file.
	std::string mEventScript;
    
	//Lines of text from event's csv file
	std::vector<std::string> mEventText;

	LEventDataDOMNode(std::string name);

	void SetEventArchivePath(std::string path){ mEventArchivePath = path; }

    void RenderDetailsUI(float dt, TextEditor* editor);
	void RenderHierarchyUI(std::shared_ptr<LEventDataDOMNode> self, LEditorSelection* mode_selection);

/*=== Type operations ===*/
	// Returns whether this node is of the given type, or derives from a node of that type.
	virtual bool IsNodeType(EDOMNodeType type) const override
	{
		if (type == EDOMNodeType::EventData)
			return true;

		return Super::IsNodeType(type);
	}
};