import java.net.*;
import java.nio.ByteBuffer;
import java.util.Scanner;
import java.io.*;
import java.util.*;
import java.lang.*;
import java.util.regex.*;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.Objects;
import java.io.*;
import java.nio.ByteOrder;

public class clientPourLestest {
    static String id;
    static String ip;
    static int port;
    static int numberOfGame;
    static boolean littleEndian;

    public static void main(String[] args) {

        if (args.length != 2) {
            System.out.println("pleases tape java client (ip) (port)");
            return;
        }

        ip = args[0];
        port = Integer.parseInt(args[1]);
        // Thread for rcv TCP
        receiveTCP rcvTCP = new receiveTCP("localhost", port);
        new Thread(rcvTCP).start();

        // Thread for sending TCP
        sendTCP sendTCP = new sendTCP("localhost", port);
        new Thread(sendTCP).start();

        // TODO Change multicast ip address
        // Thread for rcv UDP Multicast
        receiveUDPMulticast rcvUDPMul = new receiveUDPMulticast("225.1.2.4", port);
        new Thread(rcvUDPMul).start();

    }

    public static String chooseID() {
        try {
            Scanner scanner = new Scanner(System.in);

            System.out.println("choose id:");
            String id = scanner.nextLine();
            while (id.length() == 0 || id.length() != 8) {
                System.out.println("id length != 8");
                id = scanner.nextLine();
            }
            return id;
        } catch (Exception e) {
            System.out.println(e);
            e.printStackTrace();
        }
        return null;
    }

    public static byte[] readMessage(DataInputStream din) throws IOException {
        int msgLen = din.readInt();
        byte[] msg = new byte[msgLen];
        din.readFully(msg);
        return msg;
    }

    public static void sendingCommand(String command, DataOutputStream dOut) throws IOException {
        Scanner scanner = new Scanner(System.in);
        String commands = scanner.nextLine();
        byte[] commandsByte = convertStringToByteArray(commands);
        dOut.writeInt(commands.length());
        dOut.write(commandsByte);
        dOut.flush();
    }

    static void checkLittleBigEndian() {
        if (ByteOrder.nativeOrder().equals(ByteOrder.BIG_ENDIAN)) {
            System.out.println("Big-endian");
            littleEndian = false;
        } else {
            littleEndian = true;
            System.out.println("Little-endian");
        }
    }

    // Trouve les chiffre/nombre dans un string random
    static List<String> findIntegers(String stringToSearch) {
        Pattern integerPattern = Pattern.compile("-?\\d+");
        Matcher matcher = integerPattern.matcher(stringToSearch);

        List<String> integerList = new ArrayList<>();
        while (matcher.find()) {
            integerList.add(matcher.group());
        }
        return integerList;
    }

    public static byte[] concat(byte[]... arrays) {
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        if (arrays != null) {
            Arrays.stream(arrays).filter(Objects::nonNull)
                    .forEach(array -> out.write(array, 0, array.length));
        }
        return out.toByteArray();
    }

    public static void printByteArrayAsBinary(byte[] array) {
        for (byte b : array) {
            System.out.println(Integer.toBinaryString(b & 255 | 256).substring(1));
        }
    }

