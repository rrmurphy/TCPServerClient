import java.io.IOException;
import java.io.InputStreamReader;
import java.net.Socket;

public class ClientTCP {
    
    private static final int HOST_INDEX = 0;
    private static final int PORT_NO_INDEX = 1;
    private static final int OPERATION_INDEX = 2;
    private static final int STRING_INDEX = 3;
    
    private static int sRequestId = 1;
    
    public static void main(String[] args) {
        if (args.length != 4) {
            System.err.println("Usage: hostname port operation string");
            System.exit(1);
        }
        try {
            String hostName = args[HOST_INDEX];
            int portNo = Integer.parseInt(args[PORT_NO_INDEX]);
            int operation = Integer.parseInt(args[OPERATION_INDEX]);
            String data = args[STRING_INDEX];
            
            Socket socket = new Socket(hostName, portNo);
            byte[] buffer = toFormattedByteArray(operation, data);
            long startTime = System.nanoTime();
            socket.getOutputStream().write(buffer);
            socket.getInputStream().read(buffer, 0, buffer.length);
            long endTime = System.nanoTime();
            
            String responseOutput;
            if (operation == 80 || operation == 10) {
                responseOutput = "\"" + getResultString(buffer) + "\"";
            } else if (operation == 5) {
                responseOutput = String.valueOf(getCLength(buffer));
            } else {
                System.err.println("INVALID OPERATION CODE");
                System.exit(1);
                return;
            }
            System.out.printf("Request ID: %d\nResponse: %s\nRound-Trip Time: %d nanoseconds\n", 
                    buffer[1], responseOutput, endTime - startTime);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    private static byte[] toFormattedByteArray(int opCode, String data) {
        byte[] bytes = new byte[data.length() + 3];
        char[] chars = data.toCharArray();
        
        bytes[0] = (byte) bytes.length;
        bytes[1] = (byte) sRequestId++;
        bytes[2] = (byte) opCode;
        
        for (int i = 0; i < chars.length; i++) {
            bytes[i + 3] = (byte) chars[i];
        }
        
        return bytes;
    }
    
    private static String getResultString(byte[] response) {
        int length = response[0];
        char[] array = new char[length - 2];
        for (int i = 0; i < length - 2; i++) {
            array[i] = (char) response[i + 2];
        }
        return new String(array);
    }
    
    private static int getCLength(byte[] response) {
        return response[2];
    }
}
