#define ZMQ_CPP11 1

#include <csignal>
#include <iostream>
#include <string>
#include <string.h>
#include <unistd.h>
#include "zmq/zmq.hpp"
#include "zmq/zmq_addon.hpp"

#include "event.pb.h"


    /*

    We do in the following order...

    0. We request a list of datarefs to be monitored
    1. Subscribe to get published stateupdates
    2. We request a snapshot
    3. We queue received stateupdates
    4. We receive snapshot, drop state updates prior to snapshot, apply stateupdates
       after snapshot

    5. Announce we are in sync

    6. Publish received state updates
    7. Allow for pushing of dataref value changes (Tab complete of dref? :-))

    */

enum ValueType { INTVAL, FLOATVAL, DOUBLEVAL, INTARRAYVAL, FLOATARRAYVAL, STRINGVAL, UNKNOWN };

struct RecvdPubValue
{
    size_t index;
    std::string dataref;
    int dref_index;
    std::variant<int, float, double, std::pair<std::string_view, size_t>> value;
    ValueType chosenType;
};

std::map<size_t, RecvdPubValue> recvdPubValues;

int interrupted = 0;

void signal_handler(int signal_value) { interrupted = 1; }

void catch_signals() {
  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);
  std::signal(SIGSEGV, signal_handler);
  std::signal(SIGABRT, signal_handler);
}

bool waiting_for_snapshot;

unsigned int latest_frame;

