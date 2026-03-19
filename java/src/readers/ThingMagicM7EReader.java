package readers;

import com.thingmagic.*;
import java.util.ArrayList;
import java.util.List;

/**
 * Implementación específica para ThingMagic M7E usando Mercury API
 *
 * POR QUÉ USAR MERCURY API:
 * - Es el SDK oficial de ThingMagic
 * - Maneja automáticamente el protocolo de comunicación
 * - Incluye manejo de errores, timeouts, reconexiones
 * - Soporta todas las características avanzadas del lector
 * - Es la única forma documentada y soportada
 *
 * Comunicación directa serial sería:
 * - Propenso a errores de protocolo
 * - Difícil de mantener
 * - Sin soporte oficial
 * - Tendrías que implementar el protocolo ThingMagic manualmente
 */
public class ThingMagicM7EReader implements RFIDReader {

    private Reader reader;
    private boolean inventoryRunning = false;
    private List<String> detectedTags = new ArrayList<>();

    // URI de conexión para lector serial (Raspberry Pi). Cambia según tu sistema:
    // - Linux: tmr:///dev/ttyUSB0
    // - Windows: tmr://COM4
    // - Raspberry Pi con /dev/serial0: tmr:///dev/serial0
    private static final String SERIAL_PORT = "/dev/serial0";

    @Override
    public void connect() throws Exception {
        try {
            // Crear instancia del reader usando Mercury API
            // Se puede incluir el baudrate en la URI si el lector lo requiere
            String uri = "tmr:///" + SERIAL_PORT + "?baud=115200";
            reader = Reader.create(uri);

            // Conectar al lector
            reader.connect();

            System.out.println("Connected to ThingMagic M7E using URI: " + uri);

            // Configurar el reader
            configureReader();

        } catch (ReaderException e) {
            throw new Exception("Failed to connect to ThingMagic M7E: " + e.getMessage(), e);
        }
    }

    @Override
    public void disconnect() {
        try {
            if (reader != null) {
                stopInventory(); // Detener inventario antes de desconectar
                reader.destroy();
                reader = null;
                System.out.println("Disconnected from ThingMagic M7E");
            }
        } catch (Exception e) {
            System.err.println("Error disconnecting: " + e.getMessage());
        }
    }

    @Override
    public void configureReader() throws Exception {
        try {
            // Configuración básica del lector M7E
            // Potencia de transmisión (en dBm). En Mercury API se especifica en centésimas de dBm.
            // 2000 -> 20 dBm
            reader.paramSet("/reader/radio/readPower", 2000);

            // Potencia de escritura (para tags con EEPROM)
            reader.paramSet("/reader/radio/writePower", 2000);

            // Región de frecuencia (depende de tu ubicación)
            reader.paramSet("/reader/region/id", Reader.Region.NA); // Norteamérica
            // Opciones: EU, IN, JP, PRC, AU, NZ, etc.

            // Modo de sesión (para evitar lecturas duplicadas)
            reader.paramSet("/reader/gen2/session", Gen2.Session.S0);

            // Filtro duplicados
            reader.paramSet("/reader/tagReadData/uniqueByData", true);

            System.out.println("ThingMagic M7E configured successfully");

        } catch (ReaderException e) {
            throw new Exception("Failed to configure reader: " + e.getMessage(), e);
        }
    }

    @Override
    public void startInventory() throws Exception {
        if (inventoryRunning) {
            System.out.println("Inventory already running");
            return;
        }

        try {
            // Configurar listener para tags detectados
            reader.addReadListener(new ReadListener() {
                @Override
                public void tagRead(Reader reader, TagReadData tagData) {
                    String epc = tagData.getTag().epcString();
                    synchronized (detectedTags) {
                        if (!detectedTags.contains(epc)) {
                            detectedTags.add(epc);
                            System.out.println("Tag detected: " + epc +
                                             " (RSSI: " + tagData.getRssi() + " dBm)");
                        }
                    }
                }
            });

            // Configurar parámetros de lectura
            SimpleReadPlan readPlan = new SimpleReadPlan(
                new int[]{1, 2},     // Antennas 1 y 2
                TagProtocol.GEN2,    // Protocolo Gen2
                null,                // Filtro (null = todos los tags)
                null,                // Tagop (null = solo lectura)
                1000                 // Timeout por antenna en ms
            );

            reader.paramSet("/reader/read/plan", readPlan);

            // Iniciar lectura asíncrona
            reader.startReading();
            inventoryRunning = true;

            System.out.println("ThingMagic M7E inventory started");

        } catch (ReaderException e) {
            throw new Exception("Failed to start inventory: " + e.getMessage(), e);
        }
    }

    @Override
    public void stopInventory() throws Exception {
        if (!inventoryRunning) {
            System.out.println("Inventory not running");
            return;
        }

        // stopReading no lanza excepciones comprobadas en la API de Mercury,
        // por lo que no es necesario capturar ReaderException aquí.
        reader.stopReading();
        inventoryRunning = false;
        System.out.println("ThingMagic M7E inventory stopped");
    }

    @Override
    public String[] readTags() {
        synchronized (detectedTags) {
            String[] tags = detectedTags.toArray(new String[0]);
            detectedTags.clear(); // Limpiar después de leer
            return tags;
        }
    }

    @Override
    public boolean isInventoryRunning() {
        return inventoryRunning;
    }

    /**
     * Método adicional para configuración avanzada
     */
    public void setReadPower(int powerDbm) throws Exception {
        try {
            // El API utiliza centésimas de dBm (p.ej. 2000 = 20 dBm)
            reader.paramSet("/reader/radio/readPower", powerDbm * 100);
            System.out.println("Read power set to " + powerDbm + " dBm");
        } catch (ReaderException e) {
            throw new Exception("Failed to set read power: " + e.getMessage(), e);
        }
    }

    /**
     * Método adicional para cambiar región de frecuencia
     */
    public void setRegion(Reader.Region region) throws Exception {
        try {
            reader.paramSet("/reader/region/id", region);
            System.out.println("Region set to " + region);
        } catch (ReaderException e) {
            throw new Exception("Failed to set region: " + e.getMessage(), e);
        }
    }
}