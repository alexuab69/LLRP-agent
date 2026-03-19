#pragma once
#include <string>

namespace llrp {

/**
 * Configuración global del agente LLRP.
 * Equivalente a llrp/Config.java — carga config.properties desde CWD o ../
 */
class Config {
public:
    static std::string READER_IP;
    static int         READER_PORT;

    // Carga config.properties. Llamar una sola vez al inicio.
    static void load();

private:
    Config() = delete;
};

} // namespace llrp
