/**
 * LLRPTestClient.cpp
 * Cliente de prueba LLRP — equivalente a LLRPTestClient.java.
 *
 * Uso:
 *   ./llrp_test_client [host] [port] [durationSeconds]
 *
 * Envía START_ROSPEC, imprime respuestas durante N segundos, luego STOP_ROSPEC.
 */
#include "llrp/Config.h"
#include "platform/SocketUtils.h"

#include <csignal>

// STL
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

static std::atomic<bool> g_stopSent{false};
static socket_t g_sockFd = invalid_socket_value;

#ifdef _WIN32
struct WsaSession {
    WsaSession() {
        WSADATA wsaData{};
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw std::runtime_error("WSAStartup failed");
        }
    }

    ~WsaSession() {
        WSACleanup();
    }
};
#endif

// ─────────────────────────────────────────────────────────────────────────────
// Construcción de frames LLRP raw (igual que la versión Java)
// ─────────────────────────────────────────────────────────────────────────────

static std::vector<uint8_t> buildROSpecControl(uint16_t msgType, uint32_t msgId, uint32_t rospecId) {
    constexpr int VERSION = 1;
    std::vector<uint8_t> buf(14);
    uint16_t header = static_cast<uint16_t>((VERSION << 10) | msgType);
    uint32_t length = 14;
    buf[0]  = (header   >> 8)  & 0xFF;
    buf[1]  =  header          & 0xFF;
    buf[2]  = (length   >> 24) & 0xFF;
    buf[3]  = (length   >> 16) & 0xFF;
    buf[4]  = (length   >> 8)  & 0xFF;
    buf[5]  =  length          & 0xFF;
    buf[6]  = (msgId    >> 24) & 0xFF;
    buf[7]  = (msgId    >> 16) & 0xFF;
    buf[8]  = (msgId    >> 8)  & 0xFF;
    buf[9]  =  msgId           & 0xFF;
    buf[10] = (rospecId >> 24) & 0xFF;
    buf[11] = (rospecId >> 16) & 0xFF;
    buf[12] = (rospecId >> 8)  & 0xFF;
    buf[13] =  rospecId        & 0xFF;
    return buf;
}

static bool sendAll(socket_t fd, const uint8_t *data, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = ::send(fd, reinterpret_cast<const char*>(data + sent), static_cast<int>(len - sent), 0);
        if (n <= 0) return false;
        sent += static_cast<size_t>(n);
    }
    return true;
}

static void sendStopROSpec(const std::string &reason) {
    if (g_sockFd == invalid_socket_value || g_stopSent.exchange(true)) return;
    auto msg = buildROSpecControl(23, 2, 1); // type 23 = STOP_ROSPEC
    sendAll(g_sockFd, msg.data(), msg.size());
    std::cout << "Sent STOP_ROSPEC (" << reason << ")\n";
}

static std::string bytesToHex(const uint8_t *data, size_t len) {
    std::ostringstream oss;
    for (size_t i = 0; i < len; ++i)
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)data[i] << ' ';
    return oss.str();
}

// ─────────────────────────────────────────────────────────────────────────────
// Señal SIGINT/SIGTERM — equivalente al shutdownHook de Java
// ─────────────────────────────────────────────────────────────────────────────

static void sigHandler(int) {
    sendStopROSpec("signal received");
    if (g_sockFd != invalid_socket_value) {
        socket_close(g_sockFd);
        g_sockFd = invalid_socket_value;
    }
    std::exit(0);
}

// ─────────────────────────────────────────────────────────────────────────────
// main
// ─────────────────────────────────────────────────────────────────────────────

int main(int argc, char *argv[]) {
#ifdef _WIN32
    WsaSession wsaSession;
#endif

    llrp::Config::load();

    std::string host = "127.0.0.1";
    int port         = llrp::Config::READER_PORT;
    int duration     = 10;

    if (argc > 1) host     = argv[1];
    if (argc > 2) { try { port    = std::stoi(argv[2]); } catch (...) {} }
    if (argc > 3) { try { duration = std::max(1, std::stoi(argv[3])); } catch (...) {} }

    // Buscar --duration=N entre los argumentos
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg.rfind("--duration=", 0) == 0) {
            try { duration = std::max(1, std::stoi(arg.substr(11))); } catch (...) {}
        }
    }

    // Conectar
    struct addrinfo hints{}, *res = nullptr;
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    std::string portStr = std::to_string(port);

    if (::getaddrinfo(host.c_str(), portStr.c_str(), &hints, &res) != 0)
        throw std::runtime_error("getaddrinfo failed for " + host);

    g_sockFd = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (g_sockFd == invalid_socket_value || ::connect(g_sockFd, res->ai_addr, res->ai_addrlen) < 0) {
        ::freeaddrinfo(res);
        throw std::runtime_error("Could not connect to " + host + ":" + portStr);
    }
    ::freeaddrinfo(res);

    // Registrar manejador de señales
    ::signal(SIGINT,  sigHandler);
    ::signal(SIGTERM, sigHandler);

    // Timeout de lectura 5 s
    socket_set_recv_timeout(g_sockFd, 5000);

    std::cout << "Connected to LLRP reader at " << host << ":" << port << "\n";
    std::cout << "Listening for responses for " << duration << " seconds\n";

    // Enviar START_ROSPEC (tipo 22)
    auto startMsg = buildROSpecControl(22, 1, 1);
    sendAll(g_sockFd, startMsg.data(), startMsg.size());
    std::cout << "Sent START_ROSPEC\n";

    // Leer durante N segundos
    auto end = std::chrono::steady_clock::now() + std::chrono::seconds(duration);
    std::vector<uint8_t> buffer(4096);

    while (std::chrono::steady_clock::now() < end) {
        ssize_t len = ::recv(g_sockFd, reinterpret_cast<char*>(buffer.data()), static_cast<int>(buffer.size()), 0);
        if (len > 0) {
            std::cout << "Received response: " << len << " bytes\n";
            std::cout << bytesToHex(buffer.data(), static_cast<size_t>(len)) << "\n";
        } else if (len == 0) {
            std::cout << "Server closed connection\n";
            break;
        }
        // errno == EAGAIN → timeout de lectura → seguir
    }

    sendStopROSpec("Normal completion");
    socket_close(g_sockFd);
    g_sockFd = invalid_socket_value;
    std::cout << "Disconnected\n";
    return 0;
}
