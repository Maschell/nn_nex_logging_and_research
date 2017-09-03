package de.mas.wiiu.tools.common;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.ServerSocket;
import java.net.Socket;

class TCPServer {
    public static void main(String argv[]) throws Exception {
        AutoRunFromConsole.runYourselfInConsole(true);
        String clientSentence;
        ServerSocket welcomeSocket = new ServerSocket(4405);
        System.out.println("Logger started!");
        PrintWriter outPrinter = new PrintWriter("tcp_log.txt");
        while (true) {
          
            Socket connectionSocket = welcomeSocket.accept();
            BufferedReader inFromClient = new BufferedReader(new InputStreamReader(connectionSocket.getInputStream()));
            while (true) {
                try {
                    clientSentence = inFromClient.readLine();
                    if (clientSentence == null) break;
                    System.out.println(clientSentence);
                    outPrinter.println(clientSentence);
                    outPrinter.flush();
                } catch (Exception e) {
                    break;
                }                
            }
            
            System.out.println("Connection closed.");
        }
    }
}