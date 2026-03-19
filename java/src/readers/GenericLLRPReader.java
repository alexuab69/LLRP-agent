package readers;

import java.net.*;
import java.io.*;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import llrp.Config;
import org.llrp.ltk.generated.LLRPMessageFactory;
import org.llrp.ltk.generated.interfaces.EPCParameter;
import org.llrp.ltk.generated.messages.ADD_ROSPEC;
import org.llrp.ltk.generated.messages.ADD_ROSPEC_RESPONSE;
import org.llrp.ltk.generated.messages.ERROR_MESSAGE;
import org.llrp.ltk.generated.messages.ENABLE_ROSPEC_RESPONSE;
import org.llrp.ltk.generated.messages.READER_EVENT_NOTIFICATION;
import org.llrp.ltk.generated.messages.RO_ACCESS_REPORT;
import org.llrp.ltk.generated.messages.START_ROSPEC_RESPONSE;
import org.llrp.ltk.generated.enumerations.AISpecStopTriggerType;
import org.llrp.ltk.generated.enumerations.AirProtocols;
import org.llrp.ltk.generated.enumerations.ROReportTriggerType;
import org.llrp.ltk.generated.enumerations.ROSpecStartTriggerType;
import org.llrp.ltk.generated.enumerations.ROSpecState;
import org.llrp.ltk.generated.enumerations.ROSpecStopTriggerType;
import org.llrp.ltk.generated.parameters.AISpec;
import org.llrp.ltk.generated.parameters.AISpecStopTrigger;
import org.llrp.ltk.generated.parameters.LLRPStatus;
import org.llrp.ltk.generated.parameters.ROBoundarySpec;
import org.llrp.ltk.generated.parameters.ROReportSpec;
import org.llrp.ltk.generated.parameters.ROSpec;
import org.llrp.ltk.generated.parameters.ROSpecStartTrigger;
import org.llrp.ltk.generated.parameters.ROSpecStopTrigger;
import org.llrp.ltk.generated.parameters.TagReportContentSelector;
import org.llrp.ltk.generated.parameters.EPCData;
import org.llrp.ltk.generated.parameters.EPC_96;
import org.llrp.ltk.generated.parameters.InventoryParameterSpec;
import org.llrp.ltk.generated.parameters.TagReportData;
import org.llrp.ltk.types.Bit;
import org.llrp.ltk.types.LLRPMessage;
import org.llrp.ltk.types.UnsignedByte;
import org.llrp.ltk.types.UnsignedInteger;
import org.llrp.ltk.types.UnsignedShort;
import org.llrp.ltk.types.UnsignedShortArray;

/**
 * Generic LLRP reader client using simple socket calls and minimal LLRP messages
 * This implementation uses the LLRP Toolkit if available (classpath), otherwise
 * it falls back to a minimal ADD/ENABLE/START sequence built manually.
 *
 * Nota: Para compatibilidad máxima con lectores reales se recomienda tener
 * `llrp-toolkit.jar` (ltkjava) en el classpath. Esta clase intenta una
 * estrategia sencilla que funciona con muchos lectores que aceptan ADD_ROSPEC
 * básico.
 */
public class GenericLLRPReader implements RFIDReader {

    private String host = Config.READER_IP;
    private int port = Config.READER_PORT;
    private Socket socket;
    private DataInputStream in;
    private DataOutputStream out;
    private volatile boolean inventoryRunning = false;
    private Thread readerThread;
    private static final int DEFAULT_ROSPEC_ID = 1;
    private final Map<String, Long> lastLoggedAtMsByEpc = new HashMap<>();
    private long minLogIntervalMs = 1500L;

    public GenericLLRPReader() {
        // default constructor
        this.host = Config.READER_IP;
        this.port = Config.READER_PORT;

        String envHost = System.getProperty("llrp.reader.host");
        if (envHost == null || envHost.isEmpty()) {
            envHost = System.getenv("LLRP_READER_HOST");
        }
        if (envHost != null && !envHost.isEmpty()) {
            this.host = envHost;
        }

        String envPort = System.getProperty("llrp.reader.port");
        if (envPort == null || envPort.isEmpty()) {
            envPort = System.getenv("LLRP_READER_PORT");
        }
        if (envPort != null && !envPort.isEmpty()) {
            try { this.port = Integer.parseInt(envPort); } catch (Exception ignored) {}
        }

        String intervalMs = System.getProperty("llrp.tag.log.minIntervalMs");
        if (intervalMs == null || intervalMs.isEmpty()) {
            intervalMs = System.getenv("LLRP_TAG_LOG_MIN_INTERVAL_MS");
        }
        if (intervalMs != null && !intervalMs.isEmpty()) {
            try {
                minLogIntervalMs = Math.max(0L, Long.parseLong(intervalMs));
            } catch (NumberFormatException ignored) {
                // Keep default value.
            }
        }
    }

