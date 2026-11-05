/* Local Waveshare 1.75C adaptation for this project (2026-11-05), derived
 * from this project's board_waveshare_s3_143c.c and therefore from the Muse
 * board interfaces. Original notices are retained below. */
/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Waveshare ESP32-S3-Touch-AMOLED-1.75C: 466 px round AMOLED (CO5300, QSPI),
 * CST9217 touch, ES8311 playback and ES7210 capture, AXP2101 PMIC,
 * ESP32-S3R8 (8 MB octal PSRAM) with 32 MB flash on current units.
 * Hardware references (not copied; pins and sequences cross-checked):
 *  - waveshare/esp32_s3_touch_amoled_1_75c BSP 3.0.0 (Apache-2.0) from the
 *    ESP Component Registry: pins, CO5300 init table, gap, touch mirroring
 *    and codec gain.
 *  - waveshare/esp_lcd_touch_cst9217 2.0.0 (Apache-2.0): point register
 *    0xD000, 10-byte report, 0xAB ack byte and coordinate packing.
 *  - waveshareteam/ESP32-S3-Touch-AMOLED-1.75C at 4bfdc80, examples/esp-idf/
 *    01_AXP2101: AXP2101 status, ADC and ALDO1 (microphone supply) registers.
 *  - Liu-zhongxian/zongzhu-bot at 62ebe83, xiaozhi-esp32/main/boards/
 *    waveshare/esp32-s3-touch-amoled-1.75 (the _1_75C branch of config.h).
 * Only ALDO1 is switched on the PMIC; the remaining rails and the charger
 * keep their power-on configuration. The PWR key stays a hardware control.
 */

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "driver/i2s_std.h"
#include "driver/spi_master.h"
#include "driver/usb_serial_jtag.h"
#include "es7210_adc.h"
#include "es8311_codec.h"
#include "esp_check.h"
#include "esp_codec_dev_defaults.h"
#include "esp_lcd_co5300.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "esp_lv_adapter.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "muse_audio.h"
#include "muse_board.h"
#include "muse_lcd_bands.h"
#include "muse_mem.h"

static const char *TAG = "board";

#define LCD_RES 466
#define LCD_GAP_X 6
#define LCD_HOST SPI2_HOST
#define LCD_D0 GPIO_NUM_4
#define LCD_D1 GPIO_NUM_5
#define LCD_D2 GPIO_NUM_6
#define LCD_D3 GPIO_NUM_7
#define LCD_CS GPIO_NUM_12
#define LCD_SCLK GPIO_NUM_38
#define LCD_RST GPIO_NUM_1
#define DRAW_BUF_LINES 118
#define LCD_CHUNK_BYTES (LCD_RES * 8 * 2)

#define I2C_SDA GPIO_NUM_15
#define I2C_SCL GPIO_NUM_14
#define TOUCH_RST GPIO_NUM_2
#define TOUCH_INT GPIO_NUM_11
#define TOUCH_ADDR 0x5A
#define TOUCH_ACK 0xAB
#define PMU_ADDR 0x34
#define PMU_CHIP_ID 0x4A

#define I2S_MCLK GPIO_NUM_16
#define I2S_BCLK GPIO_NUM_9
#define I2S_WS GPIO_NUM_45
#define I2S_DOUT GPIO_NUM_8
#define I2S_DIN GPIO_NUM_10
#define PA_EN GPIO_NUM_46
#define BOOT_GPIO GPIO_NUM_0

/* AXP2101 registers used here. */
#define AXP_STATUS1 0x00      /* bit 5 VBUS good, bit 3 battery present */
#define AXP_STATUS2 0x01      /* bits 7:5 = 01 while charging */
#define AXP_IC_TYPE 0x03
#define AXP_ADC_CTRL 0x30     /* bit 0 battery voltage measurement */
#define AXP_VBAT_H 0x34       /* battery voltage in mV: 5 high bits here, 8 in 0x35 */
#define AXP_LDO_ONOFF0 0x90   /* bit 0 ALDO1 */
#define AXP_ALDO1_VOLT 0x92   /* 500 mV + 100 mV per step */
#define AXP_BATT_PCT 0xA4

