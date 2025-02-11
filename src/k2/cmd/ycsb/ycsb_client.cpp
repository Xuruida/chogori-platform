/*
MIT License

Copyright(c) 2021 Futurewei Cloud

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

#include <atomic>
#include <chrono>

#include <k2/appbase/Appbase.h>
#include <k2/appbase/AppEssentials.h>
#include <k2/dto/FieldTypes.h>
#include <k2/module/k23si/client/k23si_client.h>
#include <k2/transport/RetryStrategy.h>
#include <k2/tso/client/Client.h>
#include <k2/common/Timer.h>
#include <seastar/core/sleep.hh>

#include <k2/cmd/ycsb/Log.h>
#include <k2/cmd/ycsb/data.h>
#include <k2/cmd/ycsb/dataload.h>
#include <k2/cmd/ycsb/transactions.h>

using namespace k2;
using namespace dto;

// variable to keep track of number of cores that complete benchmark
std::atomic<uint32_t> cores_finished = 0;

/* This function is used for obtaining the range of YCSB Data keys that will be stored each partition.
   It is invoked while creating YCSB Data collection
   "numPartitions" is the total number of partitions
   "numRows" records is the max number of YCSB Data records that will be stored
   "lenfield" is the max length of each field in a YCSB Data record */
std::vector<String> getRangeEnds(uint32_t numPartitions, size_t numRows, uint32_t lenField) {

    std::deque<String> keys;

    for(size_t i = 0; i < numRows+1; ++i){ // generate all keys for YCSB Data to get Range
        keys.push_back(YCSBData::idToKey(i,lenField));
    }

    std::sort(keys.begin(),keys.end());

    // share is the number of records that will be stored per partition
    // the last partition can have more than share records
    uint32_t share = numRows / numPartitions;
    share = share == 0 ? 1 : share;
    std::vector<String> rangeEnds;

    // range end is open interval
    for (uint32_t i = 1; i <= numPartitions; ++i) {
        String range_end = FieldToKeyString<String>(keys[(i*share)]);
        K2LOG_D(log::ycsb, "RangeEnd: {}", range_end);
        rangeEnds.push_back(range_end);
    }

    // range end of last partition is set to "" as range end is open interval and "" will not be counted as key
    rangeEnds[numPartitions-1] = "";

    return rangeEnds;
}

class Client {
public:  // application lifespan
    Client():
        _client(K23SIClient(K23SIClientConfig())),
        _stopped(true),
        _timer([this] {
            _stopped = true;
        }) {
        K2LOG_I(log::ycsb, "ctor");
    };

    ~Client() {
        K2LOG_I(log::ycsb, "dtor");
    }

    // required for seastar::distributed interface
    seastar::future<> gracefulStop() {
        K2LOG_I(log::ycsb, "stop");
        _stopped = true;
        // unregister all observers
        k2::RPC().registerLowTransportMemoryObserver(nullptr);

        return std::move(_benchFuture);
    }