    @Override
    public void connect() throws Exception {
        socket = new Socket();
        socket.connect(new InetSocketAddress(host, port), 10000);
        socket.setSoTimeout(5000);
        in = new DataInputStream(socket.getInputStream());
        out = new DataOutputStream(socket.getOutputStream());
        System.out.println("GenericLLRPReader connected to " + host + ":" + port);
    }

    @Override
    public void disconnect() {
        try {
            if (readerThread != null && readerThread.isAlive()) {
                readerThread.interrupt();
            }
            if (socket != null) socket.close();
            System.out.println("GenericLLRPReader disconnected");
        } catch (Exception e) {
            // ignore
        }
    }

    @Override
    public void startInventory() throws Exception {
        synchronized (lastLoggedAtMsByEpc) {
            lastLoggedAtMsByEpc.clear();
        }
        sendDefaultAddROSpec();
        sendSimpleEnableROSpec();
        sendSimpleStartROSpec();

        inventoryRunning = true;

        // Read full LLRP frames and decode RO_ACCESS_REPORT entries.
        readerThread = new Thread(() -> {
            try {
                while (inventoryRunning && !Thread.currentThread().isInterrupted()) {
                    try {
                        byte[] frame = readSingleLLRPFrame();
                        if (frame == null) {
                            break;
                        }
                        handleReaderMessage(frame);
                    } catch (SocketTimeoutException ste) {
                        // No message in this interval; keep waiting.
                    }
                }
            } catch (Exception e) {
                System.err.println("Reader thread error: " + e.getMessage());
            }
        });
        readerThread.setName("GenericLLRPReader-Thread");
        readerThread.start();
    }

    @Override
    public void stopInventory() throws Exception {
        inventoryRunning = false;
        sendSimpleStopROSpec();
        if (readerThread != null) {
            readerThread.interrupt();
            readerThread = null;
        }
    }

    @Override
    public String[] readTags() {
        // Este reader no mantiene una caché de tags; usar logs en consola.
        return new String[0];
    }

    @Override
    public boolean isInventoryRunning() {
        return inventoryRunning;
    }

    @Override
    public void configureReader() throws Exception {
        // No-op for now
    }

    // Mensajes LLRP simplificados (solo headers o con payload mínimo)
    private void sendSimpleStartROSpec() throws IOException {
        byte[] msg = createROSpecControlMessage(22, 1, DEFAULT_ROSPEC_ID);
        out.write(msg);
        out.flush();
        System.out.println("Sent START_ROSPEC (ROSpecID=" + DEFAULT_ROSPEC_ID + ")");
    }

    private void sendSimpleStopROSpec() throws IOException {
        byte[] msg = createROSpecControlMessage(23, 2, DEFAULT_ROSPEC_ID);
        out.write(msg);
        out.flush();
        System.out.println("Sent STOP_ROSPEC (ROSpecID=" + DEFAULT_ROSPEC_ID + ")");
    }

    private void sendSimpleEnableROSpec() throws IOException {
        byte[] msg = createROSpecControlMessage(24, 3, DEFAULT_ROSPEC_ID);
        out.write(msg);
        out.flush();
        System.out.println("Sent ENABLE_ROSPEC (ROSpecID=" + DEFAULT_ROSPEC_ID + ")");
    }

    private void sendDefaultAddROSpec() throws Exception {
        ADD_ROSPEC addRospec = new ADD_ROSPEC();
        addRospec.setMessageID(new UnsignedInteger(4));
        addRospec.setROSpec(buildDefaultROSpec(DEFAULT_ROSPEC_ID));

        byte[] msg = addRospec.encodeBinary();
        out.write(msg);
        out.flush();
        System.out.println("Sent ADD_ROSPEC (valid ROSpecID=" + DEFAULT_ROSPEC_ID + ")");
    }

