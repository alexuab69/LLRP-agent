#include "GenericLLRPReader.h"
#include "../llrp/Config.h"

// ── LTKCPP ──────────────────────────────────────────────────────────────────
#include "ltkcpp.h"
using namespace LLRP;

// Forward declaration: global type registry initialised in main.cpp
extern LLRP::CTypeRegistry *g_pTypeRegistry;

// ── STL ─────────────────────────────────────────────────────────────────────
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace readers {

namespace {

std::vector<uint8_t> encodeMessageToFrame(const CMessage& message)
{
    std::vector<uint8_t> buffer(32u * 1024u);
    CFrameEncoder encoder(buffer.data(), static_cast<unsigned int>(buffer.size()));
    encoder.encodeElement(&message);
    buffer.resize(encoder.getLength());
    return buffer;
}

CMessage* decodeMessageFromFrame(const CTypeRegistry* typeRegistry, const std::vector<uint8_t>& frame)
{
    CFrameDecoder decoder(typeRegistry, const_cast<unsigned char*>(frame.data()), static_cast<unsigned int>(frame.size()));
    return decoder.decodeMessage();
}

std::string jsonEscape(const std::string& value)
{
    std::ostringstream oss;
    for (char c : value) {
        switch (c) {
            case '"': oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\b': oss << "\\b"; break;
            case '\f': oss << "\\f"; break;
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    oss << "\\u"
                        << std::hex << std::setw(4) << std::setfill('0')
                        << static_cast<int>(static_cast<unsigned char>(c));
                } else {
                    oss << c;
                }
        }
    }
    return oss.str();
}

std::string nowIso8601Utc()
{
    using namespace std::chrono;
    const auto now = system_clock::now();
    const auto secs = time_point_cast<std::chrono::seconds>(now);
    const auto ms = duration_cast<milliseconds>(now - secs).count();

    std::time_t tt = system_clock::to_time_t(now);
    std::tm tmUtc{};
#ifdef _WIN32
    gmtime_s(&tmUtc, &tt);
#else
    gmtime_r(&tt, &tmUtc);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tmUtc, "%Y-%m-%dT%H:%M:%S")
        << '.' << std::setw(3) << std::setfill('0') << ms << 'Z';
    return oss.str();
}

std::string iso8601FromEpochMicros(llrp_u64_t micros)
{
    using namespace std::chrono;
    const auto tp = system_clock::time_point{} + microseconds(micros);
    const auto secs = time_point_cast<std::chrono::seconds>(tp);
    const auto ms = duration_cast<milliseconds>(tp - secs).count();

    std::time_t tt = system_clock::to_time_t(tp);
    std::tm tmUtc{};
#ifdef _WIN32
    gmtime_s(&tmUtc, &tt);
#else
    gmtime_r(&tt, &tmUtc);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tmUtc, "%Y-%m-%dT%H:%M:%S")
        << '.' << std::setw(3) << std::setfill('0') << ms << 'Z';
    return oss.str();
}

std::string epcFromTagReport(CTagReportData* td)
{
    if (!td || !td->getEPCParameter()) {
        return "";
    }

    std::ostringstream oss;
    auto *e96 = dynamic_cast<CEPC_96*>(td->getEPCParameter());
    if (e96) {
        for (int i = 0; i < 12; ++i) {
            oss << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(e96->getEPC().m_aValue[i]);
        }
        return oss.str();
    }

    auto *edata = dynamic_cast<CEPCData*>(td->getEPCParameter());
    if (edata) {
        llrp_u1v_t value = edata->getEPC();
        for (unsigned int i = 0; i < value.m_nBit / 8; ++i) {
            oss << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(value.m_pValue[i]);
        }
        return oss.str();
    }

    return "";
}

std::string toJsonNumberOrNull(bool hasValue, long long value)
{
    if (!hasValue) {
        return "null";
    }
    return std::to_string(value);
}

