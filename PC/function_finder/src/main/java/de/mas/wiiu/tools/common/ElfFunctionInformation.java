package de.mas.wiiu.tools.common;

import lombok.Data;

@Data
public class ElfFunctionInformation {
    private final String name;
    private byte[] functionData = null;
}
