package reader;

import readers.RFIDReader;
import readers.ThingMagicM7EReader;
import readers.GenericLLRPReader;
import java.util.Timer;
import java.util.TimerTask;

/**
 * Adaptador principal que delega a implementaciones específicas de lectores
 * Arquitectura modular: el agente LLRP no sabe qué lector físico está usando
 */
public class ReaderAdapter {

    private RFIDReader realReader;
    private Timer inventoryTimer;
    private boolean inventoryRunning = false;

    public ReaderAdapter() {
        // Selección dinámica de reader:
        // 1) Si se define la propiedad del sistema `llrp.reader.type`, se usa esa.
        // 2) Si no, por defecto se usa el modo mock (permite ejecutar el agente sin hardware).
        String requested = System.getProperty("llrp.reader.type");
        if (requested == null || requested.isEmpty()) {
            requested = System.getenv("LLRP_READER_TYPE");
        }

        if (requested == null || requested.isEmpty()) {
            // Por defecto intentamos usar un reader LLRP genérico (para lectores que implementan LLRP)
            requested = "llrp";
        }

        setReaderType(requested);
    }

    /**
     * Método para cambiar dinámicamente el tipo de reader
     */
    public void setReaderType(String readerType) {
        // Desconectar reader actual
        if (realReader != null) {
            try {
                realReader.disconnect();
            } catch (Exception ignored) {
            }
        }

        // Crear nuevo reader según tipo
        RFIDReader candidate;
        switch (readerType.toLowerCase()) {
            case "thingmagic":
            case "m7e":
                candidate = new ThingMagicM7EReader();
                break;
            case "llrp":
                candidate = new GenericLLRPReader();
                break;
            case "mock":
            default:
                candidate = new MockRFIDReader();
                break;
        }

        // Intentar conectar; si falla, regresar a mock para que el agente siga funcionando
        try {
            candidate.connect();
            realReader = candidate;
        } catch (Exception e) {
            System.err.println("Could not connect using reader type '" + readerType + "': " + e.getMessage());
            System.err.println("Falling back to mock reader (no hardware connection)");
            realReader = new MockRFIDReader();
            try {
                realReader.connect();
            } catch (Exception ignored) {
                // no-op
            }
        }
    }

    public void startInventory() {
        try {
            if (realReader != null) {
                realReader.startInventory();
                inventoryRunning = true;

                // Iniciar timer para mostrar tags periódicamente
                inventoryTimer = new Timer();
                inventoryTimer.scheduleAtFixedRate(new TimerTask() {
                    @Override
                    public void run() {
                        String[] tags = readTags();
                        if (tags.length > 0) {
                            System.out.println("Tags detected: " + java.util.Arrays.toString(tags));
                        }
                    }
                }, 0, 2000); // cada 2 segundos
            }
        } catch (Exception e) {
            System.err.println("Failed to start inventory: " + e.getMessage());
        }
    }

    public void stopInventory() {
        try {
            if (realReader != null) {
                realReader.stopInventory();
                inventoryRunning = false;
            }

            if (inventoryTimer != null) {
                inventoryTimer.cancel();
                inventoryTimer = null;
            }
        } catch (Exception e) {
            System.err.println("Failed to stop inventory: " + e.getMessage());
        }
    }

    public String[] readTags() {
        if (realReader != null) {
            try {
                return realReader.readTags();
            } catch (Exception e) {
                System.err.println("Failed to read tags: " + e.getMessage());
                return new String[0];
            }
        }
        return new String[0];
    }

    public boolean isInventoryRunning() {
        return inventoryRunning && (realReader != null ? realReader.isInventoryRunning() : false);
    }

    /**
     * Implementación mock para cuando no hay lector real disponible
     */
    private class MockRFIDReader implements RFIDReader {
        @Override
        public void connect() throws Exception {
            System.out.println("Mock reader connected (no real hardware)");
        }

        @Override
        public void disconnect() {
            System.out.println("Mock reader disconnected");
        }

        @Override
        public void startInventory() throws Exception {
            System.out.println("Mock inventory started");
        }

        @Override
        public void stopInventory() throws Exception {
            System.out.println("Mock inventory stopped");
        }

        @Override
        public String[] readTags() {
            return new String[]{
                "E2000017221101441890B3AA",
                "E2000017221101441890B3AB"
            };
        }

        @Override
        public boolean isInventoryRunning() {
            return true;
        }

        @Override
        public void configureReader() throws Exception {
            System.out.println("Mock reader configured");
        }
    }
}