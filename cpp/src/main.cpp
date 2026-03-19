#include "llrp/Config.h"
#include "platform/SocketUtils.h"
#include "reader/ReaderAdapter.h"

// LTKCPP
#include "ltkcpp.h"

// STL
#include <atomic>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <vector>

// ── Type registry global (usado también en GenericLLRPReader.cpp) ────────────
LLRP::CTypeRegistry *g_pTypeRegistry = nullptr;

// ── ReaderAdapter global (equivalente a Main.reader) ────────────────────────
static reader::ReaderAdapter *g_reader = nullptr;

static constexpr uint16_t AGENT_PORT = 5084;

namespace {

LLRP::llrp_utf8v_t toUtf8Vector(const std::string& value)
{
    LLRP::llrp_utf8v_t utf8(static_cast<unsigned int>(value.size()));
    for (size_t i = 0; i < value.size(); ++i) {
        utf8.m_pValue[i] = static_cast<LLRP::llrp_utf8_t>(value[i]);
    }
    return utf8;
}

std::vector<uint8_t> encodeMessageToFrame(const LLRP::CMessage& message)
{
    std::vector<uint8_t> buffer(32u * 1024u);
    LLRP::CFrameEncoder encoder(buffer.data(), static_cast<unsigned int>(buffer.size()));
    encoder.encodeElement(&message);
    buffer.resize(encoder.getLength());
    return buffer;
}

LLRP::CMessage* decodeMessageFromFrame(const LLRP::CTypeRegistry* typeRegistry, const std::vector<uint8_t>& frame)
{
    LLRP::CFrameDecoder decoder(typeRegistry, const_cast<unsigned char*>(frame.data()), static_cast<unsigned int>(frame.size()));
    return decoder.decodeMessage();
}

} // namespace

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
// Helpers de I/O
// ─────────────────────────────────────────────────────────────────────────────

static bool recvAll(socket_t fd, uint8_t *buf, size_t len) {
    size_t received = 0;
    while (received < len) {
        ssize_t n = ::recv(fd, reinterpret_cast<char*>(buf + received), static_cast<int>(len - received), 0);
        if (n <= 0) return false;
        received += static_cast<size_t>(n);
    }
    return true;
}

static bool sendAll(socket_t fd, const uint8_t *buf, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = ::send(fd, reinterpret_cast<const char*>(buf + sent), static_cast<int>(len - sent), 0);
        if (n <= 0) return false;
        sent += static_cast<size_t>(n);
    }
    return true;
}

// Leer un frame LLRP completo del socket cliente
static std::vector<uint8_t> readClientFrame(socket_t fd) {
    uint8_t header[10];
    if (!recvAll(fd, header, 10)) return {};

    uint32_t length =
        (static_cast<uint32_t>(header[2]) << 24) |
        (static_cast<uint32_t>(header[3]) << 16) |
        (static_cast<uint32_t>(header[4]) << 8)  |
         static_cast<uint32_t>(header[5]);

    if (length < 10 || length > 1024u * 1024u) return {};

    std::vector<uint8_t> frame(length);
    std::memcpy(frame.data(), header, 10);
    if (length > 10 && !recvAll(fd, frame.data() + 10, length - 10)) return {};
    return frame;
}

// ─────────────────────────────────────────────────────────────────────────────
// Handler de cliente (un thread por conexión)
// ─────────────────────────────────────────────────────────────────────────────

