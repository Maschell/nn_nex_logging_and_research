package de.mas.wiiu.tools.function_finder;

import java.io.File;
import java.io.IOException;
import java.math.BigInteger;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.stream.Collectors;

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
import org.riversun.bigdoc.bin.BigFileSearcher;

import de.mas.wiiu.tools.common.ElfFunctionInformation;
import de.mas.wiiu.tools.common.RPXFile;
import de.mas.wiiu.tools.common.RPXLoader;
import de.mas.wiiu.tools.common.Utilities;
import lombok.extern.java.Log;

@Log
public class StarterFunctionFinder {
    private static String RPX_WITH_SYMBOLS_FOLDER = "rpx_with_symbols";
    private static String RPX_WITHOUT_SYMBOLS_FOLDER = "rpx_without_symbols";
    private static int SIGNATURE_LENGTH = 32;
    private static boolean LOG = false;

    private static final String PARAMETER_INPUT_FILE = "in";
    private static final String PARAMETER_FUNCTION_NAME = "function";
    private static final String PARAMETER_MAX_SIGNATURE_LENGTH = "max-signature-length";
    private static final String PARAMETER_FILTER_FILE = "rpx-id-filter";
    private static final String PARAMETER_VERBOSE_LOGGING = "verbose-logging";
    private static final String PARAMETER_EXTRA_FUNCTIONS = "extra-function-names";
    private static final String PARAMETER_RPX_WITH_SYMBOLS_FOLDER = "rpx-with-symbols-folder";

    private static List<Long> RPX_FILTER = new ArrayList<>();

    public static void main(String[] args) throws ParseException, IOException {
        System.out.println("RPX function finder - 0.1 - by Maschell");
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
        new StarterFunctionFinder(cmd);
    }

    public StarterFunctionFinder(CommandLine cmd) throws IOException {
        String toCheck = "";
        String searchForFunction = "";

        if (cmd.hasOption(PARAMETER_INPUT_FILE)) {
            toCheck = cmd.getOptionValue(PARAMETER_INPUT_FILE);
        }
        boolean noFunction = true;
        if (cmd.hasOption(PARAMETER_FUNCTION_NAME)) {
            searchForFunction = cmd.getOptionValue(PARAMETER_FUNCTION_NAME);
            noFunction = false;
        }

        if (cmd.hasOption(PARAMETER_FILTER_FILE)) {
            String filterFile = cmd.getOptionValue(PARAMETER_FILTER_FILE);
            System.out.println("Reading filter from file: " + filterFile);
            RPX_FILTER = Utilities.readLongFromFile(filterFile);
            System.out.println("Found " + RPX_FILTER.size() + " elements in the filter list");
        }
        if (cmd.hasOption(PARAMETER_MAX_SIGNATURE_LENGTH)) {
            try {
                SIGNATURE_LENGTH = Integer.parseInt(cmd.getOptionValue(PARAMETER_MAX_SIGNATURE_LENGTH));
                System.out.println("Set maximum signature length to " + SIGNATURE_LENGTH);
            } catch (NumberFormatException e) {
                System.out.println("Error while parsing the maximum signature length from argument." + e.getMessage());
                return;
            }
        }
        
        if(cmd.hasOption(PARAMETER_RPX_WITH_SYMBOLS_FOLDER)){
            RPX_WITH_SYMBOLS_FOLDER = cmd.getOptionValue(PARAMETER_RPX_WITH_SYMBOLS_FOLDER);
        }

        if (cmd.hasOption(PARAMETER_VERBOSE_LOGGING)) {
            LOG = true;
        }
        
        String extraFunctionNamesPath = "";
        if (cmd.hasOption(PARAMETER_EXTRA_FUNCTIONS)) {
            extraFunctionNamesPath = cmd.getOptionValue(PARAMETER_EXTRA_FUNCTIONS);
            noFunction = false;
        }
        
        if(noFunction){
            System.out.println("Please specify a function you want to find.");
            showHelp(getOptions());
            return;
        }
        String toCheckElf = toCheck + ".elf";

        //System.out.println("Checking \"" + toCheck + "\" for \"" + searchForFunction + "\"");

        RPXFile rpxFile = RPXLoader.getRPXFileByFile(new File(toCheck));
        if (rpxFile.hasSymbols()) {
            System.out.println(toCheck + " has symbols! Searching for the function");
            if (rpxFile.getSymbol(searchForFunction) != null) { // Load function
                System.out.println("Found " + searchForFunction + " at 0x" + Long.toHexString(rpxFile.getSymbol(searchForFunction).value()));
            } else {
                System.out.println("Couldn't find it =(");
            }
            return;
        }

        System.out.println("Looking for possible Signatures in non-stipped .rpx from the folder: " + RPX_WITH_SYMBOLS_FOLDER);

        List<File> rpxs = Utilities.getFilesInFolder(new File(RPX_WITH_SYMBOLS_FOLDER), ".rpx");
        rpxs = rpxs.stream().filter(file -> {
            long titleid = 0;
            try {
                titleid = new BigInteger(file.getParentFile().getName(), 16).longValue();
            } catch (Exception e) {
                return false;
            }
            if (!RPX_FILTER.isEmpty() && !RPX_FILTER.contains(titleid)) {
                return false;
            }
            return true;
        }).collect(Collectors.toList());
        
        Map<String,List<byte[]>> functionDataMap = new HashMap<>();
        
        List<String> functions = new ArrayList<>();
        functions.addAll(Utilities.readStringFromFile(extraFunctionNamesPath));
        //System.out.println("Found " + functions.size() + " functions in file");
        
       
        functions.add(searchForFunction);
        
        for(String function : functions){
            List<byte[]> functionData = new ArrayList<>();
            functionDataMap.put(function, functionData);
            System.out.println("Searching for signatures of " + function);
            for (File file : rpxs) {
                ElfFunctionInformation res = getFunctionFromRPX(file, function);
                if (res != null) {
                    byte data[] = res.getFunctionData();
                    if (data == null) continue;
                    int size = (data.length > SIGNATURE_LENGTH) ? SIGNATURE_LENGTH : data.length - 4;
                    if(size <= 8){
                        System.out.println("Function is too short. Skipping it.");
                        break;
                    }
                    byte shrunkdata[] = Arrays.copyOf(res.getFunctionData(), size);
    
                    boolean notFound = true;
                    for (byte[] d : functionData) {
                        if (Arrays.equals(d, shrunkdata)) {
                            notFound = false;
                            break;
                        }
                    }
                    if (notFound) {
                        functionData.add(shrunkdata);
                    }
                }
            }
        }
        
        for(Entry<String,List<byte[]>> entry : functionDataMap.entrySet()){
            List<byte[]> functionData = entry.getValue();
            System.out.println("Found " + entry.getValue().size() + " different signatures for the function " + entry.getKey());
            long offset = rpxFile.getFunctionsOffset();
            for (byte[] data : functionData) {
                BigFileSearcher searcher = new BigFileSearcher();
                List<Long> findList = searcher.searchBigFile(new File(toCheckElf), data);
                if (findList.size() > 0) {
                    for (Long res : findList) {
                        System.out.println("Found possible match at: 0x" + Long.toHexString(offset + res));
                    }
                }
            }
        }        
       
        System.out.println("Searching for the function in: " + toCheck);
        System.out.println("done.");
    }

