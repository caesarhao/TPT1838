#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SYSFS_EC_FLASH_MODE  "/sys/EcControl/ECflashMode"
#define SYSFS_EC_FLASH_READ  "/sys/EcControl/ECflashread"
#define SYSFS_EC_FLASH_WRITE "/sys/EcControl/ECflashwrite"

#define ROM_EXPECTED_SIZE 0x20000UL
#define ROM_INFO_OFFSET   0x4000L
#define BLOCK_SIZE        0x20U

enum {
    CMD_READ_ID_VER_A = 0x30,
    CMD_READ_ID_VER_B = 0x31,
    CMD_READ_BASE_OFF = 0xB1,
    CMD_READ_STATUS   = 0xB0,
    CMD_PHASE_CTRL    = 0xB7,
    CMD_SET_OFFSET    = 0xB8,
    CMD_POLL_PROGRESS = 0xB6,
    CMD_STREAM_DATA   = 0xB9,
    CMD_FINISH_MODE   = 0xBA,
};

enum {
    PHASE_NORMAL      = 1,
    PHASE_WITH_BBL    = 2,
    PHASE_FINALIZE    = 4,
};

typedef struct {
    uint8_t rom_hdr[5];
    uint8_t ram_id;
    uint8_t ram_mver;
    uint8_t ram_sver;
    uint8_t ram_tver;

    uint8_t erase_mask;
    bool include_bootloader;
    bool ignore_nid;
    bool ota_mode;
    bool sd_mode;

    char rom_file[256];
    unsigned long file_size;
    uint32_t file_offset;
    uint32_t operation_volume;
} flash_ctx_t;

static void print_version(void) {
    puts("***************************************************************************** \\r");
    puts("*  ENE Flash Utility for ARM platform                                       * \\r");
    printf("*  V%u.%u.%u                                                   Created by LJC  * \\r\\n", 1U, 0U, 2U);
    puts("*                                              Modified based on Nick, RYC  * \\r");
    puts("***************************************************************************** \\r");
}

static void print_usage(void) {
    puts("\\r");
    puts("Usage: EC_flash [-bbl] [-e1] [-e2] RomFileName   \\r\\n");
    puts(" -bbl  Flash boot block too                      \\r");
    puts(" -e1   Erase EC EEPROM 116K - 120K               \\r");
    puts(" -e2   Erase system EEPROM 120K - 128K           \\r\\n");
    puts(" -nid  Ignore ROM ID/size guard                  \\r");
    puts(" -ota  OTA reboot mode                           \\r");
    puts(" -sd   SD reboot mode                            \\r\\n");
}

static void exit_pro(int rc, int code) {
    if (code != 0) {
        puts("ERROR\\r");
        puts("******************************************************************************\\r");
    }

    switch (code) {
    case 0:
        break;
    case 1:
        puts("WARNING : DOSKEY INSTALLED. PLEASE DO NOT INSTALL DOSKEY.\\r");
        break;
    case 2:
        puts("WARNING : Please Check AC.\\r");
        break;
    case 3:
        puts("FAIL TO OPEN PORT!!\\r");
        break;
    case 4:
        puts("Project ID cannot be obtained.\\r");
        break;
    case 5:
        puts("Index I/O port can't be reached. \\r");
        puts("Please Open this I/O Range from South Bridge... \\r");
        break;
    case 6:
        puts("UnSupported KBC ID !! ....Check with EnE FAE....\\r");
        break;
    case 7:
        puts("Start Address + Operation Volume > Flash Size.. Error!!!\\r");
        break;
    case 8:
        puts(" Volume size Error or Start Address Error!!!\\r");
        break;
    case 9:
        puts("ROM Crashed!!\\r");
        break;
    case 10:
        puts("59 F6 failed.\\r");
        break;
    case 11:
        puts("59 F7 failed.\\r");
        break;
    case 12:
        puts("59 F8 failed.\\r");
        break;
    case 13:
        puts("59 F9 failed.\\r");
        break;
    default:
        puts("\\r");
        break;
    }

    if (code != 0) {
        puts("******************************************************************************\\r");
    }
    exit(rc);
}