static void handleClient(socket_t clientFd) {
    std::cout << "Client connected\n";

    auto buildStatus = [](LLRP::EStatusCode code, const std::string &desc) {
        auto *s = new LLRP::CLLRPStatus();
        s->setStatusCode(code);
        s->setErrorDescription(toUtf8Vector(desc));
        return s;
    };

    auto sendMsg = [&](LLRP::CMessage *pMsg) {
        std::vector<uint8_t> frame = encodeMessageToFrame(*pMsg);
        sendAll(clientFd, frame.data(), frame.size());
        delete pMsg;
    };

    while (true) {
        auto frame = readClientFrame(clientFd);
        if (frame.empty()) {
            std::cout << "Client disconnected\n";
            break;
        }

        LLRP::CMessage *pMsg = decodeMessageFromFrame(g_pTypeRegistry, frame);
        if (!pMsg) {
            std::cerr << "Could not decode client message (" << frame.size() << " bytes)\n";
            continue;
        }

        uint32_t msgId = pMsg->getMessageID();
        std::cout << "Received: " << pMsg->m_pType->m_pName << "\n";

        if (pMsg->m_pType == &LLRP::CGET_READER_CAPABILITIES::s_typeDescriptor) {

            auto *pResp = new LLRP::CGET_READER_CAPABILITIES_RESPONSE();
            pResp->setMessageID(msgId);
            pResp->setLLRPStatus(buildStatus(LLRP::StatusCode_M_Success, "OK"));
            sendMsg(pResp);

        } else if (pMsg->m_pType == &LLRP::CADD_ROSPEC::s_typeDescriptor) {

            auto *pResp = new LLRP::CADD_ROSPEC_RESPONSE();
            pResp->setMessageID(msgId);
            pResp->setLLRPStatus(buildStatus(LLRP::StatusCode_M_Success, "OK"));
            sendMsg(pResp);

        } else if (pMsg->m_pType == &LLRP::CSTART_ROSPEC::s_typeDescriptor) {

            g_reader->startInventory();
            auto *pResp = new LLRP::CSTART_ROSPEC_RESPONSE();
            pResp->setMessageID(msgId);
            pResp->setLLRPStatus(buildStatus(LLRP::StatusCode_M_Success, "Started"));
            sendMsg(pResp);

        } else if (pMsg->m_pType == &LLRP::CSTOP_ROSPEC::s_typeDescriptor) {

            g_reader->stopInventory();
            auto *pResp = new LLRP::CSTOP_ROSPEC_RESPONSE();
            pResp->setMessageID(msgId);
            pResp->setLLRPStatus(buildStatus(LLRP::StatusCode_M_Success, "Stopped"));
            sendMsg(pResp);

        } else {
            std::cout << "Unhandled message: " << pMsg->m_pType->m_pName << "\n";
            delete pMsg;
            continue;
        }

        delete pMsg;
    }

    socket_close(clientFd);
}

// ─────────────────────────────────────────────────────────────────────────────
// main
// ─────────────────────────────────────────────────────────────────────────────

int main() {
#ifdef _WIN32
    WsaSession wsaSession;
#endif

    // 1. Cargar configuración
    llrp::Config::load();

    // 2. Inicializar type registry de LTKCPP
    g_pTypeRegistry = LLRP::getTheTypeRegistry();

    // 3. Crear ReaderAdapter (selecciona e inicia el hardware)
    reader::ReaderAdapter adapter;
    g_reader = &adapter;

    // 4. Abrir socket servidor
    socket_t serverFd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd == invalid_socket_value)
        throw std::runtime_error("socket() failed");

#ifdef _WIN32
    BOOL opt = TRUE;
    ::setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));
#else
    int opt = 1;
    ::setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(AGENT_PORT);

    if (::bind(serverFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
        throw std::runtime_error("bind() failed on port " + std::to_string(AGENT_PORT));

    if (::listen(serverFd, 10) < 0)
        throw std::runtime_error("listen() failed");

    std::cout << "LLRP Agent listening on port " << AGENT_PORT << "\n";

    // 5. Bucle principal: aceptar clientes
    while (true) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        socket_t clientFd = ::accept(serverFd, reinterpret_cast<sockaddr*>(&clientAddr), &clientLen);
        if (clientFd == invalid_socket_value) {
            std::cerr << "accept() failed\n";
            continue;
        }

        // Cada cliente en su propio thread (detached, igual que Java)
        std::thread(handleClient, clientFd).detach();
    }

    socket_close(serverFd);
    return 0;
}
