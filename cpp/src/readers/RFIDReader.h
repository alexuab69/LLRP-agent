#pragma once
#include <string>
#include <vector>
#include <stdexcept>

namespace readers {

/**
 * Interfaz base para todos los lectores RFID.
 * Equivalente a readers/RFIDReader.java — arquitectura modular.
 */
class RFIDReader {
public:
    virtual ~RFIDReader() = default;

    /** Conecta al lector. Lanza std::runtime_error si falla. */
    virtual void connect() = 0;

    /** Desconecta del lector. */
    virtual void disconnect() = 0;

    /** Inicia el inventario de tags. Lanza std::runtime_error si falla. */
    virtual void startInventory() = 0;

    /** Detiene el inventario de tags. */
    virtual void stopInventory() = 0;

    /** Devuelve EPCs detectados (puede ser vacío si el reader pushea por callback). */
    virtual std::vector<std::string> readTags() = 0;

    /** Devuelve true si el inventario está activo. */
    virtual bool isInventoryRunning() const = 0;

    /** Configura parámetros del lector (potencia, región, sesión, etc.). */
    virtual void configureReader() = 0;
};

} // namespace readers
