#include "io/PrmIO.hpp"
#include "UIUtil.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include <filesystem>
#include "Options.hpp"
#include "ImGuiFileDialog/ImGuiFileDialog.h"
#include "stb_image.h"
#include <glad/glad.h>
#include "scene/ModelViewer.hpp"
#include "GhostImg.hpp"

constexpr float pi = 3.14159f;

namespace {
    uint32_t mGhostImg = 0;
}

void SwapVec4(glm::vec4* s)
{
    float a = s->r;
    s->r = s->a;
    s->a = a;
    a = s->g;
    s->g = s->b;
    s->b = a;
}

bool AngleVisualizer(const char* label, float* angle)
{
	ImGuiIO& io = ImGui::GetIO();
	ImGuiStyle& style = ImGui::GetStyle();

	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 center = ImVec2(pos.x + 32, pos.y + 32);

	float angle_cos = cosf((*angle) * (pi / 180)), angle_sin = sinf((*angle) * (pi / 180));
	float radius_inner = 16.0f;
	draw_list->AddLine(ImVec2(center.x - angle_sin*radius_inner, center.y + angle_cos*radius_inner), ImVec2(center.x - angle_sin*(30), center.y + angle_cos*(30)), ImGui::GetColorU32(ImGuiCol_SliderGrabActive), 2.0f);
	draw_list->AddLine(ImVec2(center.x, center.y + radius_inner), ImVec2(center.x, center.y + 30), ImGui::GetColorU32(ImGuiCol_SliderGrabActive), 2.0f);

    //TODO: figure out why this breaks the combo box
    //draw_list->PathArcTo(ImVec2(center.x, center.y), 30.0f, 0.0f, -((*angle) * (pi / 180)), 10);

	draw_list->AddImage(static_cast<uintptr_t>(mGhostImg), ImVec2(center.x - 8, center.y - 8), ImVec2(center.x + 8, center.y + 8));


    pos.y += 80;
    ImGui::SetCursorScreenPos(pos);

	return true;
}

LPrmIO::LPrmIO() : mConfigsLoaded(false), mSelectedConfig(0) {
}
LPrmIO::~LPrmIO() {
    glDeleteTextures(1, &mGhostImg);
}