std::string buildTagEventJson(CTagReportData* td)
{
    const std::string epc = epcFromTagReport(td);
    const bool hasEpc = !epc.empty();

    const bool hasAntenna = td && td->getAntennaID();
    const long long antenna = hasAntenna ? td->getAntennaID()->getAntennaID() : 0;

    const bool hasRssi = td && td->getPeakRSSI();
    const long long rssi = hasRssi ? td->getPeakRSSI()->getPeakRSSI() : 0;

    const bool hasReadCount = td && td->getTagSeenCount();
    const long long readCount = hasReadCount ? td->getTagSeenCount()->getTagCount() : 0;

    const bool hasChannelIndex = td && td->getChannelIndex();
    const long long channelIndex = hasChannelIndex ? td->getChannelIndex()->getChannelIndex() : 0;

    std::string timestamp = nowIso8601Utc();
    if (td && td->getFirstSeenTimestampUTC()) {
        timestamp = iso8601FromEpochMicros(td->getFirstSeenTimestampUTC()->getMicroseconds());
    } else if (td && td->getLastSeenTimestampUTC()) {
        timestamp = iso8601FromEpochMicros(td->getLastSeenTimestampUTC()->getMicroseconds());
    }

    // Base LLRP reports don't always include phase, doppler, TID, or user memory.
    std::ostringstream oss;
    oss << '{'
        << "\"eventType\":\"RFID_TAG_READ\","
        << "\"timestamp\":\"" << jsonEscape(timestamp) << "\","
        << "\"EPC\":" << (hasEpc ? ("\"" + jsonEscape(epc) + "\"") : "null") << ','
        << "\"tagId\":" << (hasEpc ? ("\"" + jsonEscape(epc) + "\"") : "null") << ','
        << "\"RSSI\":" << toJsonNumberOrNull(hasRssi, rssi) << ','
        << "\"antenna\":" << toJsonNumberOrNull(hasAntenna, antenna) << ','
        << "\"readCount\":" << toJsonNumberOrNull(hasReadCount, readCount) << ','
        << "\"frequency\":" << toJsonNumberOrNull(hasChannelIndex, channelIndex) << ','
        << "\"phase\":null,"
        << "\"doppler\":null,"
        << "\"TID\":null,"
        << "\"userMemory\":null"
        << '}';

    return oss.str();
}

} // namespace

// ────────────────────────────────────────────────────────────────────────────
// Constructor / Destructor
// ────────────────────────────────────────────────────────────────────────────

GenericLLRPReader::GenericLLRPReader()
    : host_(llrp::Config::READER_IP)
    , port_(llrp::Config::READER_PORT)
    , minLogIntervalMs_(1500)
    , sockFd_(invalid_socket_value)
    , inventoryRunning_(false)
{
    // Sobreescribir desde env/propiedades de proceso si están definidas
    if (const char *h = std::getenv("LLRP_READER_HOST"))
        host_ = h;
    if (const char *p = std::getenv("LLRP_READER_PORT")) {
        try { port_ = std::stoi(p); } catch (...) {}
    }
    if (const char *ms = std::getenv("LLRP_TAG_LOG_MIN_INTERVAL_MS")) {
        try { minLogIntervalMs_ = std::max(0L, std::stol(ms)); } catch (...) {}
    }
}

GenericLLRPReader::~GenericLLRPReader() {
    disconnect();
}

// ────────────────────────────────────────────────────────────────────────────
// Conexión
// ────────────────────────────────────────────────────────────────────────────

void GenericLLRPReader::connect() {
    struct addrinfo hints{}, *res = nullptr;
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    std::string portStr = std::to_string(port_);

    int rc = ::getaddrinfo(host_.c_str(), portStr.c_str(), &hints, &res);
    if (rc != 0)
        throw std::runtime_error("getaddrinfo failed for " + host_ + ": " + ::gai_strerror(rc));

    sockFd_ = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockFd_ == invalid_socket_value) {
        ::freeaddrinfo(res);
        throw std::runtime_error("socket() failed");
    }

    socket_set_recv_timeout(sockFd_, 10000);
    socket_set_send_timeout(sockFd_, 10000);

    if (::connect(sockFd_, res->ai_addr, res->ai_addrlen) < 0) {
        ::freeaddrinfo(res);
        socket_close(sockFd_);
        sockFd_ = invalid_socket_value;
        throw std::runtime_error("connect() to " + host_ + ":" + portStr + " failed");
    }
    ::freeaddrinfo(res);

    socket_set_recv_timeout(sockFd_, 5000);

    std::cout << "GenericLLRPReader connected to " << host_ << ":" << port_ << "\n";
}

