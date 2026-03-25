#pragma once
#include "RFIDReader.h"
#include "../platform/SocketUtils.h"
#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

// Forward-declaration para no arrastrar ltkcpp.h a todos los que incluyan este header
struct LLRP_tSConnection; // LTKC opaque type

namespace readers {

/**
 * Cliente LLRP genérico usando sockets POSIX + LTKCPP.
 * Equivalente a readers/GenericLLRPReader.java.
 *
 * Flujo:
 *   connect()        → abre TCP al reader real
 *   startInventory() → ADD_ROSPEC + ENABLE_ROSPEC + START_ROSPEC
 *                      → hilo background leyendo RO_ACCESS_REPORT
 *   stopInventory()  → STOP_ROSPEC + join hilo
 *   disconnect()     → cierra socket
 */
class GenericLLRPReader : public RFIDReader {
public:
    GenericLLRPReader();
    ~GenericLLRPReader() override;

    void connect()    override;
    void disconnect() override;

    void startInventory() override;
    void stopInventory()  override;

    std::vector<std::string> readTags()          override;
    bool                     isInventoryRunning() const override;
    void                     configureReader()    override;

private:
    std::string host_;
    int         port_;
    long        minLogIntervalMs_;

    socket_t sockFd_;
    std::atomic<bool> inventoryRunning_;
    std::thread readerThread_;

    std::mutex                          lastLogMtx_;
    std::unordered_map<std::string, int64_t> lastLoggedMs_;

    static constexpr int DEFAULT_ROSPEC_ID = 1;

    // Helpers de mensajería
    void sendRawMessage(const uint8_t *data, size_t len);
    void sendROSpecControl(uint16_t msgType, uint32_t msgId, uint32_t rospecId);
    void sendAddROSpec();
    void sendDeleteROSpec();
    void sendEnableROSpec();
    void sendStartROSpec();
    void sendStopROSpec();

    // Lectura de frames
    std::vector<uint8_t> readSingleLLRPFrame();
    void handleReaderMessage(const std::vector<uint8_t> &frame);

    // Logging
    bool shouldLogTag(const std::string &epc);
    static std::string bytesToHex(const uint8_t *data, size_t len);

    int64_t nowMs() const;
};

} // namespace readers