    private ROSpec buildDefaultROSpec(int rospecId) {
        ROSpec roSpec = new ROSpec();
        roSpec.setROSpecID(new UnsignedInteger(rospecId));
        roSpec.setPriority(new UnsignedByte(0));
        roSpec.setCurrentState(new ROSpecState(ROSpecState.Disabled));

        ROBoundarySpec boundary = new ROBoundarySpec();
        ROSpecStartTrigger startTrigger = new ROSpecStartTrigger();
        startTrigger.setROSpecStartTriggerType(new ROSpecStartTriggerType(ROSpecStartTriggerType.Null));
        ROSpecStopTrigger stopTrigger = new ROSpecStopTrigger();
        stopTrigger.setROSpecStopTriggerType(new ROSpecStopTriggerType(ROSpecStopTriggerType.Null));
        stopTrigger.setDurationTriggerValue(new UnsignedInteger(0));
        boundary.setROSpecStartTrigger(startTrigger);
        boundary.setROSpecStopTrigger(stopTrigger);
        roSpec.setROBoundarySpec(boundary);

        AISpec aiSpec = new AISpec();
        UnsignedShortArray antennaIds = new UnsignedShortArray();
        antennaIds.add(new UnsignedShort(1));
        aiSpec.setAntennaIDs(antennaIds);

        AISpecStopTrigger aiStopTrigger = new AISpecStopTrigger();
        aiStopTrigger.setAISpecStopTriggerType(new AISpecStopTriggerType(AISpecStopTriggerType.Null));
        aiStopTrigger.setDurationTrigger(new UnsignedInteger(0));
        aiSpec.setAISpecStopTrigger(aiStopTrigger);

        InventoryParameterSpec inventorySpec = new InventoryParameterSpec();
        inventorySpec.setInventoryParameterSpecID(new UnsignedShort(1));
        inventorySpec.setProtocolID(new AirProtocols(AirProtocols.EPCGlobalClass1Gen2));
        aiSpec.addToInventoryParameterSpecList(inventorySpec);

        roSpec.addToSpecParameterList(aiSpec);

        ROReportSpec reportSpec = new ROReportSpec();
        reportSpec.setROReportTrigger(new ROReportTriggerType(ROReportTriggerType.Upon_N_Tags_Or_End_Of_ROSpec));
        reportSpec.setN(new UnsignedShort(1));

        TagReportContentSelector selector = new TagReportContentSelector();
        selector.setEnableROSpecID(new Bit(1));
        selector.setEnableSpecIndex(new Bit(0));
        selector.setEnableAntennaID(new Bit(1));
        selector.setEnableChannelIndex(new Bit(0));
        selector.setEnablePeakRSSI(new Bit(1));
        selector.setEnableFirstSeenTimestamp(new Bit(0));
        selector.setEnableLastSeenTimestamp(new Bit(0));
        selector.setEnableTagSeenCount(new Bit(1));
        selector.setEnableInventoryParameterSpecID(new Bit(1));
        selector.setEnableAccessSpecID(new Bit(0));
        reportSpec.setTagReportContentSelector(selector);

        roSpec.setROReportSpec(reportSpec);
        return roSpec;
    }

    private byte[] createROSpecControlMessage(int type, int messageID, int roSpecID) {
        int version = 1;
        int length = 14;
        ByteBuffer buffer = ByteBuffer.allocate(length).order(ByteOrder.BIG_ENDIAN);
        int header = (version << 10) | type;
        buffer.putShort((short) header);
        buffer.putInt(length);
        buffer.putInt(messageID);
        buffer.putInt(roSpecID);
        return buffer.array();
    }

    private byte[] readSingleLLRPFrame() throws IOException {
        byte[] header = new byte[10];
        in.readFully(header);

        int length =
                ((header[2] & 0xFF) << 24) |
                ((header[3] & 0xFF) << 16) |
                ((header[4] & 0xFF) << 8) |
                (header[5] & 0xFF);

        if (length < 10 || length > 1024 * 1024) {
            throw new IOException("Invalid LLRP frame length: " + length);
        }

        byte[] frame = new byte[length];
        System.arraycopy(header, 0, frame, 0, 10);
        if (length > 10) {
            in.readFully(frame, 10, length - 10);
        }
        return frame;
    }

    private void handleReaderMessage(byte[] frame) {
        try {
            LLRPMessage message = LLRPMessageFactory.createLLRPMessage(frame);
            if (message == null) {
                System.out.println("[GenericLLRPReader] Unknown message: " + frame.length + " bytes");
                System.out.println(bytesToHex(frame, frame.length));
                return;
            }

            System.out.println("[GenericLLRPReader] Received " + message.getName());

            if (message instanceof RO_ACCESS_REPORT) {
                logTagReport((RO_ACCESS_REPORT) message);
            } else if (message instanceof ERROR_MESSAGE) {
                logErrorMessage((ERROR_MESSAGE) message);
            } else if (message instanceof READER_EVENT_NOTIFICATION) {
                logReaderEvent((READER_EVENT_NOTIFICATION) message);
            } else if (message instanceof ENABLE_ROSPEC_RESPONSE) {
                logStatus("ENABLE_ROSPEC_RESPONSE", ((ENABLE_ROSPEC_RESPONSE) message).getLLRPStatus());
            } else if (message instanceof START_ROSPEC_RESPONSE) {
                logStatus("START_ROSPEC_RESPONSE", ((START_ROSPEC_RESPONSE) message).getLLRPStatus());
            } else if (message instanceof ADD_ROSPEC_RESPONSE) {
                logStatus("ADD_ROSPEC_RESPONSE", ((ADD_ROSPEC_RESPONSE) message).getLLRPStatus());
            }
        } catch (Exception e) {
            System.out.println("[GenericLLRPReader] Could not decode reader frame (" + frame.length + " bytes)");
            System.out.println(bytesToHex(frame, frame.length));
        }
    }

