#pragma once
#include "../readers/RFIDReader.h"
#include <atomic>
#include <memory>
#include <string>
#include <thread>

namespace reader {

/**
 * Adaptador principal que delega a implementaciones específicas de lectores.
 * Equivalente a reader/ReaderAdapter.java.
 *
 * Selección del reader:
 *   Env var LLRP_READER_TYPE:
 *     "llrp" → GenericLLRPReader (por defecto)
 *
 * El soporte M7E queda fuera del build por defecto porque ahora la prioridad
 * es clonar el flujo Java de LLRP directo contra la IP/puerto de config.
 */
class ReaderAdapter {
public:
    ReaderAdapter();
    ~ReaderAdapter();

    void setReaderType(const std::string &type);

    void startInventory();
    void stopInventory();
    std::vector<std::string> readTags();
    bool isInventoryRunning() const;

private:
    std::unique_ptr<readers::RFIDReader> realReader_;
    bool inventoryRunning_ = false;

    // Timer periódico para mostrar tags
    std::thread timerThread_;
    std::atomic<bool> timerRunning_{false};
    void startPeriodicTagPrint();
    void stopPeriodicTagPrint();
};

} // namespace reader
