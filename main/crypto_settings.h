// I2C master port on host
// #define I2C_PORT                0

// SDA pin
// #define SDA_GPIO                4

// SCL pin
// #define SCL_GPIO                15

// I2C baud rate
#define BAUD_SPEED              400000

// Cryptochip type
#define ECCX08_TYPE             "608"

// Cryptochip address
#define ECCX08_ADDRESS          0X5A

#include "cryptoauthlib.h"

extern "C"
{
	i2c_port_t I2C_PORT = I2C_NUM_0;
	gpio_num_t SDA_GPIO = GPIO_NUM_4;
	gpio_num_t SCL_GPIO = GPIO_NUM_15;
	ATCAIfaceCfg atca_cfg_init(void);
	int i2c_init(void);
}



/*
 * Initialize ATCA configuration
 */
ATCAIfaceCfg atca_cfg_init()
{
	ATCADeviceType type;

	if(strcmp(ECCX08_TYPE, "608") == 0) type = ATECC608A;
	else if(strcmp(ECCX08_TYPE, "508") == 0) type = ATECC508A;
	else if(strcmp(ECCX08_TYPE, "204") == 0) type = ATSHA204A;
	else if(strcmp(ECCX08_TYPE, "108") == 0) type = ATECC108A;
	else type = ATCA_DEV_UNKNOWN;

	// ATCAIfaceCfg cfg = 
	// {
	// 	.iface_type 			= ATCA_I2C_IFACE,
	// 	.devtype			= type,
	// 	.atcai2c.slave_address		= ECCX08_ADDRESS,
	// 	.atcai2c.bus			= I2C_PORT + 1,
	// 	.atcai2c.baud			= BAUD_SPEED,
	// 	.wake_delay			= 1500,
	// 	.rx_retries			= 20
	// };
	ATCAIfaceCfg cfg;
	cfg.iface_type = ATCA_I2C_IFACE;
	cfg.devtype = type;
	cfg.atcai2c.slave_address = ECCX08_ADDRESS;
	cfg.atcai2c.bus	= I2C_PORT + 1;
	cfg.atcai2c.baud = BAUD_SPEED;
	cfg.wake_delay = 1500;
	cfg.rx_retries = 20;

	return cfg;
}

/*
 * Initialize I2C configuration
 */
int i2c_init()
{
	i2c_config_t conf;
	conf.mode 				= I2C_MODE_MASTER;
	conf.sda_io_num			= SDA_GPIO;
	conf.sda_pullup_en			= GPIO_PULLUP_ENABLE;
	conf.scl_io_num			= SCL_GPIO;
	conf.scl_pullup_en			= GPIO_PULLUP_ENABLE;
	conf.master.clk_speed		= BAUD_SPEED;

	i2c_param_config(I2C_PORT, &conf);

	return i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0);
}
