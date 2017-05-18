package de.mas.wiiu.tools.common;

import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

public class RPXLoader {
    private static final Map<String,RPXFile> rpxFiles = new HashMap<>();
    public static RPXFile getRPXFileByFile(File f) throws IOException{
        return getRPXFileByFile(f,false);
    }
    public static RPXFile getRPXFileByFile(File f,boolean replaceElf) throws IOException{
        if(f == null || !f.exists() || !f.getName().endsWith(".rpx")){
            return null;
        }
        String absolutePath = f.getAbsolutePath();
        RPXFile cached = rpxFiles.get(absolutePath);
        
        if(cached == null){
            cached = new RPXFile(f,replaceElf);
            rpxFiles.put(absolutePath, cached);
        }        
        return cached;
    }
    
    public static void unloadRPX(File f){
        if(f == null || !f.exists() || !f.getName().endsWith(".rpx")){
            return;
        }
        String absolutePath = f.getAbsolutePath();
        rpxFiles.remove(absolutePath);
    }

}
