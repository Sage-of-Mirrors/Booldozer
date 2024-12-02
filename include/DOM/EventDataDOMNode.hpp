#pragma once

//#include "io/CmnIo.hpp"
#include <Archive.hpp>
#include "BGRenderDOMNode.hpp"
#include "TextEditor.h"

class LEventMode;

class LEventDataDOMNode : public LBGRenderDOMNode {
private:

	friend LEventMode;

	std::filesystem::path mEventPath;

	std::string mEventScriptPath, mEventMessagePath;

	std::shared_ptr<Archive::Rarc> mEventArchive;
	
	//Data from event's txt file.
	std::string mEventScript;
    
	//Lines of text from event's csv file
	std::vector<std::string> mEventText;

	uint32_t mEventNo;

public:
	typedef LBGRenderDOMNode Super;

    //These should be private with getters and setters.


	LEventDataDOMNode(std::string name);

	void LoadEventArchive(std::shared_ptr<Archive::Rarc> arc, std::filesystem::path eventPath, std::string eventScriptName, std::string eventCsvName);
	void SaveEventArchive(bool createIfNotExist=true);

    void RenderDetailsUI(float dt, TextEditor* event);
	void RenderHierarchyUI(std::shared_ptr<LEventDataDOMNode> self, LEditorSelection* mode_selection);

	uint32_t GetEventNo() { return mEventNo; }

/*=== Type operations ===*/
	// Returns whether this node is of the given type, or derives from a node of that type.
	virtual bool IsNodeType(EDOMNodeType type) const override
	{
		if (type == EDOMNodeType::EventData)
			return true;

		return Super::IsNodeType(type);
	}
};