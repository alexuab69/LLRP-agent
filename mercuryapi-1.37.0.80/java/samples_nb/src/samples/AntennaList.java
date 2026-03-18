/*
 * Sample program that shows how to create a
 * simpleReadPlan that uses a list of antennas as passed by the user
 * and prints the tags found.
 */
package samples;

import com.thingmagic.Reader;
import com.thingmagic.SerialReader;
import com.thingmagic.SimpleReadPlan;
import com.thingmagic.TMConstants;
import com.thingmagic.TagProtocol;
import com.thingmagic.TagReadData;
import com.thingmagic.TransportListener;

/**
 *
 * @author qvantel
 */
public class AntennaList
{
    static void usage()
    {
        System.out.printf("Usage: Please provide valid arguments, such as:\n"
                + "AntennaList [-v] [reader-uri] [--ant n[,n...]] \n" +
                  "-v  Verbose: Turn on transport listener\n" +
                  "reader-uri  Reader URI: e.g., \"tmr:///COM1\", \"tmr://astra-2100d3\"\n"
                + "--ant  Antenna List: e.g., \"--ant 1\", \"--ant 1,2\"\n"
                + "e.g: tmr:///com1 --ant 1,2 ; tmr://10.11.115.32 --ant 1,2\n ");
        System.exit(1);
    }

    public static void setTrace(Reader r, String args[])
    {
        if (args[0].toLowerCase().equals("on"))
        {
            r.addTransportListener(r.simpleTransportListener);
        }
    }

    static class StringPrinter implements TransportListener
    {
        public void message(boolean tx, byte[] data, int timeout)
        {
            System.out.println((tx ? "Sending:\n" : "Receiving:\n")
                    + new String(data));
        }
    }

    static class SerialPrinter implements TransportListener
    {
        public void message(boolean tx, byte[] data, int timeout)
        {
            System.out.print(tx ? "Sending: " : "Received:");
            for (int i = 0; i < data.length; i++)
            {
                if (i > 0 && (i & 15) == 0) {
                    System.out.printf("\n         ");
                }
                System.out.printf(" %02x", data[i]);
            }
            System.out.printf("\n");
        }
    }

    public static void main(String argv[])
    {
        Reader r = null;
        int[] antennaList = null;
        TagReadData[] tagReads;
        int nextarg = 0;
        boolean trace = false;
        
        if (argv.length < 1)
        {
            usage();
        }

        if (argv[nextarg].equals("-v"))
        {
            trace = true;
            nextarg++;
        }

        try
        {
            String readerURI = argv[nextarg];
            nextarg++;
            
            for ( ; nextarg < argv.length; nextarg++)
            {
                String arg = argv[nextarg];
                if (arg.equalsIgnoreCase("--ant"))
                {
                    if (antennaList != null)
                    {
                        System.out.println("Duplicate argument: --ant specified more than once");
                        usage();
                    }
                    antennaList = parseAntennaList(argv, nextarg);
                    nextarg++;
                }
                else
                {
                    System.out.println("Argument "+argv[nextarg] +" is not recognised");
                    usage();
                }
            }
            
            r = Reader.create(readerURI);
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
            if (Reader.Region.UNSPEC == (Reader.Region) r.paramGet(TMConstants.TMR_PARAM_REGION_ID))
            {
                Reader.Region[] supportedRegions = (Reader.Region[]) r.paramGet(TMConstants.TMR_PARAM_REGION_SUPPORTEDREGIONS);
                if (supportedRegions.length < 1) {
                    throw new Exception("Reader doesn't support any regions");
                } else {
                    r.paramSet(TMConstants.TMR_PARAM_REGION_ID, supportedRegions[0]);
                }
            }
            String model = (String)r.paramGet("/reader/version/model");
            if (!(model.equalsIgnoreCase("M3e")))
            {
                if (r.isAntDetectEnabled(antennaList))
                {
                    System.out.println("Module doesn't has antenna detection support, please provide antenna list");
                    r.destroy();
                    usage();
                }
            }
            // Create a simplereadplan which uses the antenna list created above
            SimpleReadPlan plan;
            if (model.equalsIgnoreCase("M3e"))
            {
               // initializing the simple read plan
               plan = new SimpleReadPlan(antennaList, TagProtocol.ISO14443A, null, null, 1000);
            }
            else
            {
               plan = new SimpleReadPlan(antennaList, TagProtocol.GEN2, null, null, 1000);
            }
            r.paramSet(TMConstants.TMR_PARAM_READ_PLAN, plan);
            // Read tags
            tagReads = r.read(500);
            for (TagReadData tr : tagReads)
            {
                System.out.println(tr.toString());
            }
            // Shut down reader
            r.destroy();
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }
    
    static  int[] parseAntennaList(String[] args,int argPosition)
    {
        int[] antennaList = null;
        try
        {
            String argument = args[argPosition + 1];
            String[] antennas = argument.split(",");
            int i = 0;
            antennaList = new int[antennas.length];
            for (String ant : antennas)
            {
                antennaList[i] = Integer.parseInt(ant);
                i++;
            }
        }
        catch (IndexOutOfBoundsException ex)
        {
            System.out.println("Missing argument after " + args[argPosition]);
            usage();
        }
        catch (Exception ex)
        {
            System.out.println("Invalid argument at position " + (argPosition + 1) + ". " + ex.getMessage());
            usage();
        }
        return antennaList;
    }
}
