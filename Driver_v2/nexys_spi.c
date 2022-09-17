
#include <stdio.h>
#include "nexys_spi.h"
struct SPI_regs * pADXL_SPI_REGS= (struct SPI_regs *) 0x80000600;

struct config_adxl_mode_free_fall_struct f_fall_1;
struct config_adxl_mode_motion_switch_struct m_switch_1;


void printREGS()
{
	printf("----------------------------------\n" );
	printf("Capability0:  %08X\n",  pADXL_SPI_REGS->Capability0 );
	printf("Capability1:  %08X\n",  pADXL_SPI_REGS->Capability1 );
	printf("Mode:         %08X\n",  pADXL_SPI_REGS->Mode );
	printf("Event:        %08X\n",  pADXL_SPI_REGS->Event );
	printf("Mask:         %08X\n",  pADXL_SPI_REGS->Mask );
	printf("Slave Select: %08X\n",  pADXL_SPI_REGS->SlaveSelect );
	printf("----------------------------------\n" );
}

void config_spi_adxl(void)
{
	pADXL_SPI_REGS->Mode = 0; 	   //Reset

	pADXL_SPI_REGS->Mode &=~(1<<30);   //LOOP mode disabled
	pADXL_SPI_REGS->Mode |=(1<<25);    //Master enabled
	pADXL_SPI_REGS->Mode |=(1<<26);    //REV enabled: MSB transmited first, see TX/RX regs
	pADXL_SPI_REGS->Mode |=(4<<16);    //PM:4  70Mhz/20 => 3,5 Mhz (SCK)
	pADXL_SPI_REGS->Mode |=(1<<2);     //Ignore SPISEL
	pADXL_SPI_REGS->Mode &=~(0xF<<20); //LEN = 0 (WLEN = 32 bits)

	pADXL_SPI_REGS->Mode |=(1<<24); //Core enabled

	printREGS();
}

uint8_t read_nexys_adxl(uint8_t reg_id)
{
	uint32_t write_timeout=0, word;
	pADXL_SPI_REGS->Event |= (0x3<<11); //Clean flags UN and OV
	while(((pADXL_SPI_REGS->Event & (1<<8))==0) && (write_timeout < 0xAAAAA))
	{
		write_timeout++; //Wait until NF=1
	}
	if(write_timeout < 0xAAAAA)
	{
		pADXL_SPI_REGS->SlaveSelect &= ~(1); // Slave select 1 goes down 
		// TX request: <CMD>  :  <REG_ID>  :  <0>  :  <0>
		// RX data:                     <REG Value>:<REG+1 Value>  
		word=0;
		word |= ADXL_READ  << 24;	// CMD
		word |= reg_id << 16;		// REG_ID 

		pADXL_SPI_REGS->Transmit=word; //Transmit word

		while(pADXL_SPI_REGS->Event & (1<<31))
			; // Wait while transfer in progress 

		pADXL_SPI_REGS->SlaveSelect |= (1); // Slave select 1 goes up 

		// Read received data
		if (pADXL_SPI_REGS->Event & (1<<9))
		{
			// RX register 16 LSB bits contains REG value and REG+1 value 
			return (pADXL_SPI_REGS->Receive >> 8);
		}
		else
		{
			printf("Standing by no data at the moment...\n");
		}
	}
	else
	{
		printf("Read reg NF Timeout...\n");
	}
	return 0xFF;
}

void write_nexys_adxl(uint8_t reg_id, uint8_t value)
{
	uint32_t write_timeout=0, word;
	uint8_t scratch;

	pADXL_SPI_REGS->Event |= (0x3<<11); //Clean flags UN and OV
	while(((pADXL_SPI_REGS->Event & (1<<8))==0) && (write_timeout < 0xAAAAA))
	{
		write_timeout++; //Wait until NF=1
	}
	if(write_timeout < 0xAAAAA)
	{
		pADXL_SPI_REGS->SlaveSelect &= ~(1); // Slave select 1 goes down 
		// TX request: <CMD>  :  <REG_ID>  :  <0>  :  <0>

		word=0;
		word |= ADXL_WRITE << 24;	// CMD
		word |= reg_id << 16;		// REG_ID 
		word |= value << 8;		// Value 
		pADXL_SPI_REGS->Transmit=word; //Transmit word

		while(pADXL_SPI_REGS->Event & (1<<31))
			; // Wait while transfer in progress 
		pADXL_SPI_REGS->SlaveSelect |= (1); // Slave select 1 goes up 

		// Clean received buffer
		while (pADXL_SPI_REGS->Event & (1<<9))
		{
			scratch = pADXL_SPI_REGS->Receive;
		}
	}
	else
	{
		printf("Write reg NF Timeout...\n");
	}
	return;
}

