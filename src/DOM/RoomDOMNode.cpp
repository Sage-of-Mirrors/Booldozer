#include <bstream.h>
#include <format>
#include "ImGuiFileDialog/ImGuiFileDialog.h"
#include "DOM/RoomDOMNode.hpp"
#include "DOM/ObserverDOMNode.hpp"
#include "DOM/EnemyDOMNode.hpp"
#include "UIUtil.hpp"
#include "Options.hpp"
#include "IconsForkAwesome.h"
#include "imgui.h"
#include "io/BinIO.hpp"
#include "modes/ActorMode.hpp"
#include "scene/ModelViewer.hpp"
#include <Bti.hpp>
#include <memory>
#include "ResUtil.hpp"

#include "stb_image.h"
#include "stb_image_write.h"

enum class SelectedResourceType {
  Texture,
  Sampler,
  Material,
  Batch,
  None
};

namespace {
	std::shared_ptr<Archive::Rarc> ActiveRoomArchive = nullptr;
	std::shared_ptr<Archive::File> EditFileName = nullptr;
	std::shared_ptr<Archive::File> SelectedFile = nullptr;
	std::shared_ptr<Archive::File> SelectedAnimation = nullptr;
	void* BinSelectedResource = nullptr;
	SelectedResourceType SelectedType { SelectedResourceType::None };
	std::string FileName = "(null)";
	uint32_t PreviousRoomID = 0xFFFFFFFF;
	uint32_t CurRoomNameImgID = 0x00000000;
	Bti RoomTitlecard;
}

std::string const LRoomEntityTreeNodeNames[LRoomEntityType_Max] = {
	"Characters",
	"Enemies",
	"Furniture",
	"Generators",
	"Objects",
	"Observers",
	"Paths",
	"Events",
	"Keys",
	"Characters (Blackout)",
	"Enemies (Blackout)",
	"Observers (Blackout)"
};

LRoomEntityType DOMToEntityType(EDOMNodeType type){
	switch (type)
	{
	case EDOMNodeType::Furniture: return LRoomEntityType_Furniture;
	case EDOMNodeType::Observer: return LRoomEntityType_Observers;
	case EDOMNodeType::Object: return LRoomEntityType_Objects;
	case EDOMNodeType::Enemy: return LRoomEntityType_Enemies;
	case EDOMNodeType::Generator: return LRoomEntityType_Generators;
	case EDOMNodeType::Path: return LRoomEntityType_Paths;
	case EDOMNodeType::Character: return LRoomEntityType_Characters;
	case EDOMNodeType::Event: return LRoomEntityType_Events;
	case EDOMNodeType::Key: return LRoomEntityType_Keys;

	default: return LRoomEntityType_Max;
	}
}

LRoomDOMNode::LRoomDOMNode(std::string name) : LBGRenderDOMNode(name)
{
	mType = EDOMNodeType::Room;
	mRoomModels.push_back("(null)");
}