void LPrmIO::LoadConfigs(std::shared_ptr<LMapDOMNode>& map)
{
    mMap = map;

    if(GCResourceManager.mLoadedGameArchive){
        std::shared_ptr<Archive::Folder> ctpFolder = GCResourceManager.mGameArchive->GetFolder("/param/ctp");

        if(ctpFolder != nullptr){
            for(auto paramFile : ctpFolder->GetFiles()){
                auto name = std::filesystem::path(paramFile->GetName()).filename().stem();
                bStream::CMemoryStream prm(paramFile->GetData(), paramFile->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
                Load(name.string(), &prm);
                mLoadedConfigs.push_back(name.string());

            }
        }
    }
    
    mConfigsLoaded = true;
}

void LPrmIO::SaveConfigsToFile()
{
    if(GCResourceManager.mLoadedGameArchive && mConfigsLoaded){
        std::shared_ptr<Archive::Folder> ctpFolder = GCResourceManager.mGameArchive->GetFolder("/param/ctp");

        if(ctpFolder != nullptr){
            for(auto paramFile : ctpFolder->GetFiles()){
                auto name = std::filesystem::path(paramFile->GetName()).filename().stem();
                bStream::CMemoryStream prm(700, bStream::Endianess::Big, bStream::OpenMode::Out);
                Save(name.string(), &prm); 
                paramFile->SetData(prm.getBuffer(), prm.getSize());
            }
        }
    	std::filesystem::path gameArcPath = std::filesystem::path(OPTIONS.mRootPath) / "files" / "Game" / "game_usa.szp";
        GCResourceManager.mGameArchive->SaveToFile(gameArcPath.string(), Compression::Format::YAY0);
    }
    
}

// Because these properties are consistent, we can write all of them and the game wont really care, so long as the ones it is looking for are present.

void WritePropertyInt32(bStream::CStream* stream, uint16_t hash, std::string name, uint32_t v){
    stream->writeUInt16(hash);
    stream->writeUInt16(name.size());
    stream->writeString(name);
    stream->writeUInt32(4);
    stream->writeUInt32(v);
};

void WritePropertyFloat(bStream::CStream* stream, uint16_t hash, std::string name, float v){
    stream->writeUInt16(hash);
    stream->writeUInt16(name.size());
    stream->writeString(name);
    stream->writeUInt32(4);
    stream->writeFloat(v);
};

void WritePropertyInt16(bStream::CStream* stream, uint16_t hash, std::string name, uint16_t v){
    stream->writeUInt16(hash);
    stream->writeUInt16(name.size());
    stream->writeString(name);
    stream->writeUInt32(2);
    stream->writeUInt16(v);
};

void LPrmIO::Save(std::string name, bStream::CStream * stream)
{
    auto itemFishingNodes = mMap.lock()->GetChildrenOfType<LItemFishingDOMNode>(EDOMNodeType::ItemFishing);
    auto itemAppearNodes = mMap.lock()->GetChildrenOfType<LItemAppearDOMNode>(EDOMNodeType::ItemAppear);
    auto prm = mCtpParams[name];

    uint32_t brightColor = 0;
    brightColor |= ((uint32_t)(prm->mBrightColor.r*255)) << 24;
    brightColor |= ((uint32_t)(prm->mBrightColor.g*255)) << 16;
    brightColor |= ((uint32_t)(prm->mBrightColor.b*255)) << 8;
    brightColor |= ((uint32_t)(prm->mBrightColor.a*255));

    uint32_t ambColor = 0;
    ambColor |= ((uint32_t)(prm->mAmbColor.r*255)) << 24;
    ambColor |= ((uint32_t)(prm->mAmbColor.g*255)) << 16;
    ambColor |= ((uint32_t)(prm->mAmbColor.b*255)) << 8;
    ambColor |= ((uint32_t)(prm->mAmbColor.a*255));

    stream->writeUInt32(28);

    for(int m = 0; m < mCtpParams[name]->mEnabledProperties.size(); m++){
        switch(mCtpParams[name]->mEnabledProperties[m]){
        case 0xa62f:
            WritePropertyInt32(stream, 0xa62f, "mLife", prm->mLife);
            break;
        case 0x2528:
            WritePropertyInt32(stream, 0x2528, "mHitDamage", prm->mHitDamage);
            break;
        case 0xad88:
            WritePropertyInt32(stream, 0xad88, "mSpeed", prm->mSpeed);
            break;
        case 0x93d1:
            WritePropertyInt32(stream, 0x93d1, "mSpeedUnseen", prm->mSpeedUnseen);
            break;
        case 0x4b45:
            WritePropertyInt32(stream, 0x4b45, "mSpeedFight", prm->mSpeedFight);
            break;
        case 0x5f6a:
            WritePropertyInt32(stream, 0x5f6a, "mEyesight", prm->mEyesight);
            break;
        case 0xd9f9:
            WritePropertyInt32(stream, 0xd9f9, "mLightBindFrame", prm->mLightBindFrame);
            break;
        case 0x29fe:
            WritePropertyFloat(stream, 0x29fe, "mMinLightBindRange", prm->mMinLightBindRange);
            break;
        case 0xac49:
            WritePropertyFloat(stream, 0xac49, "mMaxLightBindRange", prm->mMaxLightBindRange);
            break;
        case 0x30aa:
            WritePropertyInt32(stream, 0x30aa, "mNumAtkKarakai", prm->mNumAtkKarakai);
            break;
        case 0x560a:
            WritePropertyInt32(stream, 0x560a, "mNumAtkOrooro", prm->mNumAtkOrooro);
            break;
        case 0xcc48:
            WritePropertyFloat(stream, 0xcc48, "mHikiPower", prm->mHikiPower);
            break;
        case 0xc42e:
            WritePropertyFloat(stream, 0xc42e, "mEffectiveDeg", prm->mEffectiveDegree);
            break;
        case 0x7a1c:
            WritePropertyFloat(stream, 0x7a1c, "mTsuriHeight", prm->mTsuriHeight);
            break;
        case 0xe753:
            WritePropertyInt32(stream, 0xe753, "mDissapearFrame", prm->mDissapearFrame);
            break;
        case 0x11db:
            WritePropertyInt32(stream, 0x11db, "mActAfterSu", prm->mActAfterSu);
            break;
        case 0x04a9:
            WritePropertyInt32(stream, 0x04a9, "mActAfterFa", prm->mActAfterFa);
            break;
        case 0x3960:
            WritePropertyInt32(stream, 0x3960, "mTsuriType", (uint32_t)prm->mTsuriType);
            break;
        case 0x1f58:
            WritePropertyInt32(stream, 0x1f58, "mAttackPattern1", (uint32_t)prm->mAttackPattern);
            break;
        case 0xf8f2:
            WritePropertyInt32(stream, 0xf8f2, "mElement", (uint32_t)prm->mElement);
            break;
        case 0x55a0:
            {
                std::ptrdiff_t tsuriTblId = LGenUtility::VectorIndexOf(itemFishingNodes, prm->mTsuriItemTblId.lock());
                WritePropertyInt32(stream, 0x55a0, "mTsuriItemTblId", tsuriTblId);
            }
            break;
        case 0x7d81:
            {
                std::ptrdiff_t normalTblId = LGenUtility::VectorIndexOf(itemAppearNodes, prm->mNormalItemTblId.lock());
                WritePropertyInt32(stream, 0x7d81, "mNormalItemTblId", normalTblId);
            }
            break;
        case 0x9b49:
            WritePropertyFloat(stream, 0x9b49, "mPointerRange", prm->mPointerRange);
            break;
        case 0x61f8:
            WritePropertyInt32(stream, 0x61f8, "mBrightColor", brightColor);
            break;
        case 0xcf8a:
            WritePropertyInt32(stream, 0xcf8a, "mAmbColor", ambColor);
            break;
        case 0x97f5:
            WritePropertyInt32(stream, 0x97f5, "mKiryuCount", prm->mKiryuCount);
            break;
        case 0xc135:
            WritePropertyInt32(stream, 0xc135, "mNumGround", prm->mNumGround);
            break;
        case 0x31d1:
            WritePropertyInt16(stream, 0x31d1, "mCheckbox", prm->mCheckbox);
            break;
        }
    }

}

void LPrmIO::Load(std::string name, bStream::CStream* stream)
{
    mCtpParams[name] = std::make_shared<LCTPrm>();
    mCtpParams[name]->mEnabledProperties.reserve(32);

    auto itemFishingNodes = mMap.lock()->GetChildrenOfType<LItemFishingDOMNode>(EDOMNodeType::ItemFishing);
    auto itemAppearNodes = mMap.lock()->GetChildrenOfType<LItemAppearDOMNode>(EDOMNodeType::ItemAppear);

    uint32_t memberCount = stream->readUInt32();
    for(int m = 0; m < memberCount; m++){
        uint16_t hash = stream->readUInt16(); //skip hash
        std::string memberName = stream->readString(stream->readUInt16());
        stream->skip(4);
        mCtpParams[name]->mEnabledProperties.push_back(hash);
        switch(hash){
        case 0xa62f:
            mCtpParams[name]->mLife = stream->readUInt32();
            break;
        case 0x2528:
            mCtpParams[name]->mHitDamage = stream->readUInt32();
            break;
        case 0xad88:
            mCtpParams[name]->mSpeed = stream->readUInt32();
            break;
        case 0x93d1:
            mCtpParams[name]->mSpeedUnseen = stream->readUInt32();
            break;
        case 0x4b45:
            mCtpParams[name]->mSpeedFight = stream->readUInt32();
            break;
        case 0x5f6a:
            mCtpParams[name]->mEyesight = stream->readUInt32();
            break;
        case 0xd9f9:
            mCtpParams[name]->mLightBindFrame = stream->readUInt32();
            break;
        case 0x29fe:
            mCtpParams[name]->mMinLightBindRange = stream->readFloat();
            break;
        case 0xac49:
            mCtpParams[name]->mMaxLightBindRange = stream->readFloat();
            break;
        case 0x30aa:
            mCtpParams[name]->mNumAtkKarakai = stream->readUInt32();
            break;
        case 0x560a:
            mCtpParams[name]->mNumAtkOrooro = stream->readUInt32();
            break;
        case 0xcc48:
            mCtpParams[name]->mHikiPower = stream->readFloat();
            break;
        case 0xc42e:
            mCtpParams[name]->mEffectiveDegree = stream->readFloat();
            break;
        case 0x7a1c:
            mCtpParams[name]->mTsuriHeight = stream->readFloat();
            break;
        case 0xe753:
            mCtpParams[name]->mDissapearFrame = stream->readUInt32();
            break;
        case 0x11db:
            mCtpParams[name]->mActAfterSu = stream->readUInt32();
            break;
        case 0x04a9:
            mCtpParams[name]->mActAfterFa = stream->readUInt32();
            break;
        case 0x3960:
            mCtpParams[name]->mTsuriType = (EFleePattern)stream->readUInt32();
            break;
        case 0x1f58:
            mCtpParams[name]->mAttackPattern = (EAttackPattern)stream->readUInt32();
            break;
        case 0xf8f2:
            mCtpParams[name]->mElement = (EShieldType)stream->readUInt32();
            break;
        case 0x55a0:
            {
                std::size_t fid = stream->readUInt32();
                if(fid < itemFishingNodes.size() && itemFishingNodes.size() > 0){
                    mCtpParams[name]->mTsuriItemTblId = itemFishingNodes.at(fid < itemFishingNodes.size() ? fid : itemFishingNodes.size() - 1);
                }
            }
            break;
        case 0x7d81:
            {
                std::size_t nid = stream->readUInt32();
                if(nid < itemAppearNodes.size() && itemAppearNodes.size() > 0){
                    mCtpParams[name]->mNormalItemTblId = itemAppearNodes.at(nid < itemAppearNodes.size() ? nid : itemAppearNodes.size() - 1);
                }
            }
            break;
        case 0x9b49:
            mCtpParams[name]->mPointerRange = stream->readFloat();
            break;
        case 0x61f8:
            mCtpParams[name]->mBrightColor = glm::vec4((float)stream->readUInt8() / 255, (float)stream->readUInt8() / 255, (float)stream->readUInt8() / 255, (float)stream->readUInt8() / 255);
            SwapVec4(&mCtpParams[name]->mBrightColor);
            break;
        case 0xcf8a:
            mCtpParams[name]->mAmbColor = glm::vec4((float)stream->readUInt8() / 255, (float)stream->readUInt8() / 255, (float)stream->readUInt8() / 255, (float)stream->readUInt8() / 255);
            SwapVec4(&mCtpParams[name]->mAmbColor);
            break;
        case 0x97f5:
            mCtpParams[name]->mKiryuCount = stream->readUInt32();
            break;
        case 0xc135:
            mCtpParams[name]->mNumGround = stream->readUInt32();
            break;
        case 0x31d1:
            mCtpParams[name]->mCheckbox = stream->readUInt16();
            break;
        default:
            LGenUtility::Log << name << std::endl;
            stream->skip(4);
        }
    }
}

bool LPrmIO::RenderUI()
{
    if(mGhostImg == 0){
        int x, y, n;
        uint8_t* data = stbi_load_from_memory(&ghostImgData[0], ghostImgData_size, &x, &y, &n, 4);
        
        glCreateTextures(GL_TEXTURE_2D, 1, &mGhostImg);
        glTextureStorage2D(mGhostImg, 1, GL_RGBA8, x, y);
        glTextureSubImage2D(mGhostImg, 0, 0, 0, x, y, GL_RGBA, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
    }

    bool shouldSave = false;
    if(!mConfigsLoaded || mLoadedConfigs.empty()) {
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        if(ImGui::BeginPopupModal("ActorEditor", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiChildFlags_AlwaysAutoResize)){
            ImGui::Text("Actor Editor");
            ImGui::Separator();
            ImGui::Text("Open a Map first!");
            if(ImGui::Button("Close")){
                ImGui::CloseCurrentPopup();
                PreviewWidget::UnloadModel();
                PreviewWidget::SetInactive();
            }
            ImGui::EndPopup();
        }
        return shouldSave;
    }

	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if(ImGui::BeginPopupModal("ActorEditor", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiChildFlags_AlwaysAutoResize)){
        ImGui::Text("Actor Editor");
        LUIUtility::RenderTooltip("Hover the model preview and use the scroll wheel (+ shift) to zoom and rotate the view");
        ImGui::Separator();

        ImGui::BeginChild("##actorPropertiesWin", {250, 500});

        auto actorNameMap = LResUtility::GetNameMap("ActorConfigs");
        std::string name = mLoadedConfigs[mSelectedConfig];
        if(actorNameMap.contains(mLoadedConfigs[mSelectedConfig])){
            name = actorNameMap[mLoadedConfigs[mSelectedConfig]]["name"].get<std::string>();
        }

        if (ImGui::BeginCombo("Ghost Config", name.data(), 0))
        {
            for (int n = 0; n < mLoadedConfigs.size(); n++)
            {
                const bool is_selected = (mSelectedConfig == n);

                std::string name = mLoadedConfigs[n];
                if(actorNameMap.contains(mLoadedConfigs[n])){
                    name = actorNameMap[mLoadedConfigs[n]]["name"].get<std::string>();
                }

                if (ImGui::Selectable(std::format("{}##config{}", name.data(), n).c_str(), is_selected)){
                    mSelectedConfig = n;

                    PreviewWidget::UnloadModel();

                    std::string modelName = "";
                    std::string materialName = "";
                    bool fromGameArchive = false;
                    if(actorNameMap.contains(mLoadedConfigs[n]) && actorNameMap[mLoadedConfigs[n]].contains("model")){
                        modelName = actorNameMap[mLoadedConfigs[n]]["model"].get<std::string>();
                        if(actorNameMap[mLoadedConfigs[n]].contains("material")){
                            materialName = actorNameMap[mLoadedConfigs[n]]["material"].get<std::string>();
                        }
                    } else {
                        std::tuple<std::string, std::string, bool> actorRef = LResUtility::GetActorModelFromName(mLoadedConfigs[mSelectedConfig]);
                        modelName = std::get<0>(actorRef);
                        materialName = std::get<1>(actorRef);
                        fromGameArchive = std::get<2>(actorRef);
                    }
                    
                    std::filesystem::path modelPath = std::filesystem::path(OPTIONS.mRootPath) / "files" / "model" / (modelName + ".szp");
                    
                    if(!fromGameArchive && std::filesystem::exists(modelPath)){
                        std::shared_ptr<Archive::Rarc> modelArchive = Archive::Rarc::Create();
                        bStream::CFileStream modelArchiveStream(modelPath.string(), bStream::Endianess::Big, bStream::OpenMode::In);
                        if(!modelArchive->Load(&modelArchiveStream)){
                            LGenUtility::Log << "[PRMIO]: Unable to load model archive " << modelPath.string() << std::endl;
                        } else {
                            std::shared_ptr<Archive::File> modelFile = modelArchive->GetFile(std::filesystem::path("model") / (modelName + ".mdl"));
                            if(modelFile == nullptr){
                                LGenUtility::Log << "[PRMIO]: Couldn't find model/" << modelName << ".mdl in archive" << std::endl;
                            } else {
                                bStream::CMemoryStream modelData(modelFile->GetData(), modelFile->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
                                PreviewWidget::LoadModel(&modelData, EModelType::Actor);
                            }
                            if(materialName != ""){
                                std::shared_ptr<Archive::File> txpFile = modelArchive->GetFile(std::filesystem::path("txp") / (materialName + ".txp"));
                                if(txpFile == nullptr){
                                    LGenUtility::Log << "[PRMIO]: Couldn't find txp/" << materialName << ".txp in archive" << std::endl;
                                } else {
                                    LGenUtility::Log << "[PRMIO]: Loading txp " << materialName << std::endl;
                                    bStream::CMemoryStream txpData(txpFile->GetData(), txpFile->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
                                    PreviewWidget::SetModelAnimation(&txpData);
                                }
                            }
                        }
                    } else {
                        std::filesystem::path fullModelPath = std::filesystem::path("model") / (modelName + ".arc") / "model" / (modelName + ".mdl");
                        
                        if(GCResourceManager.mLoadedGameArchive){
                            std::shared_ptr<Archive::File> modelFile = GCResourceManager.mGameArchive->GetFile(fullModelPath);
                            
                            if(modelFile == nullptr){
                                LGenUtility::Log << "[PRMIO]: Couldn't find " << modelName<< ".mdl in game archive" << std::endl;
                            } else {
                                bStream::CMemoryStream modelData(modelFile->GetData(), modelFile->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
                                PreviewWidget::LoadModel(&modelData, EModelType::Actor);
                            }
                        }
                    }
                }

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        for(int m = 0; m < mCtpParams[mLoadedConfigs[mSelectedConfig]]->mEnabledProperties.size(); m++){
            switch(mCtpParams[mLoadedConfigs[mSelectedConfig]]->mEnabledProperties[m]){
            case 0xa62f:
                ImGui::InputInt("HP", (int*)&mCtpParams[mLoadedConfigs[mSelectedConfig]]->mLife);
                LUIUtility::RenderTooltip("Ghost Health.");
                break;
            case 0x2528:
                ImGui::InputInt("Bump Damage", (int*)&mCtpParams[mLoadedConfigs[mSelectedConfig]]->mHitDamage);
                LUIUtility::RenderTooltip("Damage from getting bumped.");
                break;
            case 0xad88:
                ImGui::InputInt("Speed", (int*)&mCtpParams[mLoadedConfigs[mSelectedConfig]]->mSpeed);
                LUIUtility::RenderTooltip("General movement speed.");
                break;
            case 0x93d1:
                ImGui::InputInt("Invis Move Speed", (int*)&mCtpParams[mLoadedConfigs[mSelectedConfig]]->mSpeedUnseen);
                LUIUtility::RenderTooltip("Movement speed while invisisble.");
                break;
            case 0x4b45:
                ImGui::InputInt("Chase Speed", (int*)&mCtpParams[mLoadedConfigs[mSelectedConfig]]->mSpeedFight);
                LUIUtility::RenderTooltip("How fast a ghost will move when chasing luigi");
                break;
            case 0x5f6a:
                ImGui::InputInt("Agro Range", (int*)&mCtpParams[mLoadedConfigs[mSelectedConfig]]->mEyesight);
                LUIUtility::RenderTooltip("The distance a ghost can notice luigi from.");
                break;
            case 0xd9f9:
                ImGui::InputInt("Stun Duration", (int*)&mCtpParams[mLoadedConfigs[mSelectedConfig]]->mLightBindFrame);
                LUIUtility::RenderTooltip("The time a ghost will stay stunned before disappearing.");
                break;
            case 0x29fe:
                ImGui::InputFloat("Min Stun Distance", &mCtpParams[mLoadedConfigs[mSelectedConfig]]->mMinLightBindRange);
                LUIUtility::RenderTooltip("The minimum distance a ghost can be stunned with the flashlight from.");
                break;
            case 0xac49:
                ImGui::InputFloat("Max Stun Distance", &mCtpParams[mLoadedConfigs[mSelectedConfig]]->mMaxLightBindRange);
                LUIUtility::RenderTooltip("The maximum distance a ghost can be stunned with the flashlight from.");
                break;
            case 0x30aa:
                ImGui::InputInt("Success Anim Loops", (int*)&mCtpParams[mLoadedConfigs[mSelectedConfig]]->mNumAtkKarakai);
                LUIUtility::RenderTooltip("The number of times a ghost will play the successful attack animation.");
                break;
            case 0x560a:
                ImGui::InputInt("Fail Anim Loops", (int*)&mCtpParams[mLoadedConfigs[mSelectedConfig]]->mNumAtkOrooro);
                LUIUtility::RenderTooltip("The number of times a ghost will play the failed attack animation.");
                break;
            case 0xcc48:
                ImGui::SliderFloat("Pull Strength", &mCtpParams[mLoadedConfigs[mSelectedConfig]]->mHikiPower, 0.0f, 1.0f);
                LUIUtility::RenderTooltip("How hard the ghost will pull luigi during vacuuming. Value is a percentage.");
                break;
            case 0xc42e:
                ImGui::SliderFloat("Effective Pull Range", &mCtpParams[mLoadedConfigs[mSelectedConfig]]->mEffectiveDegree, 0.0f, 180.0f);
                LUIUtility::RenderTooltip("Sets a minimum for the difference in angle between the direction a ghost is moving and the direction luigi is pulling (the control stick direction).");
                AngleVisualizer("effectivedegree", &mCtpParams[mLoadedConfigs[mSelectedConfig]]->mEffectiveDegree);
                break;
            case 0x7a1c:
                ImGui::InputFloat("Flee Height", &mCtpParams[mLoadedConfigs[mSelectedConfig]]->mTsuriHeight);
                LUIUtility::RenderTooltip("The height a ghost will float at when fleeing the vacuum.");
                break;
            case 0xe753:
                // input for prm->mDissapearFrame
                break;
            case 0x11db:
                // input for prm->mActAfterSu
                break;
            case 0x04a9:
                // input for prm->mActAfterFa
                break;
            case 0x3960:
                LUIUtility::RenderComboEnum<EFleePattern>("Flee", mCtpParams[mLoadedConfigs[mSelectedConfig]]->mTsuriType);
                LUIUtility::RenderTooltip("How a ghost will attempt to escape the vaccum.");
                break;
            case 0x1f58:
                LUIUtility::RenderComboEnum<EAttackPattern>("Attack", mCtpParams[mLoadedConfigs[mSelectedConfig]]->mAttackPattern);
                LUIUtility::RenderTooltip("Sets the attack of a ghost.");
                break;
            case 0xf8f2:
                LUIUtility::RenderComboEnum<EShieldType>("Shield Type", mCtpParams[mLoadedConfigs[mSelectedConfig]]->mElement);
                LUIUtility::RenderTooltip("Sets the elemental type of a ghost.");
                break;
            case 0x55a0:
                LUIUtility::RenderNodeReferenceCombo<LItemFishingDOMNode>("During Capture Drop Group", EDOMNodeType::ItemFishing, mMap, mCtpParams[mLoadedConfigs[mSelectedConfig]]->mTsuriItemTblId);
                break;
            case 0x7d81:
                LUIUtility::RenderNodeReferenceCombo<LItemAppearDOMNode>("On Capture Drop Group", EDOMNodeType::ItemAppear, mMap, mCtpParams[mLoadedConfigs[mSelectedConfig]]->mNormalItemTblId);
                break;
            case 0x9b49:
                ImGui::InputFloat("Ghost Pulse Size", &mCtpParams[mLoadedConfigs[mSelectedConfig]]->mPointerRange);
                LUIUtility::RenderTooltip("The size of the colored ring that appears on the floor under a ghost");
                break;
            case 0x61f8:
                ImGui::ColorEdit4("Glow Color", &mCtpParams[mLoadedConfigs[mSelectedConfig]]->mBrightColor.r);
                LUIUtility::RenderTooltip("Sets the color of a ghosts glowing orbs.");
                break;
            case 0xcf8a:
                ImGui::ColorEdit4("Overlay Color", &mCtpParams[mLoadedConfigs[mSelectedConfig]]->mAmbColor.r);
                LUIUtility::RenderTooltip("Sets an ambient color to be mixed with the ghost texture.");
                break;
            case 0x97f5:
                ImGui::InputInt("Kiryu Count", (int*)&mCtpParams[mLoadedConfigs[mSelectedConfig]]->mKiryuCount);
                break;
            case 0xc135:
                ImGui::InputInt("Num Ground", (int*)&mCtpParams[mLoadedConfigs[mSelectedConfig]]->mNumGround);
                break;
            case 0x31d1:
                LUIUtility::RenderCheckBox("Check", &mCtpParams[mLoadedConfigs[mSelectedConfig]]->mCheckbox);
                break;
            }
        }

		ImGui::EndChild();

		ImGui::SameLine();
        
		ImGui::Image(static_cast<uintptr_t>(PreviewWidget::PreviewID()), {600, 500},  {0.0f, 1.0f}, {1.0f, 0.0f});
		if(ImGui::IsItemHovered()){
			if(ImGui::IsKeyDown(ImGuiKey_ModShift)){
				PreviewWidget::DoRotate(ImGui::GetIO().MouseWheel * 0.1f);
			} else {
				PreviewWidget::DoZoom(ImGui::GetIO().MouseWheel*50);
			}
		}

        if(ImGui::Button("Save All")){
            ImGui::CloseCurrentPopup();
            PreviewWidget::UnloadModel();
            PreviewWidget::SetInactive();
            ImGui::OpenPopup("SavingConfigsModal");
            shouldSave = true;
        }

        ImGui::SameLine();
        if(ImGui::Button("Close")){
            ImGui::CloseCurrentPopup();
            PreviewWidget::UnloadModel();
            PreviewWidget::SetInactive();
        }

        ImGui::End();
    }
    return shouldSave;
}
