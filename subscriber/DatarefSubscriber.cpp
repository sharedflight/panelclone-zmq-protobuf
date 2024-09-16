//
//  DatarefSubscriber.cpp
//
//  Created by Justin Snapp on 8/31/24.
//  Copyright © 2024 Justin Snapp. All rights reserved.
//

#include "DatarefSubscriber.hpp"

DatarefSubscriber& DatarefSubscriber::getInstance() {
    static DatarefSubscriber instance;   // Guaranteed to be destroyed.
                                         // Instantiated on first use.
    return instance;
}

DatarefSubscriber::DatarefSubscriber()
{
    
}

DatarefSubscriber::~DatarefSubscriber()
{
    
}

void DatarefSubscriber::Init()
{
    keep_running.store(true);
    latest_frame.store(0);

    // Create ZMQ Context
    context = zmq::context_t( 1 );
    // Create the Updater socket
    updater = zmq::socket_t( context, ZMQ_PUSH );
    // Create the Snapshot socket
    snapshot = zmq::socket_t( context, ZMQ_DEALER);
    // Create the Subscriber socket
    subscriber = zmq::socket_t( context, ZMQ_SUB);

    subscriber.set(zmq::sockopt::subscribe, "");
    
    std::cout << "connecting..." << std::endl;

    std::string socketaddress;

    // Bind to a tcp sockets
    if (local) {
        socketaddress = "tcp://" + _ipv4address + ":5559";
        snapshot.connect(socketaddress.c_str());
        socketaddress = "tcp://" + _ipv4address + ":5560";
        subscriber.connect(socketaddress.c_str());
        socketaddress = "tcp://" + _ipv4address + ":5561";
        updater.connect(socketaddress.c_str());
    } else {
        socketaddress = "tcp://" + _ipv4address + ":5556";
        snapshot.connect(socketaddress.c_str());
        socketaddress = "tcp://" + _ipv4address + ":5557";
        subscriber.connect(socketaddress.c_str());
        socketaddress = "tcp://" + _ipv4address + ":5558";
        updater.connect(socketaddress.c_str());
    }

    std::cout << "connected..." << std::endl;


    waiting_for_snapshot.store(false);
    received_snapshot.store(false);
}

void DatarefSubscriber::Start(std::string ipv4address)
{
    _ipv4address = ipv4address;

    Init();
    t = std::thread(&DatarefSubscriber::SubscriberWorker, this);
    std::cout << "subscriber worker started..." << std::endl;
}

bool DatarefSubscriber::Ready()
{
    return (!waiting_for_snapshot.load() && received_snapshot.load());
}

void DatarefSubscriber::Finish()
{
    if (!keep_running.load()) {
        return;
    }

    std::cout << "subscriber worker finishing..." << std::endl;

    lock.lock();
    keep_running.store(false);
    lock.unlock();
    
    t.join();
    
    std::cout << "subscriber worker finished..." << std::endl;
}

bool DatarefSubscriber::GetLatestFrame(unsigned int cur_frame, panelclone::StateUpdate & stateUpdate)
{
    lock.lock();

    if (!Ready() || latest_stateUpdate.frame() <= cur_frame) {
        lock.unlock();
        return false;
    }

    stateUpdate = latest_stateUpdate;

    lock.unlock();
    return true;
}

bool DatarefSubscriber::GetLocalSnapshot(panelclone::Snapshot & txSnapshot)
{
    lock.lock();

    if (!Ready()) {
        lock.unlock();
        return false;
    }

    auto * frameSnapshot = txSnapshot.mutable_framesnapshot();

    frameSnapshot->set_frame(latest_stateUpdate.frame());

    for (const auto & [index, pubVal] : recvdPubValues) {
        
        auto drefvalue =  frameSnapshot->add_drefchanges();

        drefvalue->set_index(index);

        if (pubVal.chosenType == panelclone::DrefValue::ValueCase::kIntVal) {
            drefvalue->set_intval(std::get<int>(pubVal.value));
        } else if (pubVal.chosenType == panelclone::DrefValue::ValueCase::kFloatVal) {
            drefvalue->set_floatval(std::get<float>(pubVal.value));
        } else if (pubVal.chosenType == panelclone::DrefValue::ValueCase::kDoubleVal) {
            drefvalue->set_doubleval(std::get<double>(pubVal.value));
        } else if (pubVal.chosenType == panelclone::DrefValue::ValueCase::kByteVal) {

        }

        auto pubvalindex = frameSnapshot->add_publishedvalueindexes();

        pubvalindex->set_index(index);
        pubvalindex->set_dataref(pubVal.dataref);
        pubvalindex->set_dref_index(pubVal.dref_index);
    }

    lock.unlock();
    return true;
}

