//#include "hwlib.h"
#include "ADXL345.h"

// api for register access, defined in main.c
void ADXL345_REG_WRITE(uint8_t address, uint8_t value);
void ADXL345_REG_READ(uint8_t address, uint8_t *value);
void ADXL345_REG_MULTI_READ(uint8_t address, uint8_t values[], uint8_t len);



#define DATA_READY_TIMEOUT  (alt_ticks_per_second()/3)



// Initialize the ADXL345 chip
void ADXL345_Init(){
    
	// +- 16g range, full resolution
	ADXL345_REG_WRITE(ADXL345_REG_DATA_FORMAT, XL345_RANGE_16G | XL345_FULL_RESOLUTION);

	// Output Data Rate: 200Hz
	ADXL345_REG_WRITE(ADXL345_REG_BW_RATE, XL345_RATE_200);
	// The DATA_READY bit is not reliable. It is updated at a much higherrate than the Data Rate
	// Use the Activity and Inactivity interrupts as indicators for newdata.

    ADXL345_REG_WRITE(ADXL345_REG_THRESH_ACT, 0x04); //activity threshold
    ADXL345_REG_WRITE(ADXL345_REG_THRESH_INACT, 0x02); //inactivity threshold
    ADXL345_REG_WRITE(ADXL345_REG_TIME_INACT, 0x02); //time for inactivity
    ADXL345_REG_WRITE(ADXL345_REG_ACT_INACT_CTL, 0xFF); //Enables AC coupling for thresholds
  
 
    //enable interrupts
    ADXL345_REG_WRITE(ADXL345_REG_INT_ENABLE, XL345_ACTIVITY | XL345_INACTIVITY ); 
    
    // stop measure
    ADXL345_REG_WRITE(ADXL345_REG_POWER_CTL, XL345_STANDBY);
    
    // start measure
    ADXL345_REG_WRITE(ADXL345_REG_POWER_CTL, XL345_MEASURE);
    //printf("ADXL345 inicializado \n");
}


  

// Return true if there is new data
int ADXL345_IsDataReady(void){
    int bReady = false;
    uint8_t data8;
    ADXL345_REG_READ(ADXL345_REG_INT_SOURCE,&data8);
    if (data8 & XL345_ACTIVITY)
    {
        bReady = true;
        //printf("bREADY ES TRUE");
    }
    return bReady;
}


// Read acceleration data of all three axes
void ADXL345_XYZ_Read(int16_t szData16[3]){

    uint8_t szData8[6];

    ADXL345_REG_MULTI_READ(0x32, (uint8_t *)&szData8, sizeof(szData8));

    szData16[0] = (szData8[1] << 8) | szData8[0];
    szData16[1] = (szData8[3] << 8) | szData8[2];
    szData16[2] = (szData8[5] << 8) | szData8[4];
}

/*bool ADXL345_IdRead(int file, uint8_t *pId){
    
    ADXL345_REG_READ(file, ADXL345_REG_DEVID, pId);
}
*/