static i2c_master_bus_handle_t s_i2c;
static i2c_master_dev_handle_t s_touch;
static i2c_master_dev_handle_t s_pmu;
static esp_lcd_panel_io_handle_t s_io;
static esp_lcd_panel_handle_t s_panel;
static muse_gpio_button_t s_boot;

/* CO5300 table from the Waveshare 1.75C BSP: the FE 20 page selects QSPI. */
static const co5300_lcd_init_cmd_t s_lcd_init[] = {
    { 0xFE, (uint8_t[]){ 0x20 }, 1, 0 },
    { 0x19, (uint8_t[]){ 0x10 }, 1, 0 },
    { 0x1C, (uint8_t[]){ 0xA0 }, 1, 0 },
    { 0xFE, (uint8_t[]){ 0x00 }, 1, 0 },
    { 0xC4, (uint8_t[]){ 0x80 }, 1, 0 },
    { 0x3A, (uint8_t[]){ 0x55 }, 1, 0 },
    { 0x35, (uint8_t[]){ 0x00 }, 1, 0 },
    { 0x53, (uint8_t[]){ 0x20 }, 1, 0 },
    { 0x51, (uint8_t[]){ 0xFF }, 1, 0 },
    { 0x63, (uint8_t[]){ 0xFF }, 1, 0 },
    { 0x2A, (uint8_t[]){ 0x00, 0x06, 0x01, 0xD7 }, 4, 0 },
    { 0x2B, (uint8_t[]){ 0x00, 0x00, 0x01, 0xD1 }, 4, 600 },
    { 0x11, NULL, 0, 600 },
    { 0x29, NULL, 0, 0 },
};

static esp_err_t pmu_read(uint8_t reg, uint8_t *value)
{
    return i2c_master_transmit_receive(s_pmu, &reg, 1, value, 1, 50);
}

static esp_err_t pmu_write(uint8_t reg, uint8_t value)
{
    const uint8_t data[2] = { reg, value };
    return i2c_master_transmit(s_pmu, data, sizeof(data), 50);
}

static esp_err_t pmu_set_bits(uint8_t reg, uint8_t bits)
{
    uint8_t value;
    ESP_RETURN_ON_ERROR(pmu_read(reg, &value), TAG, "PMU read 0x%02x", reg);
    return (value & bits) == bits ? ESP_OK : pmu_write(reg, value | bits);
}

/* The ES7210 microphones are fed from ALDO1. Voltage is set before enabling. */
static esp_err_t pmu_init(void)
{
    ESP_RETURN_ON_ERROR(i2c_master_probe(s_i2c, PMU_ADDR, 100), TAG, "AXP2101 probe");
    const i2c_device_config_t pmu = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = PMU_ADDR,
        .scl_speed_hz = 400000,
    };
    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(s_i2c, &pmu, &s_pmu), TAG, "PMU device");
    uint8_t id = 0;
    ESP_RETURN_ON_ERROR(pmu_read(AXP_IC_TYPE, &id), TAG, "PMU id");
    if (id != PMU_CHIP_ID) {
        ESP_LOGW(TAG, "unexpected PMU id 0x%02x", id);
    }
    ESP_RETURN_ON_ERROR(pmu_write(AXP_ALDO1_VOLT, (3300 - 500) / 100), TAG, "ALDO1 3.3 V");
    ESP_RETURN_ON_ERROR(pmu_set_bits(AXP_LDO_ONOFF0, 0x01), TAG, "ALDO1 on");
    ESP_RETURN_ON_ERROR(pmu_set_bits(AXP_ADC_CTRL, 0x01), TAG, "battery ADC");
    return ESP_OK;
}

