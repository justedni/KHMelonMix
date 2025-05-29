#pragma once

#include "Plugin.h"
#include "../NDS.h"

namespace Plugins {

class PluginCastlevaniaEcclesia : public Plugin
{
public:
    PluginCastlevaniaEcclesia(u32 gameCode);

    void onLoadROM() override;

    static u32 usGamecode;
    static bool isCart(u32 gameCode) { return gameCode == usGamecode; };
    bool isUsaCart()    { return GameCode == usGamecode; };

    std::string assetsFolder() { return "ecclesia"; }

    bool isBackgroundMusicReplacementImplemented() const override { return true; }
    u16 getMidiBgmId() override;
    u8 getMidiBgmStateEcclesia();
    u32 getMidiSequenceAddress(u16 bgmId) override;
    u16 getMidiSequenceSize(u16 bgmId) override;

    void refreshBackgroundMusic() override;

    std::vector<ShapeData2D> renderer_2DShapes(int gameScene, int gameSceneState);
    int renderer_screenLayout();
    int renderer_brightnessMode();
    bool renderer_showOriginalUI();

    void applyAddonKeysToInputMaskOrTouchControls(u32* InputMask, u16* touchX, u16* touchY, bool* isTouching, u32* HotkeyMask, u32* HotkeyPress);
    void applyTouchKeyMaskToTouchControls(u16* touchX, u16* touchY, bool* isTouching, u32 TouchKeyMask);

    u32 getAspectRatioAddress();

    const char* getGameSceneName() { return "OoE"; }
    int detectGameScene();

    u32 arm9Callback(u32 addr);

private:
    u8 _EcclesiaMusicState = 0;
    u16 _CurrentBgmId = 0;
};

} // namespace Plugins
