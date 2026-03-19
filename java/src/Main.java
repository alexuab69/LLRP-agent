import java.net.*;
import java.io.*;

import reader.ReaderAdapter;

import org.llrp.ltk.generated.messages.*;
import org.llrp.ltk.types.*;
import org.llrp.ltk.generated.parameters.*;
import org.llrp.ltk.generated.enumerations.*;
import org.llrp.ltk.generated.LLRPMessageFactory;

public class Main {

    public static ReaderAdapter reader = new ReaderAdapter();

    public static void main(String[] args) {
        // ReaderAdapter manages the hardware connection lifecycle.

        try {

            ServerSocket server = new ServerSocket(5084);
            System.out.println("LLRP Agent listening on port 5084");

            while (true) {

                Socket socket = server.accept();
                System.out.println("Client connected: " + socket.getInetAddress());

                new Thread(new ClientHandler(socket)).start();
            }

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    static class ClientHandler implements Runnable {

        private Socket socket;
        private DataInputStream in;
        private DataOutputStream out;

        public ClientHandler(Socket socket) {
            this.socket = socket;
        }

        @Override
        public void run() {

            try {

                in = new DataInputStream(socket.getInputStream());
                out = new DataOutputStream(socket.getOutputStream());

                while (true) {

                    byte[] message = readMessage();

                    if (message == null)
                        break;

                    LLRPMessage llrpMessage = LLRPMessageFactory.createLLRPMessage(message);

                    handleMessage(llrpMessage);

                }

            } catch (Exception e) {
                System.out.println("Client disconnected");
            }
        }

        private byte[] readMessage() throws Exception {

            byte[] header = new byte[10];

            in.readFully(header);

            int length =
                    ((header[2] & 0xFF) << 24) |
                    ((header[3] & 0xFF) << 16) |
                    ((header[4] & 0xFF) << 8) |
                    (header[5] & 0xFF);

            byte[] data = new byte[length];

            System.arraycopy(header, 0, data, 0, 10);

            in.readFully(data, 10, length - 10);

            return data;
        }

        private void handleMessage(LLRPMessage message) throws Exception {

            System.out.println("Received: " + message.getName());

            if (message instanceof GET_READER_CAPABILITIES) {

                sendCapabilitiesResponse((GET_READER_CAPABILITIES) message);

            } else if (message instanceof ADD_ROSPEC) {

                ADD_ROSPEC msg = (ADD_ROSPEC) message;

                ADD_ROSPEC_RESPONSE response = new ADD_ROSPEC_RESPONSE();

                response.setMessageID(msg.getMessageID());

                LLRPStatus status = new LLRPStatus();
                status.setStatusCode(new StatusCode(StatusCode.M_Success));
                status.setErrorDescription(new UTF8String_UTF_8("OK"));

                response.setLLRPStatus(status);

                send(response);

            } else if (message instanceof START_ROSPEC) {

                START_ROSPEC msg = (START_ROSPEC) message;

                reader.startInventory();

                START_ROSPEC_RESPONSE response = new START_ROSPEC_RESPONSE();

                response.setMessageID(msg.getMessageID());

                LLRPStatus status = new LLRPStatus();
                status.setStatusCode(new StatusCode(StatusCode.M_Success));
                status.setErrorDescription(new UTF8String_UTF_8("Started"));

                response.setLLRPStatus(status);

                send(response);

            } else if (message instanceof STOP_ROSPEC) {

                STOP_ROSPEC msg = (STOP_ROSPEC) message;

                reader.stopInventory();

                STOP_ROSPEC_RESPONSE response = new STOP_ROSPEC_RESPONSE();

                response.setMessageID(msg.getMessageID());

                LLRPStatus status = new LLRPStatus();
                status.setStatusCode(new StatusCode(StatusCode.M_Success));
                status.setErrorDescription(new UTF8String_UTF_8("Stopped"));

                response.setLLRPStatus(status);

                send(response);

            } else {

                System.out.println("Unhandled message: " + message.getName());
            }
        }

        private void sendCapabilitiesResponse(GET_READER_CAPABILITIES msg) throws Exception {

            GET_READER_CAPABILITIES_RESPONSE response =
                    new GET_READER_CAPABILITIES_RESPONSE();

            response.setMessageID(msg.getMessageID());

            LLRPStatus status = new LLRPStatus();
            status.setStatusCode(new StatusCode(StatusCode.M_Success));
            status.setErrorDescription(new UTF8String_UTF_8("OK"));

            response.setLLRPStatus(status);

            send(response);
        }

        private void send(LLRPMessage message) throws Exception {

            byte[] data = message.encodeBinary();

            out.write(data);
            out.flush();

            System.out.println("Sent: " + message.getName());
        }
    }
}