static esp_err_t init(void)
{
    const gpio_config_t outputs = {
        .pin_bit_mask = (1ULL << PA_EN) | (1ULL << TOUCH_RST),
        .mode = GPIO_MODE_OUTPUT,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&outputs), TAG, "amp/reset pins");
    ESP_RETURN_ON_ERROR(gpio_set_level(PA_EN, 0), TAG, "amp off");

    const gpio_config_t inputs = {
        .pin_bit_mask = 1ULL << TOUCH_INT,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&inputs), TAG, "touch interrupt pin");

    const i2c_master_bus_config_t bus = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = I2C_SDA,
        .scl_io_num = I2C_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&bus, &s_i2c), TAG, "i2c");

    /* Without the PMU the screen still works; audio_init reports the codecs. */
    if (pmu_init() != ESP_OK) {
        ESP_LOGE(TAG, "AXP2101 unavailable; microphone supply and battery unknown");
    }

    const i2c_device_config_t tp = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = TOUCH_ADDR,
        .scl_speed_hz = 400000,
    };
    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(s_i2c, &tp, &s_touch), TAG, "touch device");
    /* Reset timing of the Waveshare CST9217 driver; it stays in report mode. */
    ESP_RETURN_ON_ERROR(gpio_set_level(TOUCH_RST, 0), TAG, "touch reset low");
    vTaskDelay(pdMS_TO_TICKS(10));
    ESP_RETURN_ON_ERROR(gpio_set_level(TOUCH_RST, 1), TAG, "touch reset release");
    vTaskDelay(pdMS_TO_TICKS(50));

    ESP_RETURN_ON_ERROR(muse_gpio_button_init(&s_boot, BOOT_GPIO), TAG, "boot button");
    /* A BOOT still held after flashing is not a new talk/setup press. */
    s_boot.pressed = gpio_get_level(BOOT_GPIO) == 0;
    return ESP_OK;
}

static void round_area(lv_event_t *event)
{
    lv_area_t *area = lv_event_get_param(event);
    area->x1 &= ~1;
    area->y1 &= ~1;
    area->x2 |= 1;
    area->y2 |= 1;
}

/* LVGL alone reads the CST9217. The register write and the report read are
 * separate transactions, 2 ms apart, as in the Waveshare driver. */
static void touch_read(lv_indev_t *indev, lv_indev_data_t *data)
{
    (void)indev;
    static const uint8_t reg[2] = { 0xD0, 0x00 };
    uint8_t raw[10];
    data->state = LV_INDEV_STATE_RELEASED;
    if (i2c_master_transmit(s_touch, reg, sizeof(reg), 20) != ESP_OK) {
        return;
    }
    vTaskDelay(pdMS_TO_TICKS(2));
    if (i2c_master_receive(s_touch, raw, sizeof(raw), 20) != ESP_OK ||
        raw[6] != TOUCH_ACK || (raw[5] & 0x7F) == 0 || (raw[0] & 0x0F) != 0x06) {
        return;
    }
    int x = (raw[1] << 4) | (raw[3] >> 4);
    int y = (raw[2] << 4) | (raw[3] & 0x0F);
    if (x >= LCD_RES || y >= LCD_RES) {
        return;
    }
    /* The touch layer is mounted with both axes reversed (BSP mirror_x/y). */
    data->point.x = LCD_RES - 1 - x;
    data->point.y = LCD_RES - 1 - y;
    data->state = LV_INDEV_STATE_PRESSED;
}