void config_adxl_select_mode(void)
{
	int mode;
	char THRESH_INACTL;
	char THRESH_T_INACTL;
	int range;


	printf("Enter desired mode (1-freefall  2-autonomous motion switch  3-free fall custom mode 4-motion switch custom mode)\n");
    scanf("%d", &mode);
	switch (mode)
	{
    	case 1:
			printf("--> FREE FALL MODE\n");
      		config_adxl_mode_free_fall();
      		break;

    	case 2:
			printf("--> AUTONOMOUS MOTION SWITCH MODE\n");
      		config_adxl_mode_autonomous_switch();
      		break;

		case 3:
			printf("--> CUSTOM FREE FALL MODE\n");
			config_custom_mode_params_freefall();
			break;
		case 4:
			printf("--> CUSTOM AUTONOMOUS MOTION SWITCH MODE\n");
			config_custom_mode_params_motion_switch();
			break;
		

		default:
		printf("Mode not available");
	}

}

void config_adxl_mode_free_fall(void)
{
// Configures the accelerometer to ±8 g range, 100 Hz
write_nexys_adxl( ADXL_FILTER_CTL, 0x83 );

// Enables absolute inactivity detection
write_nexys_adxl( ADXL_ACT_INACT_CTL, 0x04 );

// Free fall threshold to 600mg
write_nexys_adxl( ADXL_THRESH_INACTL, 0x96 );

// Free fall time to 30 ms
write_nexys_adxl( ADXL_TIME_INACTL, 0x03 ); 


//Map an interruption (optional)
write_nexys_adxl( ADXL_INTMAP2, 0x20);


//Set POWER CONTROL register
write_nexys_adxl( ADXL_POWER_CTL, 0x02 );
}

void config_adxl_mode_autonomous_switch(void)
{
	//set Activity Thershold
	write_nexys_adxl( ADXL_THRESH_ACTL, 0xFA ); 
	write_nexys_adxl( ADXL_THRESH_ACTH, 0x00 ); 

	//set Inactivity Thershold
	write_nexys_adxl( ADXL_THRESH_INACTL, 0x96 );
	write_nexys_adxl( ADXL_THRESH_INACTH, 0x00 );

	//Set Inactivity Time Threshold
	write_nexys_adxl( ADXL_TIME_INACTL, 0x1E );

	//Set ACTIVITY/INACTIVITY control register
	write_nexys_adxl( ADXL_ACT_INACT_CTL, 0x3F);

	//Map an interruption (optional)
	write_nexys_adxl( ADXL_INTMAP2, 0x40); 

	//Set POWER CONTROL register
	write_nexys_adxl( ADXL_POWER_CTL, 0x0E);
}

