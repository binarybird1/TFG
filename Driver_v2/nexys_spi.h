
#ifndef NEXYS_SPI_H_
#define NEXYS_SPI_H_

#include "leon3_types.h"

#define ADXL_WRITE      	0x0a
#define ADXL_READ       	0x0b

#define ADXL_DEVID_AD		0x00	// Analog Devices device ID: 0xAD
#define ADXL_DEVID_MST		0x01	// Analog Devices MEMS device ID: 0x1D
#define ADXL_PARTID		0x02	// Device ID: 0xF2
#define ADXL_REVID		0x03	// Product revision ID

/* -- RO data -- */
#define ADXL_XDATA8             0x08
#define ADXL_YDATA8             0x09
#define ADXL_ZDATA8             0x0A
#define ADXL_STATUS             0x0B
#define ADXL_FIFO_ENTRIES_L     0x0C
#define ADXL_FIFO_ENTRIES_H     0x0D
#define ADXL_XDATAL             0x0e
#define ADXL_XDATAH             0x0f
#define ADXL_YDATAL             0x10
#define ADXL_YDATAH             0x11
#define ADXL_ZDATAL             0x12
#define ADXL_ZDATAH             0x13
#define ADXL_TEMPL              0x14
#define ADXL_TEMPH              0x15
#define ADXL_X_ADCL             0x16
#define ADXL_X_ADCH             0x17

/* -- Control and Config -- */
#define ADXL_SOFT_RESET         0x1f
#define ADXL_THRESH_ACTL        0x20
#define ADXL_THRESH_ACTH        0x21
#define ADXL_TIME_ACT           0x22
#define ADXL_THRESH_INACTL      0x23
#define ADXL_THRESH_INACTH      0x24
#define ADXL_TIME_INACTL        0x25
#define ADXL_TIME_INACTH        0x26
#define ADXL_ACT_INACT_CTL      0x27
#define ADXL_FIFO_CONTROL       0x28
#define ADXL_FIFO_SAMPLES       0x29
#define ADXL_INTMAP1            0x2a
#define ADXL_INTMAP2            0x2b
#define ADXL_FILTER_CTL         0x2c
#define ADXL_POWER_CTL          0x2d
#define ADXL_SELF_TEST          0x2e

struct SPI_regs
{
	volatile uint32_t Capability0;
	volatile uint32_t Capability1;
	volatile uint32_t Reserved2;
	volatile uint32_t Reserved3;
	volatile uint32_t Reserved4;
	volatile uint32_t Reserved5;
	volatile uint32_t Reserved6;
	volatile uint32_t Reserved7;
	volatile uint32_t Mode;
	volatile uint32_t Event;
	volatile uint32_t Mask;
	volatile uint32_t Command;
	volatile uint32_t Transmit;
	volatile uint32_t Receive;
	volatile uint32_t SlaveSelect;
};

struct config_adxl_mode_free_fall_struct
{

	// Configures the accelerometer to ±x g range,
	uint32_t Parameter1;

	// Configures ODR 100 Hz (x hz)
	uint32_t Parameter2;

	// desired free fall threshold Low
	uint32_t Parameter3;
	// desired free fall threshold High
	uint32_t Parameter3_H;

	// Free fall time  	
	// minimum amount of time that the acceleration on all axes must be less
	//than the free fall threshold to generate a free fall condition
	uint32_t Parameter4;
};

struct config_adxl_mode_motion_switch_struct
{

	// Configures the accelerometer to ±x g 
	uint32_t Parameter1;

	// Configures ODR  (x hz)
	uint32_t Parameter2;

	// desired inactivity threshold
	uint32_t Parameter3;
	uint32_t Parameter3_H;

	// desired inactivity time threshold
	uint32_t Parameter4;
	uint32_t Parameter4_H;

	// desired activity threshold
	uint32_t Parameter5;
	uint32_t Parameter5_H;
};

void config_spi_adxl(void);
void config_adxl_select_mode(void);
void config_adxl_mode_free_fall(void);
void config_custom_mode_params_freefall(void);
void config_adxl_mode_free_fall_custom (struct config_adxl_mode_free_fall_struct);
void config_custom_mode_params_motion_switch(void);
void config_adxl_custom_mode_autonomous_switch(struct config_adxl_mode_motion_switch_struct);
void config_adxl_mode_autonomous_switch(void);
uint8_t read_nexys_adxl(uint8_t reg_id);
void write_nexys_adxl(uint8_t reg_id, uint8_t value);

#endif /* NEXYS_SPI_H_ */
