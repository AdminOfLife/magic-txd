#pragma once

#include "shared.h"

struct TxdBuildModule abstract : public MessageReceiver, public rw::WarningManagerInterface
{
    inline TxdBuildModule( rw::Interface *rwEngine )
    {
        this->rwEngine = rwEngine;
    }

    struct run_config
    {
        std::wstring gameRoot = L"massbuild_in/";
        std::wstring outputRoot = L"massbuild_out/";

        rwkind::eTargetPlatform targetPlatform = rwkind::PLATFORM_PC;
        rwkind::eTargetGame targetGame = rwkind::GAME_GTASA;

        bool generateMipmaps = false;
        int curMipMaxLevel = 0;
    };

    bool RunApplication( const run_config& cfg );

protected:
    void OnWarning( std::string&& msg ) override;

private:
    rw::Interface *rwEngine;
};