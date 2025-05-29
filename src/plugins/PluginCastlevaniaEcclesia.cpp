#include "PluginCastlevaniaEcclesia.h"

#define ARM9_ADDRESS_BGM_PLAY_US 0x0209e848

namespace Plugins {

u32 PluginCastlevaniaEcclesia::usGamecode = 0x45395259;

PluginCastlevaniaEcclesia::PluginCastlevaniaEcclesia(u32 gameCode)
{
    GameCode = gameCode;

    Plugin::hudToggle();
}

void PluginCastlevaniaEcclesia::onLoadROM() {
    Plugin::onLoadROM();

    constexpr u16 breakPointsCount = 1;
    u32 breakpoints[breakPointsCount] = { ARM9_ADDRESS_BGM_PLAY_US };
    nds->AddBreakpointCallback(
        breakpoints, breakPointsCount,
        std::bind( &PluginCastlevaniaEcclesia::arm9Callback, this, std::placeholders::_1));
}

u32 PluginCastlevaniaEcclesia::arm9Callback(u32 addr) {
    if (addr == ARM9_ADDRESS_BGM_PLAY_US) {
        const auto& registers = nds->ARM9.R;
        u16 bgmId = registers[3];
        u32 callingAddress = registers[15];
        printf("Callback received at 0x%08x. BgmId: %d...", callingAddress, bgmId);

        std::string replacementBgmPath = getReplacementBackgroundMusicFilePath(bgmId);
        if (replacementBgmPath != "") {
            printf(" replacement found: Muting SSEQ via ARM skip.\n");
            return 0x0209e86c;
        } else {
            printf(" no replacement: playing original SSEQ.\n");
            return 0;
        }
    }
}

std::vector<ShapeData2D> PluginCastlevaniaEcclesia::renderer_2DShapes(int gameScene, int gameSceneState) {
    return std::vector<ShapeData2D>();
}

int PluginCastlevaniaEcclesia::renderer_screenLayout() {
    return screenLayout_Top;
};

int PluginCastlevaniaEcclesia::renderer_brightnessMode() {
    return brightnessMode_TopScreen;
}

bool PluginCastlevaniaEcclesia::renderer_showOriginalUI() {
    return true;
}

void PluginCastlevaniaEcclesia::applyAddonKeysToInputMaskOrTouchControls(u32* InputMask, u16* touchX, u16* touchY, bool* isTouching, u32* HotkeyMask, u32* HotkeyPress) {

}
void PluginCastlevaniaEcclesia::applyTouchKeyMaskToTouchControls(u16* touchX, u16* touchY, bool* isTouching, u32 TouchKeyMask) {
    _superApplyTouchKeyMaskToTouchControls(touchX, touchY, isTouching, TouchKeyMask, 3, true);
}

u32 PluginCastlevaniaEcclesia::getAspectRatioAddress() {
    return 0;
}

int PluginCastlevaniaEcclesia::detectGameScene() {
    if (nds == nullptr) {
        return GameScene;
    }

    return 0;
}

u16 PluginCastlevaniaEcclesia::getMidiBgmId() {
    return nds->ARM7Read16(0x0215AA08);
}

u8 PluginCastlevaniaEcclesia::getMidiBgmStateEcclesia() {
    return nds->ARM7Read8(0x0215AA13);
}

u32 PluginCastlevaniaEcclesia::getMidiSequenceAddress(u16 bgmId) {
    return nds->ARM7Read32(0x0215AA88);
}

u16 PluginCastlevaniaEcclesia::getMidiSequenceSize(u16 bgmId) {
    return nds->ARM7Read16(0x0215AA8C);
}

enum EMidiStateEcclesia : u8 {
    OoEStopped = 0x00,
    OoEPlaying = 0x01,
};

std::string getMidiStateEcclesiaName(EMidiStateEcclesia state) {
    switch(state) {
    case EMidiStateEcclesia::OoEStopped: return "Stopped";
    case EMidiStateEcclesia::OoEPlaying: return "Playing";
    default: return "Invalid";
    }
}

void PluginCastlevaniaEcclesia::refreshBackgroundMusic() {
#if !REPLACEMENT_BGM_ENABLED
    return;
#endif

    const u16 BGM_INVALID_ID_ECCLESIA = 0xFFF;

    if (!isBackgroundMusicReplacementImplemented()) {
        return;
    }

    u8 state = getMidiBgmStateEcclesia();
    u16 bgmId = getMidiBgmId();

    if (bgmId != _CurrentBgmId)
    {
        printf("Music BGM Id:%d State: %d (%s)\n",
            bgmId, state, getMidiStateEcclesiaName(static_cast<EMidiStateEcclesia>(state)).c_str());

        switch(state) {
        case EMidiStateEcclesia::OoEStopped: {
            if (!_CurrentBgmIsStream && _CurrentBackgroundMusic != BGM_INVALID_ID_ECCLESIA) {
                stopBackgroundMusic(0);
            }
            break;
        }
        case EMidiStateEcclesia::OoEPlaying: {
            // SSEQ is loaded and ready to play
            if (bgmId != _CurrentBackgroundMusic) {
                // Previous bgm should have already been stopped, but just in case:
                stopBackgroundMusic(200);

                std::string replacementBgmPath = getReplacementBackgroundMusicFilePath(bgmId);
                if (replacementBgmPath != "") {
                    _ShouldStartReplacementBgmMusic = true;
                    _CurrentBackgroundMusicFilepath = replacementBgmPath;
                    _CurrentBackgroundMusic = bgmId;
                    u16 bgmResumeId = getMidiBgmToResumeId();
                    _ResumeBackgroundMusicPosition = (bgmResumeId == _CurrentBackgroundMusic && bgmResumeId != BGM_INVALID_ID);
                    _BackgroundMusicDelayAtStart = delayBeforeStartReplacementBackgroundMusic(bgmId);
                    _CurrentBgmIsStream = false;
                } else {
                    _CurrentBackgroundMusic = BGM_INVALID_ID_ECCLESIA;
                }
            }
            break;
        }
        default: {
            break;
        }
        }
        _EcclesiaMusicState = state;
        _CurrentBgmId = bgmId;
    }

    u8 currVolume = getMidiBgmVolume();
    if (_BackgroundMusicVolume != currVolume) {
        _BackgroundMusicVolume = currVolume;
        _ShouldUpdateReplacementBgmMusicVolume = true;
    }
}

}