static
std::pair<std::string, int> datarefAndIndex(const std::string & dr_name)
{
    int index = -1;

    std::size_t lbracepos = dr_name.find_last_of("[");
    if (lbracepos != std::string::npos) {
        std::size_t rbracepos = dr_name.find_last_of("]");
        if (rbracepos != std::string::npos && rbracepos > lbracepos) {
            try {
                index = std::stoi(dr_name.substr(lbracepos+1, rbracepos - lbracepos -1));
            }
            catch (std::invalid_argument const& ex)
            {
                std::cout << "std::invalid_argument::what(): " << ex.what() << std::endl;
            }
            if (index >= 0) {
                return std::make_pair(dr_name.substr(0, lbracepos), index);
            }
        }
    }

    return std::make_pair(dr_name, -1);
}

bool DatarefSubscriber::FindFloatValue(PubFloatValue *dr, const std::string & dr_name) {

    auto [dataref, dref_index] = datarefAndIndex(dr_name);

    //     int index;
    // int dref_index;
    // float value;
    // return 0;

    for (auto & [index, recvdPubValue] : recvdPubValues) {
        if (recvdPubValue.dataref == dataref && recvdPubValue.dref_index == dref_index) {
            dr->index = recvdPubValue.index;
            dr->dref_index = dref_index;
            dr->value = recvdPubValue.floatValue();

            return true;
        }
    }

    return false;
}

bool DatarefSubscriber::HavePubValue(std::string dataref, int dref_index)
{
    lock.lock();
    for (auto & [index, recvdPubValue] : recvdPubValues) {
        if (recvdPubValue.dataref == dataref && recvdPubValue.dref_index == dref_index) {
            lock.unlock();
            return true;
        }
    }
    lock.unlock();
    return false;
}


void DatarefSubscriber::RequestDatarefs(std::vector<std::string>& datarefList)
{
    lock.lock();

    received_snapshot.store(false);

    panelclone::StateRequest stateRequest;

    for (const auto & dr_name : datarefList) {

        const auto & [dataref, index] = datarefAndIndex(dr_name);

        auto newRegisteredDref = stateRequest.add_drefs();
        newRegisteredDref->set_dataref(dataref);
        newRegisteredDref->set_index(index);

    }

    // Create zmq message
    std::string data;
    stateRequest.SerializeToString(&data);

    zmq::message_t request( data.length() );
    // Copy contents to zmq message
    memcpy( request.data(), data.c_str(), data.length() );
    // Send snapshot request message
    
    try {
        zmq::send_result_t res;
        
        res = snapshot.send( request, zmq::send_flags::dontwait );    
        
        if (res.has_value() && res.value() > 0) {
            std::cout << "sending snapshot request with " << stateRequest.drefs_size() << " drefs" << std::endl;
            waiting_for_snapshot.store(true);
        } else {
            std::cout << "[error] failed to send snapshot request..." << std::endl;
        }

    } catch (zmq::error_t &e) {
        std::cout << "[error] zmq error on snapshot send..." << std::endl;
    }

    lock.unlock();
}

