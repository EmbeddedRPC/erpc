package org.example;

import org.example.hello_world.server.AbstractTextServiceService;

public class TextServiceService extends AbstractTextServiceService {
    @Override
    public void printText(String text) {
        System.out.println(text);
    }
}
