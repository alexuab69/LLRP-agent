#pragma once
#include "RFIDReader.h"
#include <atomic>
#include <mutex>
#include <string>
#include <vector>

// Forward-declarations de Mercury API C
typedef struct TMR_Reader TMR_Reader;

namespace readers {

/**
 * Implementación para ThingMagic M7E usando Mercury C API.
 * Equivalente a readers/ThingMagicM7EReader.java.
 *
 * La Mercury API C (libmercuryapi) es el SDK oficial de ThingMagic.
 * Gestiona el protocolo serial propietario, timeouts y reconexiones.
 */
class ThingMagicM7EReader : public RFIDReader {
public:
    ThingMagicM7EReader();
    ~ThingMagicM7EReader() override;

    void connect()    override;
    void disconnect() override;

    void startInventory() override;
    void stopInventory()  override;

    std::vector<std::string> readTags()          override;
    bool                     isInventoryRunning() const override;
    void                     configureReader()    override;

    void setReadPower(int powerDbm);
    void setRegion(const std::string &region);

private:
    TMR_Reader        *reader_;
    std::atomic<bool>  inventoryRunning_;
    std::mutex         tagsMtx_;
    std::vector<std::string> detectedTags_;

    static constexpr const char *SERIAL_URI = "tmr:///dev/serial0";
};

} // namespace readers
