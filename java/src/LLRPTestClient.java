import java.net.*;
import java.io.*;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.concurrent.atomic.AtomicBoolean;

public class LLRPTestClient {

    public static void main(String[] args) {

        String host = args.length > 0 ? args[0] : "127.0.0.1";
        int port = args.length > 1 ? Integer.parseInt(args[1]) : llrp.Config.READER_PORT;
        int durationSeconds = parseDurationSeconds(args, 10);

        try {
            Socket socket = new Socket(host, port);
            System.out.println("Connected to LLRP reader at " + host + ":" + port);
            System.out.println("Listening for responses for " + durationSeconds + " seconds");
            socket.setSoTimeout(5000); // 5s read timeout

            OutputStream out = socket.getOutputStream();
            InputStream in = socket.getInputStream();
            AtomicBoolean stopSent = new AtomicBoolean(false);

            Thread shutdownHook = new Thread(() -> {
                sendStopIfNeeded(out, stopSent, "Ctrl+C detected");
            }, "LLRPTestClient-ShutdownHook");
            Runtime.getRuntime().addShutdownHook(shutdownHook);

            // Enviar START_ROSPEC (tipo 22)
            byte[] startROSpec = createStartROSpecMessage();
            out.write(startROSpec);
            out.flush();
            System.out.println("Sent START_ROSPEC");

            // Leer respuestas/frames entrantes durante el tiempo configurado
            byte[] buffer = new byte[4096];
            long end = System.currentTimeMillis() + (durationSeconds * 1000L);
            while (System.currentTimeMillis() < end) {
                try {
                    if (in.available() > 0) {
                        int len = in.read(buffer);
                        if (len > 0) {
                            System.out.println("Received response: " + len + " bytes");
                            System.out.println(bytesToHex(buffer, len));
                        }
                    } else {
                        Thread.sleep(200);
                    }
                } catch (SocketTimeoutException ste) {
                    // Ignorar timeouts de lectura
                }
            }

            sendStopIfNeeded(out, stopSent, "Normal completion");

            socket.close();
            try {
                Runtime.getRuntime().removeShutdownHook(shutdownHook);
            } catch (IllegalStateException ignored) {
                // JVM is already shutting down; hook removal is not needed.
            }
            System.out.println("Disconnected");

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private static int parseDurationSeconds(String[] args, int defaultValue) {
        if (args.length > 2) {
            try {
                int parsed = Integer.parseInt(args[2]);
                return Math.max(parsed, 1);
            } catch (NumberFormatException ignored) {
                // Fallback to flag parsing below.
            }
        }

        for (String arg : args) {
            if (arg != null && arg.startsWith("--duration=")) {
                String value = arg.substring("--duration=".length());
                try {
                    int parsed = Integer.parseInt(value);
                    return Math.max(parsed, 1);
                } catch (NumberFormatException ignored) {
                    return defaultValue;
                }
            }
        }

        return defaultValue;
    }

    private static byte[] createStartROSpecMessage() {

        int version = 1;
        int type = 22;
        int length = 10 + 4; // header + rospecID (para START)
        int messageID = 1;
        int rospecID = 1;

        ByteBuffer buffer = ByteBuffer.allocate(length).order(ByteOrder.BIG_ENDIAN);
        int header = (version << 10) | type;
        buffer.putShort((short) header);
        buffer.putInt(length);
        buffer.putInt(messageID);
        buffer.putInt(rospecID);

        return buffer.array();
    }

    private static byte[] createStopROSpecMessage() {
        // Versión 1, Tipo 23 (STOP_ROSPEC), MessageID 2, ROSpecID 1
        int version = 1;
        int type = 23;
        int length = 14; // header + rospecID
        int messageID = 2;
        int rospecID = 1;

        ByteBuffer buffer = ByteBuffer.allocate(length).order(ByteOrder.BIG_ENDIAN);

        int header = (version << 10) | type;
        buffer.putShort((short) header);
        buffer.putInt(length);
        buffer.putInt(messageID);
        buffer.putInt(rospecID);

        return buffer.array();
    }

    private static void sendStopIfNeeded(OutputStream out, AtomicBoolean stopSent, String reason) {
        if (out == null || stopSent.getAndSet(true)) {
            return;
        }
        try {
            byte[] stopROSpec = createStopROSpecMessage();
            out.write(stopROSpec);
            out.flush();
            System.out.println("Sent STOP_ROSPEC (" + reason + ")");
        } catch (Exception e) {
            System.out.println("Could not send STOP_ROSPEC during shutdown: " + e.getMessage());
        }
    }

    private static String bytesToHex(byte[] bytes, int len) {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < len; i++) {
            sb.append(String.format("%02X ", bytes[i]));
        }
        return sb.toString();
    }
}