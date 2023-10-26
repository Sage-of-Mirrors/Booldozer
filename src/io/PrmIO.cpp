#include "io/PrmIO.hpp"
#include "UIUtil.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include <filesystem>
#include "Options.hpp"
#include "ImGuiFileDialog/ImGuiFileDialog.h"
#include "stb_image.h"
#include <glad/glad.h>

constexpr float pi = 3.14159f;

void SwapVec4(glm::vec4* s)
{
    float a = s->r;
    s->r = s->a;
    s->a = a;
    a = s->g;
    s->g = s->b;
    s->b = a;
}

bool AngleVisualizer(const char* label, float* angle, uint32_t sGhostImg)
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

	draw_list->AddImage((void*)(intptr_t)sGhostImg, ImVec2(center.x - 8, center.y - 8), ImVec2(center.x + 8, center.y + 8));


    pos.y += 80;
    ImGui::SetCursorScreenPos(pos);

	return true;
}

LPrmIO::LPrmIO() : mConfigsLoaded(false), mSelectedConfig(0), mGhostImg(0) {
}
LPrmIO::~LPrmIO() {
    glDeleteTextures(1, &mGhostImg);
}

void LPrmIO::LoadConfigs(std::shared_ptr<LMapDOMNode>& map)
{
    int x, y, n;
	uint8_t* data = stbi_load_from_memory(&ghostImgData[0], ghostImgData_size, &x, &y, &n, 4);
	
    glCreateTextures(GL_TEXTURE_2D, 1, &mGhostImg);
    glTextureStorage2D(mGhostImg, 1, GL_RGBA8, x, y);
    glTextureSubImage2D(mGhostImg, 0, 0, 0, x, y, GL_RGBA, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);

    mMap = map;

	for (size_t f = 0; f < GCResourceManager.mGameArchive.dirnum; f++)
	{
		if(std::string(GCResourceManager.mGameArchive.dirs[f].name) == "ctp"){
			for (size_t i = GCResourceManager.mGameArchive.dirs[f].fileoff; i < GCResourceManager.mGameArchive.dirs[f].fileoff + GCResourceManager.mGameArchive.dirs[f].filenum; i++)
			{
                auto name = std::filesystem::path(GCResourceManager.mGameArchive.files[i].name).filename().stem();
                if(name == "." || name == "..") continue;
                bStream::CMemoryStream prm((uint8_t*)GCResourceManager.mGameArchive.files[i].data, GCResourceManager.mGameArchive.files[i].size, bStream::Endianess::Big, bStream::OpenMode::In);
                Load(name.string(), &prm);
                mLoadedConfigs.push_back(name.string());
			}
			
		}
	}
    
    mConfigsLoaded = true;
}

void LPrmIO::SaveConfigsToFile()
{
    std::string paramsPth;
    ImGuiFileDialog::Instance()->OpenDialog("SaveParamFilesDlg", "Choose a Folder", nullptr, OPTIONS.mLastSavedDirectory);
    if (LUIUtility::RenderFileDialog("SaveParamFilesDlg", paramsPth))
	{
        std::filesystem::path outFolder = paramsPth;

        for (const auto& file : mLoadedConfigs)
        {
            bStream::CFileStream out((outFolder / file).string(), bStream::Endianess::Big, bStream::OpenMode::Out);
            Save(file, &out);
        }
	}
    
}

// Because these properties are consistent, we can write all of them and the game wont really care, so long as the ones it is looking for are present.

void WritePropertyInt32(bStream::CFileStream* stream, uint16_t hash, std::string name, uint32_t v){
    stream->writeUInt16(hash);
    stream->writeUInt16(name.size());
    stream->writeString(name);
    stream->writeUInt32(4);
    stream->writeUInt32(v);
};

void WritePropertyFloat(bStream::CFileStream* stream, uint16_t hash, std::string name, float v){
    stream->writeUInt16(hash);
    stream->writeUInt16(name.size());
    stream->writeString(name);
    stream->writeUInt32(4);
    stream->writeFloat(v);
};

void WritePropertyInt16(bStream::CFileStream* stream, uint16_t hash, std::string name, uint16_t v){
    stream->writeUInt16(hash);
    stream->writeUInt16(name.size());
    stream->writeString(name);
    stream->writeUInt32(2);
    stream->writeUInt16(v);
};

