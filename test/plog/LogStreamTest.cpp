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

#include <k2/appbase/Appbase.h>
#include <k2/appbase/AppEssentials.h>
#include <k2/persistence/logStream/LogStream.h>
#include <seastar/core/reactor.hh>
#include <seastar/core/sleep.hh>
#include <k2/dto/PersistenceCluster.h>
#include <k2/dto/MessageVerbs.h>

using namespace k2;

namespace k2::log {
inline thread_local k2::logging::Logger ltest("k2::ltest");
}

class LogStreamTest {

private:
    int exitcode = -1;
    std::unique_ptr<k2::TXEndpoint> _cpoEndpoint;
    k2::MetadataMgr _mmgr;
    k2::LogStream* _client;
    k2::MetadataMgr _reload_mmgr;
    k2::LogStream* _reload_client;
    String _initPlogId;
    k2::ConfigVar<std::vector<k2::String>> _plogConfigEps{"plog_server_endpoints"};
    seastar::future<> _testFuture = seastar::make_ready_future();
    seastar::timer<> _testTimer;
    
public:  // application lifespan
    LogStreamTest() {
        K2LOG_I(log::ltest, "ctor");
    }
    ~LogStreamTest() {
        K2LOG_I(log::ltest, "ctor");
    }

    // required for seastar::distributed interface
    seastar::future<> gracefulStop() {
        K2LOG_I(log::ltest, "stop");
        return std::move(_testFuture);
    }

    seastar::future<> start() {
        K2LOG_I(log::ltest, "start");
        ConfigVar<String> configEp("cpo_url");
        _cpoEndpoint = RPC().getTXEndpoint(configEp());

        // let start() finish and then run the tests
        _testTimer.set_callback([this] {
            _testFuture = runTest1()
            .then([this] { return runTest2(); })
            .then([this] { return runTest3(); })
            .then([this] { return runTest4(); })
            .then([this] { return runTest5(); })
            .then([this] { return runTest6(); })
            .then([this] {
                K2LOG_I(log::ltest, "======= All tests passed ========");
                exitcode = 0;
            })
            .handle_exception([this](auto exc) {
                try {
                    std::rethrow_exception(exc);
                } catch (RPCDispatcher::RequestTimeoutException& exc) {
                    K2LOG_E(log::ltest, "======= Test failed due to timeout ========");
                    exitcode = -1;
                } catch (std::exception& e) {
                    K2LOG_E(log::ltest, "======= Test failed with exception [{}] ========", e.what());
                    exitcode = -1;
                }
            })
            .finally([this] {
                K2LOG_I(log::ltest, "======= Test ended ========");
                seastar::engine().exit(exitcode);
            });
        });
        _testTimer.arm(0ms);
        return seastar::make_ready_future<>();
    }


    seastar::future<> runTest1() {
        K2LOG_I(log::ltest, ">>> Test1: create a persistence cluster");
        dto::PersistenceGroup group1{.name="Group1", .plogServerEndpoints = _plogConfigEps()};
        dto::PersistenceCluster cluster1;
        cluster1.name="Persistence_Cluster_1";
        cluster1.persistenceGroupVector.push_back(group1);

        auto request = dto::PersistenceClusterCreateRequest{.cluster=std::move(cluster1)};
        return RPC()
        .callRPC<dto::PersistenceClusterCreateRequest, dto::PersistenceClusterCreateResponse>(dto::Verbs::CPO_PERSISTENCE_CLUSTER_CREATE, request, *_cpoEndpoint, 1s)
        .then([](auto&& response) {
            auto& [status, resp] = response;
            K2EXPECT(log::ltest, status, Statuses::S201_Created);
        });
    }

    seastar::future<> runTest2() {
        K2LOG_I(log::ltest, ">>> Test2: init and create a logstream");
        ConfigVar<String> configEp("cpo_url");
        String cpo_url = configEp();

        return _mmgr.init(cpo_url, "Partition-1", "Persistence_Cluster_1")
        .then([this] (){
            _client = _mmgr.obtainLogStream("WAL");
        })
        .then([this] (){
            return _mmgr.create();
        })
        .then([this] (){
            return _client -> create();
        });
    }

    seastar::future<> runTest3() {
        K2LOG_I(log::ltest, ">>> Test3: Write and Read a Value from log stream");
        String header;
        for (uint32_t i = 0; i < 10000; ++i){
            header = header + "0";
        }
        Payload payload([] { return Binary(20000); });
        payload.write(header);
        return _client->append(std::move(payload))
        .then([this, header] (auto&& response){
            auto& [plogId, appended_offset] = response;
            _initPlogId = plogId;
            return _client->read(plogId, 0, appended_offset);
        })
        .then([this, header] (auto&& payloads){
            String str;
            for (auto& payload:payloads){
                payload.seek(0);
                payload.read(str);
                K2EXPECT(log::ltest, str, header);
            }
            return seastar::make_ready_future<>();
        });
    }

