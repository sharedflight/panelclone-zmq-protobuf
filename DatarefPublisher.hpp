//
//  DatarefPublisher.hpp
//
//  Created by Justin Snapp on 8/31/24.
//  Copyright © 2024 Justin Snapp. All rights reserved.
//

#ifndef DatarefPublisher_hpp
#define DatarefPublisher_hpp


// #include <memory>
// #include <map>
// #include <queue>
// #include <atomic>

#if IBM
#include <mingw.thread.h>
#include <mingw.mutex.h>  
#else
#include <thread>
#include <mutex>  
#endif

#include "zmq/zmq.hpp"
#include "zmq/zmq_addon.hpp"

#include "event.pb.h"

#include "Dref.hpp"

enum ValueType { INTVAL, FLOATVAL, DOUBLEVAL, STRINGVAL };

// We need to keep a "last sent" value in case we have multiple flight loops
// per send ... 

// What if a value changes and then changes back before we send?
// we would miss edges...

// We should send EVERY FLIGHT LOOP? ... NO.   Can create a separate mechanism
// in the future for publishing "edges" and events and commands if necessary...


struct PubValue
{
    Dref *dref;
    int index;
    std::variant<int, float, double, std::pair<std::string_view, size_t>> value;
    
    std::variant<int, float, double, std::pair<std::string_view, size_t>> last_sent_value;
    
    ValueType chosenType;
    bool published;

    /*void initValue() {
        switch(chosenType) {
            case INTVAL:
                value = dref->geti();
                break;
            case FLOATVAL:
                value = dref->getf();
                break;
            case INTARRAYVAL:
                value = dref->geti(index);
                break;
            case FLOATARRAYVAL:
                value = dref->getf(index);
                break;
            case DOUBLEVAL:
                value = dref->getd();
                break;
            // case STRINGVAL:
            //     std::string str;
            //     gets(&str, gets(nullptr, 0));
            //     break;
            default:
                ;
        }

        last_sent_value = value;
    }*/

    void flightLoopRead() {
        if (chosenType == INTVAL) {
            if (index == -1) {
                value = dref->geti();
            } else {
                value = dref->geti(index);
            }
        } else if (chosenType == FLOATVAL) {
            if (index == -1) {
                value = dref->getf();
            } else {
                value = dref->getf(index);
            }
        } else if (chosenType == DOUBLEVAL) {
            value = dref->getd();
        }
    }

    bool changed() {
        return (value != last_sent_value);
    }

    bool shouldSend() {
        return (!published || changed());
    }

    void sent() {
        published = true;
        last_sent_value = value;
    }
};

class DatarefPublisher
{
    std::mutex lock;
    
    // Create ZMQ Context
    zmq::context_t context;
    // Create the Publish socket
    zmq::socket_t publisher;
    // Create the Snapshot socket
    zmq::socket_t snapshot;
    // Create the Collector socket
    zmq::socket_t collector;

    //panelclone::StateUpdate latest_frame;

    std::map<std::string, Dref> drefs_map;

    std::vector<PubValue> published_values;

    std::atomic<unsigned int> latest_frame;

    //std::vector<size_t> unannounced_indexes;

    std::thread t;
    std::atomic<bool> keep_running;

    std::atomic<bool> have_unread_drefs;
    std::vector<std::string> peers_requesting_snapshots;

public:

    static DatarefPublisher& getInstance();

    DatarefPublisher();
    ~DatarefPublisher();
    
    void Init();
    void Start();
    void Finish();

    void GetFrame();

private:

    void PublishWorker();
    void PublishLatestFrame();
    void AnswerSnapshotRequests();
};

#endif /* DatarefPublisher_hpp */