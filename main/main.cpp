#include <stdio.h>
#include <stdlib.h>
#include "driver/i2c.h"
#include "freertos/task.h"

#include "DHT22.h"

#include "cryptoauthlib.h"
#include "LiquidCrystal.h"

extern "C"
{
	void app_main(void);
}

int ECCX08_ADDRESS = 0x5A;

float cToF(float c)
{
	return (c * 9/5) + 32;
}

void app_main()
{
	ATCAIfaceCfg cfg;
	cfg.iface_type = ATCA_I2C_IFACE;
	cfg.devtype = ATECC608A;
	cfg.atcai2c.slave_address = ECCX08_ADDRESS;
	cfg.atcai2c.bus	= 0;
	cfg.atcai2c.baud = 400000;
	cfg.wake_delay = 1500;
	cfg.rx_retries = 20;

	LiquidCrystal lcd(19, 23, 18, 17, 16, 15);
	lcd.begin(16, 2);
	lcd.print("Starting up...");

	atcab_init(&cfg);	

	assert (0 == atcab_init(&cfg));

	uint8_t temppubkey[64];
	assert(0 == atcab_genkey(ATCA_TEMPKEY_KEYID, temppubkey));
	printf("\nYour temporary public key:\n");
	for(int i =  1; i <= 64; i++)
	{
		printf("%02X ", temppubkey[i - 1]);
		if(i % 8 == 0) printf("\n");
	}

	setDHTgpio(22);
	lcd.clear();

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
