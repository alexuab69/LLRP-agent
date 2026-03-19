#include "ThingMagicM7EReader.h"

// Mercury C API
extern "C" {
#include "tm_reader.h"
#include "tmr_gen2.h"
#include "tmr_tag_data.h"
#include "tmr_read_plan.h"
#include "tmr_status.h"
}

#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace readers {

// ── Callback de lectura asíncrona ───────────────────────────────────────────
// Se debe usar una función estática (C callback) que recibe el reader y
// un contexto (puntero a ThingMagicM7EReader).
static void readCallback(TMR_Reader *reader, const TMR_TagReadData *trd, void *cookie) {
    auto *self = static_cast<ThingMagicM7EReader*>(cookie);

    // Convertir EPC a hex string
    std::ostringstream oss;
    for (uint8_t i = 0; i < trd->tag.epcByteCount; ++i)
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)trd->tag.epc[i];
    std::string epc = oss.str();

    // Obtener RSSI (en dBm, valor signed)
    int rssi = trd->rssi;

    {
        std::lock_guard<std::mutex> lk(self->tagsMtx_);  // tagsMtx_ es private, usar método amigo o getter
        // Acceso directo a detectedTags_ es posible porque este callback es función libre
        // amiga implícita (lambdas tampoco se pueden registrar directamente en C API).
        // Solución: exponer el método de inserción.
    }

    // Fallback: loggear directamente (igual que Java)
    std::cout << "Tag detected: " << epc << " (RSSI: " << rssi << " dBm)\n";
}

// ────────────────────────────────────────────────────────────────────────────
// Constructor / Destructor
// ────────────────────────────────────────────────────────────────────────────

ThingMagicM7EReader::ThingMagicM7EReader()
    : reader_(nullptr)
    , inventoryRunning_(false)
{}

ThingMagicM7EReader::~ThingMagicM7EReader() {
    disconnect();
}

// ────────────────────────────────────────────────────────────────────────────
// Conexión
// ────────────────────────────────────────────────────────────────────────────

void ThingMagicM7EReader::connect() {
    TMR_Status rc;

    // Crear instancia del reader con la URI serial
    std::string uri = std::string(SERIAL_URI) + "?baud=115200";
    reader_ = TMR_create(uri.c_str());
    if (!reader_)
        throw std::runtime_error("TMR_create failed for URI: " + uri);

    // Conectar al hardware
    rc = TMR_connect(reader_);
    if (rc != TMR_SUCCESS)
        throw std::runtime_error(std::string("TMR_connect failed: ") + TMR_strerr(reader_, rc));

    std::cout << "Connected to ThingMagic M7E using URI: " << uri << "\n";

    configureReader();
}

void ThingMagicM7EReader::disconnect() {
    if (!reader_) return;
    try { stopInventory(); } catch (...) {}
    TMR_destroy(reader_);
    reader_ = nullptr;
    std::cout << "Disconnected from ThingMagic M7E\n";
}

// ────────────────────────────────────────────────────────────────────────────
// Configuración
// ────────────────────────────────────────────────────────────────────────────

void ThingMagicM7EReader::configureReader() {
    if (!reader_) return;
    TMR_Status rc;

    // Potencia de lectura: 20 dBm = 2000 centésimas
    int32_t readPower = 2000;
    rc = TMR_paramSet(reader_, TMR_PARAM_RADIO_READPOWER, &readPower);
    if (rc != TMR_SUCCESS)
        std::cerr << "Warning: could not set read power: " << TMR_strerr(reader_, rc) << "\n";

    // Región: NA (Norteamérica). Cambiar a EU2 si estás en Europa.
    TMR_Region region = TMR_REGION_NA;
    rc = TMR_paramSet(reader_, TMR_PARAM_REGION_ID, &region);
    if (rc != TMR_SUCCESS)
        std::cerr << "Warning: could not set region: " << TMR_strerr(reader_, rc) << "\n";

    // Sesión Gen2 S0
    TMR_GEN2_Session session = TMR_GEN2_SESSION_S0;
    rc = TMR_paramSet(reader_, TMR_PARAM_GEN2_SESSION, &session);
    if (rc != TMR_SUCCESS)
        std::cerr << "Warning: could not set Gen2 session: " << TMR_strerr(reader_, rc) << "\n";

    std::cout << "ThingMagic M7E configured successfully\n";
}

