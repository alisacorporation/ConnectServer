#include "SocketManager.h"
#include "IpManager.h"
#include "Util.h"
#include <iostream>

SocketManager* g_socket_manager = nullptr;

SocketManager::SocketManager(boost::asio::io_context& io)
    : io_context_(io)
    , acceptor_(io)
    , running_(false)
    , port_(0)
{
    sessions_.resize(MAX_CLIENT);
}

SocketManager::~SocketManager() {
    stop();
}

bool SocketManager::start(uint16_t port) {
    try {
        port_ = port;
        
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
        
        acceptor_.open(endpoint.protocol());
        acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        acceptor_.bind(endpoint);
        acceptor_.listen();
        
        running_ = true;
        
        LogAdd(2, "[SocketManager] TCP server started on port %d", port);
        
        start_accept();
        
        return true;
        
    } catch (const std::exception& e) {
        LogAdd(1, "[SocketManager] Failed to start TCP server: %s", e.what());
        return false;
    }
}

void SocketManager::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    boost::system::error_code ec;
    acceptor_.close(ec);
    
    // Close all sessions
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    for (auto& session : sessions_) {
        if (session) {
            session->close();
        }
    }
    
    LogAdd(2, "[SocketManager] TCP server stopped");
}

void SocketManager::start_accept() {
    if (!running_) {
        return;
    }
    
    int index = find_free_index();
    if (index == -1) {
        LogAdd(1, "[SocketManager] No free client slots available");
        
        // Try again after a short delay
        auto timer = std::make_shared<boost::asio::steady_timer>(io_context_);
        timer->expires_after(std::chrono::milliseconds(100));
        timer->async_wait([this, timer](const boost::system::error_code&) {
            start_accept();
        });
        return;
    }
    
    auto session = std::make_shared<ClientSession>(io_context_, index);
    
    acceptor_.async_accept(session->socket(),
        [this, session, index](const boost::system::error_code& error) {
            handle_accept(session, error);
        });
}

void SocketManager::handle_accept(std::shared_ptr<ClientSession> session,
                                  const boost::system::error_code& error) {
    if (error) {
        if (error != boost::asio::error::operation_aborted) {
            LogAdd(1, "[SocketManager] Accept error: %s", error.message().c_str());
        }
        start_accept();
        return;
    }
    
    try {
        // Get client IP
        std::string ip = session->ip_address();
        if (ip.empty()) {
            ip = session->socket().remote_endpoint().address().to_string();
        }
        
        // Check IP connection limit
        if (!check_ip_limit(ip)) {
            LogAdd(1, "[SocketManager] IP connection limit exceeded: %s", ip.c_str());
            session->close();
            start_accept();
            return;
        }
        
        // Store session
        {
            std::lock_guard<std::mutex> lock(sessions_mutex_);
            sessions_[session->index()] = session;
            gClientCount++;
        }
        
        // Track IP
        gIpManager.InsertIpAddress(ip.c_str());
        
        // Start session
        session->start();
        
        LogAdd(2, "[SocketManager] Client accepted: Index=%d, IP=%s, Total=%d",
               session->index(), ip.c_str(), gClientCount);
        
    } catch (const std::exception& e) {
        LogAdd(1, "[SocketManager] Error handling accept: %s", e.what());
    }
    
    // Accept next connection
    start_accept();
}

int SocketManager::find_free_index() {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    
    for (int i = 0; i < MAX_CLIENT; i++) {
        if (!sessions_[i] || !sessions_[i]->is_connected()) {
            return i;
        }
    }
    
    return -1;
}

bool SocketManager::check_ip_limit(const std::string& ip) {
    if (MaxIpConnection == 0) {
        return true;  // No limit
    }
    
    return gIpManager.CheckIpAddress(ip.c_str());
}

std::shared_ptr<ClientSession> SocketManager::get_session(int index) {
    if (index < 0 || index >= MAX_CLIENT) {
        return nullptr;
    }
    
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    return sessions_[index];
}

int SocketManager::get_active_count() const {
    return gClientCount;
}

uint32_t SocketManager::get_queue_size() const {
    // TODO: Implement queue size tracking
    return 0;
}
