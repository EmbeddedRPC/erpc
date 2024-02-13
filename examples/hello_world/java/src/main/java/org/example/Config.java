package org.example;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

public class Config {
    Map<String, String> variables = new HashMap<>();

    public Config(String path) {
        loadConfig(path);
    }

    private void loadConfig(String path) {
        BufferedReader reader;

        try {
            reader = new BufferedReader(new FileReader(path));
            String line = reader.readLine();

            while (line != null) {
                if (!line.startsWith("#define")) {
                    line = reader.readLine();
                    continue;
                }

                String[] context = line.split(" ");

                if (context.length != 3) {
                    continue;
                }

                variables.put(context[1], context[2].replace("\"", ""));
                line = reader.readLine();
            }

            reader.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public String getString(String key) {
        return variables.get(key);
    }

    public int getInt(String key) {
        return Integer.parseInt(variables.get(key));
    }
}