    private ElfFunctionInformation getFunctionFromRPX(File file, String function) {
        ElfFunctionInformation result = new ElfFunctionInformation(function);
        RPXFile rpxFile;
        try {
            rpxFile = RPXLoader.getRPXFileByFile(file);
        } catch (IOException e) {
            log.info(e.getMessage());
            return result;
        }
        if (rpxFile != null) {
            result.setFunctionData(rpxFile.getFunctionData(function));
        }
        return result;
    }

    private static Options getOptions() {
        Options options = new Options();
        options.addOption(
                Option.builder("in").longOpt(PARAMETER_INPUT_FILE).argName("input file").hasArg().desc("Input file. Can be a .rpx or .rpx").required().build());
        options.addOption(Option.builder("f").longOpt(PARAMETER_FUNCTION_NAME).argName("The function name to be searched in the input file").hasArg().desc("").build());
        options.addOption(Option.builder("m").longOpt(PARAMETER_MAX_SIGNATURE_LENGTH).hasArg()
                .desc("Maximum length of the signature that will be checked. Default is " + SIGNATURE_LENGTH).optionalArg(true).build());
        options.addOption(Option.builder("t").longOpt(PARAMETER_FILTER_FILE).hasArg().argName("file").desc("A file containing a list of Hex Strings representing a list of TitleIDs in the folder " + RPX_WITH_SYMBOLS_FOLDER + " that should be used.").build());
        options.addOption(Option.builder("v").longOpt(PARAMETER_VERBOSE_LOGGING).desc("Enables verbose logging").build());
        options.addOption(Option.builder("e").longOpt(PARAMETER_EXTRA_FUNCTIONS).argName("filename").hasArg().desc("A file contains a list of functions that should be checked.").build());
        options.addOption(Option.builder("s").longOpt(PARAMETER_RPX_WITH_SYMBOLS_FOLDER).hasArg().argName("filename").desc("Sets the path to the folder which contains folders (named the TITLEID) which contain the RPX files with symbols.=> A folder containing this: /1234567812345678/test.rpx /1234567812345698/test2.rpx").build());

        options.addOption("help", false, "Shows this text");

        return options;
    }

    private static void showHelp(Options options) {
        HelpFormatter formatter = new HelpFormatter();
        formatter.setWidth(100);
        formatter.printHelp(" ", options);
    }
}
