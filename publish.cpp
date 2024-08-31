#include <csignal>
#include <iostream>
#include <string>
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

int sequence = 0;

int main( int argc, char *argv[] )
{
    // Message to send to the subscribers
    std::string msg = "test";

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

    catch_signals();

    int i = 1;

    zmq_pollitem_t items [] = {
        { collector, 0, ZMQ_POLLIN, 0 },
        { snapshot, 0, ZMQ_POLLIN, 0 }
    };

    while (true) {
        
        try {
            
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
                std::cout << "received collector message: " << message.str() << std::endl;
            }
            if (items [1].revents & ZMQ_POLLIN) {
                snapshot.recv(message, zmq::recv_flags::none);
                //  Process weather update
                std::cout << "received snapshot message: " << message.to_string() << std::endl;
                if (message.to_string() == "ICANHAZ?") {
                    std::cout << "received snapshot request!" << std::endl;
                
                    // std::string foobar = "KTHXBAI";

                    // zmq::message_t request( foobar.length() );
                    // // Copy contents to zmq message
                    // memcpy( request.data(), foobar.c_str(), foobar.length() );
                    // // Publish the message
                    // std::array<zmq::const_buffer, 2> bufs = {
                    //   zmq::str_buffer("PEER2"),
                    //   //zmq::str_buffer(""),
                    //   zmq::buffer(foobar)
                    // };
                    // zmq::send_multipart(snapshot, bufs);
                    // std::cout << "sent " << foobar << " to PEER2.." << std::endl;

                    panelclone::Snapshot txSnapshot;

                    auto * indexTable = txSnapshot.mutable_drefindextable();
                    auto * frameSnapshot = txSnapshot.mutable_framesnapshot();

                    frameSnapshot->set_frame(i++);

                    //trodes::proto::Event event;
                    //event.set_name("myevent");
                    //event.set_origin("myorigin");

                    std::string data;
                    txSnapshot.SerializeToString(&data);

                    // Publish the message
                    std::array<zmq::const_buffer, 2> bufs = {
                      zmq::str_buffer("PEER2"),
                      zmq::buffer(data)
                    };
                    zmq::send_multipart(snapshot, bufs);

                    // send the reply to the client
                    //socket.send(zmq::buffer(data), zmq::send_flags::none);
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
    std::cout << sequence << " messages handled" << std::endl;
    //zhash_destroy (&kvmap);

    return 0;
}