#include "ResUtil.hpp"
#include "Options.hpp"
#include "ui/BloEditor.hpp"
#include "UIUtil.hpp"
#include "imgui.h"
#include <map>
#include <format>
#include "ImGuiFileDialog/ImGuiFileDialog.h"
#include <Bti.hpp>
#include "stb_image.h"

namespace BloEditor {

std::map<std::string, std::tuple<std::filesystem::path, std::string, std::string, std::string>> Menus {
    { "Save Screen", {"Kawano/ENGLISH/res_save.szp", "save_2.blo", "timg", "font"}},
    { "File Select", {"Kawano/ENGLISH/res_slct.szp", "blo/file_select_1.blo", "timg", "font"}},
    { "New Game", {"Kawano/ENGLISH/res_slct.szp", "blo/file_select_2.blo", "timg", "font"}},
    { "Options Menu", {"Kawano/ENGLISH/res_slct.szp", "blo/option_1.blo", "timg", "font"}},
    { "Area Complete 0", {"Kawano/ENGLISH/res_acnt.szp", "blo/adjustment_0.blo", "timg", "font"}},
    { "Area Complete 1", {"Kawano/ENGLISH/res_acnt.szp", "blo/adjustment_1.blo", "timg", "font"}},
    { "Area Complete 2", {"Kawano/ENGLISH/res_acnt.szp", "blo/adjustment_2.blo", "timg", "font"}},
    { "Controls Explantion", {"Kawano/ENGLISH/res_cont.szp", "controller_2.blo", "timg", "font"}},
    { "GBH Treasure", {"game", "kawano/list/blo/sgb_1.blo", "kawano/base/timg", "kawano/base/font"}},
    { "Map Screen", {"game", "kawano/base/blo/map_1.blo", "kawano/base/timg", "kawano/base/font"}},
    { "GBH Scan Menu", {"game", "kawano/base/blo/gbf_1.blo", "kawano/base/timg", "kawano/base/font"}},
    { "GBH Border Menu", {"game", "kawano/base/blo/gbf_0.blo", "kawano/base/timg", "kawano/base/font"}},
    { "Hidden Mansion Star", {"game", "kawano/base/blo/star_1.blo", "kawano/base/timg", "kawano/base/font"}}
};

bool BloEditorOpen { false };
std::string MenuSelected = "Save Screen";
std::shared_ptr<Archive::Rarc> MenuArc { nullptr };
std::shared_ptr<Archive::Folder> ImageFolder { nullptr };
std::shared_ptr<Archive::Folder> FontFolder { nullptr };
std::shared_ptr<Blo::Screen> ScreenLoaded { nullptr };
std::shared_ptr<Blo::Pane> SelectedNode { nullptr };
std::shared_ptr<Blo::Pane> DraggingNode { nullptr };
ImVec2 DragStart {};

void Render(){
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize({ImGui::GetMainViewport()->Size.x * 0.675f, ImGui::GetMainViewport()->Size.y * 0.78f}, ImGuiCond_Always);
	if (ImGui::BeginPopupModal("BloEditorTool", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiChildFlags_AlwaysAutoResize))
	{
		ImGui::Text("Menu Editor");
        ImGui::Separator();

        if(ScreenLoaded == nullptr){
            auto menuInfo = Menus["Save Screen"];
            ScreenLoaded = std::make_shared<Blo::Screen>();
            std::filesystem::path menuResPath = std::get<0>(menuInfo);
            std::string menuBloPath = std::get<1>(menuInfo);
            std::string menuTimgPath = std::get<2>(menuInfo);
            std::string menuFontPath = std::get<3>(menuInfo);

            std::shared_ptr<Archive::Rarc> menuarc = Archive::Rarc::Create();
            bStream::CFileStream fstrm((std::filesystem::path(OPTIONS.mRootPath) / "files" / menuResPath).string(), bStream::Endianess::Big, bStream::OpenMode::In);
            if(menuarc->Load(&fstrm)){
                auto file = menuarc->GetFile(menuBloPath);
                auto timg = menuarc->GetFolder(menuTimgPath);
                
                MenuArc = menuarc;

                ImageFolder = timg;
                if(ImageFolder == nullptr){
                    ImageFolder = Archive::Folder::Create(MenuArc);
                    ImageFolder->SetName("timg");

                    auto parent = MenuArc->GetFolder(std::filesystem::path(menuTimgPath).parent_path());
                    if(parent != nullptr) parent->AddSubdirectory(ImageFolder);
                }

                FontFolder = menuarc->GetFolder(menuFontPath);
                if(FontFolder == nullptr){
                    FontFolder = Archive::Folder::Create(MenuArc);
                    FontFolder->SetName("font");

                    auto parent = MenuArc->GetFolder(std::filesystem::path(menuTimgPath).parent_path());
                    if(parent != nullptr) parent->AddSubdirectory(FontFolder);
                }

                bStream::CMemoryStream stream(file->GetData(), file->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
                ScreenLoaded->Load(&stream, timg);
            }
        }

        if(ImGui::BeginCombo("##menuSelect", MenuSelected.c_str())){
            for(auto [menuName, menuInfo] : Menus){
                if(ImGui::Selectable(menuName.c_str())){
                    MenuSelected = menuName;
                    ScreenLoaded = std::make_shared<Blo::Screen>();
                    std::filesystem::path menuResPath = std::get<0>(menuInfo);
                    std::string menuBloPath = std::get<1>(menuInfo);
                    std::string menuTimgPath = std::get<2>(menuInfo);

                    if(menuResPath.string() == "game" && GCResourceManager.mLoadedGameArchive){
                        auto file = GCResourceManager.mGameArchive->GetFile(menuBloPath);
                        auto timg = GCResourceManager.mGameArchive->GetFolder(menuTimgPath);
                        ImageFolder = timg;
                        MenuArc = GCResourceManager.mGameArchive;
                        bStream::CMemoryStream stream(file->GetData(), file->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
                        ScreenLoaded->Load(&stream, timg);
                    } else if(menuResPath.string() != "game") {
                        std::shared_ptr<Archive::Rarc> menuarc = Archive::Rarc::Create();
                        bStream::CFileStream fstrm((std::filesystem::path(OPTIONS.mRootPath) / "files" / menuResPath).string(), bStream::Endianess::Big, bStream::OpenMode::In);
                        if(menuarc->Load(&fstrm)){
                            auto file = menuarc->GetFile(menuBloPath);
                            auto timg = menuarc->GetFolder(menuTimgPath);
                            ImageFolder = timg;
                            bStream::CMemoryStream stream(file->GetData(), file->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
                            ScreenLoaded->Load(&stream, timg);
                        }
                        MenuArc = menuarc;
                    }
                }
            }
            ImGui::EndCombo();
        }        

        ImGui::SameLine();
        
        auto style = ImGui::GetStyle();
        float saveButton = ImGui::CalcTextSize("Save").x + style.FramePadding.x * 2.0f;
        float closeButton = ImGui::CalcTextSize("Close").x + style.FramePadding.x * 2.0f;
        float widthNeeded = saveButton + style.ItemSpacing.x + closeButton;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - widthNeeded);

        if(ImGui::Button("Save")){
            auto menuInfo = Menus[MenuSelected];
            std::filesystem::path menuResPath = std::get<0>(menuInfo);
            std::string menuBloPath = std::get<1>(menuInfo);
            std::string menuTimgPath = std::get<2>(menuInfo);

            if(menuResPath.string() == "game" && GCResourceManager.mLoadedGameArchive){
                auto file = GCResourceManager.mGameArchive->GetFile(menuBloPath);
                auto timg = GCResourceManager.mGameArchive->GetFolder(menuTimgPath);

                bStream::CMemoryStream stream(64, bStream::Endianess::Big, bStream::OpenMode::In);
                ScreenLoaded->Save(&stream);
                file->SetData(stream.getBuffer(), stream.getSize());
                std::filesystem::path gameArcPath = std::filesystem::path(OPTIONS.mRootPath) / "files" / "Game" / "game_usa.szp";
                GCResourceManager.mGameArchive->SaveToFile(gameArcPath.string(), Compression::Format::YAY0);

            } else if(menuResPath.string() != "game") {
                if(MenuArc != nullptr){
                    auto file = MenuArc->GetFile(menuBloPath);
                    auto timg = MenuArc->GetFolder(menuTimgPath);

                    bStream::CMemoryStream stream(64, bStream::Endianess::Big, bStream::OpenMode::In);
                    ScreenLoaded->Save(&stream);

                    LGenUtility::Log << "Finished Saving BLO, writing to archive..." << std::endl;

                    file->SetData(stream.getBuffer(), stream.getSize());

                    MenuArc->SaveToFile((std::filesystem::path(OPTIONS.mRootPath) / "files" / menuResPath).string(), Compression::Format::YAY0, 0, true);

                }
            }

        }

        ImGui::SameLine();

        if(ImGui::Button("Close")){
            ImGui::CloseCurrentPopup();
        }

        ImGui::Separator();

        auto blopos = ImGui::GetCursorScreenPos();
        if(ScreenLoaded != nullptr){
            ScreenLoaded->Draw(DraggingNode);
            if(DraggingNode != nullptr && DragStart.x == 0 && DragStart.y == 0){
                SelectedNode = DraggingNode;
                if(ImGui::IsKeyDown(ImGuiKey_LeftShift)){
                    DragStart = {static_cast<float>(DraggingNode->mRect[2]), static_cast<float>(DraggingNode->mRect[3])};
                } else {
                    DragStart = {static_cast<float>(DraggingNode->mRect[0]), static_cast<float>(DraggingNode->mRect[1])};
                }
            }
            ImGui::SameLine();
            ImGui::BeginChild("##leftHandWindow");
            ImGui::Text("Resources");
            ImGui::Separator();
            ImGui::BeginChild("##resView", {ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y*0.15f});
            if(ImGui::TreeNode("Images (timg)")){
                std::shared_ptr<Archive::File> toDelete = nullptr;
                for(auto file : ImageFolder->GetFiles()){
                    ImGui::Text(file->GetName().data());
                    if(ImGui::BeginPopupContextItem(std::format("##rm{}", file->GetName()).c_str())){
                        if(ImGui::Selectable("Remove")){
                            toDelete = file;
                        }
                        ImGui::EndPopup();
                    }
                }
                if(toDelete != nullptr){
                    std::erase(ImageFolder->GetFiles(), toDelete);
                }
                ImGui::Text(ICON_FK_PLUS_CIRCLE);
                if(ImGui::IsItemClicked(0) && MenuArc != nullptr){
                    ImGuiFileDialog::Instance()->OpenModal("ImportBloPNG", "Import PNG", "PNG Image (*.png){.png}", std::filesystem::current_path().string());
                    ImGui::CloseCurrentPopup();
                }
                ImGui::TreePop();
            }

            if(ImGui::TreeNode("Fonts (font)")){
                std::shared_ptr<Archive::File> toDelete = nullptr;
                for(auto file : FontFolder->GetFiles()){
                    ImGui::Text(file->GetName().data());
                    if(ImGui::BeginPopupContextItem(std::format("##rm{}", file->GetName()).c_str())){
                        if(ImGui::Selectable("Remove")){
                            toDelete = file;
                        }
                        ImGui::EndPopup();
                    }
                }
                if(toDelete != nullptr){
                    std::erase(FontFolder->GetFiles(), toDelete);
                }
                ImGui::Text(ICON_FK_PLUS_CIRCLE);
                if(ImGui::IsItemClicked(0) && MenuArc != nullptr){
                    ImGuiFileDialog::Instance()->OpenModal("ImportBfnFont", "Import BFN", "BFN Font (*.bfn){.bfn}", std::filesystem::current_path().string());
                    ImGui::CloseCurrentPopup();
                }
                ImGui::TreePop();
            }

            ImGui::EndChild();
            ImGui::Text("Screen Elements");
            ImGui::Separator();
            ImGui::BeginChild("##hierarchy", {ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y*(0.5f-0.15f)});
            ScreenLoaded->DrawHierarchy(SelectedNode);
            ImGui::EndChild();
            ImGui::BeginChild("##properties", {ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y});
            ImGui::Text("Selected");
            ImGui::Separator();
            if(SelectedNode != nullptr){
                std::vector<char> id(4, '\0');
                std::memcpy(id.data(), &SelectedNode->mID, sizeof(uint32_t));
                std::reverse(id.begin(), id.end());
                id.push_back('\0');
                
                if(ImGui::InputText("##idEdit", id.data(), id.size())){
                    id.pop_back();
                    std::reverse(id.begin(), id.end());
                    std::memcpy(&SelectedNode->mID, id.data(), sizeof(uint32_t));
                }

                ImGui::Text("Rect");
                int x = SelectedNode->mRect[0], y = SelectedNode->mRect[1], w = SelectedNode->mRect[2], h = SelectedNode->mRect[3];
                ImGui::InputInt("x", &x);
                ImGui::InputInt("y", &y);
                ImGui::InputInt("w", &w);
                ImGui::InputInt("h", &h);
                SelectedNode->mRect[0] = x, SelectedNode->mRect[1] = y, SelectedNode->mRect[2] = w, SelectedNode->mRect[3] = h;

                ImGui::Text("Alpha");
                int paneAlpha = static_cast<int>(SelectedNode->mAlpha);
                if(ImGui::SliderInt("##elementAlpha", &paneAlpha, 0, 255)){
                    SelectedNode->mAlpha = static_cast<uint8_t>(paneAlpha);
                }

                switch (SelectedNode->Type()) {
                case Blo::ElementType::Picture: {
                    auto node = std::reinterpret_pointer_cast<Blo::Picture>(SelectedNode);
                    if(LUIUtility::RenderTextInput("Texture Name", &node->GetTexture()->mPath)){
                        std::shared_ptr<Archive::File> imgFile = ImageFolder->GetFile(node->GetTexture()->mPath);

                        if(imgFile == nullptr){
                            std::string lowerPath = node->GetTexture()->mPath;
                            std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), [](unsigned char c){ return std::tolower(c); });
                            imgFile = ImageFolder->GetFile(lowerPath);
                        }

                        if(imgFile != nullptr){
                            //std::cout << "Loading Image " << mPath << std::endl;
                            bStream::CMemoryStream img(imgFile->GetData(), imgFile->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
                            node->GetTexture()->mTexture.Load(&img);
                            node->SetWidth(node->GetTexture()->mTexture.mWidth);
                            node->SetHeight(node->GetTexture()->mTexture.mHeight);
                            
                            glCreateTextures(GL_TEXTURE_2D, 1, &node->GetTexture()->mTextureID);
                            glTextureParameteri(node->GetTexture()->mTextureID, GL_TEXTURE_WRAP_S, GL_REPEAT);
                            glTextureParameteri(node->GetTexture()->mTextureID, GL_TEXTURE_WRAP_T, GL_REPEAT);
                            glTextureParameteri(node->GetTexture()->mTextureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                            glTextureParameteri(node->GetTexture()->mTextureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                            glTextureStorage2D(node->GetTexture()->mTextureID, 1, GL_RGBA8, node->GetTexture()->mTexture.mWidth, node->GetTexture()->mTexture.mHeight);
                            glTextureSubImage2D(node->GetTexture()->mTextureID, 0, 0, 0, node->GetTexture()->mTexture.mWidth, node->GetTexture()->mTexture.mHeight, GL_RGBA, GL_UNSIGNED_BYTE, node->GetTexture()->mTexture.GetData());
                        } else {
                            node->GetTexture()->mTextureID = 0xFFFFFFFF;
                            LGenUtility::Log << "Couldn't load image resource " << node->GetTexture()->mPath << std::endl;
                        }
                    }

                    ImGui::Text("To Color");
                    ImGui::ColorEdit4("##windowToColorEdit", &node->GetToColor()->r);

                    ImGui::Text("From Color");
                    ImGui::ColorEdit4("##windowFromColorEdit", &node->GetFromColor()->r);

                    Blo::ResourceType type = (Blo::ResourceType)node->GetTexture()->mType;
                    if(LUIUtility::RenderComboEnum<Blo::ResourceType>("Resource Type", type)){
                        node->GetTexture()->mType = (uint8_t)type;
                    }
                    break;
                }
                case Blo::ElementType::Textbox: {
                    auto node = std::reinterpret_pointer_cast<Blo::Textbox>(SelectedNode);
                    
                    LUIUtility::RenderTextInput("Font Path ", &node->GetFont()->mPath);
                    Blo::ResourceType type = (Blo::ResourceType)node->GetFont()->mType;
                    if(LUIUtility::RenderComboEnum<Blo::ResourceType>("Resource Type", type)){
                        node->GetFont()->mType = (uint8_t)type;
                    }

                    LUIUtility::RenderTextInput("Text ", node->GetText());
                    ImGui::Text("Top Color");
                    ImGui::ColorEdit4("##windowTopColorEdit", &node->mTopColor.r);

                    ImGui::Text("Bottom Color");
                    ImGui::ColorEdit4("##windowBottomColorEdit", &node->mBottomColor.r);

                    ImGui::Text("To Color");
                    ImGui::ColorEdit4("##windowToColorEdit", &node->GetToColor()->r);

                    ImGui::Text("From Color");
                    ImGui::ColorEdit4("##windowFromColorEdit", &node->GetFromColor()->r);

                    int spacing = node->GetFontSpacing();
                    int leading = node->GetFontLeading();
                    int width = node->GetFontWidth();
                    int height = node->GetFontHeight();

                    ImGui::Text("Font Spacing");
                    ImGui::SameLine();
                    if(ImGui::InputInt("##textSpacingEdit", &spacing)){
                        node->SetFontSpacing(spacing);
                    }

                    ImGui::Text("Font Leading");
                    ImGui::SameLine();
                    if(ImGui::InputInt("##textLeadingEdit", &leading)){
                        node->SetFontLeading(leading);
                    }

                    ImGui::Text("Font Width");
                    ImGui::SameLine();
                    if(ImGui::InputInt("##textWidthEdit", &width)){
                        node->SetFontWidth(width);
                    }

                    ImGui::Text("Font Height");
                    ImGui::SameLine();
                    if(ImGui::InputInt("##textHeightEdit", &height)){
                        node->SetFontHeight(height);
                    }

                    break;
                }
                case Blo::ElementType::Window: {
                    auto node = std::reinterpret_pointer_cast<Blo::Window>(SelectedNode);
                    ImGui::Text("To Color");
                    ImGui::ColorEdit4("##windowToColorEdit", &node->GetToColor()->r);

                    ImGui::Text("From Color");
                    ImGui::ColorEdit4("##windowFromColorEdit", &node->GetFromColor()->r);

                    if(node->GetContentTexture() != nullptr){
                        LUIUtility::RenderTextInput("Content Texture", &node->GetContentTexture()->mPath);
                        Blo::ResourceType type = (Blo::ResourceType)node->GetContentTexture()->mType;
                        if(LUIUtility::RenderComboEnum<Blo::ResourceType>("Resource Type##contentTexWin", type)){
                            node->GetContentTexture()->mType = (uint8_t)type;
                        }
                    }

                    LUIUtility::RenderTextInput("Texture 1 Name", &node->GetTexture(0)->mPath);
                    ImGui::Text("Texture 1 Overlay Color");
                    ImGui::ColorEdit4("##tex1OverlayColor", &node->GetTexture(0)->mColor.r);                    
                    Blo::ResourceType type = (Blo::ResourceType)node->GetTexture(0)->mType;
                    if(LUIUtility::RenderComboEnum<Blo::ResourceType>("Resource Type##win0", type)){
                        node->GetTexture(0)->mType = (uint8_t)type;
                    }
                    
                    LUIUtility::RenderTextInput("Texture 2 Name", &node->GetTexture(1)->mPath);
                    ImGui::Text("Texture 2 Overlay Color");
                    ImGui::ColorEdit4("##tex2OverlayColor", &node->GetTexture(1)->mColor.r);
                    type = (Blo::ResourceType)node->GetTexture(1)->mType;
                    if(LUIUtility::RenderComboEnum<Blo::ResourceType>("Resource Type##win1", type)){
                        node->GetTexture(1)->mType = (uint8_t)type;
                    }
                    
                    LUIUtility::RenderTextInput("Texture 3 Name", &node->GetTexture(2)->mPath);
                    ImGui::Text("Texture 3 Overlay Color");
                    ImGui::ColorEdit4("##tex3OverlayColor", &node->GetTexture(2)->mColor.r);
                    type = (Blo::ResourceType)node->GetTexture(2)->mType;
                    if(LUIUtility::RenderComboEnum<Blo::ResourceType>("Resource Type##win2", type)){
                        node->GetTexture(2)->mType = (uint8_t)type;
                    }                    

                    LUIUtility::RenderTextInput("Texture 4 Name", &node->GetTexture(3)->mPath);
                    ImGui::Text("Texture 4 Overlay Color");
                    ImGui::ColorEdit4("##tex4OverlayColor", &node->GetTexture(3)->mColor.r);
                    type = (Blo::ResourceType)node->GetTexture(3)->mType;
                    if(LUIUtility::RenderComboEnum<Blo::ResourceType>("Resource Type##win3", type)){
                        node->GetTexture(3)->mType = (uint8_t)type;
                    }

                    ImGui::Text("Content Rect");
                    int x = node->mContentRect[0], y = node->mContentRect[1], w = node->mContentRect[2], h = node->mContentRect[3];
                    ImGui::InputInt("x##content", &x);
                    ImGui::InputInt("y##content", &y);
                    ImGui::InputInt("w##content", &w);
                    ImGui::InputInt("h##content", &h);
                    node->mContentRect[0] = x, node->mContentRect[1] = y, node->mContentRect[2] = w, node->mContentRect[3] = h;

                    break;
                }
                default:
                    break;
                }
            }
            ImGui::EndChild();
            ImGui::EndChild();
        }

        if(DraggingNode != nullptr){
            auto delta = ImGui::GetMouseDragDelta();
            if(ImGui::IsKeyDown(ImGuiKey_LeftShift)){
                DraggingNode->mRect[2] = static_cast<uint16_t>(DragStart.x + delta.x);
                DraggingNode->mRect[3] = static_cast<uint16_t>(DragStart.y + delta.y);
            } else {
                DraggingNode->mRect[0] = static_cast<uint16_t>(DragStart.x + delta.x);
                DraggingNode->mRect[1] = static_cast<uint16_t>(DragStart.y + delta.y);
            }

            if(ImGui::IsMouseReleased(0) || ImGui::IsKeyReleased(ImGuiKey_LeftShift) || ImGui::IsKeyPressed(ImGuiKey_LeftShift)){
                DraggingNode = nullptr;
                DragStart = {0, 0};
            }
        }

		ImGui::EndPopup();
	}

    std::string resPath;
    if(LUIUtility::RenderFileDialog("ImportBloPNG", resPath)){
		Bti img;
        img.SetFormat(0x05);

		int x,y,n;
		unsigned char* imgBuffer = stbi_load(resPath.c_str(), &x, &y, &n, 0);
		
		std::vector<uint8_t> imgData(x*y*n);
		std::memcpy(imgData.data(), imgBuffer, x*y*n);
        stbi_image_free(imgBuffer);

        bStream::CMemoryStream file(0x20 + (x * y), bStream::Endianess::Big, bStream::OpenMode::Out);
        img.SetData(x, y, imgData.data());
		img.Save(&file);

        std::shared_ptr<Archive::File> newImg = Archive::File::Create();
        newImg->SetData(file.getBuffer(), file.getSize());
        newImg->SetName(std::filesystem::path(resPath).stem().replace_extension(".bti").string());
        ImageFolder->AddFile(newImg);

        ImGui::OpenPopup("BloEditorTool");
    }

    if(LUIUtility::RenderFileDialog("ImportBfnFont", resPath)){
        std::shared_ptr<Archive::File> newImg = Archive::File::Create();
        
        bStream::CFileStream file(resPath, bStream::Endianess::Big, bStream::OpenMode::In);

        std::vector<uint8_t> fileData(file.getSize());
        file.readBytesTo(fileData.data(), fileData.size());

        newImg->SetData(fileData.data(), fileData.size());
        newImg->SetName(std::filesystem::path(resPath).filename().string());
        FontFolder->AddFile(newImg);
        ImGui::OpenPopup("BloEditorTool");
    }
}

}