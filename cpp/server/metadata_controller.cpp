#include "metadata_controller.hpp"
#include "router.hpp"
#include "player_api.hpp"
#include "player_api_json.hpp"
#include "player_api_parsers.hpp"

namespace msrv {

MetadataController::MetadataController(Request* request, Player* player, SettingsDataPtr settings)
    : ControllerBase(request), player_(player), settings_(std::move(settings))
{
}

ResponsePtr MetadataController::getPlayingMetadata()
{
    auto metadata = player_->getPlayingRawMetadata();
    return Response::json({{"metadata", metadata}});
}

ResponsePtr MetadataController::getPlaylistItemMetadata()
{
    auto plref = param<PlaylistRef>("plref");
    auto index = param<int32_t>("index");
    auto metadata = player_->getRawMetadata(plref, index);
    return Response::json({{"metadata", metadata}});
}

void MetadataController::defineRoutes(
    Router* router, WorkQueue* workQueue, Player* player, SettingsDataPtr settings)
{
    auto routes = router->defineRoutes<MetadataController>();

    routes.createWith([=](Request* request) {
        return new MetadataController(request, player, settings);
    });

    routes.useWorkQueue(workQueue);
    routes.setPrefix("api/metadata");

    routes.get("raw/playing", &MetadataController::getPlayingMetadata);
    routes.get("raw/:plref/:index", &MetadataController::getPlaylistItemMetadata);
}

}
