#include "ClientSession.h"
#include "ProtocolDefines.h"
#include "ConnectServerProtocol.h"
#include "Console.h"
#include "IpManager.h"
#include "Util.h"
#include <iostream>

// Forward declaration
void CCServerInitSend(int index, int result);

ClientSession::ClientSession(boost::asio::io_context& io, int index)
    : socket_(io)
    , strand_(boost::asio::make_strand(io))
    , recv_buffer_size_(0)
    , write_in_progress_(false)
    , index_(index)
    , connected_(false)
{
}

ClientSession::~ClientSession() {
    close();
}

void ClientSession::start() {
    try {
        // Get remote endpoint info
        auto endpoint = socket_.remote_endpoint();
        ip_address_ = endpoint.address().to_string();
        connected_ = true;
        
        // Set timestamps
        connect_time_ = std::chrono::steady_clock::now();
        last_packet_time_ = connect_time_;
        
        LogAdd(2, "[ClientSession] Client connected: Index=%d, IP=%s", 
               index_, ip_address_.c_str());
        
        // Send init packet to client
        CCServerInitSend(index_, 1);
        
        // Start reading
        start_read();
        
    } catch (const std::exception& e) {
        LogAdd(1, "[ClientSession] Error starting session: %s", e.what());
        close();
    }
}

void ClientSession::start_read() {
    if (!connected_) {
        return;
    }
    
    auto self = shared_from_this();
    
    socket_.async_read_some(
        boost::asio::buffer(recv_buffer_.data() + recv_buffer_size_, 
                           recv_buffer_.size() - recv_buffer_size_),
        boost::asio::bind_executor(strand_,
            [this, self](const boost::system::error_code& error, size_t bytes) {
                handle_read(error, bytes);
            })
    );
}

void ClientSession::handle_read(const boost::system::error_code& error, size_t bytes) {
    if (error) {
        if (error != boost::asio::error::eof && 
            error != boost::asio::error::operation_aborted) {
            LogAdd(1, "[ClientSession] Read error: Index=%d, Error=%s", 
                   index_, error.message().c_str());
        }
        close();
        return;
    }
    
    if (bytes == 0) {
        close();
        return;
    }
    
    recv_buffer_size_ += bytes;
    last_packet_time_ = std::chrono::steady_clock::now();
    
    // Parse and process packets
    if (parse_packets()) {
        // Continue reading
        start_read();
    } else {
        // Parse error - disconnect
        LogAdd(1, "[ClientSession] Packet parse error: Index=%d", index_);
        close();
    }
}

bool ClientSession::parse_packets() {
    size_t processed = 0;
    
    while (recv_buffer_size_ - processed >= 3) {
        uint8_t* buffer = recv_buffer_.data() + processed;
        size_t remaining = recv_buffer_size_ - processed;
        
        uint8_t header = buffer[0];
        int packet_size = 0;
        int header_size = 0;
        
        // Determine packet type and size
        if (header == 0xC1 || header == 0xC3) {
            // Small packet
            packet_size = buffer[1];
            header_size = 2;
        } else if (header == 0xC2 || header == 0xC4) {
            // Large packet
            if (remaining < 3) {
                break; // Need more data
            }
            packet_size = MAKEWORD(buffer[2], buffer[1]);
            header_size = 3;
        } else {
            // Invalid header
            LogAdd(1, "[ClientSession] Invalid packet header: 0x%02X", header);
            return false;
        }
        
        // Validate packet size
        if (packet_size < header_size || packet_size > MAX_PACKET_SIZE) {
            LogAdd(1, "[ClientSession] Invalid packet size: %d", packet_size);
            return false;
        }
        
        // Check if we have the complete packet
        if (remaining < static_cast<size_t>(packet_size)) {
            break; // Need more data
        }
        
        // Extract packet head
        uint8_t head = buffer[header_size];
        
        // Log packet if enabled
        ConsoleProtocolLog(CON_PROTO_TCP_RECV, buffer, packet_size);
        
        // Process packet
        process_packet(head, buffer, packet_size);
        
        processed += packet_size;
    }
    
    // Move remaining data to beginning of buffer
    if (processed > 0) {
        if (processed < recv_buffer_size_) {
            std::memmove(recv_buffer_.data(), 
                        recv_buffer_.data() + processed,
                        recv_buffer_size_ - processed);
        }
        recv_buffer_size_ -= processed;
    }
    
    return true;
}

void ClientSession::process_packet(uint8_t head, const uint8_t* data, size_t size) {
    // Call protocol handler
    ConnectServerProtocolCore(index_, head, data, size);
}

void ClientSession::async_send(const uint8_t* data, size_t size) {
    if (!connected_ || size == 0 || size > MAX_PACKET_SIZE) {
        return;
    }
    
    std::vector<uint8_t> packet(data, data + size);
    
    {
        std::lock_guard<std::mutex> lock(send_mutex_);
        send_queue_.push(std::move(packet));
    }
    
    // Log packet if enabled
    ConsoleProtocolLog(CON_PROTO_TCP_SEND, data, size);
    
    // Start write if not already in progress
    auto self = shared_from_this();
    boost::asio::post(strand_, [this, self]() {
        if (!write_in_progress_) {
            start_write();
        }
    });
}

void ClientSession::start_write() {
    std::lock_guard<std::mutex> lock(send_mutex_);
    
    if (send_queue_.empty() || !connected_) {
        write_in_progress_ = false;
        return;
    }
    
    write_in_progress_ = true;
    auto& packet = send_queue_.front();
    
    auto self = shared_from_this();
    
    boost::asio::async_write(
        socket_,
        boost::asio::buffer(packet.data(), packet.size()),
        boost::asio::bind_executor(strand_,
            [this, self](const boost::system::error_code& error, size_t bytes) {
                handle_write(error, bytes);
            })
    );
}

void ClientSession::handle_write(const boost::system::error_code& error, size_t bytes) {
    if (error) {
        LogAdd(1, "[ClientSession] Write error: Index=%d, Error=%s", 
               index_, error.message().c_str());
        close();
        return;
    }
    
    {
        std::lock_guard<std::mutex> lock(send_mutex_);
        send_queue_.pop();
    }
    
    // Continue writing if there are more packets
    start_write();
}

void ClientSession::close() {
    if (!connected_) {
        return;
    }
    
    connected_ = false;
    
    // Remove IP tracking
    if (!ip_address_.empty()) {
        gIpManager.RemoveIpAddress(ip_address_.c_str());
    }
    
    // Decrement client count
    gClientCount--;
    
    boost::system::error_code ec;
    socket_.close(ec);
    
    LogAdd(2, "[ClientSession] Client disconnected: Index=%d, IP=%s", 
           index_, ip_address_.c_str());
}

bool ClientSession::check_timeout(uint32_t timeout_seconds) const {
    if (!connected_) {
        return false;
    }
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        now - last_packet_time_).count();
    
    return elapsed >= timeout_seconds;
}