    static byte[] convertStringToByteArray(String command) {
        System.out.println("command: " + command);
        String commandWithSpace = command.substring(0, command.length() - 3) + " "
                + command.substring(command.length() - 3, command.length());
        // System.out.println(commandWithSpace);
        String[] currencies = commandWithSpace.split(" ");
        // System.out.println(Arrays.toString(currencies));
        System.out.println("currencies " + currencies[0]);

        // NEWPL␣id␣port***
        if (currencies[0].equals("NEWPL")) {
            byte[] res = concat(currencies[0].getBytes(), " ".getBytes(), currencies[1].getBytes(), " ".getBytes(),
                    currencies[2].getBytes(),
                    currencies[3].getBytes());
            printByteArrayAsBinary(res);
            return res;
        }
        // REGIS␣id␣port␣m***
        else if (currencies[0].equals("REGIS")) {
            byte[] m = ByteBuffer.allocate(4).putInt(Integer.parseInt(currencies[3])).array();
            byte[] res = concat(currencies[0].getBytes(), " ".getBytes(), currencies[1].getBytes(), " ".getBytes(),
                    currencies[2].getBytes(), " ".getBytes(),
                    m, currencies[4].getBytes());
            printByteArrayAsBinary(res);
            return res;
        }
        // START*** UNREG*** GAME?*** IQUIT*** GLIS?***
        else if (currencies[0].equals("START")) {
            byte[] res = concat(currencies[0].getBytes(), currencies[1].getBytes());
            printByteArrayAsBinary(res);
            return res;
        }
        // SIZE? m*** LIST? m*** UPMOV␣d*** DOMOV␣d*** LEMOV␣d*** RIMOV␣d***
        else if (currencies[0].equals("SIZE?") || currencies[0].equals("LIST?") || currencies[0].equals("UPMOV")
                || currencies[0].equals("DOMOV") || currencies[0].equals("LEMOV") || currencies[0].equals("RIMOV")) {

            byte[] m = ByteBuffer.allocate(1).putInt(Integer.parseInt(currencies[1])).array();
            byte[] res = concat(currencies[0].getBytes(), " ".getBytes(), m, currencies[2].getBytes());
            printByteArrayAsBinary(res);
            return res;
        }
        // MALL?␣mess***
        else if (currencies[0].equals("MESS?")) {
            byte[] res = concat(currencies[0].getBytes(), " ".getBytes(), currencies[1].getBytes(),
                    currencies[2].getBytes());
            printByteArrayAsBinary(res);
            return res;
        }
        // SEND?␣id␣mess***
        else if (currencies[0].equals("SEND?")) {
            byte[] res = concat(currencies[0].getBytes(), " ".getBytes(), currencies[1].getBytes(), " ".getBytes(),
                    currencies[2].getBytes(),
                    currencies[3].getBytes());
            printByteArrayAsBinary(res);
            return res;
        } else {
            System.out.println("commande non reconnu !");
            return command.getBytes();
        }
    }

    static byte[] convertStringToByteArrayButMessageFromServer(String command) {
        String commandWithSpace = command.substring(0, command.length() - 3) + " "
                + command.substring(command.length() - 3, command.length());
        // System.out.println(commandWithSpace);
        String[] currencies = commandWithSpace.split(" ");
        // System.out.println(Arrays.toString(currencies));

        // GAMES n***
        if (currencies[0].equals("GAMES")) {
            byte[] m = ByteBuffer.allocate(8).putInt(Integer.parseInt(currencies[1])).array();
            byte[] res = concat(currencies[0].getBytes(), m, currencies[2].getBytes());
            printByteArrayAsBinary(res);
            return res;
        }
        // OGAME m s***
        else if (currencies[0].equals("OGAME")) {
            byte[] m = ByteBuffer.allocate(4).putInt(Integer.parseInt(currencies[1])).array();
            byte[] s = ByteBuffer.allocate(4).putInt(Integer.parseInt(currencies[2])).array();
            byte[] res = concat(currencies[0].getBytes(), m, s, currencies[3].getBytes());
            byte[] res2 = concat(res, res); // Double the length to make tests
            printByteArrayAsBinary(res2);
            return res2;
        }
        // REGOK m*** UNROK m***
        else if (currencies[0].equals("REGOK") || currencies[0].equals("UNROK")) {
            byte[] m = ByteBuffer.allocate(4).putInt(Integer.parseInt(currencies[1])).array();
            byte[] res = concat(currencies[0].getBytes(), m, currencies[2].getBytes());
            printByteArrayAsBinary(res);
            return res;
        }
        // REGNO*** DUNNO*** GODBYE***
        else if (currencies[0].equals("REGNO") || currencies[0].equals("DUNNO") || currencies[0].equals("GODBYE")) {
            byte[] res = concat(currencies[0].getBytes(), currencies[1].getBytes());
            printByteArrayAsBinary(res);
            System.out.println("coucou");
            return res;
        }
        // WELCO m h w f ip port***
        else if (currencies[0].equals("WELCO")) {
            byte[] m = ByteBuffer.allocate(4).putInt(Integer.parseInt(currencies[1])).array();
            byte[] h = ByteBuffer.allocate(4).putInt(Integer.parseInt(currencies[2])).array();
            byte[] w = ByteBuffer.allocate(4).putInt(Integer.parseInt(currencies[3])).array();
            byte[] f = ByteBuffer.allocate(4).putInt(Integer.parseInt(currencies[4])).array();
            byte[] res = concat(currencies[0].getBytes(), m, h, w, f, currencies[5].getBytes(),
                    currencies[6].getBytes(), currencies[7].getBytes());
            return res;
        }
        // Pas fait toute les commandes
        else {
            System.out.println("commande non reconnu !");
            return null;
        }
    }

