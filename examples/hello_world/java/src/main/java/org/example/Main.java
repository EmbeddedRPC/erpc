package org.example;

import java.io.IOException;

public class Main {
    public static void main(String[] args) throws IOException {
        if (args[0].equals("client")) {
            new Client().run();
        } else if (args[0].equals("server")) {
            new Server().run();
        }
    }
}