void config_custom_mode_params_freefall(void){
	int ffthresh;
	printf("->Enter desired g range: 2g , 4g, 8g. 8g is recommended\n");
    scanf("%d", &ffthresh); 
	printf("%dg Selected\n",ffthresh);
		switch(ffthresh){
			case 8:
				f_fall_1.Parameter1=0x80;
				break;
			case 4:
				f_fall_1.Parameter1=0x40;
				break;
			case 2:
				f_fall_1.Parameter1=0x00;
				break;
			default:
				f_fall_1.Parameter1=0x83;
				
				
		}
		printf("->Enter desired ODR: 100hz, 200hz, 400hz.  100Hz ODR is recommended.\n");
    	scanf("%d", &ffthresh);
		printf("%dhz Selected\n",ffthresh);
		switch(ffthresh){
			case 100:
				f_fall_1.Parameter2=0x03;
				break;
			case 200:
				f_fall_1.Parameter2=0x04;
				break;
			case 400:
				f_fall_1.Parameter2=0x05;
				break;
			default:
				f_fall_1.Parameter2=0x03;
					
		}
		
		printf("->Enter desired Free fall threshold 100 mg , 300mg or 600mg are recommended\n");
    	scanf("%d", &ffthresh);  // THRESH_INACT [g] = THRESH_INACT [codes]/Sensitivity [codes per g]
		printf("%dmg Selected\n",ffthresh);
		double inact_thresh = 0.001*ffthresh;
    	printf("%f :double ffthresh\n",inact_thresh);
		int code;
		int scale_factor;
		int low_masked;
		int high_masked;
		int high_masked_shifted;
    	
		switch(f_fall_1.Parameter1){//g
			case 0x80://8g
				scale_factor=235; 
				break;

			case 0x40://4g
				scale_factor=500; 
				break;

			case 0x00://2g
				scale_factor=1000; 
				break;
    				
		}

		code=inact_thresh*scale_factor;  	
		low_masked = code & 0xff;
		f_fall_1.Parameter3=low_masked;
		high_masked = code & 0xf00;
		high_masked_shifted= high_masked>>8;
		f_fall_1.Parameter3_H=high_masked_shifted;
		//printf("%d :thresh Code\n",code);
		//printf("%d :thresh Code LOW\n",low_masked);
		//printf("%d :thresh Code HIGH\n",high_masked_shifted);


	
		
		printf("->Enter desired Free fall time 30ms , 100ms. 300ms is recommended\n");
    	scanf("%d", &ffthresh);  //time=TIME_ACT/ODR
		printf("%dms Selected\n",ffthresh);

		double time_inact= 0.001*ffthresh;
		int odr;
		
		switch(f_fall_1.Parameter2){//ODR

			case 0x03://100hz
				odr=100;
				break;
			case 0x04://200Hz 
				odr=200;
				break;
			case 0x05://400Hz 
				odr=400;
				break;
		}
		f_fall_1.Parameter4=odr*time_inact;
		printf("%d :time inact\n",f_fall_1.Parameter4);
		config_adxl_mode_free_fall_custom(f_fall_1);
}




void config_adxl_mode_free_fall_custom(struct config_adxl_mode_free_fall_struct f_fall_1)
{

	// Free fall threshold to x mg
	write_nexys_adxl( ADXL_THRESH_INACTL, f_fall_1.Parameter3 );
	write_nexys_adxl( ADXL_THRESH_INACTH, f_fall_1.Parameter3_H );

	// Free fall time to x ms
	write_nexys_adxl( ADXL_TIME_INACTL, f_fall_1.Parameter4 ); 

	// Enables absolute inactivity detection
	write_nexys_adxl( ADXL_ACT_INACT_CTL, 0x04 );

	// Interrupt not mapped, main carries out a programmed I/O

	// Configures the accelerometer to ±x g range and ODR 
	int var;
	var=(f_fall_1.Parameter1)|(f_fall_1.Parameter2);
	write_nexys_adxl( ADXL_FILTER_CTL, var );


	// Start mesurement
	write_nexys_adxl( ADXL_POWER_CTL, 0x02 );
}


