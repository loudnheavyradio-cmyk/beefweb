#include "signal_path_controller.hpp"
#include "router.hpp"
#include "player_api.hpp"
#include "player_api_json.hpp"

namespace msrv {

SignalPathController::SignalPathController(Request* request, Player* player)
    : ControllerBase(request), player_(player)
{
}

ResponsePtr SignalPathController::getSignalPath()
{
    auto info = player_->getSignalPath();
    return Response::json({{"signalPath", info}});
}

void SignalPathController::defineRoutes(Router* router, WorkQueue* workQueue, Player* player)
{
    auto routes = router->defineRoutes<SignalPathController>();
    routes.createWith([=](Request* r) { return new SignalPathController(r, player); });
    routes.useWorkQueue(workQueue);
    routes.setPrefix("api/signal-path");
    routes.get("", &SignalPathController::getSignalPath);
}

}
