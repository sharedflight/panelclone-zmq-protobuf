//
//  PanelTextureSubscriber.hpp
//
//  Created by Justin Snapp on 8/31/24.
//  Copyright © 2024 Justin Snapp. All rights reserved.
//

#ifndef PanelTextureSubscriber_hpp
#define PanelTextureSubscriber_hpp

#include <string>
#include <regex>

#include <thread>
#include <mutex>
#include <atomic>

#include <zmq.hpp>
#include <zmq_addon.hpp>

#include <event.pb.h>

#include "Pool.h"

typedef std::function<void(void*, void*, int, int)> UpdateTextureFunction;

class PanelTextureSubscriber
{
    std::mutex lock, texture_update_lock;
    
    // Create ZMQ Context
    zmq::context_t context;
    
    // Create the Subscriber socket
    zmq::socket_t subscriber;

    std::thread t;
    std::atomic<bool> keep_running;

    std::string _ipv4address;

    UpdateTextureFunction m_textureUpdater;
    std::vector<void*> m_pnlren_ptrs;

    int latest_rendered_nonce = -1;
    int vid_frames_sent = 0;

public:

    static PanelTextureSubscriber& getInstance();

    PanelTextureSubscriber();
    ~PanelTextureSubscriber();
    
    void Init();
    void Start(std::string ipv4address);
    void Finish();

    bool Ready();

    void SetUpdateTextureFunctionAndPnlRenderers(UpdateTextureFunction func, std::vector<void*> pnlrens);

private:

    void SubscriberWorker();

};

#endif /* PanelTextureSubscriber_hpp */
