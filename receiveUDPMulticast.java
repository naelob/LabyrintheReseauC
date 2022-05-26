import java.net.*;
import java.io.*;

public class receiveUDPMulticast implements Runnable {
    String ip;
    int port;
    protected static MulticastSocket socket = null;
    protected static byte[] buf = new byte[256];

    public receiveUDPMulticast(String ip, int port) {
        this.ip = ip;
        this.port = port;
    }

    @Override
    public void run() {
        try {
            receiveUDPMulticast(ip, port);
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    public static void receiveUDPMulticast(String ip, int port) throws IOException {
        
        
        socket = new MulticastSocket(port);
        InetAddress group = InetAddress.getByName(ip);
        socket.joinGroup(group);
        while (true) {
            DatagramPacket packet = new DatagramPacket(buf, buf.length);
            socket.receive(packet);
            String received = new String(packet.getData(), 0, packet.getLength());
            System.out.println(received);
            if ("end".equals(received)) {
                break;
            }
        }
        socket.leaveGroup(group);
        socket.close();
        /*protected MulticastSocket socket = null;
        protected byte[] buf = new byte[256];
    
        public void run() {
            socket = new MulticastSocket(4446);
            InetAddress group = InetAddress.getByName("230.0.0.0");
            socket.joinGroup(group);
            while (true) {
                DatagramPacket packet = new DatagramPacket(buf, buf.length);
                socket.receive(packet);
                String received = new String(
                  packet.getData(), 0, packet.getLength());
                if ("end".equals(received)) {
                    break;
                }
            }
            socket.leaveGroup(group);
            socket.close();
        }*/



    }
}