static bool disable_batterypolling(void) {
    uint8_t mode = 0x31;
    int fd = open(SYSFS_EC_FLASH_MODE, O_RDWR);
    if (fd < 0) {
        puts("error: cannot open EC device file!");
        return false;
    }

    if (write(fd, &mode, 1) < 0) {
        puts("error: write ec node.");
        close(fd);
        return false;
    }

    close(fd);
    return true;
}

static bool read_ec_device(uint8_t cmd, uint8_t out[2]) {
    int fd = open(SYSFS_EC_FLASH_READ, O_RDWR);
    if (fd < 0) {
        puts("peter ==== error: cannot open EC device file!");
        return false;
    }

    if (write(fd, &cmd, 1) < 0) {
        puts("peter ===== error: write ec node.");
        close(fd);
        return false;
    }

    if (lseek(fd, 0, SEEK_SET) < 0) {
        puts("peter ===== error: lseek ec node.");
        close(fd);
        return false;
    }

    if (read(fd, out, 2) < 0) {
        puts("peter ===== error: read ec node.");
        close(fd);
        return false;
    }

    close(fd);
    return true;
}

static bool write_ec_device(uint8_t cmd, const uint8_t data[2], int data_len) {
    uint8_t frame[3];
    int fd;

    frame[0] = cmd;
    frame[1] = data[0];
    frame[2] = (data_len == 1) ? 0 : data[1];

    fd = open(SYSFS_EC_FLASH_WRITE, O_RDWR);
    if (fd < 0) {
        puts("peter ==== error: cannot open EC device file!");
        return false;
    }

    if (write(fd, frame, sizeof(frame)) < 0) {
        puts("peter ===== error: write ec node.");
        close(fd);
        return false;
    }

    close(fd);
    return true;
}

static bool kbc_cmd_datas(uint8_t cmd, uint8_t io[2], int mode, uint8_t out[2]) {
    if (mode < 1) {
        if (!read_ec_device(cmd, out)) {
            puts("peter ==== read_ec_device fail");
            return false;
        }
    } else {
        if (!write_ec_device(cmd, io, mode)) {
            puts("peter ==== write_ec_device fail");
            return false;
        }
    }
    return true;
}

static uint8_t crc8_calculator(const uint8_t *buf, uint8_t len) {
    uint8_t crc = buf[0];

    for (uint8_t i = 0; i < len; ++i) {
        uint8_t val = buf[i];
        for (uint8_t bit = 0; bit < 8; ++bit) {
            if ((int8_t)crc < 0) {
                crc = (uint8_t)(((val >> 7) | (crc << 1)) ^ 0x07);
            } else {
                crc = (uint8_t)((val >> 7) | (crc << 1));
            }
            val <<= 1;
        }
    }

    return (uint8_t)(crc ^ 0x80);
}

static void lower_inplace(char *s) {
    while (*s) {
        *s = (char)tolower((unsigned char)*s);
        ++s;
    }
}