/* Called on MUSE_UI_CORE: the SPI interrupt and band sender share a core. */
static lv_display_t *display_start(lv_indev_t **touch)
{
    *touch = NULL;
    const spi_bus_config_t bus =
        CO5300_PANEL_BUS_QSPI_CONFIG(LCD_SCLK, LCD_D0, LCD_D1, LCD_D2, LCD_D3, LCD_CHUNK_BYTES);
    if (spi_bus_initialize(LCD_HOST, &bus, SPI_DMA_CH_AUTO) != ESP_OK) {
        ESP_LOGE(TAG, "LCD SPI bus initialization failed");
        return NULL;
    }
    esp_lcd_panel_io_spi_config_t io = CO5300_PANEL_IO_QSPI_CONFIG(LCD_CS, NULL, NULL);
    io.pclk_hz = 40 * 1000 * 1000;
    io.trans_queue_depth = 2;
    if (esp_lcd_new_panel_io_spi(LCD_HOST, &io, &s_io) != ESP_OK) {
        ESP_LOGE(TAG, "LCD panel IO initialization failed");
        return NULL;
    }
    co5300_vendor_config_t vendor = {
        .init_cmds = s_lcd_init,
        .init_cmds_size = sizeof(s_lcd_init) / sizeof(s_lcd_init[0]),
        .flags.use_qspi_interface = 1,
    };
    const esp_lcd_panel_dev_config_t panel = {
        .reset_gpio_num = LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
        .vendor_config = &vendor,
    };
    if (esp_lcd_new_panel_co5300(s_io, &panel, &s_panel) != ESP_OK ||
        esp_lcd_panel_reset(s_panel) != ESP_OK ||
        esp_lcd_panel_init(s_panel) != ESP_OK ||
        esp_lcd_panel_set_gap(s_panel, LCD_GAP_X, 0) != ESP_OK ||
        esp_lcd_panel_disp_on_off(s_panel, true) != ESP_OK) {
        ESP_LOGE(TAG, "AMOLED initialization failed");
        return NULL;
    }

    esp_lv_adapter_config_t adapter = ESP_LV_ADAPTER_DEFAULT_CONFIG();
    adapter.task_core_id = MUSE_UI_CORE;
    adapter.task_priority = MUSE_UI_PRIORITY;
    if (esp_lv_adapter_init(&adapter) != ESP_OK) {
        return NULL;
    }
    const esp_lv_adapter_display_config_t config = {
        .panel = s_panel,
        .panel_io = s_io,
        .profile = {
            .interface = ESP_LV_ADAPTER_PANEL_IF_OTHER,
            .rotation = ESP_LV_ADAPTER_ROTATE_0,
            .hor_res = LCD_RES,
            .ver_res = LCD_RES,
        },
        .tear_avoid_mode = ESP_LV_ADAPTER_TEAR_AVOID_MODE_NONE,
    };
    lv_display_t *display = muse_lcd_bands_register(config, DRAW_BUF_LINES, LCD_CHUNK_BYTES);
    if (!display) {
        return NULL;
    }
    lv_display_add_event_cb(display, round_area, LV_EVENT_INVALIDATE_AREA, NULL);
    *touch = lv_indev_create();
    if (!*touch) {
        return NULL;
    }
    lv_indev_set_type(*touch, LV_INDEV_TYPE_POINTER);
    lv_indev_set_display(*touch, display);
    lv_indev_set_read_cb(*touch, touch_read);
    if (esp_lv_adapter_start() != ESP_OK) {
        return NULL;
    }
    return display;
}

static bool display_lock(int timeout_ms)
{
    return esp_lv_adapter_lock(timeout_ms) == ESP_OK;
}

static void send_brightness(void *arg)
{
    uint8_t level = *(uint8_t *)arg;
    esp_err_t err = esp_lcd_panel_io_tx_param(s_io, (0x02 << 24) | (0x51 << 8), &level, 1);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "AMOLED brightness: %s", esp_err_to_name(err));
    }
}

static void set_brightness(int pct)
{
    pct = pct < 0 ? 0 : pct > 100 ? 100 : pct;
    uint8_t level = (uint8_t)(pct * 255 / 100);
    muse_lcd_bands_run(send_brightness, &level);
}

