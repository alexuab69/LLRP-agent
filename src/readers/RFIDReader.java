package readers;

/**
 * Interfaz base para todos los lectores RFID
 * Arquitectura modular que permite cambiar de lector sin modificar el agente LLRP
 */
public interface RFIDReader {

    /**
     * Conecta al lector
     * @throws Exception si falla la conexión
     */
    void connect() throws Exception;

    /**
     * Desconecta del lector
     */
    void disconnect();

    /**
     * Inicia el inventario de tags
     * @throws Exception si falla el inicio
     */
    void startInventory() throws Exception;

    /**
     * Detiene el inventario de tags
     * @throws Exception si falla la detención
     */
    void stopInventory() throws Exception;

    /**
     * Lee tags detectados
     * @return Array de EPCs detectados
     */
    String[] readTags();

    /**
     * Verifica si el inventario está activo
     * @return true si está leyendo tags
     */
    boolean isInventoryRunning();

    /**
     * Configura parámetros del lector (potencia, frecuencia, etc.)
     */
    void configureReader() throws Exception;
}