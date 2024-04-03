#include <stdio.h>
#include "address_map_arm_DE10.h"
#include "altera_avalon_spi_regs.h"
#include <stdlib.h> // malloc, free
#include <string.h>
#include <stddef.h>
#include <unistd.h>  // usleep (unix standard?)
#include "alt_types.h"  // alt_u32
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include "LTC2308.h"

#include "LW_bridge.h"

void *LW_BRIDGE_virtual;
volatile int * ADC_SPI_ptr;
int fd = -1;

/********************************************************************************
 * This program demonstrates use of the ADC port in the DE1-SoC and DE0-Nano-SoC
 * Computers.
 *
 * It performs the following:
 * 1. Performs a conversion operation on all eight channels.
 * 2. Displays the converted values on the terminal window.
********************************************************************************/
void read_LTC2308 (volatile int  *SPI_0_BASE,alt_u16 adc_command, alt_u16 status, alt_u16* adc_result);

void read_LTC2308 (volatile int  *SPI_0_BASE,alt_u16 adc_command, alt_u16 status, alt_u16* adc_result)
{
	 while(!(IORD_ALTERA_AVALON_SPI_STATUS(SPI_0_BASE) & ALTERA_AVALON_SPI_STATUS_TRDY_MSK)); //write control and sequence register
			 IOWR_ALTERA_AVALON_SPI_TXDATA(SPI_0_BASE, adc_command); //read the convert result channel 0
			 
	// wait tx done
	status=0x00;
	while (!(status & ALTERA_AVALON_SPI_STATUS_TMT_MSK)){
		status = IORD_ALTERA_AVALON_SPI_STATUS(SPI_0_BASE);
	}

	while(!(IORD_ALTERA_AVALON_SPI_STATUS(SPI_0_BASE) & ALTERA_AVALON_SPI_STATUS_RRDY_MSK));
	*adc_result = IORD_ALTERA_AVALON_SPI_RXDATA(SPI_0_BASE);
}


int main(void)
{
	/* Declare volatile pointers to I/O registers (volatile means that the locations
	 * will not be cached, even in registers) */
	volatile int delay_count;	// volatile so that the C compiler doesn't remove the loop
	int port, value;
	
	if ((fd = open_physical (fd)) == -1)
    {
            return (-1);
    }
	else printf("physical opened\n");
	if (!(LW_BRIDGE_virtual = map_physical (fd, LW_BRIDGE_BASE, LW_BRIDGE_SPAN)))
	{
        printf("FALLO MAPEANDO LW_BRIDGE\n");
        return (-1);
    }

	printf("mapped \n");
	ADC_SPI_ptr = (int *) (LW_BRIDGE_virtual + ADC_SPI_BASE);
	printf("ADC_SPI_ptr declared\n");

	while (1)
	{
		
		alt_u16 adc_result;
		alt_u16 v_adc_result[100];
		alt_u16 status=0x00;
		alt_u16 adc_command;

		int i;

		printf("\033[H");
		printf("channel 0-7 voltage value:\n");

		//Initial the spi controller
		IOWR_ALTERA_AVALON_SPI_SLAVE_SEL(ADC_SPI_ptr, 0x1); //select device 0 (Mask 0x0001)
		IOWR_ALTERA_AVALON_SPI_CONTROL(ADC_SPI_ptr, 0x00);  //reset the control reg
		IOWR_ALTERA_AVALON_SPI_STATUS(ADC_SPI_ptr, 0x00);   // Clear the Status Reg
		
		//read converting result from channel 0 - channel 7

		 for(i=0;i<8;i++){

		
			switch (i)
			{
				case 0:
					adc_command=LTC2308_CH0 | LTC2308_UNIPOLAR_MODE | LTC2308_NORMAL_MODE;
					break;
				case 1:
					adc_command=LTC2308_CH1 | LTC2308_UNIPOLAR_MODE | LTC2308_NORMAL_MODE;
					break;
				case 2:
					adc_command=LTC2308_CH2 | LTC2308_UNIPOLAR_MODE | LTC2308_NORMAL_MODE;
					break;
				case 3:
					adc_command=LTC2308_CH3 | LTC2308_UNIPOLAR_MODE | LTC2308_NORMAL_MODE;
					break;
				case 4:
					adc_command=LTC2308_CH4 | LTC2308_UNIPOLAR_MODE | LTC2308_NORMAL_MODE;
					break;
				case 5:
					adc_command=LTC2308_CH5 | LTC2308_UNIPOLAR_MODE | LTC2308_NORMAL_MODE;
					break;
				case 6:
					adc_command=LTC2308_CH6 | LTC2308_UNIPOLAR_MODE | LTC2308_NORMAL_MODE;
					break;
				case 7:
					adc_command=LTC2308_CH7 | LTC2308_UNIPOLAR_MODE | LTC2308_NORMAL_MODE;
					break;
					
			}

			read_LTC2308(ADC_SPI_ptr,adc_command, status,  &adc_result);
			read_LTC2308(ADC_SPI_ptr,adc_command, status,  &adc_result);

			v_adc_result[i] = adc_result;
			}//end for

		//Print results
		for(i=0;i<8;i++) printf("CH%d = %fv %04X\n",i,2*2.5*(v_adc_result[i]&0x0fff)/4096, v_adc_result[i]);

		usleep(200000);
		
	}
	unmap_physical (LW_BRIDGE_virtual, LW_BRIDGE_SPAN);
	close_physical (fd);
}
