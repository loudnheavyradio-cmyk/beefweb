#pragma once

#include "controller.hpp"
#include "settings.hpp"

namespace msrv {

class Router;
class Player;

class MetadataController : public ControllerBase
{
public:
    MetadataController(Request* request, Player* player, SettingsDataPtr settings);

    ResponsePtr getPlayingMetadata();
    ResponsePtr getPlaylistItemMetadata();

    static void defineRoutes(Router* router, WorkQueue* workQueue, Player* player, SettingsDataPtr settings);

private:
    Player* player_;
    SettingsDataPtr settings_;

    MSRV_NO_COPY_AND_ASSIGN(MetadataController);
};

}
