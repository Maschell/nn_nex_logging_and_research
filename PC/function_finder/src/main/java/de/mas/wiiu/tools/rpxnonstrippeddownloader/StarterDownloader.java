package de.mas.wiiu.tools.rpxnonstrippeddownloader;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.StandardCopyOption;
import java.util.ArrayList;
import java.util.List;
import java.util.logging.Handler;
import java.util.logging.Level;
import java.util.logging.LogManager;
import java.util.logging.Logger;

import de.mas.wiiu.jnus.DecryptionService;
import de.mas.wiiu.jnus.NUSTitle;
import de.mas.wiiu.jnus.NUSTitleLoaderRemote;
import de.mas.wiiu.jnus.Settings;
import de.mas.wiiu.jnus.entities.fst.FSTEntry;
import de.mas.wiiu.jnus.utils.Utils;
import de.mas.wiiu.tools.common.RPXFile;
import de.mas.wiiu.tools.common.RPXLoader;
import de.mas.wiiu.tools.common.Utilities;

public class StarterDownloader {
    private static final String TEMP_PATH = "temp";
    private static final String TARGET_PATH_WITH_SYMBOL = "rpx_with_symbols";
    private static final String TARGET_PATH_WITHOUT_SYMBOL = "rpx_without_symbols";
    private static final String TITLEID_LIST_PATH = "donwloadTitleIDs.txt";

    public static void main(String[] args) throws Exception {
        System.out.println("RPX Downloader - 0.1 - by Maschell");
        System.out.println("using one-elf by (https://github.com/odnoklassniki/one-elf)");
        System.out.println("using rpl2elf by Hykem (https://github.com/Relys/rpl2elf)");
        System.out.println();
        Logger log = LogManager.getLogManager().getLogger("");
        for (Handler h : log.getHandlers()) {
            h.setLevel(Level.WARNING);
        }
        System.out.println("Reading common key from common.key");
        readKey();
        List<Long> titleIDs = new ArrayList<>();
        System.out.println("Looking in  " + TITLEID_LIST_PATH + " for Title IDs");
        titleIDs.addAll(Utilities.readLongFromFile(TITLEID_LIST_PATH));
        System.out.println("Found " + titleIDs.size() + " titleIDs that will be downloaded");
        for (Long titleId : titleIDs) {
            System.out.print(String.format("%016X: ", titleId));
            NUSTitle nusRemote = null;
            try{
             nusRemote = NUSTitleLoaderRemote.loadNUSTitle(titleId);
            }catch(Exception e){
                System.out.println("Loading failed. Proabably the common key is wrong?");
                continue;
            }
            DecryptionService decrypt = DecryptionService.getInstance(nusRemote);

            List<FSTEntry> rpx_entries = nusRemote.getFSTEntriesByRegEx("/code/.*.rpx");
            if (rpx_entries.isEmpty()) return;

            FSTEntry rpx_entry = rpx_entries.get(0);

            if (rpx_entry != null) {
                System.out.print("loading " + rpx_entry.getFilename() + "...");
                try {
                    decrypt.decryptFSTEntryTo(false, rpx_entry, TEMP_PATH, true);
                } catch (Exception e) {
                    System.out.println("Error while loading the RPX: " + e.getMessage());
                    continue;
                }
                String filename = TEMP_PATH + File.separator + rpx_entry.getFilename();
                RPXFile rpxFile = RPXLoader.getRPXFileByFile(new File(filename),true);
                if (rpxFile == null) continue;

                String downloadPath = TARGET_PATH_WITHOUT_SYMBOL + File.separator + String.format("%016X", nusRemote.getTMD().getTitleID());

                if (rpxFile.hasSymbols()) {
                    System.out.print("has symbols!");
                    downloadPath = TARGET_PATH_WITH_SYMBOL + File.separator + String.format("%016X", nusRemote.getTMD().getTitleID());
                } else {
                    System.out.print("has no symbols...");
                }

                System.out.print("Saving it to " + downloadPath);
                File target = new File(downloadPath + File.separator + rpx_entry.getFilename());
                if (target == null || !target.exists()) {
                    Utils.createDir(downloadPath);
                    File source = new File(TEMP_PATH + File.separator + rpx_entry.getFilename());
                    if (source != null && source.exists()) {
                        try {
                            Files.copy(source.toPath(), target.toPath(), StandardCopyOption.REPLACE_EXISTING);
                            System.out.print(".success");
                        } catch (IOException e) {
                            System.out.print(".failed");
                        }
                    } else {
                        System.out.print(".failed");
                    }
                } else {
                    System.out.print(". File already exists.");
                }
            }
            System.out.println();
        }
        System.out.println("done");
    }
    
    private static void readKey() throws IOException {
        File file = new File("common.key");
        if(file.isFile()){
            byte[] key = Files.readAllBytes(file.toPath());
            Settings.commonKey = key;
            System.out.println("Commonkey was loaded from file");
        }
    }
}