    private void logTagReport(RO_ACCESS_REPORT report) {
        List<TagReportData> reportDataList = report.getTagReportDataList();
        if (reportDataList == null || reportDataList.isEmpty()) {
            System.out.println("[GenericLLRPReader] RO_ACCESS_REPORT with no tags");
            return;
        }

        for (TagReportData tagData : reportDataList) {
            String epc = extractEpc(tagData.getEPCParameter());
            String antenna = tagData.getAntennaID() != null ? tagData.getAntennaID().getAntennaID().toString() : "N/A";
            String rssi = tagData.getPeakRSSI() != null ? tagData.getPeakRSSI().getPeakRSSI().toString() : "N/A";
            String seenCount = tagData.getTagSeenCount() != null ? tagData.getTagSeenCount().getTagCount().toString() : "N/A";

            if (shouldLogTag(epc)) {
                System.out.println("[GenericLLRPReader] Tag detected EPC=" + epc + " antenna=" + antenna + " rssi=" + rssi + " seenCount=" + seenCount);
            }
        }
    }

    private boolean shouldLogTag(String epc) {
        long now = System.currentTimeMillis();
        synchronized (lastLoggedAtMsByEpc) {
            Long last = lastLoggedAtMsByEpc.get(epc);
            if (last == null || (now - last) >= minLogIntervalMs) {
                lastLoggedAtMsByEpc.put(epc, now);
                return true;
            }
        }
        return false;
    }

    private String extractEpc(EPCParameter epcParameter) {
        if (epcParameter == null) {
            return "UNKNOWN";
        }
        if (epcParameter instanceof EPC_96) {
            EPC_96 epc96 = (EPC_96) epcParameter;
            return epc96.getEPC() != null ? epc96.getEPC().toString() : "UNKNOWN";
        }
        if (epcParameter instanceof EPCData) {
            EPCData epcData = (EPCData) epcParameter;
            return epcData.getEPC() != null ? epcData.getEPC().toString() : "UNKNOWN";
        }
        return epcParameter.toString();
    }

    private void logErrorMessage(ERROR_MESSAGE errorMessage) {
        LLRPStatus status = errorMessage.getLLRPStatus();
        if (status == null) {
            System.out.println("[GenericLLRPReader] ERROR_MESSAGE without status payload");
            return;
        }

        String statusCode = status.getStatusCode() != null ? status.getStatusCode().toString() : "UNKNOWN";
        String description = status.getErrorDescription() != null ? status.getErrorDescription().toString() : "(no description)";
        String fieldError = status.getFieldError() != null ? status.getFieldError().toString() : "none";
        String parameterError = status.getParameterError() != null ? status.getParameterError().toString() : "none";

        System.out.println("[GenericLLRPReader] Reader ERROR status=" + statusCode + " description=" + description + " fieldError=" + fieldError + " parameterError=" + parameterError);
    }

    private void logReaderEvent(READER_EVENT_NOTIFICATION event) {
        if (event.getReaderEventNotificationData() == null) {
            System.out.println("[GenericLLRPReader] READER_EVENT_NOTIFICATION without data");
            return;
        }
        System.out.println("[GenericLLRPReader] Reader event: " + event.getReaderEventNotificationData().toString());
    }

    private void logStatus(String label, LLRPStatus status) {
        if (status == null) {
            System.out.println("[GenericLLRPReader] " + label + " status=(null)");
            return;
        }
        String statusCode = status.getStatusCode() != null ? status.getStatusCode().toString() : "UNKNOWN";
        String description = status.getErrorDescription() != null ? status.getErrorDescription().toString() : "";
        System.out.println("[GenericLLRPReader] " + label + " status=" + statusCode + " description=" + description);
    }

    private static String bytesToHex(byte[] bytes, int len) {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < len; i++) {
            sb.append(String.format("%02X ", bytes[i]));
        }
        return sb.toString();
    }
}
