#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

main ()
{
    char Disp[] = "/dev/accel";

	// Open the pM32 device for read and write
    static int16_t XYZ[3];
    static int16_t XYZ_old[3] = {0, 0, 0};
    static int r = 0;
    int canal = open(Disp, O_RDWR);
	if (canal > 0) 
    {
        printf("Dispo abierto\n");
		while(1)
		{   
			read (canal, &XYZ, sizeof(XYZ));
            if (XYZ[0] != XYZ_old[0] || XYZ[1] != XYZ_old[1] || XYZ[2] != XYZ_old[2])
            {
                r = 1;
            }
            else 
            {
                r = 0;
            }
                        
            printf("R=%d, X=%d mg, Y=%d mg, Z=%d mg\n", r, XYZ[0], XYZ[1], XYZ[2]);
            XYZ_old[0] = XYZ[0], XYZ_old[1] = XYZ [1], XYZ_old[2] = XYZ[2];
			sleep(1);
		}
	}
    else 
    {
		printf("No se ha podido abrir el dispositivo %s \n",Disp);
		exit(2);
	}
 	close(canal);
}