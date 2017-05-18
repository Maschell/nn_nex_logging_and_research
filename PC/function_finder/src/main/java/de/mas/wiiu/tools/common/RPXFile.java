package de.mas.wiiu.tools.common;

import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;

import lombok.extern.java.Log;
import one.elf.ElfReader;
import one.elf.ElfSection;
import one.elf.ElfSymbol;
import one.elf.ElfSymbolTable;

@Log
public class RPXFile {
    final String rpx_path;
    final String elf_path;
    final ElfReader elf_reader;
    private ElfSymbolTable symbol_table = null;
    private ElfSection text_section = null;
    public static int MAX_FUNCTION_LENGTH_TO_COPY = 0;
    
    
    private static final int MIN_SYMBOLS = 2000;
    private RandomAccessFile raf = null;

    public RPXFile(File f, boolean replaceElf) throws IOException {
        rpx_path = f.getAbsolutePath();
        elf_path = rpx_path + ".elf";

        File toCheckRPXFile = new File(rpx_path);
        if (toCheckRPXFile == null || !toCheckRPXFile.exists()) {
            throw new IOException("File not found: " + rpx_path);
        }

        RPL2ELF tool = RPL2ELF.getInstance();

        File toCheckelfFile = new File(elf_path);
        if (replaceElf || (toCheckelfFile == null || !toCheckelfFile.exists())) {
            try {
                log.fine("Decompressing " + rpx_path);
                tool.unpack(rpx_path);
            } catch (Exception e) {
                throw new IOException("Exception while decompressing: " + rpx_path + ": " + e.getMessage());
            }
        } else {
            log.fine(rpx_path + " was already decompressed");
        }

        log.fine("Loading " + elf_path);

        try {
            elf_reader = new ElfReader(new File(elf_path));
        } catch (Exception e) {
            throw new IOException("Exception while loading: " + elf_path + ": " + e.getMessage());
        }
        
        raf = new RandomAccessFile(elf_path, "r");
        
    }

    public ElfSymbol getSymbol(String name) {        
        return getSymbolTable().symbol(name);
    }

    public byte[] getFunctionData(String symbol_name) {
        return getFunctionData(getSymbol(symbol_name));
    }

    public byte[] getFunctionData(ElfSymbol symbol) {
        if(raf == null) return null;
        try {
            if (symbol != null) {
                long offset = getTextSection().address() - getTextSection().offset();

                long offsetInElfFile = symbol.value() - offset;
                long size = symbol.size();
                
                if (offsetInElfFile < 0) {
                    return null;
                }
                raf.seek(offsetInElfFile);

                byte[] data = new byte[(int) size];
                raf.read(data);

                return data;
            }
        } catch (IOException e) {
            log.info(e.getMessage());
            return null;
        }
        return null;
    }
    
    public ElfSymbolTable getSymbolTable(){
        if (symbol_table == null) symbol_table = (ElfSymbolTable) elf_reader.section(".symtab");
        return symbol_table;
    }
    
    public ElfSection getTextSection(){
        if (text_section == null) text_section = elf_reader.section(".text");
        return text_section;
    }
    
    public boolean hasSymbols(){       
        return ((getSymbolTable().symbol("start") != null || getSymbolTable().symbol("_start") != null) && getSymbolTable().size() > MIN_SYMBOLS);
    }

    public ElfSymbol[] getAllSymbols() {
        return getSymbolTable().symbols();
    }

    public long getFunctionsOffset() {
        return getTextSection().address() - getTextSection().offset();
    }
}
