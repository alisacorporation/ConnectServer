#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <array>
#include <queue>
#include <mutex>
#include <chrono>
#include <cstdint>

constexpr size_t MAX_PACKET_SIZE = 2048;

class ClientSession : public std::enable_shared_from_this<ClientSession> {
public:
    ClientSession(boost::asio::io_context& io, int index);
    ~ClientSession();

    void start();
    void async_send(const uint8_t* data, size_t size);
    void close();

    boost::asio::ip::tcp::socket& socket() { return socket_; }
    int index() const { return index_; }
    std::string ip_address() const { return ip_address_; }
    bool is_connected() const { return connected_; }
    bool check_timeout(uint32_t timeout_seconds) const;

private:
    void start_read();
    void handle_read(const boost::system::error_code& error, size_t bytes);
    bool parse_packets();
    void process_packet(uint8_t head, const uint8_t* data, size_t size);

    void start_write();
    void handle_write(const boost::system::error_code& error, size_t bytes);

    boost::asio::ip::tcp::socket socket_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;

    std::array<uint8_t, MAX_PACKET_SIZE> recv_buffer_;
    size_t recv_buffer_size_;

    std::queue<std::vector<uint8_t>> send_queue_;
    std::mutex send_mutex_;
    bool write_in_progress_;

    int index_;
    std::string ip_address_;
    bool connected_;
    std::chrono::steady_clock::time_point connect_time_;
    std::chrono::steady_clock::time_point last_packet_time_;
};