    seastar::future<> runTest4() {
        K2LOG_I(log::ltest, ">>> Test4: Write and read huge data");
        String header;
        for (uint32_t i = 0; i < 10000; ++i){
            header = header + "0";
        }
        std::vector<seastar::future<std::pair<String, uint32_t>> > writeFutures;
        for (uint32_t i = 0; i < 2000; ++i){
            Payload payload([] { return Binary(20000); });
            payload.write(header);
            writeFutures.push_back(_client->append(std::move(payload)));
        }
        return seastar::when_all_succeed(writeFutures.begin(), writeFutures.end())
        .then([this, header] (std::vector<std::pair<String, uint32_t> >&& responses){
            for (auto& response:responses){
                auto& [plogId, appended_offset] = response;
                K2LOG_D(log::ltest, "{}, {}", plogId, appended_offset);
            }
            K2LOG_I(log::ltest, ">>> Test4.1: Write Done");
            return _client->read(_initPlogId, 0, 2001*10005);
        })
        .then([this, header] (auto&& payloads){
            K2LOG_I(log::ltest, ">>> Test4.2: Read Done");
            String str;
            uint32_t count = 0;
            for (auto& payload:payloads){
                payload.seek(0);
                while (payload.getDataRemaining() > 0){
                    payload.read(str);
                    K2EXPECT(log::ltest, str, header);
                    ++count;
                }
            }
            K2EXPECT(log::ltest, count, 2001);
            std::vector<seastar::future<std::pair<String, uint32_t>> > writeFutures;
            for (uint32_t i = 0; i < 2000; ++i){
                Payload payload([] { return Binary(20000); });
                payload.write(header);
                writeFutures.push_back(_client->append(std::move(payload)));
            }
            return seastar::when_all_succeed(writeFutures.begin(), writeFutures.end());
        })
        .then([this, header] (std::vector<std::pair<String, uint32_t> >&& responses){
            for (auto& response:responses){
                auto& [plogId, appended_offset] = response;
                K2LOG_D(log::ltest, "{}, {}", plogId, appended_offset);
            }
            K2LOG_I(log::ltest, ">>> Test4.3: Write Done");
            return _client->read(_initPlogId, 0, 4001*10005);
        })
        .then([this, header] (auto&& payloads){
            K2LOG_I(log::ltest, ">>> Test4.4: Read Done");
            String str;
            uint32_t count = 0;
            for (auto& payload:payloads){
                payload.seek(0);
                while (payload.getDataRemaining() > 0){
                    payload.read(str);
                    K2EXPECT(log::ltest, str, header);
                    ++count;
                }
            }
            K2EXPECT(log::ltest, count, 4001);
            return seastar::make_ready_future<>();
        });
    }

    seastar::future<> runTest5() {
        K2LOG_I(log::ltest, ">>> Test5: replay the metadata manager");
        ConfigVar<String> configEp("cpo_url");
        String cpo_url = configEp();

        return _reload_mmgr.replay(cpo_url, "Partition-1", "Persistence_Cluster_1")
        .then([this] (auto&& response){
            auto& status  = response;
            K2LOG_D(log::ltest, "{}", status);
            _reload_client = _reload_mmgr.obtainLogStream("WAL");
            return seastar::make_ready_future<>();
        });
    }


    seastar::future<> runTest6() {
        K2LOG_I(log::ltest, ">>> Test6: Read and write data from reloaded WAL");
        String header;
        for (uint32_t i = 0; i < 10000; ++i){
            header = header + "0";
        }
        return _reload_client->read(_initPlogId, 0, 4001*10005)
        .then([this, header] (auto&& payloads){
            String str;
            uint32_t count = 0;
            for (auto& payload:payloads){
                payload.seek(0);
                while (payload.getDataRemaining() > 0){
                    payload.read(str);
                    K2EXPECT(log::ltest, str, header);
                    ++count;
                }
            }
            K2EXPECT(log::ltest, count, 4001);
            std::vector<seastar::future<std::pair<String, uint32_t>> > writeFutures;
            for (uint32_t i = 0; i < 2000; ++i){
                Payload payload([] { return Binary(20000); });
                payload.write(header);
                writeFutures.push_back(_reload_client->append(std::move(payload)));
            }
            return seastar::when_all_succeed(writeFutures.begin(), writeFutures.end());
        })
        .then([this, header] (std::vector<std::pair<String, uint32_t> >&& responses){
            for (auto& response:responses){
                auto& [plogId, appended_offset] = response;
                K2LOG_D(log::ltest, "{}, {}", plogId, appended_offset);
            }
            K2LOG_I(log::ltest, ">>> Test6.2: Write Done");
            return _reload_client->read(_initPlogId, 0, 6001*10005);
        })
        .then([this, header] (auto&& payloads){
            K2LOG_I(log::ltest, ">>> Test6.3: Read Done");
            String str;
            uint32_t count = 0;
            for (auto& payload:payloads){
                payload.seek(0);
                while (payload.getDataRemaining() > 0){
                    payload.read(str);
                    K2EXPECT(log::ltest, str, header);
                    ++count;
                }
            }
            K2EXPECT(log::ltest, count, 6001);
            return seastar::make_ready_future<>();
        });
    }
};

int main(int argc, char** argv) {
    k2::App app("LogStreamTest");
    app.addOptions()("cpo_url", bpo::value<k2::String>(), "The endpoint of the CPO service");
    app.addOptions()("plog_server_endpoints", bpo::value<std::vector<k2::String>>()->multitoken(), "The endpoints of the plog servers");
    app.addApplet<LogStreamTest>();
    return app.start(argc, argv);
}