    /* Each transaction will comprise of a set of read, scan, insert, update and delete operations.
       The number of operations per transaction is set as per the specified config.
       For the metrics we measure the number of transcations and the number of each operation */
    void registerMetrics() {
        _metric_groups.clear();
        std::vector<sm::label_instance> labels;
        labels.push_back(sm::label_instance("total_cores", seastar::smp::count));

        _metric_groups.add_group("YCSB", {
            sm::make_counter("completed_txns", _completedTxns, sm::description("Number of completed YCSB transactions"), labels),
            sm::make_counter("read_ops", _readOps, sm::description("Number of completed Read operations"), labels),
            sm::make_counter("scan_ops", _scanOps, sm::description("Number of completed Scan operations"), labels),
            sm::make_counter("insert_ops", _insertOps, sm::description("Number of completed Insert operations"), labels),
            sm::make_counter("update_ops", _updateOps, sm::description("Number of completed Update operations"), labels),
            sm::make_counter("delete_ops", _deleteOps, sm::description("Number of completed Delete operations"), labels),
            sm::make_counter("insert_misses", _insertMissesLatest, sm::description("Number of Insert Misses for Latest distribution"), labels),
            sm::make_histogram("txn_latency", [this]{ return _txnLatency.getHistogram();},
                    sm::description("Latency of YCSB transactions"), labels),
            sm::make_histogram("read_latency", [this]{ return _readLatency.getHistogram();},
                    sm::description("Latency of YCSB Read Operations"), labels),
            sm::make_histogram("update_latency", [this]{ return _updateLatency.getHistogram();},
                    sm::description("Latency of YCSB  Update Operations"), labels),
            sm::make_histogram("scan_latency", [this]{ return _scanLatency.getHistogram();},
                    sm::description("Latency of YCSB  Scan Operations"), labels),
            sm::make_histogram("insert_latency", [this]{ return _insertLatency.getHistogram();},
                    sm::description("Latency of YCSB  Insert Operations"), labels),
            sm::make_histogram("delete_latency", [this]{ return _deleteLatency.getHistogram();},
                    sm::description("Latency of YCSB  Delete Operations"), labels)
        });
    }

    seastar::future<> start() {
        _stopped = false;

        _num_keys = _num_records() + _num_inserts(); // key space is expanded to include expected inserts
        YCSBData::generateSchema(_num_fields());
        setupSchemaPointers();

        registerMetrics();

        _benchFuture = _client.start().then([this] () { return _benchmark(); })
        .handle_exception([this](auto exc) {
            K2LOG_W_EXC(log::ycsb, exc, "Unable to execute benchmark");
            _stopped = true;
            return seastar::make_ready_future<>();
        }).finally([this]() {
            K2LOG_I(log::ycsb, "Done with benchmark");
            cores_finished++;
            if (cores_finished == seastar::smp::count) {
                seastar::engine().exit(0);
            }

            return seastar::make_ready_future<>();
        });

        return seastar::make_ready_future<>();
    }

private:
    // This function is used for loading the YCSB Data Schema
    seastar::future<> _schemaLoad() {

        return _client.createSchema(ycsbCollectionName, YCSBData::ycsb_schema)
        .then([] (auto&& result) {
            K2ASSERT(log::ycsb, result.status.is2xxOK(), "Failed to create schema");
        });
    }

    /* This function is used for loading data.
       It first creates a collection for YCSB data and then loads the data */
    seastar::future<> _dataLoad() {
        K2LOG_I(log::ycsb, "Creating DataLoader");
        int cpus = seastar::smp::count;
        int id = seastar::this_shard_id();
        int share = _num_keys / cpus; // number of records loaded per cpu, the last one can have more than share loads

        auto f = seastar::sleep(5s); // sleep for collection to be loaded first
        if (id == 0) { // only shard 0 loads the collection
            f = f.then ([this] {
                K2LOG_I(log::ycsb, "Creating collection");
                return _client.makeCollection("YCSB", getRangeEnds(_tcpRemotes().size(), _num_keys,_field_length()));
            }).discard_result()
            .then([this] () {
                return _schemaLoad();
            });
        } else {
            f = f.then([] { return seastar::sleep(5s); });
        }

        return f.then ([this, share, id, cpus] {
            K2LOG_I(log::ycsb, "Starting data gen and load in shard {}", id);
            _data_loader = DataLoader();
            size_t end_idx = (id*share)+share; // end_idx to load record
            if(id==(cpus-1))
                end_idx = _num_keys;
            return _data_loader.loadData(_client, _num_concurrent_txns(),(id*share),end_idx); // load records from id*share to end_idx (not inclusive)
        }).then ([this] {
            K2LOG_I(log::ycsb, "Data load done");
        });
    }