void LRoomDOMNode::RoomResourceManagerHandleType(std::shared_ptr<LDOMNodeBase> self, std::shared_ptr<Archive::Folder> dir, std::string typeName, std::string typeExt){
	std::shared_ptr<Archive::File> toDelete = nullptr;
	if(ImGui::TreeNode(typeName.c_str())){
		for(auto file : dir->GetFiles()){
			if(std::filesystem::path(file->GetName()).extension().string() != typeExt) continue;
			if(EditFileName == file){

				LUIUtility::RenderTextInput("##filename", &FileName);
				if(ImGui::IsItemFocused() && ImGui::IsKeyDown(ImGuiKey_Enter)){
					if(typeExt == ".bin"){
						auto furniture = self->GetChildrenOfType<LFurnitureDOMNode>(EDOMNodeType::Furniture);
						for(auto entry : furniture){
							if(entry->GetModelName() == std::filesystem::path(file->GetName()).stem().string()){
								entry->SetModelName(FileName);
							}
						}
						LEditorScene::GetEditorScene()->mRoomFurniture[FileName] = LEditorScene::GetEditorScene()->mRoomFurniture[std::filesystem::path(EditFileName->GetName()).stem().string()];
						LEditorScene::GetEditorScene()->mRoomFurniture.erase(std::filesystem::path(EditFileName->GetName()).stem().string());
						std::replace(mRoomModels.begin(), mRoomModels.end(), std::filesystem::path(file->GetName()).stem().string(), FileName);
					}
					file->SetName(FileName+typeExt);
					EditFileName = nullptr;
				}
				ImGui::SameLine();
				ImGui::Text(typeExt.c_str());
			} else {
				if(SelectedFile == file || SelectedAnimation == file){
					ImGui::TextColored({0x00, 0xFF, 0x00, 0xFF}, file->GetName().c_str());
				} else {
					ImGui::Text(file->GetName().c_str());
					if(ImGui::IsItemClicked()){
						if(typeExt == ".bin"){
							SelectedAnimation = nullptr;
							SelectedFile = file;
							bStream::CMemoryStream modelStream(file->GetData(), file->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
							PreviewWidget::UnloadModel();
							PreviewWidget::LoadModel(&modelStream, EModelType::Furniture);
						} else if(typeExt == ".anm" && SelectedFile != nullptr && file->GetName().find(SelectedFile->GetName().substr(0, SelectedFile->GetName().size()-4)) != std::string::npos){
							SelectedAnimation = file;
							bStream::CMemoryStream animStream(file->GetData(), file->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
							PreviewWidget::SetModelAnimation(&animStream);
						}
					}
				}
			}
			ImGui::SameLine();
			ImGui::Text(ICON_FK_PENCIL);
			if(ImGui::IsItemClicked()) {
				if(EditFileName != file){
					if(EditFileName != nullptr){
						if(typeExt == ".bin"){
							auto furniture = self->GetChildrenOfType<LFurnitureDOMNode>(EDOMNodeType::Furniture);
							for(auto entry : furniture){
								if(entry->GetModelName() == std::filesystem::path(EditFileName->GetName()).stem().string()){
									entry->SetModelName(FileName);
								}
								LEditorScene::GetEditorScene()->mRoomFurniture[FileName] = LEditorScene::GetEditorScene()->mRoomFurniture[std::filesystem::path(EditFileName->GetName()).stem().string()];
								LEditorScene::GetEditorScene()->mRoomFurniture.erase(std::filesystem::path(EditFileName->GetName()).stem().string());
								std::replace(mRoomModels.begin(), mRoomModels.end(), std::filesystem::path(EditFileName->GetName()).stem().string(), FileName);
							}
						}
						EditFileName->SetName(FileName+typeExt);
					}
					EditFileName = file;
					FileName = std::filesystem::path(file->GetName()).stem().string();
					LGenUtility::Log << "[RoomDOMNode]: Original Filename is " << FileName << std::endl;
				} else if(EditFileName == file){
					if(typeExt == ".bin"){
						auto furniture = self->GetChildrenOfType<LFurnitureDOMNode>(EDOMNodeType::Furniture);
						for(auto entry : furniture){
							if(entry->GetModelName() == std::filesystem::path(file->GetName()).stem().string()){
								entry->SetModelName(std::filesystem::path(FileName).stem().string());
							}
						}
						LEditorScene::GetEditorScene()->mRoomFurniture[FileName] = LEditorScene::GetEditorScene()->mRoomFurniture[std::filesystem::path(EditFileName->GetName()).stem().string()];
						LEditorScene::GetEditorScene()->mRoomFurniture.erase(std::filesystem::path(EditFileName->GetName()).stem().string());
						std::replace(mRoomModels.begin(), mRoomModels.end(), std::filesystem::path(file->GetName()).stem().string(), FileName);
					}
					file->SetName(FileName+typeExt);
					EditFileName = nullptr;
				}
			}

			ImGui::SameLine();
			ImGui::Text(ICON_FK_MINUS_CIRCLE);
			if(ImGui::IsItemClicked()) {
				toDelete = file;
				if(typeExt == ".bin"){
					PreviewWidget::UnloadModel();
				}
			}
		}
		ImGui::TreePop();
	}

	if(toDelete != nullptr){
		dir->DeleteFile(toDelete);
	}
}

void BinTreeNodeUI(BIN::Model* model, uint32_t index){
    if(ImGui::TreeNode(std::format("Node {}", model->mGraphNodes[index].Index).c_str())){
        ImGui::Text(ICON_FK_CUBE " Meshes");
        ImGui::Indent();
        for(auto element : model->mGraphNodes[index].mDrawElements){
            ImGui::Text(std::format("Batch {}", element.BatchIndex).c_str());
        }
        ImGui::Unindent();

        ImGui::Text(ICON_FK_PAINT_BRUSH " Materials");
        ImGui::Indent();
        for(auto element : model->mGraphNodes[index].mDrawElements){
            ImGui::Text(std::format("Material {}", element.MaterialIndex).c_str());
        }
        ImGui::Unindent();

        if(model->mGraphNodes[index].ChildIndex != -1){
            ImGui::Text(ICON_FK_LEAF " Children");
            BinTreeNodeUI(model, model->mGraphNodes[index].ChildIndex);
        }

        ImGui::TreePop();
    }
    if(model->mGraphNodes[index].NextSibIndex != -1){
        BinTreeNodeUI(model, model->mGraphNodes[index].NextSibIndex);
    }
}

void LRoomDOMNode::RenderHierarchyUI(std::shared_ptr<LDOMNodeBase> self, LEditorSelection* mode_selection)
{
	// This checkbox toggles rendering of the room and all of its children.
	LUIUtility::RenderCheckBox(this);
	ImGui::SameLine();

	// Room tree start
	bool treeSelected = false;
	bool treeOpened = LUIUtility::RenderNodeSelectableTreeNode(GetName(), GetIsSelected(), treeSelected);

	bool openRoomRes = false;
	if(GetIsSelected()){
		if(PreviousRoomID != mRoomNumber && GCResourceManager.mLoadedGameArchive){
			PreviousRoomID = mRoomNumber;

			if(CurRoomNameImgID != 0){
				glDeleteTextures(1, &CurRoomNameImgID);
			}
			auto fileData = GCResourceManager.mGameArchive->GetFile(std::format("/kawano/roomname/{}", LResUtility::GetNameMap("MapTitlecards")["titlecards"][mRoomNumber].get<std::string>()));
			bStream::CMemoryStream file(fileData->GetData(), fileData->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);

			RoomTitlecard.Load(&file);
			uint8_t* roomImage = RoomTitlecard.GetData();

			glCreateTextures(GL_TEXTURE_2D, 1, &CurRoomNameImgID);
			glTextureStorage2D(CurRoomNameImgID, 1, GL_RGBA8, RoomTitlecard.mWidth, RoomTitlecard.mHeight);
			glTextureSubImage2D(CurRoomNameImgID, 0, 0, 0, RoomTitlecard.mWidth, RoomTitlecard.mHeight, GL_RGBA, GL_UNSIGNED_BYTE, roomImage);

		}
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		if(ImGui::BeginPopupModal("##roomResources", NULL, ImGuiWindowFlags_AlwaysAutoResize)){
			if(ActiveRoomArchive != nullptr){

				if(!PreviewWidget::GetActive()){
					PreviewWidget::SetActive();
				}

				// show resources in room archive, allow add/delete of models
				// notify when archive size is > 430kb!
				ImGui::Text("Room Resource Files");
				ImGui::SameLine();
				ImGui::Text(ICON_FK_PLUS_CIRCLE);
				if(ImGui::IsItemClicked()) ImGuiFileDialog::Instance()->OpenModal("addModelToRoomArchiveDialog", "Select Room Resource", "Resource (*.bin *.anm *.bas){.bin,.anm,.bas}", OPTIONS.mRootPath);
				ImGui::Separator();

				if(ActiveRoomArchive->GetRoot()->GetFolder("anm") == nullptr){
					std::shared_ptr<Archive::Folder> anmFolder = Archive::Folder::Create(ActiveRoomArchive);
					anmFolder->SetName("anm");
					ActiveRoomArchive->GetRoot()->AddSubdirectory(anmFolder);
				}

				ImGui::BeginChild("##roomResourceTree", {250, 500});
				RoomResourceManagerHandleType(self, ActiveRoomArchive->GetRoot(), "Models", ".bin");
				RoomResourceManagerHandleType(self, ActiveRoomArchive->GetRoot()->GetFolder("anm"), "Animations", ".anm");
				RoomResourceManagerHandleType(self, ActiveRoomArchive->GetRoot(), "Sounds", ".bas");
				ImGui::EndChild();

				ImGui::SameLine();
				ImGui::BeginChild("##roomResourcePreview",  {600, 500});
				ImGui::Image(static_cast<uintptr_t>(PreviewWidget::PreviewID()), {600, 500},  {0.0f, 1.0f}, {1.0f, 0.0f});

				if(ImGui::IsItemHovered()){
					if(ImGui::IsKeyDown(ImGuiKey_ModShift)){
						PreviewWidget::DoRotate(ImGui::GetIO().MouseWheel * 0.1f);
					} else {
						PreviewWidget::DoZoom(ImGui::GetIO().MouseWheel*50);
					}
				}


				ImGui::EndChild();

				if(PreviewWidget::GetFurnitureModel() != nullptr){
    				ImGui::SameLine();
                    ImGui::BeginChild("##binEditorPanels", {180, 500});
                    ImGui::Text("SceneGraph");
                    ImGui::Separator();
    				ImGui::BeginChild("##binTree", {180, 125});
                    BinTreeNodeUI(PreviewWidget::GetFurnitureModel(), 0);
    				ImGui::EndChild();
                    ImGui::Text("Resources");
                    ImGui::Separator();
    				ImGui::BeginChild("##binResources", {180, 200});
                    if(ImGui::TreeNode(ICON_FK_PICTURE_O " Textures")){
                        for(auto [idx, texture] : PreviewWidget::GetFurnitureModel()->mTexturesHeaders){
                            ImGui::Image(static_cast<uintptr_t>(texture.TextureID), {16, 16});
                            ImGui::SameLine();
                            ImGui::Text("Texture %d", idx);
                            if(ImGui::BeginPopupContextItem(std::format("##textureResourceContextMenu{}",idx).c_str())){
                                if(ImGui::Selectable("Replace")){
                                    ImGuiFileDialog::Instance()->OpenModal("replaceTextureImageDialog", "Replace Texture", "PNG Image (*.png){.png}", OPTIONS.mRootPath);
                                    BinSelectedResource = &PreviewWidget::GetFurnitureModel()->mTexturesHeaders[idx];
                                }
                                if(ImGui::Selectable("Export")){
                                    ImGuiFileDialog::Instance()->OpenModal("exportTextureImageDialog", "Export Texture", "PNG Image (*.png){.png}", OPTIONS.mRootPath);
                                    BinSelectedResource = &PreviewWidget::GetFurnitureModel()->mTexturesHeaders[idx];
                                }
                                ImGui::EndPopup();
                            }
                        }
                        ImGui::TreePop();
                    }
    				ImGui::EndChild();
                    ImGui::BeginChild("##binResourceSelected", {180, 175});
                    if(SelectedType != SelectedResourceType::None && BinSelectedResource != nullptr){

                    }
                    ImGui::EndChild();
                    ImGui::EndChild();
				}

				if(ImGui::Button("Done")){
					auto data = GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData).front();
					ActiveRoomArchive->SaveToFile(std::filesystem::path(OPTIONS.mRootPath) / "files" / std::filesystem::path(data->GetResourcePath()).relative_path());
					ActiveRoomArchive = nullptr;

					EditFileName = nullptr;
					FileName = "";
					PreviewWidget::UnloadModel();
					PreviewWidget::SetInactive();
					ImGui::CloseCurrentPopup();
				}

				if(SelectedAnimation != nullptr){
					ImGui::SameLine();
					ImGui::Text(ICON_FK_PLAY);
					if(ImGui::IsItemClicked()){
						PreviewWidget::PlayAnimation();
					}

					ImGui::SameLine();
					ImGui::Text(ICON_FK_STOP);
					if(ImGui::IsItemClicked()){
						PreviewWidget::PauseAnimation();
					}
				}

				ImGui::EndPopup();
			}
		}

		std::string modelPath;
		if(LUIUtility::RenderFileDialog("replaceTextureImageDialog", modelPath)){// error check should be here oops!
		    if(BinSelectedResource != nullptr && reinterpret_cast<BIN::TextureHeader*>(BinSelectedResource)->ImageData != nullptr){
				int w, h, channels;
				unsigned char* img = stbi_load(modelPath.c_str(), &w, &h, &channels, 4);
				reinterpret_cast<BIN::TextureHeader*>(BinSelectedResource)->SetImage(img, w*h*4, w, h);
				stbi_image_free(img);

				bStream::CMemoryStream modelData(12, bStream::Endianess::Big, bStream::OpenMode::Out);
				PreviewWidget::SaveModel(&modelData);

				auto data = GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData).front();
                std::filesystem::path resPath = std::filesystem::path(OPTIONS.mRootPath) / "files" / std::filesystem::path(data->GetResourcePath()).relative_path();
				if(std::filesystem::exists(resPath) && resPath.extension().string() == ".arc"){
    				SelectedFile->SetData(modelData.getBuffer(), modelData.tell());
    				ActiveRoomArchive->SaveToFile(resPath.string());
				}
			}
		}

		if(LUIUtility::RenderFileDialog("exportTextureImageDialog", modelPath)){// error check should be here oops!
		    if(BinSelectedResource != nullptr && reinterpret_cast<BIN::TextureHeader*>(BinSelectedResource)->ImageData != nullptr) stbi_write_png(modelPath.c_str(), reinterpret_cast<BIN::TextureHeader*>(BinSelectedResource)->Width, reinterpret_cast<BIN::TextureHeader*>(BinSelectedResource)->Height, 4, reinterpret_cast<BIN::TextureHeader*>(BinSelectedResource)->ImageData, 4*reinterpret_cast<BIN::TextureHeader*>(BinSelectedResource)->Width);
		}

		if(LUIUtility::RenderFileDialog("addModelToRoomArchiveDialog", modelPath)){
			auto data = GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData).front();

			std::filesystem::path resPath = std::filesystem::path(OPTIONS.mRootPath) / "files" / std::filesystem::path(data->GetResourcePath()).relative_path();
			LGenUtility::Log << "[RoomDOMNode]: Loading arc " << resPath.string() << std::endl;
			if(std::filesystem::exists(resPath) && resPath.extension().string() == ".arc"){
				if(ActiveRoomArchive != nullptr){
					bStream::CFileStream modelFile(modelPath, bStream::Endianess::Big, bStream::OpenMode::In);
					std::shared_ptr<Archive::File> newFile = Archive::File::Create();

					uint8_t* modelData = new uint8_t[modelFile.getSize()]{};
					modelFile.readBytesTo(modelData, modelFile.getSize());

					std::string ext = std::filesystem::path(modelPath).extension().string();

					LGenUtility::Log << "[RoomDOMNode]: Adding model " << std::filesystem::path(modelPath).filename().string() << " to archive" << resPath.string() << std::endl;
					if(ext == ".bin"){
						mRoomModels.push_back(std::filesystem::path(modelPath).filename().stem().string());
					}
					newFile->SetName(std::filesystem::path(modelPath).filename().string());
					newFile->SetData(modelData, modelFile.getSize());

					delete[] modelData;

					if(ext == ".anm"){
						std::shared_ptr<Archive::Folder> animFolder = ActiveRoomArchive->GetRoot()->GetFolder("anm");

						if(animFolder == nullptr){
							animFolder = Archive::Folder::Create(ActiveRoomArchive);
							animFolder->SetName("anm");
							ActiveRoomArchive->GetRoot()->AddSubdirectory(animFolder);
						}

						animFolder->AddFile(newFile);

					} else {
						ActiveRoomArchive->GetRoot()->AddFile(newFile);
					}
					ActiveRoomArchive->SaveToFile(resPath.string());
				}
			}
			ImGui::OpenPopup("##roomResources");
		}
	}

	if(ImGui::BeginDragDropTarget()){
		LDOMNodeBase* dragDropNode = nullptr;

		const ImGuiPayload* payload = ImGui::GetDragDropPayload();

		if (payload != nullptr && payload->Data != nullptr)
		{
			if (ImGui::AcceptDragDropPayload(payload->DataType) != nullptr) dragDropNode = *(LEntityDOMNode**)payload->Data;
		}

		// Skip if there's no pending drag and drop to handle
		if (dragDropNode != nullptr)
		{
			std::shared_ptr<LEntityDOMNode> sharedNode = dragDropNode->GetSharedPtr<LEntityDOMNode>(EDOMNodeType::Entity);
			if(!sharedNode->Parent.expired()){
				std::shared_ptr<LRoomDOMNode> oldRoom = sharedNode->Parent.lock()->GetSharedPtr<LRoomDOMNode>(EDOMNodeType::Room);

				LRoomEntityType nodeType = DOMToEntityType(sharedNode->GetNodeType());

				// Skip if source and destination groups are the same
				if (nodeType != LRoomEntityType_Max && oldRoom.get() != this)
				{

					AddChild(sharedNode);
					mRoomEntities[nodeType].push_back(sharedNode);

					for (auto iter = oldRoom->mRoomEntities[nodeType].begin(); iter != oldRoom->mRoomEntities[nodeType].end(); ++iter)
					{
						if (*iter == sharedNode)
						{
							oldRoom->mRoomEntities[nodeType].erase(iter);
							break;
						}
					}

					oldRoom->RemoveChild(sharedNode);

					if(nodeType == LRoomEntityType_Furniture){
						std::shared_ptr<LFurnitureDOMNode> furnitureNode = std::static_pointer_cast<LFurnitureDOMNode>(sharedNode);
						std::shared_ptr<LRoomDataDOMNode> oldRoomData = oldRoom->GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData)[0];
						std::shared_ptr<LRoomDataDOMNode> newRoomData = GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData)[0];

						std::filesystem::path oldResPath = std::filesystem::path(OPTIONS.mRootPath) / "files" / std::filesystem::path(oldRoomData->GetResourcePath()).relative_path();
						std::filesystem::path newResPath = std::filesystem::path(OPTIONS.mRootPath) / "files" / std::filesystem::path(newRoomData->GetResourcePath()).relative_path();

						LGenUtility::Log << "[RoomDOMNode]: copying furniture resource from " << oldResPath.string() << " to " << newResPath.string() << std::endl;
						if(std::filesystem::exists(oldResPath) && std::filesystem::exists(newResPath)){
							std::shared_ptr<Archive::Rarc> oldResArc = Archive::Rarc::Create();
							std::shared_ptr<Archive::Rarc> newResArc = Archive::Rarc::Create();
							if(oldResPath.extension() == ".arc" && newResPath.extension() == ".arc"){
								{
									bStream::CFileStream oldArcFile(oldResPath.string(), bStream::Endianess::Big, bStream::OpenMode::In);
									if(!oldResArc->Load(&oldArcFile)){
										LGenUtility::Log << "[RoomDOMNode]: Failed to load room archive " << oldResPath.string() << std::endl;
									}
									bStream::CFileStream newArcFile(newResPath.string(), bStream::Endianess::Big, bStream::OpenMode::In);
									if(!newResArc->Load(&newArcFile)){
										LGenUtility::Log << "[RoomDOMNode]: Failed to load room archive " << newResPath.string() << std::endl;
									}
								}

								if(newResArc->GetFile(furnitureNode->GetModelName()+".bin") == nullptr && oldResArc->GetFile(furnitureNode->GetModelName()+".bin") != nullptr){
									newResArc->GetRoot()->AddFile(oldResArc->GetFile(furnitureNode->GetModelName()+".bin"));
									newResArc->SaveToFile(newResPath);
								}
							} else if(oldResPath.extension() == ".arc" && newResPath.extension() == ".bin") {

								auto rootFolder = Archive::Folder::Create(newResArc);
								rootFolder->SetName(newResPath.filename().stem().string());

								newResArc->SetRoot(rootFolder);

								auto roomModel = Archive::File::Create();
								roomModel->SetName("room.bin");

								bStream::CFileStream modelFile(newResPath.string(), bStream::Endianess::Big, bStream::OpenMode::In);
								std::size_t modelFileSize = modelFile.getSize();

								uint8_t* modelData = new uint8_t[modelFileSize];

								modelFile.readBytesTo(modelData, modelFileSize);
								roomModel->SetData(modelData, modelFileSize);

								ActiveRoomArchive->GetRoot()->AddFile(roomModel);

								delete[] modelData;

								std::filesystem::path path(newRoomData->GetResourcePath());

								path.replace_extension(".arc");

								newRoomData->SetRoomResourcePath(path.string());

								newResArc->GetRoot()->AddFile(oldResArc->GetFile(furnitureNode->GetModelName()));
								newResArc->SaveToFile(path.string());

							}
						}

					}
				}
			}

		}
		ImGui::EndDragDropTarget();
	}

	if (treeOpened)
	{
		if(GetIsSelected()){
			ImGui::SameLine();
			ImGui::Spacing();
			ImGui::SameLine();
			ImGui::Text(ICON_FK_ARCHIVE);
			if(ImGui::IsItemClicked(0)){
				openRoomRes = true;
				auto data = GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData).front();
				std::filesystem::path resPath = std::filesystem::path(OPTIONS.mRootPath) / "files" / std::filesystem::path(data->GetResourcePath()).relative_path();
				LGenUtility::Log << "[RoomDOMNode]: Loading room archive " << resPath.string() << std::endl;
				if(std::filesystem::exists(resPath)){
					if(resPath.extension() == ".arc"){
						ActiveRoomArchive = Archive::Rarc::Create();
						bStream::CFileStream arcFile(resPath.string(), bStream::Endianess::Big, bStream::OpenMode::In);
						if(!ActiveRoomArchive->Load(&arcFile)){
							LGenUtility::Log << "[RoomDOMNode]: Failed to load room archive " << resPath.string() << std::endl;
							ActiveRoomArchive = nullptr;
						}
					} else {
						// This is for archive editing so we need to make an archive if its just a model
						ActiveRoomArchive = Archive::Rarc::Create();

						auto rootFolder = Archive::Folder::Create(ActiveRoomArchive);
						rootFolder->SetName(resPath.filename().stem().string());

						ActiveRoomArchive->SetRoot(rootFolder);

						auto roomModel = Archive::File::Create();
						roomModel->SetName("room.bin");

						bStream::CFileStream modelFile(resPath.string(), bStream::Endianess::Big, bStream::OpenMode::In);
						std::size_t modelFileSize = modelFile.getSize();

						uint8_t* modelData = new uint8_t[modelFileSize];

						modelFile.readBytesTo(modelData, modelFileSize);
						roomModel->SetData(modelData, modelFileSize);

						ActiveRoomArchive->GetRoot()->AddFile(roomModel);

						delete[] modelData;

						std::filesystem::path path(data->GetResourcePath());

						path.replace_extension(".arc");

						data->SetRoomResourcePath(path.string());

						ActiveRoomArchive->SaveToFile(resPath.string());
					}
				}
			}

			if(openRoomRes) ImGui::OpenPopup("##roomResources");
		}

		// Iterating all of the entity types
		for (uint32_t i = 0; i < LRoomEntityType_Max; i++)
		{
			// ImGui ID stack is now at <room name>##<i>
			ImGui::PushID(i);

			// This checkbox toggles rendering of this entire category of entities.
			// Since it isn't a real node, we'll manually toggle the entities' visibility.
			if (LUIUtility::RenderCheckBox(&mRoomEntityVisibility[i]))
			{
				for (auto n : mRoomEntities[i])
					n->SetIsRendered(mRoomEntityVisibility[i]);
			}

			// S t y l i n g
			ImGui::SameLine();
			ImGui::Indent();

			// Entity tree <i> start
			if (ImGui::TreeNodeEx(LRoomEntityTreeNodeNames[i].c_str(), ImGuiTreeNodeFlags_AllowOverlap))
			{
				ImGui::SameLine();
				ImGui::Spacing();
				ImGui::SameLine();
				ImGui::Text(ICON_FK_PLUS_CIRCLE);
				if(ImGui::IsItemClicked())
				{
					std::shared_ptr<LEntityDOMNode> newNode = nullptr;
					switch (i)
					{
						case LRoomEntityType_Furniture: newNode = std::make_shared<LFurnitureDOMNode>("furniture"); break;
						case LRoomEntityType_Observers: newNode = std::make_shared<LObserverDOMNode>("observer"); break;
						case LRoomEntityType_Objects: newNode = std::make_shared<LObjectDOMNode>("object"); break;
						case LRoomEntityType_Enemies: newNode = std::make_shared<LEnemyDOMNode>("enemy"); break;
						case LRoomEntityType_Generators: newNode = std::make_shared<LGeneratorDOMNode>("generator"); break;
						case LRoomEntityType_Paths: newNode = std::make_shared<LPathDOMNode>("path"); break;
						case LRoomEntityType_Characters: newNode = std::make_shared<LCharacterDOMNode>("character"); break;
						case LRoomEntityType_Events: newNode = std::make_shared<LEventDOMNode>("event"); break;
						case LRoomEntityType_Keys: newNode = std::make_shared<LKeyDOMNode>("key01"); break;
					}
					if(newNode != nullptr){
						auto data = GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData).front();
						if(data != nullptr){
							glm::vec3 center = data->GetMin() + ((data->GetMax() - data->GetMin()) * 0.5f);
							newNode->SetPosition(center);
							newNode->SetRoomNumber(mRoomNumber);
							AddChild(newNode);
							mRoomEntities[i].push_back(newNode);
							mode_selection->ClearSelection();
						}
					}
				}

				if(mode_selection->IsSingleSelection() && mode_selection->GetPrimarySelection()->IsNodeType(EDOMNodeType::Entity)){
					ImGui::SameLine();
					ImGui::Text(ICON_FK_MINUS_CIRCLE);
					if(ImGui::IsItemClicked()){

						auto select = mode_selection->GetPrimarySelection();
						mode_selection->ClearSelection();

						std::erase(mRoomEntities[i], select);

						// Remove from map if child of map too
						GetParentOfType<LMapDOMNode>(EDOMNodeType::Map).lock()->RemoveChild(select);
						RemoveChild(select);
						LEditorScene::SetDirty();
					}
				}
				// Iterating all of the entities of type <i>
				for (uint32_t j = 0; j < mRoomEntities[i].size(); j++)
				{
					// ImGui ID stack is now at <room name>##<i>##<j>
					ImGui::PushID(j);

					// This checkbox toggles rendering of the individual node that
					// we're currently building the UI for.
					LUIUtility::RenderCheckBox(mRoomEntities[i][j].get());
					ImGui::SameLine();

					// Render the current node however it wants to be rendered
					ImGui::Indent();
					mRoomEntities[i][j]->RenderHierarchyUI(mRoomEntities[i][j], mode_selection);
					ImGui::Unindent();

					// ImGui ID stack returns to <room name>##<i>
					ImGui::PopID();
				}

				// End entity tree <i>
				ImGui::TreePop();
			}

			// U n s t y l i n g
			ImGui::Unindent();

			// ImGui ID stack returns to <room name>
			ImGui::PopID();
		}

		//Mirrors are technically map entities not room entities, but thats ok
		if (ImGui::TreeNode("Mirrors"))
		{
			ImGui::Text(ICON_FK_PLUS_CIRCLE);
			if(ImGui::IsItemClicked(0))
			{
				std::shared_ptr<LMirrorDOMNode> mirror = std::make_shared<LMirrorDOMNode>("Mirror");
				AddChild(mirror);
			}

			ImGui::SameLine();
			ImGui::Text(ICON_FK_MINUS_CIRCLE);
			if(ImGui::IsItemClicked(0)){
				if(mode_selection->IsSingleSelection()){
					//TODO: Remove from map as well
					RemoveChild(mode_selection->GetPrimarySelection());
					mode_selection->ClearSelection();
				}
			}

			// ImGui ID stack is now at <room name>##<i>##<j>
			ImGui::PushID("mirrors");

			std::vector<std::shared_ptr<LMirrorDOMNode>> mirrors = GetChildrenOfType<LMirrorDOMNode>(EDOMNodeType::Mirror);

			for(int i = 0; i < mirrors.size(); i++){
				ImGui::PushID(i);
				// This checkbox toggles rendering of the individual node that
				// we're currently building the UI for.
				LUIUtility::RenderCheckBox(mirrors[i].get());
				ImGui::SameLine();

				// Render the current node however it wants to be rendered
				ImGui::Indent();
				mirrors[i]->RenderHierarchyUI(mirrors[i], mode_selection);
				ImGui::Unindent();

				// ImGui ID stack returns to <room name>##<i>
				ImGui::PopID();
			}
			ImGui::PopID();

			// End entity tree <i>
			ImGui::TreePop();
		}

		// End room tree
		ImGui::TreePop();
	}

	if (treeSelected)
	{
		mode_selection->AddToSelection(GetSharedPtr<LUIRenderDOMNode>(EDOMNodeType::UIRender));
	}
}

