import java.net.*;
import java.io.*;

public class receiveUDP implements Runnable {
    int port;

    public receiveUDP(int port) {
        this.port = port;
    }

    @Override
    public void run() {
        receiveUDPMethod(port);
    }

    public static void receiveUDPMethod(int port) {

        try {
            DatagramSocket dso = new DatagramSocket(port);
            byte[] data = new byte[100];
            while (true) {
                DatagramPacket paquet = new DatagramPacket(data, data.length);
                dso.receive(paquet);
                String st = new String(paquet.getData(), 0, paquet.getLength());
                System.out.println("J'ai reçu :" + st);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

    }
}
