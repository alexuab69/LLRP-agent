/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package samples;
import com.thingmagic.*;

/**
 *
 * @author rsoni
 */
public class SavedConfig {


  static SerialPrinter serialPrinter;
  static StringPrinter stringPrinter;
  static TransportListener currentListener;

  static void usage()
  {
    System.out.printf("Usage: Please provide valid arguments, such as:\n"
                + "SavedConfig [-v] [reader-uri] \n" +
                  "-v  Verbose: Turn on transport listener\n" +
                  "reader-uri  Reader URI: e.g., \"tmr:///COM1\", \"tmr://astra-2100d3\"\n" 
                );
    System.exit(1);
  }
  
   public static void setTrace(Reader r, String args[])
  {
    if (args[0].toLowerCase().equals("on"))
    {
        r.addTransportListener(Reader.simpleTransportListener);
        currentListener = Reader.simpleTransportListener;
    }
    else if (currentListener != null)
    {
        r.removeTransportListener(Reader.simpleTransportListener);
    }
  }
   
   static class SerialPrinter implements TransportListener
  {
    public void message(boolean tx, byte[] data, int timeout)
    {
      System.out.print(tx ? "Sending: " : "Received:");
      for (int i = 0; i < data.length; i++)
      {
        if (i > 0 && (i & 15) == 0)
          System.out.printf("\n         ");
        System.out.printf(" %02x", data[i]);
      }
      System.out.printf("\n");
    }
  }

  static class StringPrinter implements TransportListener
  {
    public void message(boolean tx, byte[] data, int timeout)
    {
      System.out.println((tx ? "Sending:\n" : "Receiving:\n") +
                         new String(data));
    }
  }


  public static void main(String argv[]) throws ReaderException
  {
    // Program setup
    Reader r = null;
    int nextarg = 0;
    boolean trace = false;

    if (argv.length < 1)
      usage();

    if (argv[nextarg].equals("-v"))
    {
      trace = true;
      nextarg++;
    }
      
    try
    {
      r = Reader.create(argv[nextarg]);
      if (trace)
      {
            setTrace(r, new String[]{"on"});
      }
      try
      {
        /* MercuryAPI tries connecting to the module using default baud rate of 115200 bps.
         * The connection may fail if the module is configured to a different baud rate. If
         * that is the case, the MercuryAPI tries connecting to the module with other supported
         * baud rates until the connection is successful using baud rate probing mechanism.
         */
        r.connect();
      }
      catch (Exception ex)
      {
          if((ex.getMessage().contains("Timeout")) && (r instanceof SerialReader))
          {
              // Default baudrate connect failed. Try probing through the baudrate list
              // to retrieve the module baudrate
              int currentBaudRate = ((SerialReader)r).probeBaudRate();
              //Set the current baudrate so that next connect will use this baudrate.
              r.paramSet("/reader/baudRate", currentBaudRate);
              // Now connect with current baudrate
              r.connect();
          }
          else
          {
              throw new Exception(ex.getMessage().toString());
          }
      }
     
      String model = (String) r.paramGet("/reader/version/model");
      {
        if (Reader.Region.UNSPEC == (Reader.Region)r.paramGet("/reader/region/id"))
        {
            Reader.Region[] supportedRegions = (Reader.Region[])r.paramGet(TMConstants.TMR_PARAM_REGION_SUPPORTEDREGIONS);
            if (supportedRegions.length < 1)
            {
                  throw new Exception("Reader doesn't support any regions");
            }
            else
            {
                r.paramSet("/reader/region/id", supportedRegions[0]);
            }
        }
        // Set the Protocol
        if (model.equalsIgnoreCase("M3e"))
        {
            r.paramSet("/reader/tagop/protocol", TagProtocol.ISO14443A);
        }
        else
        {
            r.paramSet("/reader/tagop/protocol", TagProtocol.GEN2);
        }
        // Set the User Configuration
        r.paramSet("/reader/userConfig", new SerialReader.UserConfigOp(SerialReader.SetUserProfileOption.SAVE));
        System.out.println("User profile set option:save all configuration");

        r.paramSet("/reader/userConfig", new SerialReader.UserConfigOp(SerialReader.SetUserProfileOption.RESTORE));
        System.out.println("User profile set option:restore all configuration");

        r.paramSet("/reader/userConfig", new SerialReader.UserConfigOp(SerialReader.SetUserProfileOption.VERIFY));
        System.out.println("User profile set option:verify all configuration");

        // Get User Profile

        Object region = r.paramGet("/reader/region/id");
        System.out.println("Get user profile success option:Region");
        System.out.println(region.toString());


        Object proto = r.paramGet("/reader/tagop/protocol");
        System.out.println("Get user profile success option:Protocol");
        System.out.println(proto.toString());

        System.out.println("Get user profile success option:Baudrate");
        System.out.println(r.paramGet("/reader/baudRate").toString());

        //reset all the configurations
        r.paramSet("/reader/userConfig", new SerialReader.UserConfigOp(SerialReader.SetUserProfileOption.CLEAR));
        System.out.println("User profile set option:reset all configuration");
        
        if (Reader.Region.UNSPEC == (Reader.Region) r.paramGet("/reader/region/id"))
        {
            Reader.Region[] supportedRegions = (Reader.Region[]) r.paramGet(TMConstants.TMR_PARAM_REGION_SUPPORTEDREGIONS);
            if (supportedRegions.length < 1)
            {
                throw new Exception("Reader doesn't support any regions");
            }
            else
            {
                r.paramSet("/reader/region/id", supportedRegions[0]);
            }
        }
        
        if(r.paramGet("/reader/tagop/protocol") == null)
        {
            if (model.equalsIgnoreCase("M3e"))
            {
               r.paramSet("/reader/tagop/protocol", TagProtocol.ISO14443A);               
            }
            else
            {    
               r.paramSet("/reader/tagop/protocol", TagProtocol.GEN2);
            }
        }
      }
      r.destroy();
    }
    catch (ReaderException re)
    {
      System.out.println("ReaderException: " + re.getMessage());
    }
    catch (Exception re)
    {
        System.out.println("Exception: " + re.getMessage());
    }
  }
}
