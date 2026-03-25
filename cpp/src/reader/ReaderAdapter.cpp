#include "ReaderAdapter.h"
#include "../readers/GenericLLRPReader.h"
#include "../readers/MockLLRPReader.h"

#ifdef LLRP_AGENT_ENABLE_M7E
#include "../readers/ThingMagicM7EReader.h"
#endif

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace reader {

// ────────────────────────────────────────────────────────────────────────────
// Constructor / Destructor
// ────────────────────────────────────────────────────────────────────────────

ReaderAdapter::ReaderAdapter() {
    // 1) entorno, 2) defecto "llrp"
    std::string type;
    if (const char *e = std::getenv("LLRP_READER_TYPE")) type = e;
    if (type.empty()) type = "llrp";

    // Convertir a minúsculas
    std::transform(type.begin(), type.end(), type.begin(), ::tolower);

    setReaderType(type);
}

ReaderAdapter::~ReaderAdapter() {
    stopPeriodicTagPrint();
    if (realReader_) {
        try { realReader_->disconnect(); } catch (...) {}
    }
}

// ────────────────────────────────────────────────────────────────────────────
// Selección dinámica de reader
// ────────────────────────────────────────────────────────────────────────────

void ReaderAdapter::setReaderType(const std::string &type) {
    // Desconectar el actual
    if (realReader_) {
        try { realReader_->disconnect(); } catch (...) {}
        realReader_.reset();
    }

    std::unique_ptr<readers::RFIDReader> candidate;
    if (type == "llrp")
        candidate = std::make_unique<readers::GenericLLRPReader>();
#ifdef LLRP_AGENT_ENABLE_M7E
    else if (type == "thingmagic" || type == "m7e")
        candidate = std::make_unique<readers::ThingMagicM7EReader>();
#endif
    else
        throw std::invalid_argument("Unknown reader type: " + type);

    try {
        candidate->connect();
        realReader_ = std::move(candidate);
        std::cout << "[ReaderAdapter] Successfully connected using reader type '" << type << "'\n";
    } catch (const std::exception &e) {
        std::cerr << "Could not connect using reader type '" << type << "': " << e.what() << "\n";
        
        // Fallback: usar MockLLRPReader solo si se solicita explícitamente
        const char *useMockEnv = std::getenv("LLRP_USE_MOCK_IF_UNAVAILABLE");
        bool useMockFallback = useMockEnv && (std::string(useMockEnv) == "1" || std::string(useMockEnv) == "true");
        
        if (useMockFallback) {
            std::cout << "[ReaderAdapter] LLRP_USE_MOCK_IF_UNAVAILABLE=true, falling back to MockLLRPReader\n";
            try {
                auto mockCandidate = std::make_unique<readers::MockLLRPReader>();
                mockCandidate->connect();
                realReader_ = std::move(mockCandidate);
                std::cout << "[ReaderAdapter] Mock reader initialized successfully\n";
            } catch (const std::exception &mockE) {
                std::cerr << "Failed to initialize MockLLRPReader: " << mockE.what() << "\n";
                realReader_.reset();
            }
        } else {
            std::cout << "[ReaderAdapter] No reader available (set LLRP_USE_MOCK_IF_UNAVAILABLE=1 to use mock)\n";
            realReader_.reset();
        }
    }
}

// ────────────────────────────────────────────────────────────────────────────
// Control de inventario
// ────────────────────────────────────────────────────────────────────────────

void ReaderAdapter::startInventory() {
    if (inventoryRunning_) {
        return;
    }
    if (!realReader_) return;
    try {
        realReader_->startInventory();
        inventoryRunning_ = true;
        startPeriodicTagPrint();
    } catch (const std::exception &e) {
        std::cerr << "Failed to start inventory: " << e.what() << "\n";
    }
}

void ReaderAdapter::stopInventory() {
    stopPeriodicTagPrint();
    if (!realReader_) return;
    try {
        realReader_->stopInventory();
        inventoryRunning_ = false;
    } catch (const std::exception &e) {
        std::cerr << "Failed to stop inventory: " << e.what() << "\n";
    }
}

std::vector<std::string> ReaderAdapter::readTags() {
    if (!realReader_) return {};
    try {
        return realReader_->readTags();
    } catch (const std::exception &e) {
        std::cerr << "Failed to read tags: " << e.what() << "\n";
        return {};
    }
}

bool ReaderAdapter::isInventoryRunning() const {
    return inventoryRunning_;
}

// ────────────────────────────────────────────────────────────────────────────
// Timer periódico (equivalente a java.util.Timer)
// ────────────────────────────────────────────────────────────────────────────

void ReaderAdapter::startPeriodicTagPrint() {
    if (timerThread_.joinable()) {
        return;
    }
    timerRunning_ = true;
    timerThread_ = std::thread([this]() {
        while (timerRunning_) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            if (!timerRunning_) break;
            auto tags = readTags();
            if (!tags.empty()) {
                std::cout << "Tags detected: [";
                for (size_t i = 0; i < tags.size(); ++i) {
                    if (i) std::cout << ", ";
                    std::cout << tags[i];
                }
                std::cout << "]\n";
            }
        }
    });
}

void ReaderAdapter::stopPeriodicTagPrint() {
    timerRunning_ = false;
    if (timerThread_.joinable()) timerThread_.join();
}

} // namespace reader