static void send_sleep(void *arg)
{
    esp_err_t err = esp_lcd_panel_disp_sleep(s_panel, *(bool *)arg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "AMOLED sleep: %s", esp_err_to_name(err));
    }
    vTaskDelay(pdMS_TO_TICKS(120));
}

static void panel_sleep(bool sleep)
{
    muse_lcd_bands_run(send_sleep, &sleep);
}

static void audio_power(bool on)
{
    esp_err_t err = gpio_set_level(PA_EN, on);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "speaker amplifier: %s", esp_err_to_name(err));
    }
}

static esp_err_t audio_init(esp_codec_dev_handle_t *speaker, esp_codec_dev_handle_t *mic)
{
    *speaker = NULL;
    *mic = NULL;
    ESP_RETURN_ON_ERROR(i2c_master_probe(s_i2c, 0x18, 100), TAG, "ES8311 probe");
    ESP_RETURN_ON_ERROR(i2c_master_probe(s_i2c, 0x40, 100), TAG, "ES7210 probe");
    i2s_chan_handle_t tx, rx;
    i2s_chan_config_t channel = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    channel.auto_clear = true;
    ESP_RETURN_ON_ERROR(i2s_new_channel(&channel, &tx, &rx), TAG, "i2s channels");
    const i2s_std_config_t std = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(MUSE_AUDIO_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_MCLK,
            .bclk = I2S_BCLK,
            .ws = I2S_WS,
            .dout = I2S_DOUT,
            .din = I2S_DIN,
        },
    };
    ESP_RETURN_ON_ERROR(i2s_channel_init_std_mode(tx, &std), TAG, "i2s tx");
    ESP_RETURN_ON_ERROR(i2s_channel_init_std_mode(rx, &std), TAG, "i2s rx");
    ESP_RETURN_ON_ERROR(i2s_channel_enable(tx), TAG, "i2s tx enable");
    ESP_RETURN_ON_ERROR(i2s_channel_enable(rx), TAG, "i2s rx enable");

    const audio_codec_data_if_t *speaker_data = audio_codec_new_i2s_data(&(audio_codec_i2s_cfg_t){
        .port = I2S_NUM_0, .tx_handle = tx });
    const audio_codec_ctrl_if_t *speaker_ctrl = audio_codec_new_i2c_ctrl(&(audio_codec_i2c_cfg_t){
        .port = I2C_NUM_0, .addr = ES8311_CODEC_DEFAULT_ADDR, .bus_handle = s_i2c });
    const audio_codec_gpio_if_t *speaker_gpio = audio_codec_new_gpio();
    ESP_RETURN_ON_FALSE(speaker_data && speaker_ctrl && speaker_gpio, ESP_ERR_NO_MEM, TAG, "speaker interfaces");
    const audio_codec_if_t *speaker_codec = es8311_codec_new(&(es8311_codec_cfg_t){
        .ctrl_if = speaker_ctrl,
        .gpio_if = speaker_gpio,
        .codec_mode = ESP_CODEC_DEV_WORK_MODE_DAC,
        .pa_pin = PA_EN,
        .use_mclk = true,
        /* Gain model of the Waveshare 1.75C BSP. */
        .hw_gain = { .pa_voltage = 5.0, .codec_dac_voltage = 3.3 },
    });
    ESP_RETURN_ON_FALSE(speaker_codec, ESP_FAIL, TAG, "ES8311 codec");
    *speaker = esp_codec_dev_new(&(esp_codec_dev_cfg_t){
        .dev_type = ESP_CODEC_DEV_TYPE_OUT, .codec_if = speaker_codec, .data_if = speaker_data });
    ESP_RETURN_ON_FALSE(*speaker, ESP_FAIL, TAG, "speaker device");

    const audio_codec_data_if_t *mic_data = audio_codec_new_i2s_data(&(audio_codec_i2s_cfg_t){
        .port = I2S_NUM_0, .rx_handle = rx });
    const audio_codec_ctrl_if_t *mic_ctrl = audio_codec_new_i2c_ctrl(&(audio_codec_i2c_cfg_t){
        .port = I2C_NUM_0, .addr = ES7210_CODEC_DEFAULT_ADDR, .bus_handle = s_i2c });
    ESP_RETURN_ON_FALSE(mic_data && mic_ctrl, ESP_ERR_NO_MEM, TAG, "microphone interfaces");
    const audio_codec_if_t *mic_codec = es7210_codec_new(&(es7210_codec_cfg_t){
        .ctrl_if = mic_ctrl,
        /* Schematic: MIC1/MIC2 are the two microphones; MIC3 (ADC_MIC3_P/N)
         * carries the speaker reference, not a microphone. */
        .mic_selected = ES7210_SEL_MIC1 | ES7210_SEL_MIC2,
    });
    ESP_RETURN_ON_FALSE(mic_codec, ESP_FAIL, TAG, "ES7210 codec");
    *mic = esp_codec_dev_new(&(esp_codec_dev_cfg_t){
        .dev_type = ESP_CODEC_DEV_TYPE_IN, .codec_if = mic_codec, .data_if = mic_data });
    ESP_RETURN_ON_FALSE(*mic, ESP_FAIL, TAG, "microphone device");
    ESP_LOGI(TAG, "audio: ES8311 speaker and ES7210 microphones on I2S0");
    return ESP_OK;
}

