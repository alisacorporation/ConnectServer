#pragma once

#include <boost/asio.hpp>
#include <vector>
#include <memory>
#include <mutex>
#include <cstdint>
#include "ClientSession.h"

constexpr int MAX_CLIENT = 10000;

class SocketManager {
public:
    SocketManager(boost::asio::io_context& io);
    ~SocketManager();

    bool start(uint16_t port);
    void stop();
    
    std::shared_ptr<ClientSession> get_session(int index);
    int get_active_count() const;
    uint32_t get_queue_size() const;

private:
    void start_accept();
    void handle_accept(std::shared_ptr<ClientSession> session,
                      const boost::system::error_code& error);
    
    int find_free_index();
    bool check_ip_limit(const std::string& ip);

    boost::asio::io_context& io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    
    std::vector<std::shared_ptr<ClientSession>> sessions_;
    mutable std::mutex sessions_mutex_;
    
    bool running_;
    uint16_t port_;
};

extern SocketManager* g_socket_manager;