void LRoomDOMNode::RenderWaveHierarchyUI(std::shared_ptr<LDOMNodeBase> self, LEditorSelection* mode_selection)
{
	// This checkbox toggles rendering of the room and all of its children.
	LUIUtility::RenderCheckBox(this);
	ImGui::SameLine();

	// Room tree start
	if (ImGui::TreeNode(mName.c_str()))
	{
		if (Groups.size() > 0)
		{
			for (uint32_t i = 0; i < Groups.size(); i++)
			{
				ImGui::PushID(i);

				ImGui::Indent();

				bool bTreeExpanded = ImGui::TreeNode(Groups[i].GetGroupName().c_str());

				// Handle drag and drop
				if (ImGui::BeginDragDropTarget())
				{
					LDOMNodeBase* dragDropNode = GetSpawnGroupDragDropNode();

					// Skip if there's no pending drag and drop to handle
					if (dragDropNode != nullptr)
					{
						std::shared_ptr<LEntityDOMNode> sharedNode = GetSpawnGroupDragDropNode()->GetSharedPtr<LEntityDOMNode>(EDOMNodeType::Entity);
						LSpawnGroup* originGroup = GetSpawnGroupWithCreateName(sharedNode->GetCreateName());

						// Skip if source and destination groups are the same
						if (Groups[i].CreateName != originGroup->CreateName)
						{
							// Remove the node from its original group
							for (auto iter = originGroup->EntityNodes.begin(); iter != originGroup->EntityNodes.end(); ++iter)
							{
								if (*iter == sharedNode)
								{
									originGroup->EntityNodes.erase(iter);
									break;
								}
							}

							// Add the node to its new group and update its create name
							Groups[i].EntityNodes.push_back(sharedNode);
							sharedNode->SetCreateName(Groups[i].CreateName);

							LGenUtility::Log << "[RoomDOMNode]: Moved " << dragDropNode->GetName() << " from group " << originGroup->CreateName << " to group " << sharedNode->GetCreateName() << std::endl;
						}
					}

					ImGui::EndDragDropTarget();
				}

				if (bTreeExpanded)
				{
					for (auto entity : Groups[i].EntityNodes)
					{
						ImGui::Indent();
						entity->RenderHierarchyUI(entity, mode_selection);
						ImGui::Unindent();
					}

					ImGui::TreePop();
				}

				ImGui::Unindent();

				ImGui::PopID();
			}
		}

		ImGui::TreePop();
	}
}

