package de.mas.wiiu.tools.common;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.math.BigInteger;
import java.util.ArrayList;
import java.util.List;

public final class Utilities {
    private Utilities() {
        //
    }

    public static List<Long> readLongFromFile(String path) {
        File file = new File(path);
        BufferedReader in = null;
        List<Long> list = new ArrayList<>();
        if (file == null || !file.exists()) return list;
        try {
            in = new BufferedReader(new InputStreamReader(new FileInputStream(file), "UTF-8"));
            String line;
            while ((line = in.readLine()) != null) {
                try {
                    list.add(new BigInteger(line, 16).longValue());
                } catch (NumberFormatException e) {
                }
            }
            in.close();
        } catch (IOException e) {
            try {
                if (in != null) in.close();
            } catch (IOException e1) {
            }
        }
        return list;
    }

    public static List<String> readStringFromFile(String path) {
        File file = new File(path);
        BufferedReader in = null;
        List<String> list = new ArrayList<>();
        if (file == null || !file.exists()) return list;
        try {
            in = new BufferedReader(new InputStreamReader(new FileInputStream(file), "UTF-8"));
            String line;
            while ((line = in.readLine()) != null) {
                list.add(line);
            }
            in.close();
        } catch (IOException e) {
            try {
                if (in != null) in.close();
            } catch (IOException e1) {
            }
        }
        return list;
    }

    public static List<File> getFilesInFolder(File currentFolder, String extension) {
        List<File> result = new ArrayList<>();
        if (currentFolder == null || !currentFolder.isDirectory() || !currentFolder.exists()) {
            return result;
        }
        for (File child : currentFolder.listFiles()) {
            if (child.isDirectory()) {
                result.addAll(getFilesInFolder(child, extension));
            } else if (child.getName().endsWith(extension)) {
                result.add(child);
            }
        }

        return result;
    }
}
