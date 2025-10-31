#pragma once

#include <boost/asio.hpp>
#include <array>
#include <cstdint>

constexpr size_t MAX_UDP_PACKET_SIZE = 4096;

class SocketManagerUdp {
public:
    SocketManagerUdp(boost::asio::io_context& io);
    ~SocketManagerUdp();

    bool start(uint16_t port);
    void stop();
    
    void async_send(const uint8_t* data, size_t size,
                   const std::string& ip, uint16_t port);

private:
    void start_receive();
    void handle_receive(const boost::system::error_code& error, size_t bytes);
    bool parse_udp_packets(const uint8_t* data, size_t size);

    boost::asio::io_context& io_context_;
    boost::asio::ip::udp::socket socket_;
    boost::asio::ip::udp::endpoint remote_endpoint_;

    std::array<uint8_t, MAX_UDP_PACKET_SIZE> recv_buffer_;
    
    bool running_;
    uint16_t port_;
};

extern SocketManagerUdp* g_socket_manager_udp;