void LRoomDOMNode::RenderDetailsUI(float dt)
{
	std::shared_ptr<LRoomDataDOMNode> dataNode = GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData)[0];

	ImGui::BeginGroup();
	ImGui::Text(ICON_FK_FOLDER_OPEN);
	if(ImGui::IsItemClicked()){
		ImGuiFileDialog::Instance()->OpenModal("openNewRoomTitlecard", "Select New Room Titlecard", "Image (*.png){.png}", ".");
	}
	ImGui::Text(ICON_FK_FLOPPY_O);
	if(ImGui::IsItemClicked()){
		ImGuiFileDialog::Instance()->OpenModal("saveRoomTitlecard", "Save Room Titlecard", "Image (*.png){.png}", ".");
	}
	ImGui::EndGroup();
	ImGui::SameLine();
	ImGui::Image(static_cast<uintptr_t>(CurRoomNameImgID), ImVec2(RoomTitlecard.mWidth, RoomTitlecard.mHeight));

	std::string imgPath;
	if(LUIUtility::RenderFileDialog("openNewRoomTitlecard", imgPath)){
		int x,y,n;
		unsigned char* img = stbi_load(imgPath.c_str(), &x, &y, &n, 0);

		std::vector<uint8_t> imgData(x*y*n);
		std::memcpy(imgData.data(), img, x*y*n);
		stbi_image_free(img);

		auto fileData = GCResourceManager.mGameArchive->GetFile(std::format("/kawano/roomname/{}", LResUtility::GetNameMap("MapTitlecards")["titlecards"][mRoomNumber].get<std::string>()));
		bStream::CMemoryStream file(0x20 + (x * y), bStream::Endianess::Big, bStream::OpenMode::Out);
		RoomTitlecard.SetData(x, y, fileData->GetData());
		RoomTitlecard.Save(&file);
		fileData->SetData(file.getBuffer(), file.getSize());

    	std::filesystem::path gameArcPath = std::filesystem::path(OPTIONS.mRootPath) / "files" / "Game" / "game_usa.szp";
        GCResourceManager.mGameArchive->SaveToFile(gameArcPath.string(), Compression::Format::YAY0);
	}

	if(LUIUtility::RenderFileDialog("saveRoomTitlecard", imgPath)){
		stbi_write_png(imgPath.c_str(), RoomTitlecard.mWidth, RoomTitlecard.mHeight, 4, RoomTitlecard.GetData(), RoomTitlecard.mWidth*4);
	}

	// Integers
	ImGui::InputInt("Lightning Direction", &mThunder);
	LUIUtility::RenderTooltip("How frequently thunder and lightning occurs while in this room.");

	ImGui::InputInt("Dust Intensity", &mDustLevel);
	LUIUtility::RenderTooltip("How dusty this room is.");

	ImGui::InputInt("Light Falloff", &mDistance);
	LUIUtility::RenderTooltip("How far light should reach inside this room.");
	ImGui::InputInt("GBH Color", &mLv);
	LUIUtility::RenderTooltip("The index of the color that this room should be on the GameBoy Horror map.");

	ImGui::InputInt("Echo Intensity", &mSoundEchoParameter);
	LUIUtility::RenderTooltip("How much sound echoes within this room.");
	ImGui::InputInt("Sound Room Code", &mSoundRoomCode);
	LUIUtility::RenderTooltip("???");
	ImGui::InputInt("Sound Room Size", &mSoundRoomSize);
	LUIUtility::RenderTooltip("???");

	ImGui::InputInt("Camera Behavior", (int32_t*)dataNode->GetCameraBehavior());
	LUIUtility::RenderTooltip("Behavior type the camera will use in this room.");

	// Bounding Box
	ImGui::Text("Bounding Box");
	glm::vec3 min = dataNode->GetMin();
	glm::vec3 max = dataNode->GetMax();
	ImGui::InputFloat3("Min", &min[0]);
	ImGui::InputFloat3("Max", &max[0]);
	dataNode->SetMin(min);
	dataNode->SetMax(max);

	// Colors
	ImGui::ColorEdit3("Darkened Color", dataNode->GetDarkColor());
	ImGui::ColorEdit3("Lit Color", mLightColor);

	// Bools
	LUIUtility::RenderCheckBox("Skybox Visible", &mShouldRenderSkybox);
	LUIUtility::RenderTooltip("Whether the map's skybox should be visible outside this room.");

	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();

	LUIUtility::RenderNodeReferenceVector("Adjacent Rooms", EDOMNodeType::Room, Parent, dataNode->GetAdjacencyList());

	//dataNode->RenderTransformUI();

	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();

	auto chestData = GetChildrenOfType<LTreasureTableDOMNode>(EDOMNodeType::TreasureTable);
	if (chestData.size() != 0 && ImGui::TreeNode("Treasure Chest Settings"))
	{
		chestData[0]->RenderDetailsUI(dt);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Advanced"))
	{
		int index = dataNode->GetRoomIndex();
		int id = dataNode->GetRoomID();

		if(ImGui::InputInt("Room Index", &index)){
			dataNode->SetRoomIndex(index);
		}

		if(ImGui::InputInt("Room ID", &id)){
			dataNode->SetRoomID(id);
		}

		ImGui::TreePop();
	}
}

