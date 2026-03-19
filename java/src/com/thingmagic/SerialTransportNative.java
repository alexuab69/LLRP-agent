package com.thingmagic;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * A small wrapper around the ThingMagic native transport library that allows
 * overriding the native library location via system properties.
 *
 * This class is intended to replace the version bundled inside mercuryapi.jar
 * so we can run on platforms that are not pre-packaged (e.g. ARM64 Linux).
 */
public class SerialTransportNative implements SerialTransport {

  private static native int nativeInit();
  private native int nativeCreate(String port);
  private native int nativeOpen();
  private native int nativeSendBytes(int length, byte[] message, int offset,
                                     int timeoutMs);
  private native int nativeReceiveBytes(int length, byte[] message, int offset,
                                        int timeoutMs);
  private native int nativeSetBaudRate(int rate);
  private native int nativeGetBaudRate();
  private native int nativeShutdown();
  private native int nativeFlush();

  static {
    load();
    nativeInit();
  }

  SerialTransportNative() {
    // default constructor
  }

  private static void load() {
    // Allow overriding the native library by setting system properties.
    // Example:
    //   -Dcom.thingmagic.seriallib.path=/path/to/so
    //   -Dcom.thingmagic.seriallib.name=libSerialTransportNative.so.0
    String libpath = System.getProperty("com.thingmagic.seriallib.path");
    String libname = System.getProperty("com.thingmagic.seriallib.name");

    // Default name (used only if not overridden)
    if (libname == null || libname.isEmpty()) {
      String osname = System.getProperty("os.name").toLowerCase();
      String osarch = System.getProperty("os.arch");

      if (osname.startsWith("mac os")) {
        osname = "mac";
        osarch = "universal";
      }
      if (osname.startsWith("windows")) {
        osname = "win";
        if (osarch.startsWith("a") && osarch.endsWith("64")) {
          osarch = "x64";
        }
      }
      if (osarch.startsWith("i") && osarch.endsWith("86")) {
        osarch = "x86";
      }

      libname = osname + '-' + osarch + ".lib";
    }

    // If a specific file path is provided, try to load it directly.
    if (libpath != null && !libpath.isEmpty()) {
      File f = new File(libpath);
      if (f.isDirectory()) {
        f = new File(f, libname);
      }
      if (f.exists()) {
        System.load(f.getAbsolutePath());
        return;
      }
      // Allow specifying an explicit full path in the name property;
      // try that too.
      if (libname.contains(File.separator)) {
        File f2 = new File(libname);
        if (f2.exists()) {
          System.load(f2.getAbsolutePath());
          return;
        }
      }
    }

    // Fall back to the bundled native library inside the JAR.
    try {
      InputStream in = SerialTransportNative.class.getResourceAsStream(libname);
      if (in == null) {
        // Provide a better error message for missing libs
        throw new RuntimeException("libname: " + libname + " not found. " +
            "Either build/ship the native library for this platform or set " +
            "-Dcom.thingmagic.seriallib.path=<dir> and -Dcom.thingmagic.seriallib.name=<file>.");
      }

      File tmplib = File.createTempFile("libtmserialport", ".lib");
      tmplib.deleteOnExit();
      try (OutputStream out = new FileOutputStream(tmplib)) {
        byte[] buf = new byte[1024];
        for (int len; (len = in.read(buf)) != -1;) {
          out.write(buf, 0, len);
        }
      }

      System.load(tmplib.getAbsolutePath());
    } catch (IOException e) {
      throw new RuntimeException("Error loading " + libname, e);
    }
  }

  /* Used by the native code to point to its state object. Don't touch. */
  public long transportPtr = 0;

  private boolean opened = false;
  private String deviceName;

  /** Creates a new instance of SerialPort */
  public SerialTransportNative(String deviceName) throws ReaderException {
    nativeCreate(sanitizeComPortName(deviceName));
  }

  private String sanitizeComPortName(String portName) {
    if (portName.toUpperCase().startsWith("COM")) {
      portName = "\\\\.\\" + portName;
    } else if (portName.toUpperCase().startsWith("/COM")) {
      portName = "\\\\.\\" + portName.substring(1);
    }
    return portName;
  }

  public void open() throws ReaderException {
    if (!opened) {
      if (0 != nativeOpen()) {
        throw new ReaderCommException("Couldn't open device");
      }
      opened = true;
    }
  }

  public void shutdown() throws ReaderException {
    if (opened) {
      nativeShutdown();
      opened = false;
    }
  }

  public void flush() throws ReaderException {
    if (0 != nativeFlush()) {
      throw new ReaderCommException("Couldn't flush device");
    }
  }

  @Override
  public void sendBytes(int length, byte[] message, int offset, int timeoutMs)
      throws ReaderException {
    int ret = nativeSendBytes(length, message, offset, timeoutMs);
    if (ret != 0) {
      throw new ReaderCommException("Failed to send bytes");
    }
  }

  @Override
  public byte[] receiveBytes(int length, byte[] message, int offset, int timeoutMs)
      throws ReaderException {
    int ret = nativeReceiveBytes(length, message, offset, timeoutMs);
    if (ret != 0) {
      throw new ReaderCommException("Failed to receive bytes");
    }
    return message;
  }

  public void setBaudRate(int rate) throws ReaderException {
    if (0 != nativeSetBaudRate(rate)) {
      throw new ReaderCommException("Failed to set baud rate");
    }
  }

  public int getBaudRate() throws ReaderException {
    int ret = nativeGetBaudRate();
    if (ret < 0) {
      throw new ReaderCommException("Failed to get baud rate");
    }
    return ret;
  }
}