void GenericLLRPReader::disconnect() {
    inventoryRunning_ = false;
    if (readerThread_.joinable()) readerThread_.join();
    if (sockFd_ != invalid_socket_value) {
        socket_close(sockFd_);
        sockFd_ = invalid_socket_value;
        std::cout << "GenericLLRPReader disconnected\n";
    }
}

// ────────────────────────────────────────────────────────────────────────────
// Inventario
// ────────────────────────────────────────────────────────────────────────────

void GenericLLRPReader::startInventory() {
    if (inventoryRunning_) {
        return;
    }

    if (readerThread_.joinable()) {
        readerThread_.join();
    }

    {
        std::lock_guard<std::mutex> lk(lastLogMtx_);
        lastLoggedMs_.clear();
    }

    // Ensure we are not reusing a stale ROSpec from previous runs.
    try {
        sendDeleteROSpec();
    } catch (...) {
        // Reader may report missing ROSpec; safe to ignore.
    }

    sendAddROSpec();
    sendEnableROSpec();
    sendStartROSpec();

    inventoryRunning_ = true;

    readerThread_ = std::thread([this]() {
        while (inventoryRunning_) {
            try {
                auto frame = readSingleLLRPFrame();
                if (frame.empty()) break;
                handleReaderMessage(frame);
            } catch (const std::exception &e) {
                if (!inventoryRunning_) break;
                // Timeout de lectura → seguir esperando
                const std::string msg = e.what();
                if (msg.find("timeout") != std::string::npos ||
                    msg.find("EAGAIN")  != std::string::npos ||
                    msg.find("EWOULDBLOCK") != std::string::npos)
                    continue;
                std::cerr << "Reader thread error: " << msg << "\n";
                break;
            }
        }
    });
}

void GenericLLRPReader::stopInventory() {
    inventoryRunning_ = false;
    try { sendStopROSpec(); } catch (...) {}
    if (readerThread_.joinable()) readerThread_.join();
}

std::vector<std::string> GenericLLRPReader::readTags() {
    // Este reader notifica por consola; no mantiene caché.
    return {};
}

bool GenericLLRPReader::isInventoryRunning() const {
    return inventoryRunning_;
}

void GenericLLRPReader::configureReader() {
    // No-op
}

// ────────────────────────────────────────────────────────────────────────────
// Construcción y envío de mensajes LLRP
// ────────────────────────────────────────────────────────────────────────────

void GenericLLRPReader::sendRawMessage(const uint8_t *data, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = ::send(sockFd_, reinterpret_cast<const char*>(data + sent), static_cast<int>(len - sent), 0);
        if (n <= 0)
            throw std::runtime_error("send() failed");
        sent += static_cast<size_t>(n);
    }
}

// Crea un mensaje de control de ROSpec (START/STOP/ENABLE) de 14 bytes
// Header (2) + Length (4) + MessageID (4) + ROSpecID (4)
void GenericLLRPReader::sendROSpecControl(uint16_t msgType, uint32_t msgId, uint32_t rospecId) {
    constexpr int VERSION = 1;
    uint8_t buf[14];
    uint16_t header = static_cast<uint16_t>((VERSION << 10) | msgType);
    uint32_t length = 14;

    buf[0]  = (header  >> 8) & 0xFF;
    buf[1]  =  header        & 0xFF;
    buf[2]  = (length  >> 24) & 0xFF;
    buf[3]  = (length  >> 16) & 0xFF;
    buf[4]  = (length  >> 8)  & 0xFF;
    buf[5]  =  length         & 0xFF;
    buf[6]  = (msgId   >> 24) & 0xFF;
    buf[7]  = (msgId   >> 16) & 0xFF;
    buf[8]  = (msgId   >> 8)  & 0xFF;
    buf[9]  =  msgId          & 0xFF;
    buf[10] = (rospecId >> 24) & 0xFF;
    buf[11] = (rospecId >> 16) & 0xFF;
    buf[12] = (rospecId >> 8)  & 0xFF;
    buf[13] =  rospecId        & 0xFF;

    sendRawMessage(buf, sizeof(buf));
}

