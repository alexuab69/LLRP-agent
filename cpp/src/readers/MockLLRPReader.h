#pragma once

#include "RFIDReader.h"
#include <atomic>
#include <thread>
#include <vector>
#include <string>

namespace readers {

/**
 * MockLLRPReader — Simulador de lecturas RFID para testing
 * Genera tags ficticios con valores realistas de RSSI, antena, timestamp, etc.
 * Produce eventos JSON completos de inventario en rfid_inventory_events.jsonl
 */
class MockLLRPReader : public RFIDReader {
public:
    MockLLRPReader();
    ~MockLLRPReader() override;

    void connect() override;
    void disconnect() override;
    void startInventory() override;
    void stopInventory() override;
    std::vector<std::string> readTags() override;
    bool isInventoryRunning() const override { return inventoryRunning_; }
    void configureReader() override { /* No-op for mock */ }

private:
    std::atomic<bool> inventoryRunning_{false};
    std::atomic<bool> simulationRunning_{false};
    std::thread simulationThread_;

    void simulationLoop();
    std::string generateMockTagJson();
    void appendInventoryJsonToFile(const std::string &json);
};

} // namespace readers