static bool parse_args(int argc, char **argv, flash_ctx_t *ctx) {
    bool have_action = false;

    memset(ctx, 0, sizeof(*ctx));

    for (int i = 1; i < argc; ++i) {
        char tmp[128];
        strncpy(tmp, argv[i], sizeof(tmp) - 1);
        tmp[sizeof(tmp) - 1] = '\0';
        lower_inplace(tmp);

        if (strncmp(tmp, "-d", 2) == 0) {
            have_action = true;
        } else if (strncmp(tmp, "-bbl", 4) == 0) {
            ctx->include_bootloader = true;
            have_action = true;
        } else if (strncmp(tmp, "-e1", 3) == 0) {
            ctx->erase_mask |= 0x01;
        } else if (strncmp(tmp, "-e2", 3) == 0) {
            ctx->erase_mask |= 0x02;
        } else if (strncmp(tmp, "-nid", 4) == 0) {
            ctx->ignore_nid = true;
        } else if (strncmp(tmp, "-ota", 4) == 0) {
            ctx->ota_mode = true;
        } else if (strncmp(tmp, "-sd", 3) == 0) {
            ctx->sd_mode = true;
        } else if (tmp[0] != '-') {
            strncpy(ctx->rom_file, tmp, sizeof(ctx->rom_file) - 1);
            ctx->rom_file[sizeof(ctx->rom_file) - 1] = '\0';
            have_action = true;
        }
    }

    return have_action && ctx->rom_file[0] != '\0';
}

