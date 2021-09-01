#include "io/PrmIO.hpp"
#include "UIUtil.hpp"
#include "imgui.h"

#include <filesystem>

void SwapVec4(glm::vec4* s){
    float a = s->r;
    s->r = s->a;
    s->a = a;
    a = s->g;
    s->g = s->b;
    s->b = a;
}

LPrmIO::LPrmIO() : mConfigsLoaded(false){}
LPrmIO::~LPrmIO(){}

void LPrmIO::LoadConfigs(std::shared_ptr<LMapDOMNode>& map){
    if(!std::filesystem::exists("ctp")){
        std::cout << "params folder not found" << std::endl;
        return;
    }
    
    mMap = map;

    for (const auto& entry : std::filesystem::directory_iterator("ctp")) {
        if (entry.is_regular_file()) {
            bStream::CFileStream test(entry.path().string(), bStream::Endianess::Big, bStream::OpenMode::In);
            Load(entry.path().filename().stem().string(), &test);
            mLoadedConfigs.push_back(entry.path().filename().stem().string());
        }
    }
    mConfigsLoaded = true;
}

// Because these properties are consistent, we can write all of them and the game wont really care, so long as the ones it is looking for are present.

void LPrmIO::Load(std::string name, bStream::CFileStream* stream){
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
            mCtpParams[name]->mTsuriItemTblId = itemFishingNodes.at(stream->readUInt32());
            break;
        case 0x7d81:
            mCtpParams[name]->mNormalItemTblId = itemAppearNodes.at(stream->readUInt32());
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

void LPrmIO::RenderUI(){
    if(!mConfigsLoaded) return;
    ImGui::Begin("Ghost Configurations");
    
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
    
    ImGui::End();
}