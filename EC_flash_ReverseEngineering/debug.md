From 0x4000 (16kB), 5 bytes are read into variable gROM, 3F 00 21 41 00
The ROM file size must be 128 kB, (0x20000)
EC EEPROM is 4kB, from 116k to 120k (0x1D000 - 0x1DFFF)
System EEPROM is 8kB, from 120k to 128k (0x1E000 - 0x1FFFF)
It's all 0xFF in the ROM update file from 0x1D000.
Version style : V%1X%1XT%1X

KBC Command list:
0x30 read gRamID
0x31 read gRamSVer