int main(int argc, char **argv) {
    flash_ctx_t ctx;
    FILE *fp;
    uint8_t io[2] = {0, 0};
    uint8_t resp[2] = {0, 0};
    uint8_t data_block[BLOCK_SIZE];
    uint32_t base_offset;
    uint32_t remaining;
    uint32_t temp = 0;
    bool first_temp = false;
    bool need_rejump = false;

    print_version();

    if (!parse_args(argc, argv, &ctx)) {
        print_usage();
        return 1;
    }

    printf("File Name : [%s]", ctx.rom_file);
    fp = fopen(ctx.rom_file, "rb");
    if (!fp) {
        printf("\\r\\nOpen File       : %s => Failed !!!\\r\\n", ctx.rom_file);
        exit_pro(1, 0);
    }

    fseek(fp, ROM_INFO_OFFSET, SEEK_SET);
    fread(ctx.rom_hdr, 1, sizeof(ctx.rom_hdr), fp);

    fseek(fp, 0, SEEK_END);
    ctx.file_size = (unsigned long)ftell(fp);
    fseek(fp, 0, SEEK_SET);
    printf(",[0x%lX Bytes]\\n\\r", ctx.file_size);

    disable_batterypolling();
    usleep(1000000);
    usleep(1000000);

    if (!kbc_cmd_datas(CMD_READ_ID_VER_A, io, 0, resp)) {
        fclose(fp);
        return 1;
    }
    ctx.ram_id = resp[0];
    ctx.ram_mver = resp[1];

    usleep(100000);

    if (!kbc_cmd_datas(CMD_READ_ID_VER_B, io, 0, resp)) {
        fclose(fp);
        return 1;
    }
    ctx.ram_sver = resp[0];
    ctx.ram_tver = resp[1];

    printf("File EC Version : V%1X%1XT%1X\\n\\r", ctx.rom_hdr[1], ctx.rom_hdr[2], ctx.rom_hdr[4]);
    printf("Current EC Version : V%1X%1XT%1X\\n\\r", ctx.ram_mver, ctx.ram_sver, ctx.ram_tver);

    if (((ctx.rom_hdr[0] != ctx.ram_id) || (ctx.file_size != ROM_EXPECTED_SIZE)) && !ctx.ignore_nid) {
        puts("ROM file was broken!!           \\r\\n");
        fclose(fp);
        exit_pro(1, 0);
    }

    if (!kbc_cmd_datas(CMD_READ_BASE_OFF, io, 0, resp)) {
        fclose(fp);
        return 1;
    }
    base_offset = ((uint32_t)resp[0] << 8) | resp[1];

    if (!kbc_cmd_datas(CMD_READ_STATUS, io, 0, resp)) {
        fclose(fp);
        return 1;
    }

    if ((resp[0] == 0x55) && ctx.include_bootloader) {
        ctx.include_bootloader = false;
        puts("Boot block will not be updated. \\r");
    }

    if (!ctx.include_bootloader) {
        io[0] = PHASE_NORMAL;
        remaining = (uint32_t)(ctx.file_size - base_offset);
    } else {
        io[0] = PHASE_WITH_BBL;
        remaining = (uint32_t)ctx.file_size;
    }
    io[1] = ctx.erase_mask;

    if (!kbc_cmd_datas(CMD_PHASE_CTRL, io, 2, resp)) {
        fclose(fp);
        return 1;
    }

    usleep(500000);

    io[0] = (uint8_t)((uint32_t)ctx.file_size - remaining);
    io[1] = (uint8_t)(((uint32_t)ctx.file_size - remaining) >> 8);
    if (!kbc_cmd_datas(CMD_SET_OFFSET, io, 2, resp)) {
        fclose(fp);
        return 1;
    }

    while (remaining != 0) {
        if (need_rejump) {
            need_rejump = false;
            usleep(200000);
            io[0] = PHASE_NORMAL;
            io[1] = ctx.erase_mask;
            if (!kbc_cmd_datas(CMD_PHASE_CTRL, io, 2, resp)) {
                fclose(fp);
                return 1;
            }
            usleep(2000000);
            if (!kbc_cmd_datas(CMD_READ_STATUS, io, 0, resp)) {
                fclose(fp);
                return 1;
            }
            if (resp[0] != 0x55) {
                puts("\\r");
                puts("Failed to jump boot block. Process stopped! \\r");
                break;
            }
            io[0] = (uint8_t)base_offset;
            io[1] = (uint8_t)(base_offset >> 8);
            if (!kbc_cmd_datas(CMD_SET_OFFSET, io, 2, resp)) {
                fclose(fp);
                return 1;
            }
        }

        if (!kbc_cmd_datas(CMD_POLL_PROGRESS, io, 0, resp)) {
            fclose(fp);
            return 1;
        }

        if (resp[0] == 0 || resp[0] == 1 || resp[0] == 2) {
            unsigned int pct = (unsigned int)(((ctx.file_size - remaining) * 100UL) / ctx.file_size);
            printf("Flash complete: [%u%%]  \\r", pct);

            if (resp[0] == 1) {
                remaining -= BLOCK_SIZE;
                if (((uint32_t)ctx.file_size - base_offset) == remaining) {
                    need_rejump = true;
                } else if (remaining < 0x4001U) {
                    remaining = 0;
                }

                if (first_temp) {
                    ++temp;
                }
                first_temp = true;
            }

            if (!ctx.include_bootloader) {
                fseek(fp, (long)(temp * BLOCK_SIZE + base_offset), SEEK_SET);
            } else {
                fseek(fp, (long)(temp * BLOCK_SIZE), SEEK_SET);
            }

            for (size_t i = 0; i < BLOCK_SIZE; ++i) {
                int c = fgetc(fp);
                data_block[i] = (uint8_t)(c == EOF ? 0xFF : c);
            }

            for (size_t i = 0; i < BLOCK_SIZE; i += 2) {
                io[0] = data_block[i];
                io[1] = data_block[i + 1];
                usleep(100);
                if (!kbc_cmd_datas(CMD_STREAM_DATA, io, 2, resp)) {
                    fclose(fp);
                    return 1;
                }
            }

            io[0] = crc8_calculator(data_block, BLOCK_SIZE);
            io[1] = (uint8_t)temp;
            if (!kbc_cmd_datas(CMD_STREAM_DATA, io, 2, resp)) {
                fclose(fp);
                return 1;
            }
        }

        usleep(5000);
    }

    printf("Flash complete: [100%%]   \\r\\n");
    fclose(fp);

    usleep(50000);
    io[0] = PHASE_FINALIZE;
    io[1] = 0;
    if (!kbc_cmd_datas(CMD_PHASE_CTRL, io, 2, resp)) {
        return 1;
    }

    usleep(50000);
    if (ctx.ota_mode) {
        io[0] = 1;
    } else if (ctx.sd_mode) {
        io[0] = 2;
    } else {
        io[0] = 3;
    }
    io[1] = 0;

    if (!kbc_cmd_datas(CMD_FINISH_MODE, io, 2, resp)) {
        return 1;
    }

    puts("Flash Finish. Please wait for system restart ... \\r\\n");
    return 0;
}
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define APP_VER_MAJOR 1
#define APP_VER_MINOR 0
#define APP_VER_PATCH 2