int main( int argc, char *argv[] )
{
    catch_signals();

    // Create ZMQ Context
    zmq::context_t context ( 1 );
    // Create the Updater socket
    zmq::socket_t updater ( context, ZMQ_PUSH );
    // Create the Snapshot socket
    zmq::socket_t snapshot ( context, ZMQ_DEALER);
    // Create the Subscriber socket
    zmq::socket_t subscriber ( context, ZMQ_SUB);

    subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    //snapshot.set( zmq::sockopt::routing_id, "PEER2");

    std::cout << "connecting..." << std::endl;

    // Bind to a tcp sockets
    snapshot.connect("tcp://localhost:5556");
    subscriber.connect("tcp://localhost:5557");
    updater.connect("tcp://localhost:5558");

    std::cout << "connected..." << std::endl;


    // Request a list of datarefs...
    {
        panelclone::StateRequest stateRequest;

        auto newRegisteredDref = stateRequest.add_drefs();
        newRegisteredDref->set_dataref("B742/ext_light/landing_inbd_L_sw");
        newRegisteredDref->set_index(-1);

        newRegisteredDref = stateRequest.add_drefs();
        newRegisteredDref->set_dataref("B742/ADI/roll_ind_deg");
        newRegisteredDref->set_index(0);

        newRegisteredDref = stateRequest.add_drefs();
        newRegisteredDref->set_dataref("B742/ADI/roll_ind_deg");
        newRegisteredDref->set_index(1);

        // Create zmq message
        std::string data;
        stateRequest.SerializeToString(&data);

        zmq::message_t request( data.length() );
        // Copy contents to zmq message
        memcpy( request.data(), data.c_str(), data.length() );
        // Send snapshot request message
        
        try {
            snapshot.send( request );    
            std::cout << "sending snapshot request with " << stateRequest.drefs_size() << " drefs" << std::endl;

            waiting_for_snapshot = true;

        } catch (zmq::error_t &e) {
            std::cout << "interrupt received, proceeding..." << std::endl;
        }

    }
    
    zmq_pollitem_t items [] = {
        { subscriber, 0, ZMQ_POLLIN, 0 },
        { snapshot, 0, ZMQ_POLLIN, 0 }
    };

    while (true) {
        
        try {
        
            zmq::message_t message;
            zmq::poll (&items[0], 2, 0);
            
            if (!waiting_for_snapshot && items[0].revents & ZMQ_POLLIN) {
                
                std::vector<zmq::message_t> msgs;
                auto ret = zmq::recv_multipart(subscriber, std::back_inserter(msgs));

                if (!ret) {
                    return 1;
                }

                for (auto & message : msgs) {
                
                    panelclone::StateUpdate stateUpdate;

                    stateUpdate.ParseFromArray(message.data(), message.size());

                    //  Process task

                    
                    if (latest_frame < stateUpdate.frame()) {

                        auto num_changed = stateUpdate.drefchanges_size();
                        
                        latest_frame = stateUpdate.frame();

                        std::cout << "received state update with " << num_changed << " changed drefs for frame " << latest_frame << std::endl;
                    
                        auto num_new_pubvals = stateUpdate.publishedvalueindexes_size();

                        for (auto i = 0; i < num_new_pubvals; i++) {
                            auto & pubvalindex = stateUpdate.publishedvalueindexes(i);

                            RecvdPubValue newPubValue;

                            newPubValue.index = pubvalindex.index();
                            newPubValue.dataref = pubvalindex.dataref();
                            newPubValue.dref_index = pubvalindex.dref_index();
                            newPubValue.chosenType = UNKNOWN;

                            recvdPubValues[newPubValue.index] = newPubValue;
                        }

                        
                        for (auto i = 0 ; i < num_changed; i++) {
                            auto & drefValue = stateUpdate.drefchanges(i);

                            auto & pubvalinfo =  recvdPubValues[drefValue.index()];
                                    
                            switch (drefValue.value_case()) {
                                case panelclone::DrefValue::ValueCase::kIntVal:
                                    pubvalinfo.value = drefValue.intval();
                                    std::cout << " new value for " << pubvalinfo.dataref << "[" << pubvalinfo.dref_index << "] is " << std::get<int>(pubvalinfo.value) << std::endl;
                                    break;
                                case panelclone::DrefValue::ValueCase::kFloatVal:
                                    pubvalinfo.value = drefValue.floatval();
                                    std::cout << " new value for " << pubvalinfo.dataref << "[" << pubvalinfo.dref_index << "] is " << std::get<float>(pubvalinfo.value) << std::endl;
                                    break;
                                // case panelclone::DrefValue::ValueCase::kIntArrayVal:
                                //     break;
                                // case panelclone::DrefValue::ValueCase::kFloatArrayVal:
                                //     break;
                                case panelclone::DrefValue::ValueCase::kDoubleVal:
                                    pubvalinfo.value = drefValue.doubleval();
                                    std::cout << " new value for " << pubvalinfo.dataref << "[" << pubvalinfo.dref_index << "] is " << std::get<double>(pubvalinfo.value) << std::endl;
                                    break;
                                case panelclone::DrefValue::ValueCase::kByteVal:
                                    break;
                                default:
                                    std::cout << "ERROR encountered unhandled value case!" << std::endl;
                                    break;
                            }
                        }

                    } else {
                       std::cout << "ignoring state update for frame " << stateUpdate.frame() << " older than current latest frame " << latest_frame << std::endl;
                    }
                }
            }

            {
                panelclone::PushState pushMessage;

                auto drefvalue = pushMessage.mutable_drefwrite();

                drefvalue->set_index(recvdPubValues[0].index);

                if (recvdPubValues[0].chosenType == INTVAL) {
                    drefvalue->set_intval(1 - std::get<int>(recvdPubValues[0].value));
                } else if (recvdPubValues[0].chosenType == FLOATVAL) {
                    drefvalue->set_floatval(0);
                } else if (recvdPubValues[0].chosenType == DOUBLEVAL) {
                    drefvalue->set_doubleval(0);
                } else if (recvdPubValues[0].chosenType == STRINGVAL) {

                } else {
                    std::cout << "[ERROR] Not setting a value, unhandled chosen type!";
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
                    std::cout << "interrupt received, proceeding..." << std::endl;
                }
            }

            if (items[1].revents & ZMQ_POLLIN) {
                
                std::cout << "received snapshot message" << std::endl;

                if (!waiting_for_snapshot) {
                    std::cout << "WARNING received snapshot when not waiting for one" << std::endl;
                }

                std::vector<zmq::message_t> msgs;
                auto ret = zmq::recv_multipart(snapshot, std::back_inserter(msgs));

                if (!ret) {
                    return 1;
                }

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

                        RecvdPubValue newPubValue;

                        newPubValue.index = pubvalindex.index();
                        newPubValue.dataref = pubvalindex.dataref();
                        newPubValue.dref_index = pubvalindex.dref_index();
                        newPubValue.chosenType = UNKNOWN;

                        recvdPubValues[newPubValue.index] = newPubValue;
                    }

                    for (auto i = 0 ; i < num_recvd; i++) {
                        auto & drefValue = recvdSnapshot.framesnapshot().drefchanges(i);

                        auto & pubvalinfo =  recvdPubValues[drefValue.index()];
                                
                        switch (drefValue.value_case()) {
                            case panelclone::DrefValue::ValueCase::kIntVal:
                                pubvalinfo.value = drefValue.intval();
                                std::cout << " new value for " << pubvalinfo.dataref << "[" << pubvalinfo.dref_index << "] is " << std::get<int>(pubvalinfo.value) << std::endl;
                                break;
                            case panelclone::DrefValue::ValueCase::kFloatVal:
                                pubvalinfo.value = drefValue.floatval();
                                std::cout << " new value for " << pubvalinfo.dataref << "[" << pubvalinfo.dref_index << "] is " << std::get<float>(pubvalinfo.value) << std::endl;
                                break;
                            // case panelclone::DrefValue::ValueCase::kIntArrayVal:
                            //     break;
                            // case panelclone::DrefValue::ValueCase::kFloatAdseVal:
                            //     break;
                            case panelclone::DrefValue::ValueCase::kDoubleVal:
                                pubvalinfo.value = drefValue.doubleval();
                                std::cout << " new value for " << pubvalinfo.dataref << "[" << pubvalinfo.dref_index << "] is " << std::get<double>(pubvalinfo.value) << std::endl;
                                break;
                            case panelclone::DrefValue::ValueCase::kByteVal:
                                break;
                            default:
                                break;
                        }
                    }

                    waiting_for_snapshot = false;

                    //////////////////
                }
            }

        } catch (zmq::error_t &e) {
            std::cout << "interrupt received, killing program..." << std::endl;
            interrupted = 1;
        }

        if (interrupted) {
            std::cout << "interrupt received, killing program..." << std::endl;
            break;
        }
    }

    std::cout << " Interrupted " << std::endl;
    
    return 0;

}