    // Function to create and run ycsb transactions and record metrics
    seastar::future<> _ycsb() {
        K2LOG_D(log::ycsb, "Starting Transactions in ycsb()");
        return seastar::do_until(
            [this] { return _stopped; },
            [this] {
                return seastar::do_with(std::unique_ptr<YCSBTxn>(), [this] ( std::unique_ptr<YCSBTxn>& curTxn) {
                    curTxn = (std::unique_ptr<YCSBTxn>) new YCSBTxn(_random, _client, _requestDist, _scanLengthDist);

                    auto txn_start = k2::Clock::now();
                    return curTxn->run() // run the transaction
                    .then([this, txn_start, &curTxn] (bool success) {
                        if (!success) {
                            return;
                        }

                        // update metrics for succeeded transaction
                        _completedTxns++;
                        _insertMissesLatest += curTxn->getInsertMisses();
                        auto end = k2::Clock::now();
                        auto dur = end - txn_start;

                        _txnLatency.add(dur);
                        for(auto&& op: curTxn->getOps()){ // go over all operations in this transaction and record metrics
                            switch(op){
                                case Operation::Read: {
                                    _readOps++;
                                    _readLatency.add(dur);
                                    break;
                                }
                                case Operation::Update: {
                                    _updateOps++;
                                    _updateLatency.add(dur);
                                    break;
                                }
                                case Operation::Scan: {
                                    _scanOps++;
                                    _scanLatency.add(dur);
                                    break;
                                }
                                case Operation::Insert: {
                                    _insertOps++;
                                    _insertLatency.add(dur);
                                    break;
                                }
                                case Operation::Delete: {
                                    _deleteOps++;
                                    _deleteLatency.add(dur);
                                }
                            };
                        }
                    })
                    .finally([] () {
                        K2LOG_D(log::ycsb, "Completed Transaction");
                    });
                });
            }
        );
    }

    /* This function will perform data loading if do_data_load is set
       and perform YCSB Benchmarking for given workload
    */
    seastar::future<> _benchmark() {

        K2LOG_I(log::ycsb, "Creating K23SIClient");

        if (_do_data_load()) {
            return _dataLoad();
        }

        return seastar::sleep(5s)
        .then([this] {
            K2LOG_I(log::ycsb, "Starting transactions...");

            _timer.arm(_testDuration());
            _start = k2::Clock::now();
            _random = RandomContext(seastar::this_shard_id(), getOpsProportion()); // initialize Random Context with operations proportions for biased selection
            if(_requestDistName()=="latest"){ // create request Distribution
                _requestDist = getDistribution(_requestDistName(),0,_num_records()-1, seastar::this_shard_id()); // set max to the last loaded record
            } else {
                _requestDist = getDistribution(_requestDistName(),0,_num_keys-1, seastar::this_shard_id()); // set max to the total key space size
            }
            _scanLengthDist = getDistribution(_scanLengthDistName(),1,_maxScanLen(), seastar::this_shard_id()); // create Scan length Distribution

            for (int i=0; i < _num_concurrent_txns(); ++i) {
                _ycsb_futures.emplace_back(_ycsb());
            }
            return when_all(_ycsb_futures.begin(), _ycsb_futures.end());
        })
        .discard_result()
        .finally([this] () {
            auto duration = k2::Clock::now() - _start;
            auto totalsecs = ((double)k2::msec(duration).count())/1000.0;
            auto cntpsec = (double)_completedTxns/totalsecs;
            auto readpsec = (double)_client.read_ops/totalsecs;
            auto writepsec = (double)_client.write_ops/totalsecs;
            auto querypsec = (double)_client.query_ops/totalsecs;
            K2LOG_I(log::ycsb, "duration={}", duration);
            K2LOG_I(log::ycsb, "completedTxns={} ({} per sec)", _completedTxns, cntpsec);
            K2LOG_I(log::ycsb, "completed YCSB Read operations={}", _readOps);
            K2LOG_I(log::ycsb, "completed YCSB Update operations={}", _updateOps);
            K2LOG_I(log::ycsb, "completed YCSB Scan operations={}", _scanOps);
            K2LOG_I(log::ycsb, "completed YCSB Insert operations={}", _insertOps);
            K2LOG_I(log::ycsb, "completed YCSB Delete operations={}", _deleteOps);
            K2LOG_I(log::ycsb, "read ops {} per sec", readpsec);
            K2LOG_I(log::ycsb, "write ops {} per sec", writepsec);
            K2LOG_I(log::ycsb, "query ops {} per sec", querypsec);
            if(_requestDistName()=="latest") {
                  K2LOG_I(log::ycsb, "Total Insert Misses={}", _insertMissesLatest);
            }
            return seastar::make_ready_future();
        });
    }

