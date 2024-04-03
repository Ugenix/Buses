#include <stdio.h>
#include <stdint.h>
#include "address_map_arm.h"
#include "LW_bridge.h"
/********************************************************************************
 * This program demonstrates use of the ADC port in the DE1-SoC and DE0-Nano-SoC
 * Computers.
 *
 * It performs the following:
 * 1. Performs a conversion operation on all eight channels.
 * 2. Displays the converted values on the terminal window.
********************************************************************************/
void *LW_BRIDGE_virtual;
volatile int * ADC_ptr;

int fd = -1;
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
        printf("FALLO MAPEANDO i2c0\n");
        return (-1);
    }	
	printf("mapped \n");
	ADC_ptr = (int *) (LW_BRIDGE_virtual + ADC_BASE);
	printf("ADC_ptr declared\n");
	*(ADC_ptr + 1) = 1;	// Sets the ADC up to automatically perform conversions.
	
	printf("ADC_ptr + 1 = 1\n");

	while (1)
	{
		printf("\033[H"); // sets the cursor to the top-left of the terminal window
		for (port = 0; port < 8; ++port)
		{
			value = *(ADC_ptr + port);
			printf ("ADC port %d: 0x%x\n", port, value);
		}
		for (delay_count = 1500000; delay_count != 0; --delay_count); // delay loop
	}
	unmap_physical (LW_BRIDGE_virtual, LW_BRIDGE_SPAN);
	close_physical (fd);
}