void GenericLLRPReader::sendStartROSpec() {
    sendROSpecControl(22, 1, DEFAULT_ROSPEC_ID);
    std::cout << "Sent START_ROSPEC (ROSpecID=" << DEFAULT_ROSPEC_ID << ")\n";
}

void GenericLLRPReader::sendStopROSpec() {
    sendROSpecControl(23, 2, DEFAULT_ROSPEC_ID);
    std::cout << "Sent STOP_ROSPEC (ROSpecID=" << DEFAULT_ROSPEC_ID << ")\n";
}

void GenericLLRPReader::sendEnableROSpec() {
    sendROSpecControl(24, 3, DEFAULT_ROSPEC_ID);
    std::cout << "Sent ENABLE_ROSPEC (ROSpecID=" << DEFAULT_ROSPEC_ID << ")\n";
}

void GenericLLRPReader::sendAddROSpec() {
    // Construir ADD_ROSPEC usando LTKCPP
    CADD_ROSPEC *pAddROSpec = new CADD_ROSPEC();
    pAddROSpec->setMessageID(4);

    // ── ROSpec ──────────────────────────────────────────────────────────────
    CROSpec *pROSpec = new CROSpec();
    pROSpec->setROSpecID(DEFAULT_ROSPEC_ID);
    pROSpec->setPriority(0);
    pROSpec->setCurrentState(ROSpecState_Disabled);

    // Boundary
    CROBoundarySpec *pBoundary = new CROBoundarySpec();
    CROSpecStartTrigger *pStartTrig = new CROSpecStartTrigger();
    pStartTrig->setROSpecStartTriggerType(ROSpecStartTriggerType_Null);
    CROSpecStopTrigger *pStopTrig = new CROSpecStopTrigger();
    pStopTrig->setROSpecStopTriggerType(ROSpecStopTriggerType_Null);
    pStopTrig->setDurationTriggerValue(0);
    pBoundary->setROSpecStartTrigger(pStartTrig);
    pBoundary->setROSpecStopTrigger(pStopTrig);
    pROSpec->setROBoundarySpec(pBoundary);

    // AISpec
    CAISpec *pAISpec = new CAISpec();
    llrp_u16v_t antennaIds(1);
    antennaIds.m_pValue[0] = 0; // 0 = all antennas (LLRP convention)
    pAISpec->setAntennaIDs(antennaIds);

    CAISpecStopTrigger *pAIStop = new CAISpecStopTrigger();
    pAIStop->setAISpecStopTriggerType(AISpecStopTriggerType_Null);
    pAIStop->setDurationTrigger(0);
    pAISpec->setAISpecStopTrigger(pAIStop);

    CInventoryParameterSpec *pInvSpec = new CInventoryParameterSpec();
    pInvSpec->setInventoryParameterSpecID(1);
    pInvSpec->setProtocolID(AirProtocols_EPCGlobalClass1Gen2);
    pAISpec->addInventoryParameterSpec(pInvSpec);

    pROSpec->addSpecParameter(pAISpec);

    // ROReportSpec
    CROReportSpec *pReportSpec = new CROReportSpec();
    pReportSpec->setROReportTrigger(ROReportTriggerType_Upon_N_Tags_Or_End_Of_ROSpec);
    pReportSpec->setN(1);

    CTagReportContentSelector *pSelector = new CTagReportContentSelector();
    pSelector->setEnableROSpecID(1);
    pSelector->setEnableSpecIndex(0);
    pSelector->setEnableAntennaID(1);
    pSelector->setEnableChannelIndex(1);
    pSelector->setEnablePeakRSSI(1);
    pSelector->setEnableFirstSeenTimestamp(1);
    pSelector->setEnableLastSeenTimestamp(1);
    pSelector->setEnableTagSeenCount(1);
    pSelector->setEnableInventoryParameterSpecID(1);
    pSelector->setEnableAccessSpecID(0);
    pReportSpec->setTagReportContentSelector(pSelector);

    pROSpec->setROReportSpec(pReportSpec);
    pAddROSpec->setROSpec(pROSpec);

    // Codificar a binario y enviar
    std::vector<uint8_t> frame = encodeMessageToFrame(*pAddROSpec);
    sendRawMessage(frame.data(), frame.size());
    std::cout << "Sent ADD_ROSPEC (ROSpecID=" << DEFAULT_ROSPEC_ID << ")\n";
    delete pAddROSpec;
}