    // returns distribution that generates items in range [min,max] and uses given seed
    std::shared_ptr<RandomGenerator> getDistribution(String name, uint64_t min, uint64_t max, int seed = 0){

        K2ASSERT(log::ycsb, name=="uniform" || name=="zipfian" || name=="szipfian" || name=="latest", "Invalid distribution name");

        std::shared_ptr<RandomGenerator> dis;

        if(name=="uniform"){
            dis = std::make_shared<UniformGenerator>(min,max,seed);
        }else if(name=="zipfian"){
            dis = std::make_shared<ZipfianGenerator>(min,max,seed);
        }else if(name=="szipfian"){
            dis = std::make_shared<ScrambledZipfianGenerator>(min,max,seed);
        }else{
            dis = std::make_shared<LatestGenerator>(min,max,seed);
        }

        return dis;
    }

    // Function returns a vector of operation proportion to initialize discrete generator in Random context
    std::vector<double> getOpsProportion(){
        std::vector<double> opsProportion(5,0); // there are 5 operations
        uint8_t i = 0;
        for(i=0;i<5;i++){
            switch((Operation)i){
                    case Operation::Read: {
                        opsProportion[i] = _readProp();
                        break;
                    }
                    case Operation::Update: {
                        opsProportion[i] = _updateProp();
                        break;
                    }
                    case Operation::Scan: {
                        opsProportion[i] = _scanProp();
                        break;
                    }
                    case Operation::Insert: {
                        opsProportion[i] = _insertProp();
                        break;
                    }
                    case Operation::Delete: {
                        opsProportion[i] = _deleteProp();
                    }
            };
        }
        K2ASSERT(log::ycsb, accumulate(opsProportion.begin(), opsProportion.end(), 0)==100, "Operation proportions don't sum upto 100");
        return opsProportion;
    }

private:
    K23SIClient _client;
    bool _stopped = true;
    DataLoader _data_loader;
    RandomContext _random;
    k2::TimePoint _start;
    SingleTimer _timer;
    std::vector<seastar::future<>> _ycsb_futures;
    seastar::future<> _benchFuture = seastar::make_ready_future<>();
    std::shared_ptr<RandomGenerator> _requestDist; // Request distribution for selecting keys
    std::shared_ptr<RandomGenerator> _scanLengthDist; // Request distribution for selecting length of scan
    size_t _num_keys; // total keys in keyspace

    ConfigVar<std::vector<String>> _tcpRemotes{"tcp_remotes"};
    ConfigVar<bool> _do_data_load{"data_load"};
    ConfigVar<int> _num_concurrent_txns{"num_concurrent_txns"};
    ConfigVar<uint32_t> _num_fields{"num_fields"};
    ConfigVar<uint32_t> _field_length{"field_length"};
    ConfigVar<size_t> _num_records{"num_records"};
    ConfigVar<size_t> _num_inserts{"num_records_insert"};
    ConfigVar<uint64_t> _ops_per_txn{"ops_per_txn"};
    k2::ConfigDuration _testDuration{"test_duration", 30s};

    ConfigVar<String> _requestDistName{"request_dist"};
    ConfigVar<String> _scanLengthDistName{"scan_length_dist"};

    ConfigVar<double> _readProp{"read_proportion"};
    ConfigVar<double> _updateProp{"update_proportion"};
    ConfigVar<double> _scanProp{"scan_proportion"};
    ConfigVar<double> _insertProp{"insert_proportion"};
    ConfigVar<double> _deleteProp{"delete_proportion"};
    ConfigVar<uint32_t> _maxScanLen{"max_scan_length"};