#define ROM_SIZE_EXPECTED 0x20000UL
#define BLOCK_SIZE 0x20U

#define CMD_GET_ID_MAIN_VER 0x30
#define CMD_GET_SUB_TEST_VER 0x31
#define CMD_GET_FLASH_BASE   0xB1
#define CMD_GET_BOOT_STATUS  0xB0
#define CMD_SET_PHASE        0xB7
#define CMD_SET_OFFSET       0xB8
#define CMD_POLL_PROGRESS    0xB6
#define CMD_WRITE_DATA       0xB9
#define CMD_SET_REBOOT_MODE  0xBA

#define EC_FLASH_MODE_NODE  "/sys/EcControl/ECflashMode"
#define EC_FLASH_READ_NODE  "/sys/EcControl/ECflashread"
#define EC_FLASH_WRITE_NODE "/sys/EcControl/ECflashwrite"

static uint8_t g_erase_block;
static uint8_t g_boot_loader;
static uint8_t g_nid;
static uint8_t g_ota;
static uint8_t g_flash_sd;

static uint8_t g_rom[5];
static uint8_t g_ram_id;
static uint8_t g_ram_mver;
static uint8_t g_ram_sver;
static uint8_t g_ram_tver;

static uint8_t g_data_array[BLOCK_SIZE];
static uint8_t g_send_data[2];

static char g_file_name[256];
static uint32_t g_file_offset;
static uint32_t g_operation_volume;
static unsigned long g_file_size;

static void to_lower_inplace(char *s) {
    while (*s) {
        *s = (char)tolower((unsigned char)*s);
        ++s;
    }
}

static void display_version(void) {
    puts("***************************************************************************** \\r");
    puts("*  ENE Flash Utility for ARM platform                                       * \\r");
    printf("*  V%u.%u.%u                                                   Created by LJC  * \\r\\n",
           APP_VER_MAJOR, APP_VER_MINOR, APP_VER_PATCH);
    puts("*                                              Modified based on Nick, RYC  * \\r");
    puts("***************************************************************************** \\r");
}

static void display_usage(void) {
    puts("\\r");
    puts("Usage: EC_flash [-bbl] [-e1] [-e2] RomFileName   \\r\\n");
    puts(" -bbl  Flash boot block too                      \\r");
    puts(" -e1   Erase EC EEPROM 116K - 120K               \\r");
    puts(" -e2   Erase system EEPROM 120K - 128K           \\r\\n");
}

static void disable_battery_polling(void) {
    int fd;
    uint8_t mode = 0x31;

    fd = open(EC_FLASH_MODE_NODE, O_RDWR);
    if (fd < 0) {
        puts("error: cannot open EC device file!");
        return;
    }

    if (write(fd, &mode, 1) < 0) {
        puts("error: write ec node.");
    }
    close(fd);
}

static int read_ec_device(uint8_t cmd, uint8_t out[2]) {
    int fd;

    fd = open(EC_FLASH_READ_NODE, O_RDWR);
    if (fd < 0) {
        puts("peter ==== error: cannot open EC device file!");
        return 0;
    }

    if (write(fd, &cmd, 1) < 0) {
        puts("peter ===== error: write ec node.");
        close(fd);
        return 0;
    }

    if (lseek(fd, 0, SEEK_SET) < 0) {
        puts("peter ===== error: lseek ec node.");
        close(fd);
        return 0;
    }

    if (read(fd, out, 2) < 0) {
        puts("peter ===== error: read ec node.");
        close(fd);
        return 0;
    }

    close(fd);
    return 1;
}

