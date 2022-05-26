import java.net.*;
import java.io.*;

public class receiveTCP implements Runnable {
    String ip;
    int port;

    public receiveTCP(String ip, int port) {
        this.ip = ip;
        this.port = port;
    }

    @Override
    public void run() {
        receiveTCPmethod(ip, port);
    }

    public static void receiveTCPmethod(String ip, int port) {
        // try {
        // // To receive TCP in bytes

        // Socket socket = new Socket(ip, port);
        // DataInputStream dIn = new DataInputStream(socket.getInputStream());
        // while (true) {
        // System.out.println("waiting TCP");
        // // Rcv commands
        // byte[] buffer = new byte[1024];
        // dIn.read(buffer);
        // System.out.println("message readed");
        // // To rcv bytes messages
        // String mRcved = clientPourLestest.convertBytesArrayToString(buffer);
        // System.out.println("message rcv: " + mRcved);
        // }

        // } catch (Exception e) {
        // System.out.println(e);
        // e.printStackTrace();
        // }

        Socket socket;
        try {
            socket = new Socket(ip, port);
            BufferedReader br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            String firstmess = br.readLine();
            System.out.println(firstmess);

            final char endMarker = '*';

            int value;
            // or of course StringBuffer if you need to be treadsafe
            StringBuilder messageBuffer = new StringBuilder();
            // reads to the end of the stream or till end of message
            while ((value = br.read()) != -1) {
                System.out.println("truc");
                char c = (char) value;
                // end? jump out
                if (c == endMarker) {
                    break;
                }
                // else, add to buffer
                messageBuffer.append(c);
            }
            
            // message is complete!
            String message = messageBuffer.toString();
            System.out.println(message);
            
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

    }

}
