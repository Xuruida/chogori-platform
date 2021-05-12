/*
MIT License

Copyright(c) 2020 Futurewei Cloud

    Permission is hereby granted,
    free of charge, to any person obtaining a copy of this software and associated documentation files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions :

    The above copyright notice and this permission notice shall be included in all copies
    or
    substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS",
    WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
    DAMAGES OR OTHER
    LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include "seastar/core/sleep.hh"

#include <k2/common/Log.h>
#include <k2/dto/MessageVerbs.h>
#include <k2/transport/RPCDispatcher.h>  // for RPC

#include "TSOService.h"

namespace k2 {

TSOService::TSOService()
{
    K2INFO("ctor");
}

TSOService::~TSOService()
{
    K2INFO("dtor");
}

seastar::future<> TSOService::gracefulStop() {
    K2INFO("stop");

    // Always use core 0 as controller and the rest as workers
    if (seastar::this_shard_id() == 0)
    {
        K2ASSERT(_controller != nullptr, "_controller null!");
        K2INFO("TSOController stops on core:" <<seastar::this_shard_id());
        return _controller->gracefulStop();
    }
    else
    {
        K2ASSERT(_worker != nullptr, "_controller null!");
        K2INFO("TSOWorder stops on core:" <<seastar::this_shard_id());
        return _worker->gracefulStop();
    }
}

seastar::future<> TSOService::start()
{
    K2INFO("TSOService starts on core:" <<seastar::this_shard_id());

    // At least two cores, one controller and one worker is required
    if (seastar::smp::count < 2)
    {
        K2INFO("TSOService starts unexpected on less than 2 cores. Core counts:" <<seastar::smp::count);
        return seastar::make_exception_future(TSONotEnoughCoreException(seastar::smp::count));
    }

    // Always use core 0 as controller and the rest as workers
    if (seastar::this_shard_id() == 0)
    {
        K2INFO("TSOController starts on core:" <<seastar::this_shard_id());
        _controller = std::make_unique<TSOService::TSOController>(*this);
        return _controller->start();
    }
    else
    {
        K2INFO("TSOWorder starts on core:" <<seastar::this_shard_id());
        _worker = std::make_unique<TSOService::TSOWorker>(*this);
        return _worker->start();
    }
}

void TSOService::UpdateWorkerControlInfo(const TSOWorkerControlInfo& controlInfo)
{
    K2ASSERT(seastar::this_shard_id() != 0 && _worker != nullptr, "UpdateWorkerControlInfo should be on worker core only!");

    return _worker->UpdateWorkerControlInfo(controlInfo);
}

std::vector<k2::String> TSOService::GetWorkerURLs()
{
    // NOTE: this fn is called by controller during start() on worker after the transport is initialized
    // so directly return the transport config info
    K2ASSERT(seastar::this_shard_id() != 0, "GetWorkerURLs should be on worker core only!");
    K2INFO("GetWorkerURLs on worker core. tcp url is:" <<k2::RPC().getServerEndpoint(k2::TCPRPCProtocol::proto)->getURL());

    std::vector<k2::String> result;

    // return all endpoint URLs of various transport type, currently, we must have TCP and RDMA
    result.push_back(k2::RPC().getServerEndpoint(k2::TCPRPCProtocol::proto)->getURL());

    if (seastar::engine()._rdma_stack)
    {
        K2INFO("TSOWorker have RDMA transport.");
        result.push_back(k2::RPC().getServerEndpoint(k2::RRDMARPCProtocol::proto)->getURL());
    }

    return result;
}

    /*
    K2INFO("Registering message handlers");
    RPC().registerMessageObserver(MsgVerbs::GETTSOSERVERINFO,
        [this](k2::Request&& request) mutable {
            (void)request;  // TODO do something with the request
        });

    K2INFO("Core Id:" << seastar::this_shard_id());

    // call msgReceiver method on core#0
    return _baseApp.getDist<k2::TSOService>().invoke_on(0, &TSOService::msgReceiver);

}


seastar::future<> TSOService::msgReceiver()
{
    K2INFO("Message received");
    K2INFO("Core 0 Id:" << seastar::this_shard_id());
    return seastar::make_ready_future<>();
} */



} // namespace k2