void LPrmIO::Save(std::string name, bStream::CFileStream* stream)
{
    auto itemFishingNodes = mMap.lock()->GetChildrenOfType<LItemFishingDOMNode>(EDOMNodeType::ItemFishing);
    auto itemAppearNodes = mMap.lock()->GetChildrenOfType<LItemAppearDOMNode>(EDOMNodeType::ItemAppear);
    auto prm = mCtpParams[name];
    stream->writeUInt32(32);
    WritePropertyInt32(stream, 0xa62f, "mLife", prm->mLife);
    WritePropertyInt32(stream, 0x2528, "mHitDamage", prm->mHitDamage);
    WritePropertyInt32(stream, 0xad88, "mSpeed", prm->mSpeed);
    WritePropertyInt32(stream, 0x93d1, "mSpeedUnseen", prm->mSpeedUnseen);
    WritePropertyInt32(stream, 0x4b45, "mSpeedFight", prm->mSpeedFight);
    WritePropertyInt32(stream, 0x5f6a, "mEyesight", prm->mEyesight);
    WritePropertyInt32(stream, 0xd9f9, "mLightBingFrame", prm->mLightBindFrame);
    WritePropertyFloat(stream, 0x29fe, "mMinLightBindRange", prm->mMinLightBindRange);
    WritePropertyFloat(stream, 0xac49, "mMaxLightBindRange", prm->mMaxLightBindRange);
    WritePropertyInt32(stream, 0x30aa, "mNumAtkKarakai", prm->mNumAtkKarakai);
    WritePropertyInt32(stream, 0x560a, "mNumAtkOrooro", prm->mNumAtkOrooro);
    WritePropertyFloat(stream, 0xcc48, "mHikiPower", prm->mHikiPower);
    WritePropertyFloat(stream, 0xc42e, "mEffectiveDegree", prm->mEffectiveDegree);
    WritePropertyFloat(stream, 0x7a1c, "mTsuriHeight", prm->mTsuriHeight);
    WritePropertyInt32(stream, 0xe753, "mDissapearFrame", prm->mDissapearFrame);
    WritePropertyInt32(stream, 0x11db, "mActAfterSu", prm->mActAfterSu);
    WritePropertyInt32(stream, 0x04a9, "mActAfterFa", prm->mActAfterFa);
    WritePropertyInt32(stream, 0x3960, "mTsuriType", prm->mTsuriType);
    WritePropertyInt32(stream, 0x1f58, "mAttackPattern", prm->mAttackPattern);
    WritePropertyInt32(stream, 0xf8f2, "mElement", prm->mElement);
    ptrdiff_t tsuriTblId = LGenUtility::VectorIndexOf(itemFishingNodes, prm->mTsuriItemTblId.lock());
    ptrdiff_t normalTblId = LGenUtility::VectorIndexOf(itemAppearNodes, prm->mNormalItemTblId.lock());
    WritePropertyInt32(stream, 0x55a0, "mTsuriItemTblId", tsuriTblId);
    WritePropertyInt32(stream, 0x7d81, "mNormalItemTblId", normalTblId);
    WritePropertyFloat(stream, 0x9b49, "mPointerRange", prm->mPointerRange);
    WritePropertyInt32(stream, 0x61f8, "mBrightColor", ((uint8_t)prm->mBrightColor.r) << 24 | ((uint8_t)(prm->mBrightColor.b*255)) << 16 | ((uint8_t)(prm->mBrightColor.g*255)) << 8 | ((uint8_t)prm->mBrightColor.a*255));
    WritePropertyInt32(stream, 0xcf8a, "mAmbColor", ((uint8_t)prm->mAmbColor.r) << 24 | ((uint8_t)(prm->mAmbColor.b*255)) << 16 | ((uint8_t)(prm->mAmbColor.g*255)) << 8 | ((uint8_t)prm->mAmbColor.a*255));
    WritePropertyInt32(stream, 0x97f5, "mKiryuCount", prm->mKiryuCount);
    WritePropertyInt32(stream, 0xc135, "mNumGround", prm->mNumGround);
    WritePropertyInt16(stream, 0x31d1, "mCheckbox", prm->mCheckbox);
}