void GenericLLRPReader::sendDeleteROSpec() {
    sendROSpecControl(21, 10, DEFAULT_ROSPEC_ID);
    std::cout << "Sent DELETE_ROSPEC (ROSpecID=" << DEFAULT_ROSPEC_ID << ")\n";
}

// ────────────────────────────────────────────────────────────────────────────
// Lectura y decodificación de frames
// ────────────────────────────────────────────────────────────────────────────

std::vector<uint8_t> GenericLLRPReader::readSingleLLRPFrame() {
    uint8_t header[10];
    size_t received = 0;
    while (received < 10) {
        ssize_t n = ::recv(sockFd_, reinterpret_cast<char*>(header + received), static_cast<int>(10 - received), 0);
        if (n <= 0) {
            if (socket_would_block(socket_last_error()))
                throw std::runtime_error("read timeout");
            return {};
        }
        received += static_cast<size_t>(n);
    }

    uint32_t length =
        (static_cast<uint32_t>(header[2]) << 24) |
        (static_cast<uint32_t>(header[3]) << 16) |
        (static_cast<uint32_t>(header[4]) << 8)  |
         static_cast<uint32_t>(header[5]);

    if (length < 10 || length > 1024u * 1024u)
        throw std::runtime_error("Invalid LLRP frame length: " + std::to_string(length));

    std::vector<uint8_t> frame(length);
    std::memcpy(frame.data(), header, 10);

    received = 10;
    while (received < length) {
        ssize_t n = ::recv(sockFd_, reinterpret_cast<char*>(frame.data() + received), static_cast<int>(length - received), 0);
        if (n <= 0) {
            if (socket_would_block(socket_last_error()))
                throw std::runtime_error("read timeout");
            return {};
        }
        received += static_cast<size_t>(n);
    }
    return frame;
}

