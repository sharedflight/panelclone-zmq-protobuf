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
    latest_frame.store(0);

    // Create ZMQ Context
    context = zmq::context_t( 1 );
    // Create the Publish socket
    publisher = zmq::socket_t( context, ZMQ_PUB );
    // Create the Snapshot socket
    snapshot = zmq::socket_t( context, ZMQ_ROUTER);
    // Create the Collector socket
    collector = zmq::socket_t( context, ZMQ_PULL);

    // Bind to a tcp sockets
    snapshot.bind("tcp://*:5556");
    publisher.bind("tcp://*:5557");
    collector.bind("tcp://*:5558");

    have_unread_drefs.store(false);
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

    latest_frame.store(latest_frame.load() + 1);

    have_unread_drefs.store(false);

    lock.unlock();
    //panelclone::StateUpdate
}

void DatarefPublisher::PublishLatestFrame()
{
    lock.lock();

    bool somethingToSend = false;

    panelclone::StateUpdate stateUpdate;

    stateUpdate.set_frame(latest_frame.load());

    unsigned int published_values_index = 0;
    for (auto & pubVal : published_values) {
        if ( pubVal.shouldSend() ) {

            somethingToSend = true;

            auto drefvalue =  stateUpdate.add_drefchanges();

            drefvalue->set_index(published_values_index);

            if (pubVal.chosenType == INTVAL) {
                drefvalue->set_intval(std::get<int>(pubVal.value));
            } else if (pubVal.chosenType == FLOATVAL) {
                drefvalue->set_floatval(std::get<float>(pubVal.value));
            } else if (pubVal.chosenType == DOUBLEVAL) {
                drefvalue->set_doubleval(std::get<double>(pubVal.value));
            } else if (pubVal.chosenType == STRINGVAL) {

            } else {
                logMsg("[ERROR] Not setting a value, unhandled chosen type!");
            }

            if (!pubVal.published) {
                auto pubvalindex = stateUpdate.add_publishedvalueindexes();

                pubvalindex->set_index(published_values_index);
                pubvalindex->set_dataref(pubVal.dref->name());
                pubvalindex->set_dref_index(pubVal.index);

                pubVal.published = true;
            }

            pubVal.sent();
        }

        published_values_index++;
    }

    if (somethingToSend) {
        std::string data;
        stateUpdate.SerializeToString(&data);

        // Create zmq message
        //zmq::message_t request( data.length() );
        // Copy contents to zmq message
        //memcpy( request.data(), data.c_str(), data.length() );
        
        // Publish the message
        publisher.send(zmq::buffer("state"), zmq::send_flags::sndmore);
        publisher.send(zmq::buffer(data), zmq::send_flags::dontwait);

        logMsg("Sent a state update with %d changes", stateUpdate.drefchanges_size());
    }

    lock.unlock();
}