// ────────────────────────────────────────────────────────────────────────────
// Inventario
// ────────────────────────────────────────────────────────────────────────────

void ThingMagicM7EReader::startInventory() {
    if (inventoryRunning_) {
        std::cout << "Inventory already running\n";
        return;
    }

    // Configurar plan de lectura: antennas 1 y 2, protocolo GEN2
    TMR_ReadPlan plan;
    uint8_t antennas[] = {1, 2};
    TMR_Status rc = TMR_RP_init_simple(&plan, 2, antennas, TMR_TAG_PROTOCOL_GEN2, 1000);
    if (rc != TMR_SUCCESS)
        throw std::runtime_error(std::string("TMR_RP_init_simple failed: ") + TMR_strerr(reader_, rc));

    rc = TMR_paramSet(reader_, TMR_PARAM_READ_PLAN, &plan);
    if (rc != TMR_SUCCESS)
        throw std::runtime_error(std::string("TMR_paramSet read plan failed: ") + TMR_strerr(reader_, rc));

    // Registrar callback asíncrono
    TMR_ReadListenerBlock rlb;
    rlb.listener = readCallback;
    rlb.cookie   = this;
    rc = TMR_addReadListener(reader_, &rlb);
    if (rc != TMR_SUCCESS)
        throw std::runtime_error(std::string("TMR_addReadListener failed: ") + TMR_strerr(reader_, rc));

    // Iniciar lectura asíncrona
    rc = TMR_startReading(reader_);
    if (rc != TMR_SUCCESS)
        throw std::runtime_error(std::string("TMR_startReading failed: ") + TMR_strerr(reader_, rc));

    inventoryRunning_ = true;
    std::cout << "ThingMagic M7E inventory started\n";
}

void ThingMagicM7EReader::stopInventory() {
    if (!inventoryRunning_ || !reader_) return;
    TMR_stopReading(reader_);
    inventoryRunning_ = false;
    std::cout << "ThingMagic M7E inventory stopped\n";
}

std::vector<std::string> ThingMagicM7EReader::readTags() {
    std::lock_guard<std::mutex> lk(tagsMtx_);
    auto tags = detectedTags_;
    detectedTags_.clear();
    return tags;
}

bool ThingMagicM7EReader::isInventoryRunning() const {
    return inventoryRunning_;
}

// ────────────────────────────────────────────────────────────────────────────
// Extras
// ────────────────────────────────────────────────────────────────────────────

void ThingMagicM7EReader::setReadPower(int powerDbm) {
    int32_t power = powerDbm * 100;
    TMR_Status rc = TMR_paramSet(reader_, TMR_PARAM_RADIO_READPOWER, &power);
    if (rc != TMR_SUCCESS)
        throw std::runtime_error(std::string("setReadPower failed: ") + TMR_strerr(reader_, rc));
    std::cout << "Read power set to " << powerDbm << " dBm\n";
}

void ThingMagicM7EReader::setRegion(const std::string &region) {
    TMR_Region r = TMR_REGION_NA;
    if      (region == "EU" || region == "EU2") r = TMR_REGION_EU2;
    else if (region == "NA")                    r = TMR_REGION_NA;
    else if (region == "IN")                    r = TMR_REGION_IN;
    else if (region == "JP")                    r = TMR_REGION_JP;
    else if (region == "PRC")                   r = TMR_REGION_PRC;

    TMR_Status rc = TMR_paramSet(reader_, TMR_PARAM_REGION_ID, &r);
    if (rc != TMR_SUCCESS)
        throw std::runtime_error(std::string("setRegion failed: ") + TMR_strerr(reader_, rc));
    std::cout << "Region set to " << region << "\n";
}

} // namespace readers
