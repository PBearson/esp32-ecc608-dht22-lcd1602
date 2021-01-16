#include <stdio.h>
#include <stdlib.h>
#include "driver/i2c.h"
#include "freertos/task.h"

#include "DHT22.h"

#include "cryptoauthlib.h"
#include "LiquidCrystal.h"

#define LCD_ENABLED 0
#define DHT22_ENABLED 0

extern "C"
{
	void app_main(void);
}

float cToF(float c)
{
	return (c * 9/5) + 32;
}

void app_main()
{
	ATCAIfaceCfg cfg;
	cfg.iface_type = ATCA_I2C_IFACE;
	cfg.devtype = ATECC608A;
	cfg.atcai2c.bus	= 0;
	cfg.atcai2c.baud = 400000;
	cfg.wake_delay = 1500;
	cfg.rx_retries = 20;

	LiquidCrystal lcd(19, 23, 18, 17, 16, 0);
	if(LCD_ENABLED)
	{
		lcd.begin(16, 2);
		lcd.print("Starting up...");
	}
	
	// Scan for the correct I2C address
	int ret;
	for(int i = 0; i < 127; i++)
	{
		lcd.clear();
		cfg.atcai2c.slave_address = i;
		lcd.setCursor(0, 0);
		lcd.print("Trying addr.");
		lcd.setCursor(0, 1);
		lcd.print(i);
		ret = atcab_init(&cfg);
		if(ret == 0)
		{
			printf("ECC608 initialized at address %02X\n", i);
			break;
		}
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

	// Display first 32 bytes of the key on the ECC608
	for(int i = 0; i < 16; i++)
	{
		for(int j = 0; j < 2; j++)
		{
			lcd.setCursor(i, j);
			lcd.print(temppubkey[i * (j + 1)]);
		}
	}

	vTaskDelay(2000 / portTICK_RATE_MS);

	if(DHT22_ENABLED) setDHTgpio(22);
	
	if(LCD_ENABLED) lcd.clear();

	if(!(DHT22_ENABLED && LCD_ENABLED)) return;

	while(1)
	{
		int ret = readDHT();
		errorHandler(ret);
		float hum = getHumidity();
		float tmp = cToF(getTemperature());
		printf("\nHumidity:\t%.1f\n", hum);
		printf("Temperature:\t%.1f\n", tmp);
		lcd.setCursor(0, 0);
		lcd.print("Temp: ");
		lcd.print(tmp);
		lcd.setCursor(0, 1);
		lcd.print("Hum: ");
		lcd.print(hum);
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}
