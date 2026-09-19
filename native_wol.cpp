#include "native_wol.hpp"

#include <array>
#include <cerrno>
#include <cstring>
#include <sstream>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace doof_wol {
namespace {

bool parse_mac(const std::string& text, std::array<uint8_t, 6>& mac) {
    if (text.size() != 17) return false;
    for (size_t index = 0; index < mac.size(); ++index) {
        const size_t offset = index * 3;
        if (index != 0 && (text[offset - 1] != ':' && text[offset - 1] != '-')) return false;
        unsigned int value = 0;
        std::istringstream part(text.substr(offset, 2));
        part >> std::hex >> value;
        if (part.fail() || !part.eof() || value > 255) return false;
        mac[index] = static_cast<uint8_t>(value);
    }
    return true;
}

std::shared_ptr<std::vector<uint8_t>> build_packet(const std::array<uint8_t, 6>& mac) {
    auto packet = std::make_shared<std::vector<uint8_t>>(102, 0xff);
    for (size_t repetition = 0; repetition < 16; ++repetition) {
        std::memcpy(packet->data() + 6 + repetition * mac.size(), mac.data(), mac.size());
    }
    return packet;
}

#if defined(_WIN32)
class Winsock {
public:
    Winsock() : ready_(WSAStartup(MAKEWORD(2, 2), &data_) == 0) {}
    ~Winsock() { if (ready_) WSACleanup(); }
    bool ready() const { return ready_; }
private:
    WSADATA data_ {};
    bool ready_;
};
using Socket = SOCKET;
constexpr Socket kInvalidSocket = INVALID_SOCKET;
void close_socket(Socket socket) { closesocket(socket); }
std::string socket_error() { return "socket error " + std::to_string(WSAGetLastError()); }
#else
using Socket = int;
constexpr Socket kInvalidSocket = -1;
void close_socket(Socket socket) { close(socket); }
std::string socket_error() { return std::strerror(errno); }
#endif

} // namespace

doof::Result<std::shared_ptr<std::vector<uint8_t>>, std::string> magic_packet(const std::string& macAddress) {
    std::array<uint8_t, 6> mac {};
    if (!parse_mac(macAddress, mac)) return { doof::Failure<std::string> { "invalid MAC address: " + macAddress } };
    return { doof::Success<std::shared_ptr<std::vector<uint8_t>>> { build_packet(mac) } };
}

doof::Result<void, std::string> wake(const std::string& macAddress, const std::string& broadcastAddress, int32_t port) {
    if (port < 1 || port > 65535) return { doof::Failure<std::string> { "port must be between 1 and 65535" } };
    auto packet = magic_packet(macAddress);
    if (auto failure = std::get_if<doof::Failure<std::string>>(&packet)) return { doof::Failure<std::string> { failure->error } };
    auto bytes = std::get<doof::Success<std::shared_ptr<std::vector<uint8_t>>>>(packet).value;

#if defined(_WIN32)
    Winsock winsock;
    if (!winsock.ready()) return { doof::Failure<std::string> { "could not initialize Winsock" } };
#endif
    Socket socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket == kInvalidSocket) return { doof::Failure<std::string> { "could not create UDP socket: " + socket_error() } };
    int enabled = 1;
    if (setsockopt(socket, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<const char*>(&enabled), sizeof(enabled)) != 0) {
        close_socket(socket);
        return { doof::Failure<std::string> { "could not enable broadcasts: " + socket_error() } };
    }
    sockaddr_in destination {};
    destination.sin_family = AF_INET;
    destination.sin_port = htons(static_cast<uint16_t>(port));
    if (inet_pton(AF_INET, broadcastAddress.c_str(), &destination.sin_addr) != 1) {
        close_socket(socket);
        return { doof::Failure<std::string> { "invalid IPv4 broadcast address: " + broadcastAddress } };
    }
    const int sent = sendto(socket, reinterpret_cast<const char*>(bytes->data()), static_cast<int>(bytes->size()), 0,
        reinterpret_cast<const sockaddr*>(&destination), sizeof(destination));
    close_socket(socket);
    if (sent != static_cast<int>(bytes->size())) return { doof::Failure<std::string> { "could not send magic packet: " + socket_error() } };
    return { doof::Success<void> {} };
}

} // namespace doof_wol
