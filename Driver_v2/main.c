
#include <stdio.h>
#include "nexys_spi.h"

int main(){
	uint32_t dummy;
	uint8_t value;
	uint8_t value_XDATA;
	uint8_t value_YDATA;
	uint8_t value_ZDATA;
	uint8_t status_REG;
	uint8_t act_inact_REG;
	uint8_t fifo_REG;
	uint8_t fltr_ctrl_REG;
	uint8_t pwr_ctrl_REG;

	config_spi_adxl();


	value=read_nexys_adxl( ADXL_DEVID_AD );
	printf("Device ID:      %02X\n", value);
	value=read_nexys_adxl( ADXL_DEVID_MST );
	printf("MEMS Device ID: %02X\n", value);
	value=read_nexys_adxl( ADXL_STATUS );
	printf("Status:         %02X\n", value);

	
	config_adxl_select_mode();
	
while(1)
	{
		status_REG=read_nexys_adxl( ADXL_STATUS ); 
    	act_inact_REG=read_nexys_adxl( ADXL_ACT_INACT_CTL );
		fltr_ctrl_REG=read_nexys_adxl( ADXL_FILTER_CTL );
		pwr_ctrl_REG=read_nexys_adxl( ADXL_POWER_CTL );
		
		
		value_XDATA=read_nexys_adxl( ADXL_XDATA8 );
		value_YDATA=read_nexys_adxl( ADXL_YDATA8 );
		value_ZDATA=read_nexys_adxl( ADXL_ZDATA8 );

		printf("X_DATA: %02X Y_DATA: %02X Z_DATA: %02X | STATUS:%02X | ACT/INACT:%02X | FLTR CTRL:%02X | PWR CTRL:%02X\n",
		value_XDATA, value_YDATA,value_ZDATA, status_REG,act_inact_REG,fltr_ctrl_REG,pwr_ctrl_REG);
	}

	return 0; 
}