    public static List<byte[]> divideArray(byte[] source, int chunksize) {

        List<byte[]> result = new ArrayList<byte[]>();
        int start = 0;
        while (start < source.length) {
            int end = Math.min(source.length, start + chunksize);
            result.add(Arrays.copyOfRange(source, start, end));
            start += chunksize;
        }

        return result;
    }

    public static int convertByteArrayToInt(byte[] source) {
        int value = 0;
        for (byte b : source) {
            value = (value << 8) + (b & 0xFF);
        }
        return value;
    }

    // Changer octets fait
    static String convertBytesArrayToString(byte[] array) {
        String result = new String();
        // Get the first 5 char
        byte[] commmandIDb = Arrays.copyOfRange(array, 0, 5);
        String commandID = new String(commmandIDb);
        result += commandID;

        // GAMES n***
        if (commandID.equals("GAMES")) {
            // print n
            byte[] nByte = Arrays.copyOfRange(array, 6, 7);
            int n = convertByteArrayToInt(nByte);
            result += String.valueOf(n);

            // print ***
            byte[] etoileB = Arrays.copyOfRange(array, 7, 10);
            String etoile = new String(etoileB);
            result += etoile;

        }
        // REGOK m*** UNROK m***
        else if (commandID.equals("REGOK") || commandID.equals("UNROK")) {
            // print m
            byte[] mByte = Arrays.copyOfRange(array, 6, 7);
            int m = convertByteArrayToInt(mByte);
            result += String.valueOf(m);

            // print ***
            byte[] etoileB = Arrays.copyOfRange(array, 7, 10);
            String etoile = new String(etoileB);
            result += etoile;
        }
        // REGNO*** DUNNO*** GODBYE***
        else if (commandID.equals("REGNO") || commandID.equals("DUNNO") || commandID.equals("GODBYE")) {
            // print ***
            byte[] etoileB = Arrays.copyOfRange(array, 5, 8);
            String etoile = new String(etoileB);
            result += etoile;
        }
        // OGAME m s*** (a recevoir n fois)
        else if (commandID.equals("OGAME")) {
            int lengthOfCommand = 16;
            int numOfIteration = array.length / lengthOfCommand;
            // System.out.println(numOfIteration);
            for (int i = 0; i < numOfIteration; i++) {
                if (i != 0) {
                    // OGAME
                    result += commandID;
                }
                // m
                byte[] mByte = Arrays.copyOfRange(array, 6 + (lengthOfCommand * i), 7 + (lengthOfCommand * i));
                int m = convertByteArrayToInt(mByte);
                result += String.valueOf(m);
                // s
                byte[] sByte = Arrays.copyOfRange(array, 8 + (lengthOfCommand * i), 9 + (lengthOfCommand * i));
                int s = convertByteArrayToInt(sByte);
                result += String.valueOf(s);
                // ***
                byte[] etoileB = Arrays.copyOfRange(array, 9 + (lengthOfCommand * i), 12 + (lengthOfCommand * i));
                String etoile = new String(etoileB);
                result += etoile;
                result += '\n';
            }
        }
        // WELCO m h w f ip port
        else if (commandID.equals("WELCO")) {
            // print m
            byte[] mByte = Arrays.copyOfRange(array, 6, 7);
            int m = convertByteArrayToInt(mByte);
            result += String.valueOf(m);

            // print h
            byte[] hByte = Arrays.copyOfRange(array, 8, 10);
            int h = convertByteArrayToInt(hByte);
            result += String.valueOf(h);

            // print w
            byte[] wByte = Arrays.copyOfRange(array, 11, 13);
            int w = convertByteArrayToInt(wByte);
            result += String.valueOf(w);

            // print f
            byte[] fByte = Arrays.copyOfRange(array, 14, 15);
            int f = convertByteArrayToInt(fByte);
            result += String.valueOf(f);

            // print ip
            byte[] ipByte = Arrays.copyOfRange(array, 16, 31);
            String ip = new String(ipByte);
            result += ip;

            // print port
            byte[] portByte = Arrays.copyOfRange(array, 32, 36);
            String port = new String(portByte);
            result += port;

            // print ***
            byte[] etoileB = Arrays.copyOfRange(array, 36, 39);
            String etoile = new String(etoileB);
            result += etoile;
        }
        // POSIT id x y*** (pas test)
        else if (commandID.equals("POSIT")) {
            // id
            byte[] idByte = Arrays.copyOfRange(array, 6, 14);
            String id = new String(idByte);
            result += id;

            // x
            byte[] xByte = Arrays.copyOfRange(array, 15, 18);
            int x = convertByteArrayToInt(xByte);
            result += String.valueOf(x);

            // y
            byte[] yByte = Arrays.copyOfRange(array, 19, 22);
            int y = convertByteArrayToInt(yByte);
            result += String.valueOf(y);

            // print ***
            byte[] etoileB = Arrays.copyOfRange(array, 22, 25);
            String etoile = new String(etoileB);
            result += etoile;
        }
        // SIZE! m h w*** (pas test)
        else if (commandID.equals("SIZE!")) {
            // m
            byte[] mByte = Arrays.copyOfRange(array, 6, 7);
            int m = convertByteArrayToInt(mByte);
            result += String.valueOf(m);

            // h
            byte[] hByte = Arrays.copyOfRange(array, 8, 10);
            int h = convertByteArrayToInt(hByte);
            result += String.valueOf(h);

            // w
            byte[] wByte = Arrays.copyOfRange(array, 11, 13);
            int w = convertByteArrayToInt(wByte);
            result += String.valueOf(w);

            // print ***
            byte[] etoileB = Arrays.copyOfRange(array, 13, 16);
            String etoile = new String(etoileB);
            result += etoile;
        }
        // MOVE! x y*** (pas test)
        else if (commandID.equals("MOVE!")) {
            // x
            byte[] xByte = Arrays.copyOfRange(array, 6, 9);
            int x = convertByteArrayToInt(xByte);
            result += String.valueOf(x);

            // y
            byte[] yByte = Arrays.copyOfRange(array, 10, 13);
            int y = convertByteArrayToInt(yByte);
            result += String.valueOf(y);

            // print ***
            byte[] etoileB = Arrays.copyOfRange(array, 13, 16);
            String etoile = new String(etoileB);
            result += etoile;
        }
        // MOVE! x y p s*** (pas test)
        else if (commandID.equals("MOVEF")) {
            // x
            byte[] xByte = Arrays.copyOfRange(array, 6, 9);
            int x = convertByteArrayToInt(xByte);
            result += String.valueOf(x);

            // y
            byte[] yByte = Arrays.copyOfRange(array, 9, 13);
            int y = convertByteArrayToInt(yByte);
            result += String.valueOf(y);

            // p
            byte[] pByte = Arrays.copyOfRange(array, 13, 17);
            int p = convertByteArrayToInt(pByte);
            result += String.valueOf(p);

            // print ***
            byte[] etoileB = Arrays.copyOfRange(array, 17, 20);
            String etoile = new String(etoileB);
            result += etoile;

        }
        // TODO reste a faire LIST

        return result;
    }
}