void LPrmIO::Load(std::string name, bStream::CStream* stream)
{
    mCtpParams[name] = std::make_shared<LCTPrm>();

    auto itemFishingNodes = mMap.lock()->GetChildrenOfType<LItemFishingDOMNode>(EDOMNodeType::ItemFishing);
    auto itemAppearNodes = mMap.lock()->GetChildrenOfType<LItemAppearDOMNode>(EDOMNodeType::ItemAppear);

    uint32_t memberCount = stream->readUInt32();
    for(int m = 0; m < memberCount; m++){
        uint16_t hash = stream->readUInt16(); //skip hash
        std::string memberName = stream->readString(stream->readUInt16());
        stream->skip(4);
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
                size_t fid = stream->readUInt32();
                if(fid < itemFishingNodes.size() && itemFishingNodes.size() > 0){
                    mCtpParams[name]->mTsuriItemTblId = itemFishingNodes.at(fid < itemFishingNodes.size() ? fid : itemFishingNodes.size() - 1);
                }
            }
            break;
        case 0x7d81:
            {
                size_t nid = stream->readUInt32();
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
            stream->skip(4);
        }
    }
}

void LPrmIO::RenderUI()
{
    if(!mConfigsLoaded || mLoadedConfigs.empty()) return;

    if(mParamToolOpen){

        ImGuiWindowClass mainWindowOverride;
        mainWindowOverride.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
        ImGui::SetNextWindowClass(&mainWindowOverride);

        ImGui::Begin("toolWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);

        ImGui::Text("Ghost Config Editor");
        ImGui::Separator();
        
        if (ImGui::BeginCombo("Ghost", mLoadedConfigs[mSelectedConfig].data(), 0))
        {
            for (int n = 0; n < mLoadedConfigs.size(); n++)
            {
                const bool is_selected = (mSelectedConfig == n);
                if (ImGui::Selectable(mLoadedConfigs[n].data(), is_selected))
                    mSelectedConfig = n;

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::InputInt("HP", (int*)&mCtpParams[mLoadedConfigs[mSelectedConfig]]->mLife);
        LUIUtility::RenderTooltip("Ghost Health.");

        ImGui::InputInt("Bump Damage", (int*)&mCtpParams[mLoadedConfigs[mSelectedConfig]]->mHitDamage);
        LUIUtility::RenderTooltip("Damage from getting bumped.");

        ImGui::InputInt("Speed", (int*)&mCtpParams[mLoadedConfigs[mSelectedConfig]]->mSpeed);
        LUIUtility::RenderTooltip("General movement speed.");
        
        ImGui::InputInt("Invis Move Speed", (int*)&mCtpParams[mLoadedConfigs[mSelectedConfig]]->mSpeedUnseen);
        LUIUtility::RenderTooltip("Movement speed while invisisble.");

        ImGui::InputInt("Chase Speed", (int*)&mCtpParams[mLoadedConfigs[mSelectedConfig]]->mSpeedFight);
        LUIUtility::RenderTooltip("How fast a ghost will move when chasing luigi");

        ImGui::InputInt("Agro Range", (int*)&mCtpParams[mLoadedConfigs[mSelectedConfig]]->mEyesight);
        LUIUtility::RenderTooltip("The distance a ghost can notice luigi from.");

        ImGui::InputInt("Stun Duration", (int*)&mCtpParams[mLoadedConfigs[mSelectedConfig]]->mLightBindFrame);
        LUIUtility::RenderTooltip("The time a ghost will stay stunned before disappearing.");

        ImGui::InputFloat("Min Stun Distance", &mCtpParams[mLoadedConfigs[mSelectedConfig]]->mMinLightBindRange);
        LUIUtility::RenderTooltip("The minimum distance a ghost can be stunned with the flashlight from.");

        ImGui::InputFloat("Max Stun Distance", &mCtpParams[mLoadedConfigs[mSelectedConfig]]->mMaxLightBindRange);
        LUIUtility::RenderTooltip("The maximum distance a ghost can be stunned with the flashlight from.");

        ImGui::InputInt("Success Anim Loops", (int*)&mCtpParams[mLoadedConfigs[mSelectedConfig]]->mNumAtkKarakai);
        LUIUtility::RenderTooltip("The number of times a ghost will play the successful attack animation.");

        ImGui::InputInt("Fail Anim Loops", (int*)&mCtpParams[mLoadedConfigs[mSelectedConfig]]->mNumAtkOrooro);
        LUIUtility::RenderTooltip("The number of times a ghost will play the failed attack animation.");

        ImGui::SliderFloat("Pull Strength", &mCtpParams[mLoadedConfigs[mSelectedConfig]]->mHikiPower, 0.0f, 1.0f);
        LUIUtility::RenderTooltip("How hard the ghost will pull luigi during vacuuming. Value is a percentage.");

        ImGui::SliderFloat("Effective Pull Range", &mCtpParams[mLoadedConfigs[mSelectedConfig]]->mEffectiveDegree, 0.0f, 180.0f);
        LUIUtility::RenderTooltip("Sets a minimum for the difference in angle between the direction a ghost is moving and the direction luigi is pulling (the control stick direction).");
        AngleVisualizer("effectivedegree", &mCtpParams[mLoadedConfigs[mSelectedConfig]]->mEffectiveDegree, mGhostImg);


        ImGui::InputFloat("Flee Height", &mCtpParams[mLoadedConfigs[mSelectedConfig]]->mTsuriHeight);
        LUIUtility::RenderTooltip("The height a ghost will float at when fleeing the vacuum.");

        //TODO: hook this up to the item tables
        LUIUtility::RenderNodeReferenceCombo<LItemAppearDOMNode>("On Capture Drop Group", EDOMNodeType::ItemAppear, mMap, mCtpParams[mLoadedConfigs[mSelectedConfig]]->mNormalItemTblId);
        LUIUtility::RenderNodeReferenceCombo<LItemFishingDOMNode>("During Capture Drop Group", EDOMNodeType::ItemFishing, mMap, mCtpParams[mLoadedConfigs[mSelectedConfig]]->mTsuriItemTblId);

        ImGui::InputFloat("Ghost Pulse Size", &mCtpParams[mLoadedConfigs[mSelectedConfig]]->mPointerRange);
        LUIUtility::RenderTooltip("The size of the colored ring that appears on the floor under a ghost");

        LUIUtility::RenderComboEnum<EShieldType>("Shield Type", mCtpParams[mLoadedConfigs[mSelectedConfig]]->mElement);
        LUIUtility::RenderTooltip("Sets the elemental type of a ghost.");

        LUIUtility::RenderComboEnum<EAttackPattern>("Attack", mCtpParams[mLoadedConfigs[mSelectedConfig]]->mAttackPattern);
        LUIUtility::RenderTooltip("Sets the attack of a ghost.");

        LUIUtility::RenderComboEnum<EFleePattern>("Flee", mCtpParams[mLoadedConfigs[mSelectedConfig]]->mTsuriType);
        LUIUtility::RenderTooltip("How a ghost will attempt to escape the vaccum.");


        ImGui::ColorEdit4("Glow Color", &mCtpParams[mLoadedConfigs[mSelectedConfig]]->mBrightColor.r);
        LUIUtility::RenderTooltip("Sets the color of a ghosts glowing orbs.");

        ImGui::ColorEdit4("Overlay Color", &mCtpParams[mLoadedConfigs[mSelectedConfig]]->mAmbColor.r);
        LUIUtility::RenderTooltip("Sets an ambient color to be mixed with the ghost texture.");

        ImGui::Text("Unknown/Unused Settings");
        ImGui::InputInt("Kiryu Count", (int*)&mCtpParams[mLoadedConfigs[mSelectedConfig]]->mKiryuCount);
        ImGui::InputInt("Num Ground", (int*)&mCtpParams[mLoadedConfigs[mSelectedConfig]]->mNumGround);
        ImGui::Checkbox("Check", &mCtpParams[mLoadedConfigs[mSelectedConfig]]->mCheckbox);

        if(ImGui::Button("Save All Configs")){
            SaveConfigsToFile();
            mParamToolOpen = false;
        }

        ImGui::SameLine();
        if(ImGui::Button("Close")) mParamToolOpen = false;

        ImGui::End();
    }
}