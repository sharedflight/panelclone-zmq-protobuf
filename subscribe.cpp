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

int interrupted = 0;

void signal_handler(int signal_value) { interrupted = 1; }

void catch_signals() {
  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);
  std::signal(SIGSEGV, signal_handler);
  std::signal(SIGABRT, signal_handler);
}


 
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

    //subscriber.set(ZMQ_SUBSCRIBE, "");

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
        newRegisteredDref->set_index(-1u);

        newRegisteredDref = stateRequest.add_drefs();
        newRegisteredDref->set_dataref("sim/time/to/party");
        newRegisteredDref->set_index(2);

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
            
            if (items[0].revents & ZMQ_POLLIN) {
                subscriber.recv(message, zmq::recv_flags::none);
                //  Process task
                std::cout << "received pub message: " << message.to_string() << std::endl;
            }

            if (items[1].revents  & ZMQ_POLLIN) {
                
                std::cout << "received snapshot message" << std::endl;

                std::vector<zmq::message_t> msgs;
                auto ret = zmq::recv_multipart(snapshot, std::back_inserter(msgs));

                if (!ret) {
                    return 1;
                }

                std::cout << "size of msgs is " << msgs.size() << std::endl;

                for (auto & message : msgs) {
                    
                    std::cout << "Here..." << std::endl;

                    //  Process task
                    panelclone::Snapshot recvdSnapshot;
                    recvdSnapshot.ParseFromArray(message.data(), message.size());
                    std::cout << "Received snapshot at frame " << recvdSnapshot.framesnapshot().frame() << " with " << recvdSnapshot.framesnapshot().drefchanges_size() << " drefs" << std::endl; 
                    //std::cout << "snapshot message: " << message.to_string() << std::endl;
                    // if (message.str() == "KTHXBAI") {
                    //     std::cout << "Ok received KTHXBAI" << std::endl;
                    //     break;
                    // }
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