static int write_ec_device(uint8_t cmd, const uint8_t *data, int len) {
    int fd;
    uint8_t frame[3];

    frame[0] = cmd;
    frame[1] = data[0];
    frame[2] = (len == 1) ? 0 : data[1];

    fd = open(EC_FLASH_WRITE_NODE, O_RDWR);
    if (fd < 0) {
        puts("peter ==== error: cannot open EC device file!");
        return 0;
    }

    if (write(fd, frame, sizeof(frame)) < 0) {
        puts("peter ===== error: write ec node.");
        close(fd);
        return 0;
    }

    close(fd);
    return 1;
}

static int kbc_cmd_datas(uint8_t cmd, uint8_t *data, int mode, uint8_t out[2]) {
    if (mode < 1) {
        if (!read_ec_device(cmd, out)) {
            puts("peter ==== read_ec_device fail");
            return 0;
        }
    } else {
        if (!write_ec_device(cmd, data, mode)) {
            puts("peter ==== write_ec_device fail");
            return 0;
        }
    }
    return 1;
}

static uint8_t crc8_calculator(const uint8_t *buf, uint8_t len) {
    uint8_t crc = buf[0];

    for (uint8_t b = 0; b < len; ++b) {
        uint8_t d = buf[b];
        for (uint8_t i = 0; i < 8; ++i) {
            if ((int8_t)crc < 0) {
                crc = (uint8_t)(((d >> 7) | (crc << 1)) ^ 0x07U);
            } else {
                crc = (uint8_t)((d >> 7) | (crc << 1));
            }
            d <<= 1;
        }
    }
    return (uint8_t)(crc ^ 0x80U);
}

static void exit_pro(int ret, int code) {
    if (code != 0) {
        puts("ERROR\\r");
        puts("******************************************************************************\\r");
    }

    switch (code) {
    case 0:
        break;
    case 1:
        puts("WARNING : DOSKEY INSTALLED. PLEASE DO NOT INSTALL DOSKEY.\\r");
        break;
    case 2:
        puts("WARNING : Please Check AC.\\r");
        break;
    case 3:
        puts("FAIL TO OPEN PORT!!\\r");
        break;
    case 4:
        puts("Project ID cannot be obtained.\\r");
        break;
    case 5:
        puts("Index I/O port can't be reached. \\r");
        puts("Please Open this I/O Range from South Bridge... \\r");
        break;
    case 6:
        puts("UnSupported KBC ID !! ....Check with EnE FAE....\\r");
        break;
    case 7:
        puts("Start Address + Operation Volume > Flash Size.. Error!!!\\r");
        break;
    case 8:
        puts(" Volume size Error or Start Address Error!!!\\r");
        break;
    case 9:
        puts("ROM Crashed!!\\r");
        break;
    case 10:
        puts("59 F6 failed.\\r");
        break;
    case 11:
        puts("59 F7 failed.\\r");
        break;
    case 12:
        puts("59 F8 failed.\\r");
        break;
    case 13:
        puts("59 F9 failed.\\r");
        break;
    default:
        puts("\\r");
        break;
    }

    if (code != 0) {
        puts("******************************************************************************\\r");
    }
    exit(ret);
}

