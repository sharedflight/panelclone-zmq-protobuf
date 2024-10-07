//
//  PanelTextureSubscriber.cpp
//
//  Created by Justin Snapp on 8/31/24.
//  Copyright © 2024 Justin Snapp. All rights reserved.
//

#include "PanelTextureSubscriber.hpp"

//#include "base64.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

//#include "FFMPEGDecoder.hpp"

#include <chrono>

PanelTextureSubscriber& PanelTextureSubscriber::getInstance() {
    static PanelTextureSubscriber instance;   // Guaranteed to be destroyed.
                                         // Instantiated on first use.
    return instance;
}

PanelTextureSubscriber::PanelTextureSubscriber()
{
    
}

PanelTextureSubscriber::~PanelTextureSubscriber()
{
    
}

void PanelTextureSubscriber::Init()
{
    keep_running.store(true);
    
    // Create ZMQ Context
    context = zmq::context_t( 1 );
    
    // Create the Subscriber socket
    subscriber = zmq::socket_t( context, ZMQ_SUB);

    subscriber.set(zmq::sockopt::subscribe, "pnl");
    
    subscriber.set(zmq::sockopt::rcvhwm, 3);

    std::cout << "connecting to panel texture publisher..." << std::endl;

    std::string socketaddress;

    // Bind to a tcp sockets
    socketaddress = "tcp://" + _ipv4address + ":5562";
    subscriber.connect(socketaddress.c_str());
}

void PanelTextureSubscriber::Start(std::string ipv4address)
{
    _ipv4address = ipv4address;

    Init();

    t = std::thread(&PanelTextureSubscriber::SubscriberWorker, this);
    std::cout << "panel texture subscriber worker started..." << std::endl;
}

bool PanelTextureSubscriber::Ready()
{
    return (true);
}

void PanelTextureSubscriber::Finish()
{
    if (!keep_running.load()) {
        return;
    }

    std::cout << "panel texture subscriber worker finishing..." << std::endl;

    lock.lock();
    keep_running.store(false);
    lock.unlock();
    
    t.join();
    
    std::cout << "panel texture subscriber worker finished..." << std::endl;
}

void PanelTextureSubscriber::SubscriberWorker()
{
    Thread::Pool decoderPool(4);

    int FRAME_TEST = 20;
    std::chrono::high_resolution_clock::time_point beg, end;
    
    //cv::setNumThreads(8);


    zmq_pollitem_t items [] = {
        { subscriber, 0, ZMQ_POLLIN, 0 },
    };

    int lastNonce = -1;

    while (keep_running.load()) {

        try {
        
            zmq::message_t message;
            zmq::poll (&items[0], 1, 0);
            
            if (items[0].revents & ZMQ_POLLIN) {
                
                std::vector<zmq::message_t> msgs;
                auto ret = zmq::recv_multipart(subscriber, std::back_inserter(msgs));

                if (!ret) {
                    goto errout;
                }

                int i = 0;
                bool skip_to_next = false;

                for (auto & message : msgs) {
                
                    auto msg_field = i++ % 3;

                    //std::cout << "field: " << msg_field << "  | msg: " << std::string(message.data<char>()) << std::endl;

                    if (msg_field == 0) {
                        if (std::strncmp(message.data<char>(), "pnl", 3) != 0) {
                            std::cout << "skipping topic that is not pnl texture...";
                            skip_to_next = true;
                        } else {
                            skip_to_next = false;
                        }
                        continue;
                    } else if (!skip_to_next) {
                        if (msg_field == 1) {
                            int rcvdNonce = std::stoi(std::string(message.data<char>()));
                            
                            if (rcvdNonce < latest_rendered_nonce - 500 && rcvdNonce > 1) {
                                //Reset or wrap?
                                lastNonce = rcvdNonce - 1;
                                latest_rendered_nonce = rcvdNonce -1;
                            }
                            if (rcvdNonce >= lastNonce) {
                                lastNonce = rcvdNonce;
                            } else {
                                skip_to_next = true;
                            }
                            continue;
                        } else { // msg_field == 2...
                            // We process below...
                        }
                    } else {
                        continue;
                    }

                    if (vid_frames_sent == 0) {
                        beg = std::chrono::high_resolution_clock::now();
                    }

                    //  Process task

                    assert(msg_field == 2);

                    //std::cout << "received nonce is " << lastNonce << std::endl;

                    if (decoderPool.PendingTasksSize() > 4) {
                        continue;
                    }

                    int thisNonce = lastNonce;

                    lock.lock();

                    auto v = new std::vector<unsigned char>(message.data<char>(), message.data<char>()+message.size());
                        
                    auto lambda = [v, thisNonce, this]() {
                        
                        cv::Mat img = cv::imdecode(cv::Mat(1, v->size(), CV_8UC1, v->data()),  cv::IMREAD_UNCHANGED);
                        
                        delete v;
                        
                        if (!img.cols) {
                            std::cout << "error: img has no size!" << std::endl;
                            return;
                        }
                        
                        //cv::Mat img2;

                        if (thisNonce <= latest_rendered_nonce) {
                            lock.unlock();
                            return;
                        }
                        
                        //cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
                        //cv::resize(img, img, cv::Size(2048,2048), cv::INTER_NEAREST);  

                        if(m_textureUpdater) {
                            texture_update_lock.lock();
                            if (thisNonce > latest_rendered_nonce) {
                                for (auto pnlren_ptr : m_pnlren_ptrs) {
                                    //std::cout << "will send texture update..." << std::endl;
                                    m_textureUpdater(pnlren_ptr, img.data, img.cols, img.rows);
                                }
                                latest_rendered_nonce = thisNonce;
                            }
                            texture_update_lock.unlock();
                        }
                    };

                    decoderPool.Add_Simple_Task(lambda);

                    vid_frames_sent++;
                    
                    if (vid_frames_sent == FRAME_TEST) {
                        end = std::chrono::high_resolution_clock::now();
                        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - beg);
                        
                        beg = end;
                        vid_frames_sent = 0;

                        // Displaying the elapsed time
                        std::cout << "FPS for " << FRAME_TEST << " frames: " << FRAME_TEST * 1000 / duration.count() << std::endl;
                    }

                    lock.unlock();

                    std::this_thread::sleep_for( std::chrono::milliseconds(10) );
                    
                }

            }

        } catch (zmq::error_t &e) {
            std::cout << "zmq error, killing program..." << std::endl;
            goto errout;
        }

        std::this_thread::sleep_for( std::chrono::milliseconds(20) );

    }

    return;

errout:
    std::cout << "[error] panel texutre subscriber worker errored out..." << std::endl;
    keep_running.store(false);
}

void PanelTextureSubscriber::SetUpdateTextureFunctionAndPnlRenderers(UpdateTextureFunction func, std::vector<void*> pnlrens)
{
    m_textureUpdater = func;
    m_pnlren_ptrs = pnlrens;
}