void DatarefPublisher::AnswerSnapshotRequests()
{
    lock.lock();

    if (have_unread_drefs.load()) {
        lock.unlock();
        return;
    }

    // Prepare snapshot to send...

    panelclone::Snapshot txSnapshot;

    auto * frameSnapshot = txSnapshot.mutable_framesnapshot();

    frameSnapshot->set_frame(latest_frame.load());

    unsigned int published_values_index = 0;
    for (auto & pubVal : published_values) {
        
        auto drefvalue =  frameSnapshot->add_drefchanges();

        drefvalue->set_index(published_values_index);

        if (pubVal.chosenType == INTVAL) {
            drefvalue->set_intval(std::get<int>(pubVal.value));
        } else if (pubVal.chosenType == FLOATVAL) {
            drefvalue->set_floatval(std::get<float>(pubVal.value));
        } else if (pubVal.chosenType == DOUBLEVAL) {
            drefvalue->set_doubleval(std::get<double>(pubVal.value));
        // } else if (pubVal.chosenType == INTARRAYVAL) {
        //     auto intarrayval = drefvalue->mutable_intarrayval();
        //     intarrayval->set_value(std::get<int>(pubVal.value));
        //     intarrayval->set_index(pubVal.index);
        // } else if (pubVal.chosenType == FLOATARRAYVAL) {
        //     auto floatarrayval = drefvalue->mutable_floatarrayval();
        //     floatarrayval->set_value(std::get<float>(pubVal.value));
        //     floatarrayval->set_index(pubVal.index);
        } else if (pubVal.chosenType == STRINGVAL) {

        }

        auto pubvalindex = frameSnapshot->add_publishedvalueindexes();

        pubvalindex->set_index(published_values_index);
        pubvalindex->set_dataref(pubVal.dref->name());
        pubvalindex->set_dref_index(pubVal.index);

        pubVal.published = true;

        published_values_index++;
    }

    std::string data;
    txSnapshot.SerializeToString(&data);

    for (auto & requestor : peers_requesting_snapshots) {
        // Publish the message
        std::array<zmq::const_buffer, 2> bufs = {
          zmq::buffer(requestor),
          zmq::buffer(data)
        };
        zmq::send_multipart(snapshot, bufs);
    }

    peers_requesting_snapshots.clear();

    lock.unlock();
}

void DatarefPublisher::PublishWorker()
{
    
    //StThreadCrashCookie crashCookie;

    


    int i = 1;

    zmq_pollitem_t items [] = {
        { collector, 0, ZMQ_POLLIN, 0 },
        { snapshot, 0, ZMQ_POLLIN, 0 }
    };


    while (keep_running.load()) {

        try {
            
            PublishLatestFrame();

            AnswerSnapshotRequests();

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

                std::string requestor = msgs[0].to_string();

                auto & message = msgs[1];

                panelclone::StateRequest recvdStateRequest;
                recvdStateRequest.ParseFromArray(message.data(), message.size());

                logMsg("received snapshot message from %s, with %d daterefs", requestor.c_str(), recvdStateRequest.drefs_size());

                peers_requesting_snapshots.push_back(requestor);
                
                auto numrefs = recvdStateRequest.drefs_size();

                lock.lock();
    
                for (auto i = 0; i < numrefs; i++) {
                    
                    auto dref_path = recvdStateRequest.drefs(i).dataref();

                    auto recvd_index = recvdStateRequest.drefs(i).index();

                    auto it = drefs_map.find(dref_path);

                    if (it != drefs_map.end()) {

                        logMsg("[DEBUG] Found dref %s is being published", dref_path.c_str());

                        bool already_have = false;

                        // Check if we already have the same published value...
                        for (auto & pubVal : published_values) {
                            if (pubVal.dref->name() == dref_path && pubVal.index == recvd_index) {
                                logMsg("[DEBUG] Already have this published value...");
                                already_have = true;
                                break;
                            }
                        }

                        if (already_have) {
                            continue;
                        }

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
                    pubVal.index = static_cast<size_t>(recvd_index);
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
                            pubVal.chosenType = INTVAL;
                        } else if (type & xplmType_FloatArray) {
                            pubVal.chosenType = FLOATVAL;
                        } else if (type & xplmType_Data) {
                            pubVal.chosenType = STRINGVAL;
                        }
                    }

                    //We can not call this here not on the main thread...
                    //pubVal.initValue();

                    published_values.push_back(std::move(pubVal));



                    logMsg("[DEBUG] Added published value %s[%d] to be published at index %d", dref_path.c_str(), published_values.back().index, published_values.size() - 1);

                    //unannounced_indexes.push_back(published_values.size() - 1);

                    have_unread_drefs.store(true);

                }

                lock.unlock();
            }

        } catch (zmq::error_t &e) {
            std::cout << "interrupt received, proceeding..." << std::endl;
        }

        std::this_thread::sleep_for( std::chrono::milliseconds(20) );
    }

}