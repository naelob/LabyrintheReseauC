import java.net.*;
import java.util.Scanner;
import java.io.*;

public class sendTCP implements Runnable {
    String ip;
    int port;
    static Socket s;

    public sendTCP(String ip, int port) {
        this.ip = ip;
        this.port = port;
    }

    public sendTCP(Socket s) {
        this.s = s;
    }

    @Override
    public void run() {
        sendTCPMethod(ip, port);
    }

    public static void sendTCPMethod(String ip, int port) {
        try {
            // To send messages in bytes

            // Socket socket = new Socket(ip, port);
            // DataOutputStream dOut = new DataOutputStream(socket.getOutputStream());
            // System.out.println("send ");
            // byte[] toSend = new byte[100];
            // Scanner scanner = new Scanner(System.in);
            // String commands = scanner.nextLine();
            // toSend = clientPourLestest.convertStringToByteArray(commands);
            // System.out.println("tosend: " + toSend);
            // dOut.writeInt(100);
            // dOut.write(toSend);

            PrintWriter pw = new PrintWriter(new OutputStreamWriter(s.getOutputStream()));
            Scanner scanner;
            String commands;
            while (true) {
                // System.out.println("envoy");
                scanner = new Scanner(System.in);
                commands = scanner.nextLine();
                if (commands.contains("NEWPL")) {
                    String[] separated = commands.split("\\ ");
                    Client.portUDP = Integer.parseInt(separated[2].substring(0, separated[2].length() - 3));
                    System.out.println("portUDP: " + Client.portUDP);
                } else if (commands.contains("REGIS")) {
                    String[] separated = commands.split("\\ ");
                    Client.portUDP = Integer.parseInt(separated[2]);
                    System.out.println("portUDP: " + Client.portUDP);
                }
                pw.print(commands);
                pw.flush();
            }

        } catch (Exception e) {
            System.out.println(e);
            e.printStackTrace();
        }

    }

}
