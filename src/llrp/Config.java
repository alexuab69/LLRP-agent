package llrp;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;

/**
 * Clase de configuración global para el proyecto LLRP-Agent.
 * Permite configurar parámetros como la IP del reader de manera centralizada.
 */
public class Config {

    // Variable global para la IP del reader
    public static String READER_IP = "169.254.116.164";

    // Puerto del reader
    public static int READER_PORT = 5084;

    static {
        loadConfig();
    }

    /**
     * Carga la configuración desde un archivo config.properties si existe.
     */
    private static void loadConfig() {
        Properties props = new Properties();
        String[] candidates = new String[]{
            "config.properties",
            "./config.properties",
            "../config.properties"
        };

        boolean loaded = false;
        for (String candidate : candidates) {
            File f = new File(candidate);
            if (!f.exists() || !f.isFile()) {
                continue;
            }
            try (FileInputStream fis = new FileInputStream(f)) {
                props.load(fis);
                loaded = true;
                System.out.println("Loaded config from: " + f.getAbsolutePath());
                break;
            } catch (IOException ignored) {
                // Try next location
            }
        }

        if (!loaded) {
            try (InputStream is = Config.class.getClassLoader().getResourceAsStream("config.properties")) {
                if (is != null) {
                    props.load(is);
                    loaded = true;
                    System.out.println("Loaded config from classpath resource config.properties");
                }
            } catch (IOException ignored) {
                // Use defaults below
            }
        }

        if (!loaded) {
            System.out.println("config.properties not found, using default values.");
            return;
        }

        String ip = props.getProperty("reader.ip");
        if (ip != null && !ip.isEmpty()) {
            READER_IP = ip;
        }

        String portStr = props.getProperty("reader.port");
        if (portStr != null && !portStr.isEmpty()) {
            try {
                READER_PORT = Integer.parseInt(portStr);
            } catch (NumberFormatException e) {
                System.err.println("Invalid port in config.properties: " + portStr);
            }
        }
    }
}