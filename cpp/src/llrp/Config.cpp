#include "Config.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace llrp {

std::string Config::READER_IP   = "169.254.116.164";
int         Config::READER_PORT = 5084;

static std::string trim(const std::string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end   = s.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

static bool tryLoadFile(const std::string &path, std::string &outIp, int &outPort) {
    std::ifstream f(path);
    if (!f.is_open()) return false;

    std::string line;
    std::string ip;
    int port = -1;
    while (std::getline(f, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = trim(line.substr(0, eq));
        std::string val = trim(line.substr(eq + 1));

        if (key == "reader.ip" && !val.empty()) {
            ip = val;
        } else if (key == "reader.port" && !val.empty()) {
            try { port = std::stoi(val); } catch (...) {}
        }
    }

    if (!ip.empty())  outIp   = ip;
    if (port > 0)     outPort = port;
    return true;
}

void Config::load() {
    const char *candidates[] = {
        "config.properties",
        "./config.properties",
        "../config.properties",
        nullptr
    };

    for (int i = 0; candidates[i] != nullptr; ++i) {
        std::string ip   = READER_IP;
        int         port = READER_PORT;
        if (tryLoadFile(candidates[i], ip, port)) {
            READER_IP   = ip;
            READER_PORT = port;
            std::cout << "Loaded config from: " << candidates[i] << "\n";
            return;
        }
    }

    // Comprobar variable de entorno como fallback
    if (const char *envIp = std::getenv("LLRP_READER_IP"))   READER_IP   = envIp;
    if (const char *envP  = std::getenv("LLRP_READER_PORT")) {
        try { READER_PORT = std::stoi(envP); } catch (...) {}
    }

    std::cout << "config.properties not found, using default values.\n";
}

} // namespace llrp