void DatarefSubscriber::SubscriberWorker()
{
    zmq_pollitem_t items [] = {
        { subscriber, 0, ZMQ_POLLIN, 0 },
        { snapshot, 0, ZMQ_POLLIN, 0 }
    };

    while (keep_running.load()) {

        try {
        
            zmq::message_t message;
            zmq::poll (&items[0], 2, 0);
            
            if (!waiting_for_snapshot.load() && items[0].revents & ZMQ_POLLIN) {
                
                std::vector<zmq::message_t> msgs;
                auto ret = zmq::recv_multipart(subscriber, std::back_inserter(msgs));

                if (!ret) {
                    goto errout;
                }

                int i = 0;
                bool skip_next = false;

                for (auto & message : msgs) {
                
                    if (i++ % 2 == 0) {
                        if (std::strncmp(message.data<char>(), "state", 5) != 0) {
                            std::cout << "skipping topic that is not state...";
                            skip_next = true;
                        }
                        continue;
                    } else if (skip_next) {
                        skip_next = false;
                        continue;
                    }

                    
                    panelclone::StateUpdate stateUpdate;

                    stateUpdate.ParseFromArray(message.data(), message.size());

                    //  Process task

                    lock.lock();

                    latest_stateUpdate = stateUpdate;

                    auto num_changed = stateUpdate.drefchanges_size();
                    
                    if (latest_frame.load() < stateUpdate.frame()) {
                        latest_frame.store(stateUpdate.frame());
                    }

                    auto num_new_pubvals = stateUpdate.publishedvalueindexes_size();

                    for (auto i = 0; i < num_new_pubvals; i++) {
                        auto & pubvalindex = stateUpdate.publishedvalueindexes(i);

                        PubValue newPubValue;

                        // Check if have already received a value update from
                        // a future frame...
                        if (auto search = recvdPubValues.find(pubvalindex.index()); search != recvdPubValues.end()) {
                            std::cout << "WARNING: received definition after future value, for dataref " << pubvalindex.dataref() << std::endl;
                            newPubValue = search->second;
                        } else {
                            newPubValue.last_frame_updated = stateUpdate.frame();
                        }

                        newPubValue.index = pubvalindex.index();
                        newPubValue.dataref = pubvalindex.dataref();
                        newPubValue.dref_index = pubvalindex.dref_index();
                        newPubValue.chosenType = panelclone::DrefValue::ValueCase::VALUE_NOT_SET;

                        recvdPubValues[newPubValue.index] = newPubValue;
                    }

                    
                    for (auto i = 0 ; i < num_changed; i++) {
                        auto & drefValue = stateUpdate.drefchanges(i);

                        // Check if have already future frame value
                        // without having received initial definition...
                        if (auto search = recvdPubValues.find(drefValue.index()); search == recvdPubValues.end()) {
                            std::cout << "WARNING: received value before definition" << std::endl;
                            PubValue newPubValue;
                            newPubValue.index = drefValue.index();
                            newPubValue.dataref = "";
                            newPubValue.dref_index = 9999999;
                            newPubValue.chosenType = drefValue.value_case();
                            recvdPubValues[newPubValue.index] = newPubValue;
                        }

                        auto & pubvalinfo =  recvdPubValues[drefValue.index()];
                                
                        assert(pubvalinfo.chosenType == drefValue.value_case());

                        if (pubvalinfo.last_frame_updated < stateUpdate.frame()) {

                            switch (drefValue.value_case()) {
                                case panelclone::DrefValue::ValueCase::kIntVal:
                                    pubvalinfo.value = drefValue.intval();
                                    break;
                                case panelclone::DrefValue::ValueCase::kFloatVal:
                                    pubvalinfo.value = drefValue.floatval();
                                    break;
                                case panelclone::DrefValue::ValueCase::kDoubleVal:
                                    pubvalinfo.value = drefValue.doubleval();
                                    break;
                                case panelclone::DrefValue::ValueCase::kByteVal:
                                    break;
                                default:
                                    std::cout << "ERROR encountered unhandled value case!" << std::endl;
                                    break;
                            }

                            pubvalinfo.last_frame_updated = stateUpdate.frame();
                        }
                    }

                    
                    lock.unlock();
                }
            }

            /*
            {
                panelclone::PushState pushMessage;

                auto drefvalue = pushMessage.mutable_drefwrite();

                drefvalue->set_index(recvdPubValues[0].index);

                if (recvdPubValues[0].chosenType == panelclone::DrefValue::ValueCase::kIntVal) {
                    drefvalue->set_intval(1 - std::get<int>(recvdPubValues[0].value));
                } else if (recvdPubValues[0].chosenType == panelclone::DrefValue::ValueCase::kFloatVal) {
                    drefvalue->set_floatval(0);
                } else if (recvdPubValues[0].chosenType == panelclone::DrefValue::ValueCase::kDoubleVal) {
                    drefvalue->set_doubleval(0);
                } else if (recvdPubValues[0].chosenType == panelclone::DrefValue::ValueCase::kByteVal) {
                    //drefvalue->set
                } else {
                    logMsg("[ERROR] Not setting a value, unhandled chosen type!");
                }

                // Create zmq message
                std::string data;
                pushMessage.SerializeToString(&data);

                zmq::message_t request( data.length() );
                // Copy contents to zmq message
                memcpy( request.data(), data.c_str(), data.length() );
                // Send snapshot request message
                
                try {
                    updater.send( request );    
                    std::cout << "sending push message" << std::endl;
                } catch (zmq::error_t &e) {
                    std::cout << "zmq error, stopping..." << std::endl;
                    goto errout;
                }
            }
            */

            if (items[1].revents & ZMQ_POLLIN) {
                
                std::cout << "received snapshot message" << std::endl;

                if (!waiting_for_snapshot) {
                    std::cout << "WARNING received snapshot when not waiting for one" << std::endl;
                }

                std::vector<zmq::message_t> msgs;
                auto ret = zmq::recv_multipart(snapshot, std::back_inserter(msgs));

                if (!ret) {
                    goto errout;
                }

                lock.lock();

                for (auto & message : msgs) {
                    
                    //  Process task
                    panelclone::Snapshot recvdSnapshot;
                    recvdSnapshot.ParseFromArray(message.data(), message.size());
                
                    //  Process task
                    
                    latest_frame = recvdSnapshot.framesnapshot().frame();

                    auto num_new_pubvals = recvdSnapshot.framesnapshot().publishedvalueindexes_size();

                    auto num_recvd = recvdSnapshot.framesnapshot().drefchanges_size();

                    std::cout << "Received snapshot at frame " << recvdSnapshot.framesnapshot().frame() << " with " << num_recvd << " drefs" << std::endl; 

                    for (auto i = 0; i < num_new_pubvals; i++) {
                        auto & pubvalindex = recvdSnapshot.framesnapshot().publishedvalueindexes(i);

                        PubValue newPubValue;

                        newPubValue.index = pubvalindex.index();
                        newPubValue.dataref = pubvalindex.dataref();
                        newPubValue.dref_index = pubvalindex.dref_index();
                        newPubValue.chosenType = panelclone::DrefValue::ValueCase::VALUE_NOT_SET;

                        recvdPubValues[newPubValue.index] = newPubValue;
                    }

                    for (auto i = 0 ; i < num_recvd; i++) {
                        auto & drefValue = recvdSnapshot.framesnapshot().drefchanges(i);

                        auto & pubvalinfo =  recvdPubValues[drefValue.index()];
                                
                        pubvalinfo.chosenType = drefValue.value_case();

                        switch (pubvalinfo.chosenType) {
                            case panelclone::DrefValue::ValueCase::kIntVal:
                                pubvalinfo.value = drefValue.intval();
                                //std::cout << " new value for " << pubvalinfo.dataref << "[" << pubvalinfo.dref_index << "] is " << std::get<int>(pubvalinfo.value) << std::endl;
                                break;
                            case panelclone::DrefValue::ValueCase::kFloatVal:
                                pubvalinfo.value = drefValue.floatval();
                                //std::cout << " new value for " << pubvalinfo.dataref << "[" << pubvalinfo.dref_index << "] is " << std::get<float>(pubvalinfo.value) << std::endl;
                                break;
                            case panelclone::DrefValue::ValueCase::kDoubleVal:
                                pubvalinfo.value = drefValue.doubleval();
                                //std::cout << " new value for " << pubvalinfo.dataref << "[" << pubvalinfo.dref_index << "] is " << std::get<double>(pubvalinfo.value) << std::endl;
                                break;
                            case panelclone::DrefValue::ValueCase::kByteVal:
                                break;
                            default:
                                break;
                        }
                    }

                    waiting_for_snapshot.store(false);

                }

                received_snapshot.store(true);

                lock.unlock();
            }

        } catch (zmq::error_t &e) {
            std::cout << "zmq error, killing program..." << std::endl;
            goto errout;
        }

        std::this_thread::sleep_for( std::chrono::milliseconds(20) );

    }

    return;

errout:
    std::cout << "[error] subscriber worker errored out..." << std::endl;
    keep_running.store(false);
}

float DatarefSubscriber::GetFloatValue(const int index) 
{ 
    lock.unlock();
    float val = recvdPubValues[index].floatValue(); 
    lock.unlock();
    return val;
}