void LRoomDOMNode::Deserialize(LJmpIO* JmpIO, uint32_t entry_index)
{
	mInternalName = JmpIO->GetString(entry_index, "name");

	mRoomNumber = JmpIO->GetSignedInt(entry_index, "RoomNo");
	LGenUtility::Log << "[RoomDOMNode] Read Room Number " << mRoomNumber << std::endl;
	mThunder = JmpIO->GetSignedInt(entry_index, "Thunder");

	mShouldRenderSkybox = JmpIO->GetBoolean(entry_index, "VRbox");

	mDustLevel = JmpIO->GetSignedInt(entry_index, "DustLv");

	mLightColor[0] = JmpIO->GetSignedInt(entry_index, "LightColorR") / 255.f;
	mLightColor[1] = JmpIO->GetSignedInt(entry_index, "LightColorG") / 255.f;
	mLightColor[2] = JmpIO->GetSignedInt(entry_index, "LightColorB") / 255.f;

	mDistance = JmpIO->GetSignedInt(entry_index, "Distance");
	mLv = JmpIO->GetSignedInt(entry_index, "Lv");
	mSoundEchoParameter = JmpIO->GetSignedInt(entry_index, "sound_echo_parameter");
	mSoundRoomCode = JmpIO->GetSignedInt(entry_index, "sound_room_code");

	mSoundRoomSize = JmpIO->GetUnsignedInt(entry_index, "sound_room_size");
}