    sm::metric_groups _metric_groups;
    k2::ExponentialHistogram _txnLatency;
    k2::ExponentialHistogram _readLatency;
    k2::ExponentialHistogram _updateLatency;
    k2::ExponentialHistogram _scanLatency;
    k2::ExponentialHistogram _insertLatency;
    k2::ExponentialHistogram _deleteLatency;
    uint64_t _completedTxns{0};
    uint64_t _readOps{0};
    uint64_t _insertOps{0};
    uint64_t _updateOps{0};
    uint64_t _scanOps{0};
    uint64_t _deleteOps{0};
    uint64_t _insertMissesLatest{0}; // number of inserts missed when request distribution is Latest distribution

}; // class Client

int main(int argc, char** argv) {;
    k2::App app("YCSBClient");
    app.addOptions()
        ("tcp_remotes", bpo::value<std::vector<k2::String>>()->multitoken()->default_value(std::vector<k2::String>()), "A list(space-delimited) of TCP remote endpoints to assign to each core. e.g. 'tcp+k2rpc://192.168.1.2:12345'")
        ("cpo", bpo::value<k2::String>(), "URL of Control Plane Oracle (CPO), e.g. 'tcp+k2rpc://192.168.1.2:12345'")
        ("tso_endpoint", bpo::value<k2::String>(), "URL of Timestamp Oracle (TSO), e.g. 'tcp+k2rpc://192.168.1.2:12345'")
        ("data_load", bpo::value<bool>()->default_value(false), "If true, only data gen and load are performed. If false, only benchmark is performed.")
        ("num_concurrent_txns", bpo::value<int>()->default_value(2), "Number of concurrent transactions to use")
        ("test_duration", bpo::value<k2::ParseableDuration>(), "How long in seconds to run")
        ("partition_request_timeout", bpo::value<ParseableDuration>(), "Timeout of K23SI operations, as chrono literals")
        ("dataload_txn_timeout", bpo::value<ParseableDuration>(), "Timeout of dataload txn, as chrono literal")
        ("writes_per_load_txn", bpo::value<size_t>()->default_value(10), "The number of writes to do in the load phase between txn commit calls")
        ("cpo_request_timeout", bpo::value<ParseableDuration>(), "CPO request timeout")
        ("cpo_request_backoff", bpo::value<ParseableDuration>(), "CPO request backoff")
        ("num_records",bpo::value<size_t>()->default_value(5000),"How many records to load")
        ("num_records_insert",bpo::value<size_t>()->default_value(50),"How many records expected to be inserted during workloads")
        ("field_length",bpo::value<uint32_t>()->default_value(10),"The size of all fields in the table")
        ("num_fields",bpo::value<uint32_t>()->default_value(10), "The number of fields in the table")
        ("request_dist",bpo::value<String>()->default_value("szipfian"), "Request distribution") // choices are: "uniform", "szipfian" (scrambledZipfian), "latest"
        ("scan_length_dist",bpo::value<String>()->default_value("uniform"), "Scan length distribution") // choices are: "uniform", "zipfian" - favouring shorter scans
        ("read_proportion",bpo::value<double>()->default_value(70.0), "Read Proportion")
        ("update_proportion",bpo::value<double>()->default_value(10.0), "Update Proportion")
        ("scan_proportion",bpo::value<double>()->default_value(20.0), "Scan Proportion")
        ("insert_proportion",bpo::value<double>()->default_value(0), "Insert Proportion")
        ("delete_proportion",bpo::value<double>()->default_value(0), "Delete Proportion")
        ("max_scan_length",bpo::value<uint32_t>()->default_value(10), "Maximum scan length")
        ("max_fields_update",bpo::value<uint32_t>()->default_value(1), "Maximum number of fields to update")
        ("ops_per_txn",bpo::value<uint64_t>()->default_value(1), "The number of operations per transaction");

    app.addApplet<k2::tso::TSOClient>();
    app.addApplet<Client>();
    return app.start(argc, argv);
}