static unsigned poll_buttons(void)
{
    return muse_gpio_button_poll(&s_boot);
}

static esp_err_t read_power(muse_power_t *power)
{
    power->battery_pct = -1;
    power->battery_mv = 0;
    power->charging = false;
    /* As on the 1.43C, "usb" means a USB host, not merely a charger. */
    power->usb = usb_serial_jtag_is_connected();
    if (!s_pmu) {
        return ESP_OK;
    }
    uint8_t status1, status2;
    ESP_RETURN_ON_ERROR(pmu_read(AXP_STATUS1, &status1), TAG, "PMU status");
    ESP_RETURN_ON_ERROR(pmu_read(AXP_STATUS2, &status2), TAG, "PMU charge status");
    power->charging = (status2 >> 5) == 0x01;
    if (!(status1 & (1u << 3))) {
        return ESP_OK;
    }
    uint8_t reg = AXP_VBAT_H, raw[2], pct;
    ESP_RETURN_ON_ERROR(i2c_master_transmit_receive(s_pmu, &reg, 1, raw, 2, 50), TAG, "battery voltage");
    ESP_RETURN_ON_ERROR(pmu_read(AXP_BATT_PCT, &pct), TAG, "battery level");
    power->battery_mv = ((raw[0] & 0x1F) << 8) | raw[1];
    power->battery_pct = pct > 100 ? 100 : pct;
    return ESP_OK;
}

static const muse_board_t s_board = {
    .name = "Waveshare ESP32-S3-Touch-AMOLED-1.75C",
    .width = LCD_RES,
    .height = LCD_RES,
    .round = true,
    .touch = true,
    .touch_talk = true,
    .diagonal_in = 1.75f,
    .talk_button = "screen microphone / BOOT",
    .talk_hint = { LV_ALIGN_BOTTOM_MID, 0, -42 },
    .frame_ms = 40,
    .init = init,
    .display_start = display_start,
    .display_lock = display_lock,
    .display_unlock = esp_lv_adapter_unlock,
    .set_brightness = set_brightness,
    .panel_sleep = panel_sleep,
    .audio_init = audio_init,
    .audio_power = audio_power,
    .mic_slot = 0,
    .poll_buttons = poll_buttons,
    .read_power = read_power,
    /* No software power_off: the AXP2101 PWR key turns the board off. */
};

const muse_board_t *muse_board_get(void)
{
    return &s_board;
}
