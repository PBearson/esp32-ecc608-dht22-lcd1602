#include <stdio.h>
#include <stdlib.h>
#include "driver/i2c.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"
#include "esp_err.h"

#include "DHT22.h"

#include "cryptoauthlib.h"

#include "ili9340.h"
#include "fontx.h"

// Change these options to suit your build
#define DHT22_ENABLED 1
#define DEFAULT_I2C_ADDRESS 0xC0
#define SEARCH_I2C_ADDRESS 0
#define DHT22_PIN 22

// LCD parameters
#define CONFIG_WIDTH 80
#define CONFIG_HEIGHT 160
#define CONFIG_OFFSETX 26
#define CONFIG_OFFSETY 1
#define CONFIG_MOSI_GPIO 23
#define CONFIG_SCLK_GPIO 18
#define CONFIG_CS_GPIO 5
#define CONFIG_DC_GPIO 15
#define CONFIG_RESET_GPIO 4
#define CONFIG_BL_GPIO 32

static const char *TAG = "NSF_EDU";

extern "C"
{
	void app_main(void);
}

float cToF(float c)
{
	return (c * 9/5) + 32;
}

void initialize_spiffs()
{
	ESP_LOGI(TAG, "Initializing SPIFFS");

	esp_vfs_spiffs_conf_t conf = {
		.base_path = "/spiffs",
		.partition_label = NULL,
		.max_files = 10,
		.format_if_mount_failed =true
	};

	// Use settings defined above toinitialize and mount SPIFFS filesystem.
	// Note: esp_vfs_spiffs_register is anall-in-one convenience function.
	esp_err_t ret = esp_vfs_spiffs_register(&conf);

	if (ret != ESP_OK) {
		if (ret == ESP_FAIL) {
			ESP_LOGE(TAG, "Failed to mount or format filesystem");
		} else if (ret == ESP_ERR_NOT_FOUND) {
			ESP_LOGE(TAG, "Failed to find SPIFFS partition");
		} else {
			ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)",esp_err_to_name(ret));
		}
		return;
	}

	size_t total = 0, used = 0;
	ret = esp_spiffs_info(NULL, &total,&used);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG,"Failed to get SPIFFS partition information (%s)",esp_err_to_name(ret));
	} else {
		ESP_LOGI(TAG,"Partition size: total: %d, used: %d", total, used);
	}
}

void app_main()
{
	initialize_spiffs();

	FontxFile fx16G[2];
	InitFontx(fx16G,"/spiffs/ILGH16XB.FNT",""); // 12x24Dot Gothic
	
	TFT_t dev;
	spi_master_init(&dev, CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO, CONFIG_CS_GPIO, CONFIG_DC_GPIO, CONFIG_RESET_GPIO, CONFIG_BL_GPIO);

	uint16_t model = 0x7735;

	lcdInit(&dev, model, CONFIG_WIDTH, CONFIG_HEIGHT, CONFIG_OFFSETX, CONFIG_OFFSETY);

	lcdInversionOn(&dev);

	uint16_t xpos = 0;
	uint16_t ypos = 0;

	// Multi Font Test
	uint16_t color;
	uint8_t ascii[40];
	lcdFillScreen(&dev, WHITE);
	color = BLACK;
	lcdSetFontDirection(&dev, 1);

	xpos = 50;
	ypos = 5;
	strcpy((char *)ascii, "Hello from NSF");
	lcdDrawString(&dev, fx16G, xpos, ypos, ascii, color);

	ATCAIfaceCfg cfg;
	cfg.iface_type = ATCA_I2C_IFACE;
	cfg.devtype = ATECC608A;
	cfg.atcai2c.bus	= 0;
	cfg.atcai2c.baud = 400000;
	cfg.wake_delay = 1500;
	cfg.rx_retries = 20;

	// Scan for the correct I2C address
	int ret;
	if(SEARCH_I2C_ADDRESS)
	{
		for(int i = 0; i < 256; i++)
		{
			cfg.atcai2c.slave_address = i;
			ret = atcab_init(&cfg);
			printf("Checking address %02X\n", i);
			if(ret == 0)
			{
				printf("ECC608 initialized at address %02X\n", i);
				break;
			}
		}
	}
	else
	{
		cfg.atcai2c.slave_address = DEFAULT_I2C_ADDRESS;
		ret = atcab_init(&cfg);
		printf("Checking address %02X\n", DEFAULT_I2C_ADDRESS);
	}
	assert(ret == 0);

	// Get a random key from the ECC608
	uint8_t temppubkey[64];
	assert(0 == atcab_genkey(ATCA_TEMPKEY_KEYID, temppubkey));
	printf("\nYour temporary public key:\n");
	for(int i =  1; i <= 64; i++)
	{
		printf("%02X ", temppubkey[i - 1]);
		if(i % 8 == 0) printf("\n");
	}

	// Display first 32 bytes of the key on the display
	// TODO

	vTaskDelay(2000 / portTICK_RATE_MS);

	if(DHT22_ENABLED) setDHTgpio(DHT22_PIN);

	while(DHT22_ENABLED)
	{
		int ret = readDHT();
		errorHandler(ret);
		float hum = getHumidity();
		float tmp = cToF(getTemperature());
		printf("\nHumidity:\t%.1f%%\n", hum);
		printf("Temperature:\t%.1f° F\n", tmp);
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}
