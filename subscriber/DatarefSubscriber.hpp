//
//  DatarefSubscriber.hpp
//
//  Created by Justin Snapp on 8/31/24.
//  Copyright © 2024 Justin Snapp. All rights reserved.
//

#ifndef DatarefSubscriber_hpp
#define DatarefSubscriber_hpp

#if IBM
#include <mingw.thread.h>
#include <mingw.mutex.h>  
#else
#include <thread>
#include <mutex>  
#endif

#include <zmq.hpp>
#include <zmq_addon.hpp>

#include <event.pb.h>

#include "PubValue.hpp"
#include "PubFloatValue.hpp"


class DatarefSubscriber
{
    std::mutex lock;
    
    bool local = false;

    // Create ZMQ Context
    zmq::context_t context;
    // Create the Updater socket
    zmq::socket_t updater;
    // Create the Snapshot socket
    zmq::socket_t snapshot;
    // Create the Subscriber socket
    zmq::socket_t subscriber;

    std::map<size_t, PubValue> recvdPubValues;

    std::atomic<unsigned int> latest_frame;

    std::thread t;
    std::atomic<bool> keep_running;

    std::atomic<bool> waiting_for_snapshot;

    std::atomic<bool> received_snapshot;

    panelclone::StateUpdate latest_stateUpdate;

public:

    static DatarefSubscriber& getInstance();

    DatarefSubscriber();
    ~DatarefSubscriber();
    
    void Init();
    void Start();
    void Finish();

    void SetLocal(bool islocal) { local = islocal;};

    bool Ready();

    bool GetLatestFrame(unsigned int cur_frame, panelclone::StateUpdate & stateUpdate);

    bool GetLocalSnapshot(panelclone::Snapshot & txSnapshot);

    void RequestDatarefs(std::vector<std::string>& datarefList);

    bool FindFloatValue(PubFloatValue *dr, const std::string & dr_name);

    bool HavePubValue(std::string dataref, int dref_index);

    float GetFloatValue(const int index);

private:

    void SubscriberWorker();

};

#endif /* DatarefSubscriber_hpp */