void LRoomDOMNode::Serialize(LJmpIO* JmpIO, uint32_t entry_index) const
{
	JmpIO->SetString(entry_index, "name", mInternalName);

	LGenUtility::Log << "[RoomDOMNode] Writing Room Number " << mRoomNumber << std::endl;
	JmpIO->SetUnsignedInt(entry_index, "RoomNo", mRoomNumber);
	JmpIO->SetUnsignedInt(entry_index, "Thunder", mThunder);

	JmpIO->SetBoolean(entry_index, "VRbox", mShouldRenderSkybox);

	JmpIO->SetUnsignedInt(entry_index, "DustLv", mDustLevel);

	JmpIO->SetUnsignedInt(entry_index, "LightColorR", mLightColor[0] * 255.f);
	JmpIO->SetUnsignedInt(entry_index, "LightColorG", mLightColor[1] * 255.f);
	JmpIO->SetUnsignedInt(entry_index, "LightColorB", mLightColor[2] * 255.f);

	JmpIO->SetUnsignedInt(entry_index, "Distance", mDistance);
	JmpIO->SetUnsignedInt(entry_index, "Lv", mLv);
	JmpIO->SetUnsignedInt(entry_index, "sound_echo_parameter", mSoundEchoParameter);
	JmpIO->SetUnsignedInt(entry_index, "sound_room_code", mSoundRoomCode);

	JmpIO->SetUnsignedInt(entry_index, "sound_room_size", mSoundRoomSize);
}

