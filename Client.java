import java.net.*;
import java.util.Scanner;
import java.io.*;

public class Client {

    // For TCP
    static String ip;
    static int port;

    // for UDPMulti
    static String ipUDPMulti;
    static int portUDPMulti;

    // for UDP
    static int portUDP;

    public static void main(String[] args) {
        if (args.length != 2) {
            System.out.println("pleases tape java client (ip) (port)");
            return;
        }
        ip = args[0];
        port = Integer.parseInt(args[1]);

        try {
            Socket socket = new Socket(ip, port);
            BufferedReader br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            // Thread for sending TCP
            sendTCP sendTCP = new sendTCP(socket);
            new Thread(sendTCP).start();

            while (true) {
                final char endMarker = '*';
                int count = 0;
                int value;
                // or of course StringBuffer if you need to be treadsafe
                StringBuilder messageBuffer = new StringBuilder();
                // reads to the end of the stream or till end of message
                while ((value = br.read()) != -1) {
                    char c = (char) value;
                    // end? jump out
                    // System.out.println("count: " + count);
                    if (c == endMarker && count == 2) {
                        count = 0;
                        break;
                    } else if (c == endMarker) {
                        count++;
                    }
                    // else, add to buffer
                    messageBuffer.append(c);

                    // TODO Check server double send
                }

                messageBuffer.append((char) value);
                // message is complete!
                String message = messageBuffer.toString();
                System.out.println(message);

                // Check if mess is welcom to save port and ip of the game in global variable
                if (message.contains("WELCO")) {
                    String[] separated = message.split("\\ ");
                    System.out.println(separated[5]);

                    ipUDPMulti = separated[5];
                    portUDPMulti = Integer.parseInt(separated[6].substring(0, separated[6].length() - 3));

                    // System.out.println("ipUDP: " + ipUDP);
                    // System.out.println("portUDP: " + portUDP);

                    // Thread for rcv UDP Multicast
                    receiveUDPMulticast rcvUDPMul = new receiveUDPMulticast(ipUDPMulti, portUDPMulti);
                    new Thread(rcvUDPMul).start();

                    // Thread for UPD
                    receiveUDP rcvUDP = new receiveUDP(portUDP);
                    new Thread(rcvUDP).start();

                }
            }

        } catch (Exception e) {
            System.out.println(e);
            e.printStackTrace();
        }

        // pw.close();
        // br.close();

    }
}
