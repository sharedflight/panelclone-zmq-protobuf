//
//  DatarefPublisher.cpp
//
//  Created by Justin Snapp on 8/31/24.
//  Copyright © 2024 Justin Snapp. All rights reserved.
//

#include "DatarefPublisher.hpp"

// #include <string>
// #include <iostream>
// #include <chrono>
// #include <memory>

// #include <assert.h>

//#include "SFCrashHandling.hpp"

extern "C" {
    #include <acfutils/log.h>
}

DatarefPublisher& DatarefPublisher::getInstance() {
    static DatarefPublisher instance;   // Guaranteed to be destroyed.
                                         // Instantiated on first use.
    return instance;
}

DatarefPublisher::DatarefPublisher()
{
    
}

DatarefPublisher::~DatarefPublisher()
{
    
}

void DatarefPublisher::Init()
{
    keep_running.store(true);
}

void DatarefPublisher::Start()
{
    Init();
    t = std::thread(&DatarefPublisher::PublishWorker, this);
}

void DatarefPublisher::Finish()
{
    if (!keep_running.load()) {
        return;
    }

    lock.lock();
    keep_running.store(false);
    lock.unlock();
    
    t.join();
}

void DatarefPublisher::GetFrame()
{
    lock.lock();
    
    for (auto & pubVal : published_values) {
        pubVal.flightLoopRead();
    }

    lock.unlock();
    //panelclone::StateUpdate
}

void DatarefPublisher::PublishWorker()
{
    
    //StThreadCrashCookie crashCookie;

    // Create ZMQ Context
    zmq::context_t context ( 1 );
    // Create the Publish socket
    zmq::socket_t publisher ( context, ZMQ_PUB );
    // Create the Snapshot socket
    zmq::socket_t snapshot ( context, ZMQ_ROUTER);
    // Create the Collector socket
    zmq::socket_t collector ( context, ZMQ_PULL);

    // Bind to a tcp sockets
    snapshot.bind("tcp://*:5556");
    publisher.bind("tcp://*:5557");
    collector.bind("tcp://*:5558");


    int i = 1;

    zmq_pollitem_t items [] = {
        { collector, 0, ZMQ_POLLIN, 0 },
        { snapshot, 0, ZMQ_POLLIN, 0 }
    };


    while (keep_running.load() == true) {

        try {
            
            std::string msg = "blah";

            // Create zmq message
            zmq::message_t request( msg.length() );
            // Copy contents to zmq message
            memcpy( request.data(), msg.c_str(), msg.length() );
            // Publish the message
            publisher.send( request );
            //std::cout << "sending: " << i++ << std::endl;

            zmq::message_t message;
            zmq::poll (&items [0], 2, 0);
            
            if (items [0].revents & ZMQ_POLLIN) {
                collector.recv(message, zmq::recv_flags::none);
                //  Process task
                logMsg("received collector message: %s", message.str().c_str());
            }
            
            if (items[1].revents & ZMQ_POLLIN) {
                //snapshot.recv(message, zmq::recv_flags::none);
                
                std::vector<zmq::message_t> msgs;
                auto ret = zmq::recv_multipart(snapshot, std::back_inserter(msgs));

                if (!ret) {
                    return;
                }

                logMsg("size of msgs is %d", msgs.size());

                std::string requestor = msgs[0].to_string();

                auto & message = msgs[1];


                logMsg("raw msg is: %s", message.str().c_str());

                panelclone::StateRequest recvdStateRequest;
                recvdStateRequest.ParseFromArray(message.data(), message.size());

                logMsg("received snapshot message from %s, with %d daterefs", requestor.c_str(), recvdStateRequest.drefs_size());
                
                auto numrefs = recvdStateRequest.drefs_size();

                lock.lock();
    
                for (auto i = 0; i < numrefs; i++) {
                    
                    auto dref_path = recvdStateRequest.drefs(i).dataref();

                    auto it = drefs_map.find(dref_path);

                    if (it != drefs_map.end()) {

                        logMsg("[DEBUG] Found dref %s is being published", dref_path.c_str());

                    } else {
                        Dref new_dr;

                        if (!new_dr.find(dref_path)) {
                            continue;
                        }

                        logMsg("[DEBUG] Added dref %s to be published", dref_path.c_str());

                        drefs_map[dref_path] = std::move(new_dr);
                    }

                    PubValue pubVal;

                    pubVal.dref = &drefs_map[dref_path];
                    pubVal.index = static_cast<size_t>(recvdStateRequest.drefs(i).index());
                    pubVal.published = false;

                    {
                        // Determine the type we will use for the value...

                        auto type = pubVal.dref->type();

                        if (type & xplmType_Int) {
                            pubVal.chosenType = INTVAL;
                        } else if (type & xplmType_Float) {
                            pubVal.chosenType = FLOATVAL;
                        } else if (type & xplmType_Double) {
                            pubVal.chosenType = DOUBLEVAL;
                        } else if (type & xplmType_IntArray) {
                            pubVal.chosenType = INTARRAYVAL;
                        } else if (type & xplmType_FloatArray) {
                            pubVal.chosenType = FLOATARRAYVAL;
                        } else if (type & xplmType_Data) {
                            pubVal.chosenType = STRINGVAL;
                        }
                    }

                    pubVal.initValue();

                    published_values.push_back(std::move(pubVal));

                    logMsg("[DEBUG] Added published value %s[%d] to be published at index %d", dref_path.c_str(), published_values.back().index, published_values.size() - 1);

                    //unannounced_indexes.push_back(published_values.size() - 1);

                }

                panelclone::Snapshot txSnapshot;

                auto * frameSnapshot = txSnapshot.mutable_framesnapshot();

                frameSnapshot->set_frame(i++);

                auto drefChange = frameSnapshot->add_drefchanges();

                drefChange->set_index(0);
                drefChange->set_intval(10);

                lock.unlock();
    

                //trodes::proto::Event event;
                //event.set_name("myevent");
                //event.set_origin("myorigin");

                std::string data;
                txSnapshot.SerializeToString(&data);

                // Publish the message
                std::array<zmq::const_buffer, 2> bufs = {
                  zmq::buffer(requestor),
                  zmq::buffer(data)
                };
                zmq::send_multipart(snapshot, bufs);

            }

        } catch (zmq::error_t &e) {
            std::cout << "interrupt received, proceeding..." << std::endl;
        }

        std::this_thread::sleep_for( std::chrono::milliseconds(20) );
    }

}