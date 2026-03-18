using System;
using System.Collections.Generic;
using System.Text;

// Reference the API
using ThingMagic;

namespace SavedConfig
{
    class SavedConfig
    {
        static void Usage()
        {
            Console.WriteLine(String.Join("\r\n", new string[] {
                    " Usage: "+"Please provide valid reader URL, such as: [-v] [reader-uri]",
                    " -v : (Verbose)Turn on transport listener",
                    " reader-uri : e.g., 'tmr:///com4' or 'tmr:///dev/ttyS0/' or 'tmr://readerIP'",
                    " Example: 'tmr:///com4'' or '-v tmr:///com4'"
                }));
            Environment.Exit(1);
        }
        static void Main(string[] args)
        {
            // Program setup
            if (1 > args.Length)
            {
                Usage();
            }

            try
            {
                // Create Reader object, connecting to physical device.
                // Wrap reader in a "using" block to get automaticq
                // reader shutdown (using IDisposable interface).
                using (Reader r = Reader.Create(args[0]))
                {
                    //Uncomment this line to add default transport listener.
                    //r.Transport += r.SimpleTransportListener;

                    try
                    {
                        /* MercuryAPI tries connecting to the module using default baud rate of 115200 bps.
                         * The connection may fail if the module is configured to a different baud rate. If
                         * that is the case, the MercuryAPI tries connecting to the module with other supported
                         * baud rates until the connection is successful using baud rate probing mechanism.
                         */
                        r.Connect();
                    }
                    catch (Exception ex)
                    {
                        if (ex.Message.Contains("The operation has timed out") && r is SerialReader)
                        {
                            // Default baudrate connect failed. Try probing through the baudrate list
                            // to retrieve the module baudrate
                            int currentBaudRate = ((SerialReader)r).probeBaudRate();
                            //Set the current baudrate so that next connect will use this baudrate.
                            r.ParamSet("/reader/baudRate", currentBaudRate);
                            // Now connect with current baudrate
                            r.Connect();
                        }
                    }

                    string model = (string)r.ParamGet("/reader/version/model");
                    {
                        if (Reader.Region.UNSPEC == (Reader.Region)r.ParamGet("/reader/region/id"))
                        {
                            Reader.Region[] supportedRegions = (Reader.Region[])r.ParamGet("/reader/region/supportedRegions");
                            if (supportedRegions.Length < 1)
                            {
                                throw new FAULT_INVALID_REGION_Exception();
                            }
                            r.ParamSet("/reader/region/id", supportedRegions[0]);
                        }

                        if (model.Equals("M3e"))
                        {
                            r.ParamSet("/reader/tagop/protocol", TagProtocol.ISO14443A);
                        }
                        else
                        {
                            r.ParamSet("/reader/tagop/protocol", TagProtocol.GEN2);
                        }
                        r.ParamSet("/reader/userConfig", new SerialReader.UserConfigOp(SerialReader.UserConfigOperation.SAVE));
                        Console.WriteLine("User profile set option:save all configuration");

                        r.ParamSet("/reader/userConfig", new SerialReader.UserConfigOp(SerialReader.UserConfigOperation.RESTORE));
                        Console.WriteLine("User profile set option:restore all configuration");

                        /**********  Testing cmdGetUserProfile function ***********/


                        object region = r.ParamGet("/reader/region/id");
                        Console.WriteLine("Get user profile success option:Region");
                        Console.WriteLine(region.ToString());


                        object proto = r.ParamGet("/reader/tagop/protocol");
                        Console.WriteLine("Get user profile success option:Protocol");
                        Console.WriteLine(proto.ToString());

                        Console.WriteLine("Get user profile success option:Baudrate");
                        Console.WriteLine(r.ParamGet("/reader/baudRate").ToString());

                        //reset all the configurations
                        r.ParamSet("/reader/userConfig", new SerialReader.UserConfigOp(SerialReader.UserConfigOperation.CLEAR));
                        Console.WriteLine("User profile set option:reset all configuration");

                        if (TagProtocol.NONE == (TagProtocol)r.ParamGet("/reader/tagop/protocol"))
                        {
                            if (model.Equals("M3e"))
                            {
                                r.ParamSet("/reader/tagop/protocol", TagProtocol.ISO14443A);
                            }
                            else
                            {
                                r.ParamSet("/reader/tagop/protocol", TagProtocol.GEN2);
                            }
                        }
                        if (Reader.Region.UNSPEC == (Reader.Region)r.ParamGet("/reader/region/id"))
                        {
                            Reader.Region[] supportedRegions = (Reader.Region[])r.ParamGet("/reader/region/supportedRegions");
                            if (supportedRegions.Length < 1)
                            {
                                throw new FAULT_INVALID_REGION_Exception();
                            }
                            else
                            {
                                r.ParamSet("/reader/region/id", supportedRegions[0]);
                            }
                        }
                    }
                }
            }
            catch (ReaderException re)
            {
                Console.WriteLine("Error: " + re.Message);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
            }
        }
    }
}