void config_custom_mode_params_motion_switch(void){
	int ffthresh;
	printf("->Enter desired g range: 2g , 4g, 8g. 8g is recommended\n");
    	scanf("%d", &ffthresh);  
		printf("%dg Selected\n",ffthresh);
		switch(ffthresh){
			case 8:
				m_switch_1.Parameter1=0x80;
				break;
			case 4:
				m_switch_1.Parameter1=0x40;
				break;
			case 2:
				m_switch_1.Parameter1=0x00;
				break;
			default:
				m_switch_1.Parameter1=0x83;
				
				
		}
		printf("->Enter desired ODR. 100hz, 200hz, 400hz. 100Hz ODR is recommended.\n");
    	scanf("%d", &ffthresh); 
		printf("%dhz Selected\n",ffthresh);
		switch(ffthresh){
			case 100:
				m_switch_1.Parameter2=0x03;
				break;
			case 200:
				m_switch_1.Parameter2=0x04;
				break;
			case 400:
				m_switch_1.Parameter2=0x05;
				break;
			default:
				m_switch_1.Parameter2=0x03;
					
		}
		
		printf("->Enter desired ANY inactivity threshold in mg (1g=1000mg).100 mg , 300mg or 600mg are recommended\n");
    	scanf("%d", &ffthresh);  // THRESH_INACT [g] = THRESH_INACT [codes]/Sensitivity [codes per g] pag 27
		printf("%dmg Selected\n",ffthresh);
		double inact_thresh = 0.001*ffthresh;
    	printf("%f :double ffthresh\n",inact_thresh);
		int code;
		int scale_factor;
		int low_masked;
		int high_masked;
		int high_masked_shifted;
    	
		switch(m_switch_1.Parameter1){
			case 0x80://8g
				scale_factor=235; 
				break;

			case 0x40://4g
				scale_factor=500; 
				break;

			case 0x00://2g
				scale_factor=1000; 
				break;
    				
		}

		code=inact_thresh*scale_factor; 
		low_masked = code & 0xff;
		m_switch_1.Parameter3=low_masked;
		high_masked = code & 0xf00;
		high_masked_shifted= high_masked>>8;
		m_switch_1.Parameter3_H=high_masked_shifted;

		//printf("%d :thresh code \n",code);
		//printf("%d :thresh code LOW\n",low_masked);
		//printf("%d :thresh code  HIGH\n",high_masked_shifted);


	
		
		printf("->Enter desired ANY inactivity time in ms.  30ms , 100ms, 300ms are recommended\n");
    	scanf("%d", &ffthresh);  //time=TIME_ACT/ODR
		printf("%dms Selected\n",ffthresh);

		double time_inact= ffthresh; //0.001*  MS O SEGS??        
		double odr;
		
		switch(m_switch_1.Parameter2){

			case 0x03://para 100hx ODR
				odr=166.6;
				break;
			case 0x04://para 200Hz ODR
				odr=166.6;
				break;
			case 0x05://para 400Hz ODR      
				odr=166.6;
				break;
		}

		code=time_inact/odr; 
		int code_int=code;
		low_masked = code_int & 0xff;
		m_switch_1.Parameter4=low_masked;
		high_masked = code_int & 0xf00;
		high_masked_shifted= high_masked>>8;
		m_switch_1.Parameter4_H=high_masked_shifted;

		//printf("%d :odr elegido\n",odr);
		//printf("%f :time inact\n",time_inact);
		//printf("%d :code\n",code_int);
		//printf("%d :time inact LOW\n",low_masked);
		//printf("%d :time inact HIGH\n",high_masked_shifted);


		printf("->Enter desired ANY activity threshold in mg (1g=1000mg), NEEDS to be greater than inactivity threshold\n");
    	scanf("%d", &ffthresh);  // THRESH_INACT [g] = THRESH_INACT [codes]/Sensitivity [codes per g] pag 27
		printf("%dmg Selected\n",ffthresh);
		inact_thresh = 0.001*ffthresh;
    	//printf("%f :double ffthresh\n",inact_thresh);
    	
		switch(m_switch_1.Parameter1){//g
			case 0x80://8g
				scale_factor=235; 
				break;

			case 0x40://4g
				scale_factor=500; 
				break;

			case 0x00://2g
				scale_factor=1000; 
				break;
    				
		}

		code=inact_thresh*scale_factor; 
		low_masked = code & 0xff;
		m_switch_1.Parameter5=low_masked;
		high_masked = code & 0xf00;
		high_masked_shifted= high_masked>>8;
		m_switch_1.Parameter5_H=high_masked_shifted;

		//printf("%d :thresh Code \n",code);
		//printf("%d :thresh Code  LOW\n",low_masked);
		//printf("%d :thresh Code  HIGH\n",high_masked_shifted);

		config_adxl_custom_mode_autonomous_switch(m_switch_1);
}

void config_adxl_custom_mode_autonomous_switch(struct config_adxl_mode_motion_switch_struct m_switch_1)
{
	// Configures the accelerometer to ±x g range and ODR
	int var;
	var=(m_switch_1.Parameter1)|(m_switch_1.Parameter2);
	write_nexys_adxl( ADXL_FILTER_CTL, var );

	//set Inactivity Thershold
	write_nexys_adxl( ADXL_THRESH_INACTL, m_switch_1.Parameter3);
	write_nexys_adxl( ADXL_THRESH_INACTH, m_switch_1.Parameter3_H );

	//Set Inactivity Time Threshold
	write_nexys_adxl( ADXL_TIME_INACTL, m_switch_1.Parameter4 );
	write_nexys_adxl( ADXL_TIME_INACTH, m_switch_1.Parameter4_H );

	//set Activity Thershold
	write_nexys_adxl( ADXL_THRESH_ACTL, m_switch_1.Parameter5 ); 
	write_nexys_adxl( ADXL_THRESH_ACTH, m_switch_1.Parameter5_H ); 

	//Set ACTIVITY/INACTIVITY control register
	write_nexys_adxl( ADXL_ACT_INACT_CTL, 0x3F);

	//Map an interruption (optional)
	write_nexys_adxl( ADXL_INTMAP2, 0x40); 

	//Set POWER CONTROL register
	write_nexys_adxl( ADXL_POWER_CTL, 0x0E);
}