bool LRoomDOMNode::CompleteLoad()
{
	std::filesystem::path basePath = std::filesystem::path(OPTIONS.mRootPath) / "files";
	auto roomData = GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData)[0];

	std::filesystem::path t = roomData->GetResourcePath();
	std::filesystem::path fullResPath = basePath / t.relative_path();

	if (std::filesystem::exists(fullResPath))
		//LGenUtility::Log << std::format(mName, "{0} has resource at {1}", fullResPath) << std::endl;

	// Load models here

	auto isBlackoutFilter = [](auto node) { return std::static_pointer_cast<LBlackoutDOMNode>(node)->IsActiveDuringBlackout() == true; };
	auto isNotBlackoutFilter = [](auto node) { return std::static_pointer_cast<LBlackoutDOMNode>(node)->IsActiveDuringBlackout() == false; };

	for (uint32_t i = 0; i < LRoomEntityType_Max; i++)
	{
		EDOMNodeType findType = EDOMNodeType::Base;

		switch (i)
		{
			case LRoomEntityType_Characters:
			case LRoomEntityType_BlackoutCharacters:
				findType = EDOMNodeType::Character;
				break;
			case LRoomEntityType_Enemies:
			case LRoomEntityType_BlackoutEnemies:
				findType = EDOMNodeType::Enemy;
				break;
			case LRoomEntityType_Observers:
			case LRoomEntityType_BlackoutObservers:
				findType = EDOMNodeType::Observer;
				break;
			case LRoomEntityType_Furniture:
				findType = EDOMNodeType::Furniture;
				break;
			case LRoomEntityType_Generators:
				findType = EDOMNodeType::Generator;
				break;
			case LRoomEntityType_Objects:
				findType = EDOMNodeType::Object;
				break;
			case LRoomEntityType_Paths:
				findType = EDOMNodeType::Path;
				break;
			case LRoomEntityType_Events:
				findType = EDOMNodeType::Event;
				break;
			case LRoomEntityType_Keys:
				findType = EDOMNodeType::Key;
				break;
			default:
				break;
		}

		if (i == LRoomEntityType_Characters || i == LRoomEntityType_Enemies || i == LRoomEntityType_Observers)
			mRoomEntities[i] = GetChildrenOfType<LEntityDOMNode>(findType, [](auto node) { return std::static_pointer_cast<LBlackoutDOMNode>(node)->IsActiveDuringBlackout() == false; });
		else if (i == LRoomEntityType_BlackoutCharacters || i == LRoomEntityType_BlackoutEnemies || i == LRoomEntityType_BlackoutObservers)
			mRoomEntities[i] = GetChildrenOfType<LEntityDOMNode>(findType, [](auto node) { return std::static_pointer_cast<LBlackoutDOMNode>(node)->IsActiveDuringBlackout() == true; });
		else
			mRoomEntities[i] = GetChildrenOfType<LEntityDOMNode>(findType);
	}

	LSpawnGroup defaultGroup;
	GetEntitiesWithCreateName("----", LRoomEntityType_Enemies, defaultGroup.EntityNodes);
	GetEntitiesWithCreateName("----", LRoomEntityType_Characters, defaultGroup.EntityNodes);
	Groups.push_back(defaultGroup);

	std::vector<std::shared_ptr<LEntityDOMNode>> observers = mRoomEntities[LRoomEntityType_Observers];
	auto it = std::find_if(observers.begin(), observers.end(),
		[=](std::shared_ptr<LEntityDOMNode> const& object) {
			std::shared_ptr<LObserverDOMNode> castObserver = object->GetSharedPtr<LObserverDOMNode>(EDOMNodeType::Observer);
			return (castObserver->GetDoType() == EDoType::Trigger_Spawn_Group && castObserver->GetConditionType() != EConditionType::Spawn_Group_is_Dead && castObserver->GetConditionType() != EConditionType::All_Candles_are_Lit);
		}
	);

	for (auto obs : observers)
	{
		std::shared_ptr<LObserverDOMNode> castObs = obs->GetSharedPtr<LObserverDOMNode>(EDOMNodeType::Observer);
		if (castObs->GetStringArg() == "(null)")
			continue;

		LSpawnGroup newGroup(castObs->GetStringArg(), castObs);
		GetEntitiesWithCreateName(castObs->GetStringArg(), LRoomEntityType_Enemies, newGroup.EntityNodes);
		GetEntitiesWithCreateName(castObs->GetStringArg(), LRoomEntityType_Characters, newGroup.EntityNodes);
		Groups.push_back(newGroup);
	}

	auto chestData = GetChildrenOfType<LTreasureTableDOMNode>(EDOMNodeType::TreasureTable);
	if (chestData.size() == 0)
		AddChild(std::make_shared<LTreasureTableDOMNode>(""));

	return true;
}

