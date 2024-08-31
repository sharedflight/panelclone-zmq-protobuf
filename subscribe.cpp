#define ZMQ_CPP11 1

#include <csignal>
#include <iostream>
#include <string>
#include <string.h>
#include <unistd.h>
#include "zmq/zmq.hpp"
#include "zmq/zmq_addon.hpp"

#include "event.pb.h"

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
    // Create ZMQ Context
    zmq::context_t context ( 1 );
    // Create the Updater socket
    zmq::socket_t updater ( context, ZMQ_PUSH );
    // Create the Snapshot socket
    zmq::socket_t snapshot ( context, ZMQ_DEALER);
    // Create the Subscriber socket
    zmq::socket_t subscriber ( context, ZMQ_SUB);

    //subscriber.set(ZMQ_SUBSCRIBE, "");

    snapshot.set( zmq::sockopt::routing_id, "PEER2");

    std::cout << "connecting..." << std::endl;

    // Bind to a tcp sockets
    snapshot.connect("tcp://localhost:5556");
    subscriber.connect("tcp://localhost:5557");
    updater.connect("tcp://localhost:5558");

    std::cout << "connected..." << std::endl;

    int64_t sequence = 0;
    std::string msg = "ICANHAZ?";

    // Create zmq message
    zmq::message_t request( msg.length() );
    // Copy contents to zmq message
    memcpy( request.data(), msg.c_str(), msg.length() );
    // Send snapshot request message
    
    std::cout << "will send message " << msg << std::endl;
    
    try {
        
        snapshot.send( request );    
        std::cout << "sending snapshot message " << msg << std::endl;

    } catch (zmq::error_t &e) {
        std::cout << "interrupt received, proceeding..." << std::endl;
    }
    
    catch_signals();

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
                    std::cout << "Received " << recvdSnapshot.framesnapshot().frame() << std::endl; 
                    //std::cout << "snapshot message: " << message.to_string() << std::endl;
                    // if (message.str() == "KTHXBAI") {
                    //     std::cout << "Ok received KTHXBAI" << std::endl;
                    //     break;
                    // }
                }
            }

        } catch (zmq::error_t &e) {
            std::cout << "interrupt received, proceeding..." << std::endl;
        }

        if (interrupted) {
            std::cout << "interrupt received, killing program..." << std::endl;
            break;
        }
    }

    std::cout << " Interrupted " << std::endl;
    
    return 0;

}