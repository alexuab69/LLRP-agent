#include "MockLLRPReader.h"
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <fstream>

namespace readers {

MockLLRPReader::MockLLRPReader()
    : inventoryRunning_(false), simulationRunning_(false) {
}

MockLLRPReader::~MockLLRPReader() {
    disconnect();
}

void MockLLRPReader::connect() {
    // Simulación - no hace nada (siempre "conectado")
}

void MockLLRPReader::disconnect() {
    if (simulationThread_.joinable()) {
        simulationRunning_ = false;
        simulationThread_.join();
    }
}

void MockLLRPReader::startInventory() {
    if (inventoryRunning_) return;
    
    inventoryRunning_ = true;
    simulationRunning_ = true;
    simulationThread_ = std::thread(&MockLLRPReader::simulationLoop, this);
}

void MockLLRPReader::stopInventory() {
    inventoryRunning_ = false;
    if (simulationThread_.joinable()) {
        simulationRunning_ = false;
        simulationThread_.join();
    }
}

std::vector<std::string> MockLLRPReader::readTags() {
    // En la versión de mock, los tags se reportan directamente sin callback
    return {};
}

std::string MockLLRPReader::generateMockTagJson() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> rssiDist(-90, -40);
    static std::uniform_int_distribution<> antennaDist(1, 4);
    static std::uniform_int_distribution<> readCountDist(1, 5);
    static std::uniform_int_distribution<> freqDist(860, 960);
    
    // Generar EPC ficticio (96 bits = 24 hex chars)
    char epcHex[25];
    snprintf(epcHex, sizeof(epcHex), "%024llX", (unsigned long long)(gen() % 0xFFFFFFFFFFFFFFFFULL));
    
    // ISO 8601 timestamp UTC
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    
    char timestamp[30];
    std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", std::gmtime(&time_t_now));
    
    std::ostringstream json;
    json << "{"
         << "\"eventType\":\"RFID_TAG_READ\","
         << "\"timestamp\":\"" << timestamp << "." << std::setfill('0') << std::setw(3) << ms.count() << "Z\","
         << "\"EPC\":\"" << epcHex << "\","
         << "\"RSSI\":" << rssiDist(gen) << ","
         << "\"antenna\":" << antennaDist(gen) << ","
         << "\"readCount\":" << readCountDist(gen) << ","
         << "\"frequency\":" << freqDist(gen) << ","
         << "\"phase\":null,"
         << "\"doppler\":null,"
         << "\"TID\":null,"
         << "\"userMemory\":null"
         << "}";
    
    return json.str();
}

void MockLLRPReader::appendInventoryJsonToFile(const std::string &json) {
    const char *outputPath = std::getenv("LLRP_JSON_OUTPUT_PATH");
    if (!outputPath) {
        outputPath = "rfid_inventory_events.jsonl";
    }
    
    std::ofstream file(outputPath, std::ios::app);
    if (file.is_open()) {
        file << json << "\n";
        file.close();
    } else {
        std::cerr << "[MockLLRPReader] Failed to open " << outputPath << " for writing\n";
    }
}

void MockLLRPReader::simulationLoop() {
    // Delay inicial antes de empezar
    std::this_thread::sleep_for(std::chrono::milliseconds(800));
    
    int cycleCount = 0;
    while (simulationRunning_ && inventoryRunning_) {
        cycleCount++;
        
        // Generar 2-5 tags por ciclo
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> countDist(2, 5);
        int tagCount = countDist(gen);
        
        std::ostringstream inventoryJson;
        inventoryJson << "{\"eventType\":\"RFID_INVENTORY\","
                      << "\"timestamp\":\"";
        
        // ISO 8601 timestamp
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        char timestamp[30];
        std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", std::gmtime(&time_t_now));
        
        inventoryJson << timestamp << "." << std::setfill('0') << std::setw(3) << ms.count() << "Z\","
                      << "\"readerVendorProfile\":\"IMPINJ_ZEBRA_STYLE\","
                      << "\"tags\":[";
        
        for (int i = 0; i < tagCount; ++i) {
            if (i > 0) inventoryJson << ",";
            inventoryJson << generateMockTagJson();
        }
        
        inventoryJson << "]}";
        
        std::string fullJson = inventoryJson.str();
        std::cout << "[MockLLRPReader][RFID_JSON_INVENTORY] " << fullJson << "\n";
        
        // Guardar línea JSON
        appendInventoryJsonToFile(fullJson);
        
        // Ciclo cada 1.5 segundos
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    }
    
    std::cout << "[MockLLRPReader] Simulation stopped after " << cycleCount << " cycles\n";
}

} // namespace readers