int main(int argc, char **argv) {
    int mode = 0;
    uint8_t got_arg = 0;
    bool need_rejump = false;
    bool first_temp = false;
    uint8_t temp = 0;
    uint8_t re_data[2] = {0};
    FILE *fp = NULL;
    unsigned long remaining;
    uint16_t flash_base;

    g_operation_volume = 0;
    g_file_offset = 0;
    g_file_size = 0;

    display_version();

    for (int i = 1; i < argc; ++i) {
        char local[80];
        size_t n = strlen(argv[i]);
        if (n >= sizeof(local)) {
            n = sizeof(local) - 1;
        }
        memcpy(local, argv[i], n);
        local[n] = '\0';
        to_lower_inplace(local);

        if (strncmp("-d", local, 2) == 0) {
            mode = 2;
            got_arg = 1;
        } else if (strncmp("-bbl", local, 4) == 0) {
            g_boot_loader = 1;
            if (mode != 2) {
                mode = 1;
            }
        } else if (strncmp("-e1", local, 3) == 0) {
            g_erase_block |= 0x01;
        } else if (strncmp("-e2", local, 3) == 0) {
            g_erase_block |= 0x02;
        } else if (strncmp("-nid", local, 4) == 0) {
            g_nid = 1;
        } else if (strncmp("-ota", local, 4) == 0) {
            g_ota = 1;
        } else if (strncmp("-sd", local, 3) == 0) {
            g_flash_sd = 1;
        } else if (local[0] != '-') {
            strncpy(g_file_name, local, sizeof(g_file_name) - 1);
            g_file_name[sizeof(g_file_name) - 1] = '\0';
            if (mode != 2) {
                mode = 1;
            }
            got_arg = 1;
        }
    }

    if (!got_arg || mode == 0) {
        display_usage();
        return 1;
    }

    if (mode == 1) {
        printf("File Name : [%s]", g_file_name);
        fp = fopen(g_file_name, "rb");
        if (fp == NULL) {
            printf("\\r\\nOpen File       : %s => Failed !!!\\r\\n", g_file_name);
            exit_pro(1, 0);
        }

        fseek(fp, 0x4000, SEEK_SET);
        fread(g_rom, 1, sizeof(g_rom), fp);

        fseek(fp, 0, SEEK_END);
        g_file_size = (unsigned long)ftell(fp);
        fseek(fp, 0, SEEK_SET);
        printf(",[0x%lX Bytes]\\n\\r", g_file_size);

        disable_battery_polling();
        usleep(1000000);
        usleep(1000000);
    }

    if (mode != 2 && mode == 1) {
        if (!kbc_cmd_datas(CMD_GET_ID_MAIN_VER, g_send_data, 0, re_data)) {
            exit_pro(1, 0);
        }
        g_ram_id = re_data[0];
        g_ram_mver = re_data[1];

        usleep(100000);

        if (!kbc_cmd_datas(CMD_GET_SUB_TEST_VER, g_send_data, 0, re_data)) {
            exit_pro(1, 0);
        }
        g_ram_sver = re_data[0];
        g_ram_tver = re_data[1];

        printf("File EC Version : V%1X%1XT%1X\\n\\r", g_rom[1], g_rom[2], g_rom[4]);
        printf("Current EC Version : V%1X%1XT%1X\\n\\r", g_ram_mver, g_ram_sver, g_ram_tver);

        if (((g_rom[0] != g_ram_id) || (g_file_size != ROM_SIZE_EXPECTED)) && !g_nid) {
            puts("ROM file was broken!!           \\r\\n");
            exit_pro(1, 0);
        }

        if (!kbc_cmd_datas(CMD_GET_FLASH_BASE, g_send_data, 0, re_data)) {
            exit_pro(1, 0);
        }
        flash_base = (uint16_t)(((uint16_t)re_data[0] << 8) | re_data[1]);

        if (!kbc_cmd_datas(CMD_GET_BOOT_STATUS, g_send_data, 0, re_data)) {
            exit_pro(1, 0);
        }
        if (re_data[0] == 0x55 && g_boot_loader) {
            g_boot_loader = 0;
            puts("Boot block will not be updated. \\r");
        }

        if (!g_boot_loader) {
            g_send_data[0] = 1;
            remaining = g_file_size - flash_base;
        } else {
            g_send_data[0] = 2;
            remaining = g_file_size;
        }
        g_send_data[1] = g_erase_block;

        if (!kbc_cmd_datas(CMD_SET_PHASE, g_send_data, 2, re_data)) {
            exit_pro(1, 0);
        }

        usleep(500000);

        {
            unsigned int start_off = (unsigned int)(g_file_size - remaining);
            g_send_data[0] = (uint8_t)(start_off & 0xFFU);
            g_send_data[1] = (uint8_t)((start_off >> 8) & 0xFFU);
        }

        if (!kbc_cmd_datas(CMD_SET_OFFSET, g_send_data, 2, re_data)) {
            exit_pro(1, 0);
        }

        while (remaining != 0) {
            if (need_rejump) {
                need_rejump = false;
                usleep(200000);

                g_send_data[0] = 1;
                g_send_data[1] = g_erase_block;
                if (!kbc_cmd_datas(CMD_SET_PHASE, g_send_data, 2, re_data)) {
                    break;
                }

                usleep(2000000);

                if (!kbc_cmd_datas(CMD_GET_BOOT_STATUS, g_send_data, 0, re_data)) {
                    break;
                }
                if (re_data[0] != 0x55) {
                    puts("\\r");
                    puts("Failed to jump boot block. Process stopped! \\r");
                    break;
                }

                g_send_data[0] = (uint8_t)(flash_base & 0xFFU);
                g_send_data[1] = (uint8_t)((flash_base >> 8) & 0xFFU);
                if (!kbc_cmd_datas(CMD_SET_OFFSET, g_send_data, 2, re_data)) {
                    break;
                }
            }

            if (!kbc_cmd_datas(CMD_POLL_PROGRESS, g_send_data, 0, re_data)) {
                break;
            }

            if (re_data[0] == 0 || re_data[0] == 1 || re_data[0] == 2) {
                unsigned int pct = (unsigned int)(((g_file_size - remaining) * 100U) / g_file_size);
                printf("Flash complete: [%u%%]  \\r", pct);

                if (re_data[0] == 1) {
                    remaining -= BLOCK_SIZE;
                    if ((g_file_size - flash_base) == remaining) {
                        need_rejump = true;
                    } else if (remaining < 0x4001UL) {
                        remaining = 0;
                    }

                    if (first_temp) {
                        ++temp;
                    }
                    first_temp = true;
                }

                if (!g_boot_loader) {
                    fseek(fp, (long)(temp * BLOCK_SIZE + flash_base), SEEK_SET);
                } else {
                    fseek(fp, (long)(temp * BLOCK_SIZE), SEEK_SET);
                }

                for (unsigned int i = 0; i < BLOCK_SIZE; ++i) {
                    int c = fgetc(fp);
                    g_data_array[i] = (c == EOF) ? 0 : (uint8_t)c;
                }

                for (unsigned int i = 0; i < BLOCK_SIZE; ++i) {
                    if (i == 0 || (i & 1U) == 0) {
                        g_send_data[0] = g_data_array[i];
                    } else {
                        g_send_data[1] = g_data_array[i];
                        usleep(100);
                        if (!kbc_cmd_datas(CMD_WRITE_DATA, g_send_data, 2, re_data)) {
                            break;
                        }
                    }
                }

                g_send_data[0] = crc8_calculator(g_data_array, BLOCK_SIZE);
                g_send_data[1] = temp;
                if (!kbc_cmd_datas(CMD_WRITE_DATA, g_send_data, 2, re_data)) {
                    break;
                }
            }

            usleep(5000);
        }

        printf("Flash complete: [100%%]   \\r\\n");
        fclose(fp);

        usleep(50000);
        g_send_data[0] = 4;
        g_send_data[1] = 0;
        kbc_cmd_datas(CMD_SET_PHASE, g_send_data, 2, re_data);

        usleep(50000);
        if (!g_ota) {
            g_send_data[0] = g_flash_sd ? 2 : 3;
        } else {
            g_send_data[0] = 1;
        }
        g_send_data[1] = 0;
        kbc_cmd_datas(CMD_SET_REBOOT_MODE, g_send_data, 2, re_data);

        puts("Flash Finish. Please wait for system restart ... \\r\\n");
    }

    return 0;
}
