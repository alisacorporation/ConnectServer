#include "SocketManagerUdp.h"
#include "ProtocolDefines.h"
#include "ServerList.h"
#include "Console.h"
#include "Util.h"
#include <iostream>

SocketManagerUdp* g_socket_manager_udp = nullptr;

SocketManagerUdp::SocketManagerUdp(boost::asio::io_context& io)
    : io_context_(io)
    , socket_(io)
    , running_(false)
    , port_(0)
{
}

SocketManagerUdp::~SocketManagerUdp() {
    stop();
}

bool SocketManagerUdp::start(uint16_t port) {
    try {
        port_ = port;
        
        boost::asio::ip::udp::endpoint endpoint(boost::asio::ip::udp::v4(), port);
        
        socket_.open(endpoint.protocol());
        socket_.bind(endpoint);
        
        running_ = true;
        
        LogAdd(2, "[SocketManagerUdp] UDP server started on port %d", port);
        
        start_receive();
        
        return true;
        
    } catch (const std::exception& e) {
        LogAdd(1, "[SocketManagerUdp] Failed to start UDP server: %s", e.what());
        return false;
    }
}

void SocketManagerUdp::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    boost::system::error_code ec;
    socket_.close(ec);
    
    LogAdd(2, "[SocketManagerUdp] UDP server stopped");
}

void SocketManagerUdp::start_receive() {
    if (!running_) {
        return;
    }
    
    socket_.async_receive_from(
        boost::asio::buffer(recv_buffer_.data(), recv_buffer_.size()),
        remote_endpoint_,
        [this](const boost::system::error_code& error, size_t bytes) {
            handle_receive(error, bytes);
        });
}

void SocketManagerUdp::handle_receive(const boost::system::error_code& error, size_t bytes) {
    if (error) {
        if (error != boost::asio::error::operation_aborted) {
            LogAdd(1, "[SocketManagerUdp] Receive error: %s", error.message().c_str());
        }
        if (running_) {
            start_receive();
        }
        return;
    }
    
    if (bytes > 0) {
        std::string remote_ip = remote_endpoint_.address().to_string();
        uint16_t remote_port = remote_endpoint_.port();
        
        LogAdd(2, "[SocketManagerUdp] Received %zu bytes from %s:%d",
               bytes, remote_ip.c_str(), remote_port);
        
        // Parse and process UDP packets
        parse_udp_packets(recv_buffer_.data(), bytes);
    }
    
    // Continue receiving
    start_receive();
}

bool SocketManagerUdp::parse_udp_packets(const uint8_t* data, size_t size) {
    if (size < 3) {
        return false;
    }
    
    uint8_t header = data[0];
    int packet_size = 0;
    int header_size = 0;
    
    // Determine packet type and size
    if (header == 0xC1 || header == 0xC3) {
        packet_size = data[1];
        header_size = 2;
    } else if (header == 0xC2 || header == 0xC4) {
        if (size < 3) {
            return false;
        }
        packet_size = MAKEWORD(data[2], data[1]);
        header_size = 3;
    } else {
        LogAdd(1, "[SocketManagerUdp] Invalid packet header: 0x%02X", header);
        return false;
    }
    
    // Validate packet size
    if (packet_size < header_size || packet_size > static_cast<int>(size)) {
        LogAdd(1, "[SocketManagerUdp] Invalid packet size: %d", packet_size);
        return false;
    }
    
    uint8_t head = data[header_size];
    
    // Process UDP packets (GameServer/JoinServer heartbeats)
    gServerList.ServerProtocolCore(head, (uint8_t*)data, packet_size);
    
    return true;
}

void SocketManagerUdp::async_send(const uint8_t* data, size_t size,
                                  const std::string& ip, uint16_t port) {
    if (!running_ || size == 0 || size > MAX_UDP_PACKET_SIZE) {
        return;
    }
    
    try {
        auto addr = boost::asio::ip::make_address(ip);
        boost::asio::ip::udp::endpoint endpoint(addr, port);
        
        std::vector<uint8_t> packet(data, data + size);
        
        socket_.async_send_to(
            boost::asio::buffer(packet.data(), packet.size()),
            endpoint,
            [packet](const boost::system::error_code& error, size_t bytes) {
                if (error) {
                    LogAdd(1, "[SocketManagerUdp] Send error: %s", error.message().c_str());
                }
            });
            
    } catch (const std::exception& e) {
        LogAdd(1, "[SocketManagerUdp] Error sending packet: %s", e.what());
    }
}
