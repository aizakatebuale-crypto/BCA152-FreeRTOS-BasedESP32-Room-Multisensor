#include "oled.h"
#include "font5x7.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include <string.h>

#define OLED_ADDR       0x3C
#define OLED_WIDTH      128
#define OLED_PAGES      8

static i2c_master_bus_handle_t s_bus;
static i2c_master_dev_handle_t s_dev;
static uint8_t s_framebuffer[OLED_WIDTH * OLED_PAGES];

static esp_err_t oled_cmd(uint8_t cmd)
{
    const uint8_t buf[2] = { 0x00, cmd };
    return i2c_master_transmit(s_dev, buf, 2, 100);
}

esp_err_t oled_init(gpio_num_t sda, gpio_num_t scl)
{
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = -1,
        .sda_io_num = sda,
        .scl_io_num = scl,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    esp_err_t err = i2c_new_master_bus(&bus_cfg, &s_bus);
    if (err != ESP_OK) return err;

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = OLED_ADDR,
        .scl_speed_hz = 400000,
    };
    err = i2c_master_bus_add_device(s_bus, &dev_cfg, &s_dev);
    if (err != ESP_OK) return err;

    const uint8_t init_cmds[] = {
        0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40,
        0x8D, 0x14, 0x20, 0x00, 0xA1, 0xC8, 0xDA, 0x12,
        0x81, 0xCF, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6,
        0xAF
    };
    for (size_t i = 0; i < sizeof(init_cmds); i++) {
        err = oled_cmd(init_cmds[i]);
        if (err != ESP_OK) return err;
    }

    memset(s_framebuffer, 0, sizeof(s_framebuffer));
    return ESP_OK;
}

void oled_clear(void)
{
    memset(s_framebuffer, 0, sizeof(s_framebuffer));
}

void oled_draw_text(int page, int col, const char *text)
{
    int x = col;
    for (const char *p = text; *p != '\0' && x < OLED_WIDTH - 5; p++) {
        const uint8_t *glyph = NULL;
        for (size_t i = 0; i < FONT5X7_COUNT; i++) {
            if (FONT5X7[i].ch == *p) {
                glyph = FONT5X7[i].cols;
                break;
            }
        }
        if (glyph != NULL) {
            for (int c = 0; c < 5; c++) {
                s_framebuffer[page * OLED_WIDTH + x + c] = glyph[c];
            }
        }
        x += 6;
    }
}

void oled_display(void)
{
    for (int page = 0; page < OLED_PAGES; page++) {
        oled_cmd(0xB0 + page);
        oled_cmd(0x00);
        oled_cmd(0x10);

        uint8_t buf[OLED_WIDTH + 1];
        buf[0] = 0x40;
        memcpy(&buf[1], &s_framebuffer[page * OLED_WIDTH], OLED_WIDTH);
        i2c_master_transmit(s_dev, buf, OLED_WIDTH + 1, 100);
    }
}