void GenericLLRPReader::handleReaderMessage(const std::vector<uint8_t> &frame) {
    // Usar el type registry global (inicializado en main) para decodificar
    CMessage *pMsg = decodeMessageFromFrame(g_pTypeRegistry, frame);

    if (!pMsg) {
        std::cout << "[GenericLLRPReader] Unknown message: " << frame.size() << " bytes\n";
        std::cout << bytesToHex(frame.data(), frame.size()) << "\n";
        return;
    }

    std::cout << "[GenericLLRPReader] Received " << pMsg->m_pType->m_pName << "\n";

    if (pMsg->m_pType == &CADD_ROSPEC_RESPONSE::s_typeDescriptor) {
        auto *r = dynamic_cast<CADD_ROSPEC_RESPONSE*>(pMsg);
        if (r && r->getLLRPStatus())
            std::cout << "[GenericLLRPReader] ADD_ROSPEC_RESPONSE status=" <<
                r->getLLRPStatus()->getStatusCode() << "\n";

    } else if (pMsg->m_pType == &CENABLE_ROSPEC_RESPONSE::s_typeDescriptor) {
        auto *r = dynamic_cast<CENABLE_ROSPEC_RESPONSE*>(pMsg);
        if (r && r->getLLRPStatus())
            std::cout << "[GenericLLRPReader] ENABLE_ROSPEC_RESPONSE status=" <<
                r->getLLRPStatus()->getStatusCode() << "\n";

    } else if (pMsg->m_pType == &CSTART_ROSPEC_RESPONSE::s_typeDescriptor) {
        auto *r = dynamic_cast<CSTART_ROSPEC_RESPONSE*>(pMsg);
        if (r && r->getLLRPStatus())
            std::cout << "[GenericLLRPReader] START_ROSPEC_RESPONSE status=" <<
                r->getLLRPStatus()->getStatusCode() << "\n";

    } else if (pMsg->m_pType == &CRO_ACCESS_REPORT::s_typeDescriptor) {
        auto *report = dynamic_cast<CRO_ACCESS_REPORT*>(pMsg);
        if (report) {
            size_t tagReports = 0;
            std::vector<std::string> reportJsonEvents;
            for (auto it = report->beginTagReportData(); it != report->endTagReportData(); ++it) {
                auto *td = *it;
                ++tagReports;
                const std::string epc = epcFromTagReport(td);
                const bool hasEpc = !epc.empty();

                std::string antenna  = td->getAntennaID()  ? std::to_string(td->getAntennaID()->getAntennaID())  : "N/A";
                std::string rssi     = td->getPeakRSSI()   ? std::to_string(td->getPeakRSSI()->getPeakRSSI())    : "N/A";
                std::string seenCnt  = td->getTagSeenCount()? std::to_string(td->getTagSeenCount()->getTagCount()): "N/A";

                reportJsonEvents.push_back(buildTagEventJson(td));

                // If EPC is absent/unsupported, still print the available report fields.
                if (!hasEpc) {
                    std::cout << "[GenericLLRPReader] Tag report without EPC"
                              << " antenna=" << antenna
                              << " rssi=" << rssi
                              << " seenCount=" << seenCnt << "\n";
                    continue;
                }

                if (shouldLogTag(epc))
                    std::cout << "[GenericLLRPReader] Tag detected EPC=" << epc
                              << " antenna=" << antenna
                              << " rssi=" << rssi
                              << " seenCount=" << seenCnt << "\n";
            }

            if (!reportJsonEvents.empty()) {
                std::ostringstream inv;
                inv << '{'
                    << "\"eventType\":\"RFID_INVENTORY\","
                    << "\"timestamp\":\"" << jsonEscape(nowIso8601Utc()) << "\","
                    << "\"readerVendorProfile\":\"IMPINJ_ZEBRA_STYLE\","
                    << "\"tags\":[";

                for (size_t i = 0; i < reportJsonEvents.size(); ++i) {
                    if (i) inv << ',';
                    inv << reportJsonEvents[i];
                }

                inv << "]}";
                std::cout << "[GenericLLRPReader][RFID_JSON_INVENTORY] " << inv.str() << "\n";
            }

            if (tagReports == 0) {
                std::cout << "[GenericLLRPReader] RO_ACCESS_REPORT without TagReportData\n";
            }
        }
    } else if (pMsg->m_pType == &CERROR_MESSAGE::s_typeDescriptor) {
        auto *err = dynamic_cast<CERROR_MESSAGE*>(pMsg);
        if (err && err->getLLRPStatus())
            std::cerr << "[GenericLLRPReader] Reader ERROR status="
                      << err->getLLRPStatus()->getStatusCode() << "\n";
    }

    delete pMsg;
}

// ────────────────────────────────────────────────────────────────────────────
// Helpers
// ────────────────────────────────────────────────────────────────────────────

int64_t GenericLLRPReader::nowMs() const {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

bool GenericLLRPReader::shouldLogTag(const std::string &epc) {
    int64_t now = nowMs();
    std::lock_guard<std::mutex> lk(lastLogMtx_);
    auto it = lastLoggedMs_.find(epc);
    if (it == lastLoggedMs_.end() || (now - it->second) >= minLogIntervalMs_) {
        lastLoggedMs_[epc] = now;
        return true;
    }
    return false;
}

std::string GenericLLRPReader::bytesToHex(const uint8_t *data, size_t len) {
    std::ostringstream oss;
    for (size_t i = 0; i < len; ++i)
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)data[i] << ' ';
    return oss.str();
}

} // namespace readers
