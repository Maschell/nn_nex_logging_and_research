package de.mas.wiiu.tools.common;

import java.io.File;
import java.io.IOException;
import java.lang.ProcessBuilder.Redirect;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

import lombok.Getter;

public class RPL2ELF {
    private String filePath;
    
    private static RPL2ELF instance = null;
    
    public static RPL2ELF getInstance() throws IOException{
        if(instance == null) instance = new RPL2ELF();
        return instance;
    }
    
    private RPL2ELF() throws IOException {
        this("rpl2elf.exe");
    }

    private RPL2ELF(String filePath) throws IOException {
        this.filePath = filePath;
        initialize();
    }

    public void initialize() throws IOException {
        Path library = Paths.get(filePath);

        if (!Files.exists(library)) {
            throw new IOException("Missing " + filePath);
        }
    }

    public void unpack(String inputFile) throws Exception {
        runProcess(filePath, inputFile, inputFile + ".elf");
    }

    private void runProcess(String filePath, String inputFile, String outputFile) throws IOException, InterruptedException {
        ProcessBuilder processBuilder = new ProcessBuilder();
        processBuilder.command(filePath, inputFile, outputFile);
        File log = new File("rpl2elf.log");
        processBuilder.redirectErrorStream(true);
        processBuilder.redirectOutput(Redirect.appendTo(log));
        Process process = processBuilder.start();
        process.waitFor();
    }
}