void LRoomDOMNode::GetEntitiesWithCreateName(const std::string CreateName, const LRoomEntityType Type, std::vector<std::shared_ptr<LEntityDOMNode>>& TargetVec)
{
	for (auto object : mRoomEntities[Type])
	{
		if (object->GetCreateName() == CreateName)
			TargetVec.push_back(object);
	}
}

LSpawnGroup* LRoomDOMNode::GetSpawnGroupWithCreateName(std::string createName)
{
	auto it = std::find_if(Groups.begin(), Groups.end(),
		[=](LSpawnGroup const& object) {
			return object.CreateName == createName;
		}
	);

	if (it == Groups.end())
		return nullptr;

	return &(*it);
}

LEntityDOMNode* LRoomDOMNode::GetSpawnGroupDragDropNode()
{
	const ImGuiPayload* payload = ImGui::GetDragDropPayload();

	if (payload != nullptr && payload->Data != nullptr)
	{
		if (payload->IsDataType("DOM_NODE_CHARACTER") || payload->IsDataType("DOM_NODE_ENEMY") || payload->IsDataType("DOM_NODE_KEY") ||
			payload->IsDataType("DOM_NODE_CHARACTER_BLACKOUT") || payload->IsDataType("DOM_NODE_ENEMY_BLACKOUT") || payload->IsDataType("DOM_NODE_KEY_BLACKOUT"))
		{
			if (ImGui::AcceptDragDropPayload(payload->DataType) == nullptr)
				return nullptr;

			IM_ASSERT(payload->DataSize == sizeof(LDOMNodeBase*));
			return *(LEntityDOMNode**)payload->Data;
		}
	}

	return nullptr;
}

void LRoomDOMNode::PostProcess(){
	LGenUtility::Log << "[RoomDOMNode]: Post Processing Room" << std::endl;
	auto data = GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData).front();
	std::filesystem::path resPath = std::filesystem::path(OPTIONS.mRootPath) / "files" / std::filesystem::path(data->GetResourcePath()).relative_path();
	if(resPath.extension() == ".arc"){
		auto arc = Archive::Rarc::Create();
		bStream::CFileStream arcFile(resPath.string(), bStream::Endianess::Big, bStream::OpenMode::In);
		if(arc->Load(&arcFile)){
			for(auto file : arc->GetRoot()->GetFiles()){
				std::string filename = std::filesystem::path(file->GetName()).filename().stem().string();
				LGenUtility::Log << "[RoomDOMNode]: Loading room model " << filename << std::endl;
				if(std::filesystem::path(file->GetName()).extension() == ".bin"){
					mRoomModels.push_back(filename);
				}
			}
		} else {
			LGenUtility::Log << "[RoomDOMNode]: Couldn't load archive " << resPath.string() << std::endl;
		}
	}
}

void LRoomDOMNode::PreProcess(){
	// open room bin from mResourceName (either raw bin or archive -> room.bin)
	// move root by mRoomDelta

	if(mRoomModelDelta == glm::vec3(0.0f)) return; // only update model if moved

	auto data = GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData).front();

	std::filesystem::path resPath = std::filesystem::path(OPTIONS.mRootPath) / "files" / std::filesystem::path(data->GetResourcePath()).relative_path();

	if(!std::filesystem::exists(resPath)) return;

	if(resPath.extension() == ".bin"){
		size_t binFileSize = 0;
		uint32_t offset;
		float x = 0.0f, y = 0.0f, z = 0.0f;

		{
			bStream::CFileStream stream(resPath.string(), bStream::Endianess::Big, bStream::OpenMode::In);

			uint32_t offsets[21];
			stream.seek(12);

			for (std::size_t o = 0; o < 21; o++)
			{
				offsets[o] = stream.readUInt32();
			}

			offset = offsets[12] + 0x24;

			stream.seek(offsets[12] + 0x24);
			x = stream.readFloat();
			y = stream.readFloat();
			z = stream.readFloat();
		}

		{
			bStream::CFileStream binWriteStream(resPath.string(), bStream::Endianess::Big, bStream::OpenMode::Out);
			binWriteStream.seek(offset);
			binWriteStream.writeFloat(mRoomModelDelta.z + x);
			binWriteStream.writeFloat(mRoomModelDelta.y + y);
			binWriteStream.writeFloat(mRoomModelDelta.x + z);
		}

	} else if(resPath.extension() == ".arc"){
		std::shared_ptr<Archive::Rarc> roomResource = Archive::Rarc::Create();

		{
			bStream::CFileStream stream(resPath.string(), bStream::Endianess::Big, bStream::OpenMode::In);
			roomResource->Load(&stream);
		}

		std::shared_ptr<Archive::File> roomBin = roomResource->GetFile("room.bin");

		if(roomBin != nullptr){
			float x = 0.0f, y = 0.0f, z = 0.0f;
			uint32_t offsets[21];

			{
				bStream::CMemoryStream stream(roomBin->GetData(), roomBin->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);

				stream.seek(12);
				for (std::size_t o = 0; o < 21; o++)
				{
					offsets[o] = stream.readUInt32();
				}

				stream.seek(offsets[12] + 0x24);

				x = stream.readFloat();
				y = stream.readFloat();
				z = stream.readFloat();
			}

			{
				bStream::CMemoryStream stream(roomBin->GetData(), roomBin->GetSize(), bStream::Endianess::Big, bStream::OpenMode::Out);
				stream.seek(offsets[12] + 0x24);
				stream.writeFloat(mRoomModelDelta.z + x);
				stream.writeFloat(mRoomModelDelta.y + y);
				stream.writeFloat(mRoomModelDelta.x + z);
			}
		}

		roomResource->SaveToFile(resPath);
	}


}
