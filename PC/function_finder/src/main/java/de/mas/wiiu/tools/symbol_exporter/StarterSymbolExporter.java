package de.mas.wiiu.tools.symbol_exporter;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;

import org.apache.commons.cli.CommandLine;
import org.apache.commons.cli.CommandLineParser;
import org.apache.commons.cli.DefaultParser;
import org.apache.commons.cli.HelpFormatter;
import org.apache.commons.cli.MissingArgumentException;
import org.apache.commons.cli.MissingOptionException;
import org.apache.commons.cli.Option;
import org.apache.commons.cli.Options;
import org.apache.commons.cli.ParseException;
import org.apache.commons.cli.UnrecognizedOptionException;

import de.mas.wiiu.tools.common.RPXFile;
import de.mas.wiiu.tools.common.RPXLoader;
import lombok.extern.java.Log;
import one.elf.ElfSymbol;

@Log
public class StarterSymbolExporter {
    private static final String DEMANGLER_FILE_PATH = "ghs-demangle.exe";

    private static final String PARAMETER_INPUT_FILE = "in";
    private static final String PARAMETER_FUNCTION_FILTER = "function-name-filter";
    private static final String PARAMETER_OUTPUT_ADDRESS = "output-address";
    private static final String PARAMETER_DEMANGLE = "demangle-function-name";
    private static final String PARAMETER_OUTFILE = "output-file";

    public static void main(String args[]) throws IOException, ParseException {
        String filter = null;
        String inputFile = "";
        boolean output_address = false;
        boolean demangle = false;

        System.out.println("Symbol Exporter - 0.1 - by Maschell");
        System.out.println("using ghs-demangle by Chadderz (https://github.com/Chadderz121/ghs-demangle)");
        System.out.println("using one-elf by (https://github.com/odnoklassniki/one-elf)");
        System.out.println("using rpl2elf by Hykem (https://github.com/Relys/rpl2elf)");  
        System.out.println();
        Options options = getOptions();

        CommandLineParser parser = new DefaultParser();
        CommandLine cmd = null;
        try {
            cmd = parser.parse(options, args);
        } catch (MissingArgumentException e) {
            System.out.println(e.getMessage());
            showHelp(options);
            return;
        } catch (MissingOptionException e) {
            System.out.println(e.getMessage());
            showHelp(options);
            return;
        } catch (UnrecognizedOptionException e) {
            System.out.println(e.getMessage());
            showHelp(options);
            return;
        }

        if (cmd.hasOption(PARAMETER_INPUT_FILE)) {
            inputFile = cmd.getOptionValue(PARAMETER_INPUT_FILE);
        }

        if (cmd.hasOption(PARAMETER_FUNCTION_FILTER)) {
            filter = cmd.getOptionValue(PARAMETER_FUNCTION_FILTER);
        }

        if (cmd.hasOption(PARAMETER_OUTPUT_ADDRESS)) {
            output_address = true;
        }
        if (cmd.hasOption(PARAMETER_DEMANGLE)) {
            demangle = true;
        }
        String outputFile = null;
        if (cmd.hasOption(PARAMETER_OUTFILE)) {
            outputFile = cmd.getOptionValue(PARAMETER_OUTFILE);
        }

        File inputFileF = new File(inputFile);
        if (inputFileF == null || !inputFileF.exists() || inputFileF.isDirectory()) {
            log.info("Invalid input file.");
            return;
        }

        String toCheckfilename_rpx = inputFile;

        File toCheckRPXFile = new File(toCheckfilename_rpx);

        RPXFile rpxFile = RPXLoader.getRPXFileByFile(toCheckRPXFile);
        
        PrintWriter outPrinter = null;
        if(outputFile != null){
            outPrinter = new PrintWriter(outputFile);
        }
        

        BufferedWriter stdin = null;
        BufferedReader stdout = null;
        BufferedReader stderr = null;
        Process p = null;
        if (demangle) {
            File demanglerFile = new File(DEMANGLER_FILE_PATH);
            if (demanglerFile != null && demanglerFile.exists()) {
                ProcessBuilder pb = new ProcessBuilder();
                pb.command(DEMANGLER_FILE_PATH, "-");
                p = pb.start();
                stdin = new BufferedWriter(new OutputStreamWriter(p.getOutputStream()));
                stdout = new BufferedReader(new InputStreamReader(p.getInputStream()));
                stderr = new BufferedReader(new InputStreamReader(p.getErrorStream()));
            } else {
                System.out.println("Missing the file \"" + DEMANGLER_FILE_PATH + "\"");
            }
        }
      
        for (ElfSymbol symbol : rpxFile.getAllSymbols()) {
            if (filter != null && !symbol.name().contains(filter)) {
                continue;
            }

            String output = "";
            if (output_address) output += String.format("0x%08X ", symbol.value());
            boolean demangleSuccess = false;
            if (stdin != null && stdout != null) {
                try {
                    stdin.write(symbol.name() + "\n");
                    stdin.flush();

                    if (stderr.ready()) {
                        stderr.readLine();
                    }
                    String line = stdout.readLine();
                    if (line != null) {
                        output += line + " ";
                    }
                    demangleSuccess = true;
                } catch (Exception e) {
                    stdin = null;
                    stdout = null;
                }
            }
            if (!demangleSuccess) {
                output += symbol.name();
            }
            if(outPrinter != null) outPrinter.println(symbol.name());
            System.out.println(output);
        }
        if (p != null) p.destroy();
        if(outPrinter != null) outPrinter.flush();
    }

    private static Options getOptions() {
        Options options = new Options();
        options.addOption(
                Option.builder("in").longOpt(PARAMETER_INPUT_FILE).argName("input file").hasArg().desc("Input file. Can be a .rpx or .rpx").required().build());
        options.addOption(Option.builder("f").longOpt(PARAMETER_FUNCTION_FILTER).argName("function name")
                .desc("Filter for the function. The function need to contain this string if it's provided").hasArg().desc("").build());
        options.addOption(Option.builder("a").longOpt(PARAMETER_OUTPUT_ADDRESS).desc("Enables the output of the function address").build());
        options.addOption(Option.builder("d").longOpt(PARAMETER_DEMANGLE)
                .desc("Enables the function name demangling. May not work for every function. Requires " + DEMANGLER_FILE_PATH).build());
        options.addOption(Option.builder("out").longOpt(PARAMETER_OUTFILE).hasArg().desc("Where the result will be stored.").build());

        options.addOption("help", false, "Shows this text");

        return options;
    }

    private static void showHelp(Options options) {
        HelpFormatter formatter = new HelpFormatter();
        formatter.setWidth(100);
        formatter.printHelp(" ", options);
    }
}
