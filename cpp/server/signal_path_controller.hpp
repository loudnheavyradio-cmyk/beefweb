#pragma once

#include "controller.hpp"

namespace msrv {

class Router;
class Player;

class SignalPathController : public ControllerBase
{
public:
    SignalPathController(Request* request, Player* player);

    ResponsePtr getSignalPath();

    static void defineRoutes(Router* router, WorkQueue* workQueue, Player* player);

private:
    Player* player_;

    MSRV_NO_COPY_AND_ASSIGN(SignalPathController);
};

}
