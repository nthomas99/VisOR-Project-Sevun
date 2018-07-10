/******************************************************************
 * Visor Data Logger
 * Gyroscope Example
 * Developed by Sevun Scientific, Inc.
 * http://sevunscientific.com
 * *****************************************************************
 *
 *    _____/\\\\\\\\\\\_______/\\\\\\\\\\\____/\\\\\\\\\\\_
 *     ___/\\\/////////\\\___/\\\/////////\\\_\/////\\\///__
 *      __\//\\\______\///___\//\\\______\///______\/\\\_____
 *       ___\////\\\___________\////\\\_____________\/\\\_____
 *        ______\////\\\___________\////\\\__________\/\\\_____
 *         _________\////\\\___________\////\\\_______\/\\\_____
 *          __/\\\______\//\\\___/\\\______\//\\\______\/\\\_____
 *           _\///\\\\\\\\\\\/___\///\\\\\\\\\\\/____/\\\\\\\\\\\_
 *            ___\///////////_______\///////////_____\///////////__
 *
 * *****************************************************************
 */

#include <stdio.h>
#include <stdint.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <unistd.h>
#include "gy/fxas21002c_proc.h"
#include "gy/fxas21002c.h" 

// Define FXAS21002C I2C address, determined by PCB layout with pins SA0=0
#define GYRO_SLAVE_ADDR       0x20

//*****************************************************************************
// I2C Functions
//*****************************************************************************

// see fxas21002c.c for accelerometer and magnetometer functions

int main()
{
//    tRawData tAccelData;      // Accelerometer data
//    tRawData tMagData;        // Magnetometer data

    //*****************************************************************************
    // Main Code
    //*****************************************************************************

    // Clear and reset home screen
    printf("\033[2J\033[;H");
    printf("Verifying connection ");

    uint8_t ui32Data[1];

    // Get WHO_AM_I register, return should be 0x0C
    I2CGyroReceive(GYRO_SLAVE_ADDR, GYRO_WHO_AM_I, ui32Data, sizeof(ui32Data));
    if ( 0xD7 == ui32Data[0] )
    {
        printf("\r\n... FXAS21002C is alive!!!");
    }
    else
    {
        printf("\r\n... FXAS21002C is NOT alive.");
        printf("\r\n");
        return 0;
    }

    // Put the device into standby before changing register values
//    AGStandby(AG_SLAVE_ADDR);

    // Choose the range of the accelerometer (±2G,±4G,±8G)
//    AGAccelRange(AG_SLAVE_ADDR, AFSR_2G);

    // Choose the output data rate (800 Hz, 400 Hz, 200 Hz, 100 Hz,
    //  50 Hz, 12.5 Hz, 6.25 Hz, 1.56 Hz). Rate is cut in half when
    //  running in hybrid mode (accelerometer and magnetometer active)
//    AGOutputDataRate(AG_SLAVE_ADDR, ODR_6_25HZ);

    // Choose if both the acclerometer and magnetometer will both be used
    //  IF BOTH ARE USED THAN OUTPUT DATA RATE IS SHARED.
    //  E.G. 100 HZ ODR MEANS ACCELEROMETER WILL SAMPLE AT 50 HZ
    //    AND MAGNETOMETER WILL SAMPLE AT 50 HZ
//    AGHybridMode(AG_SLAVE_ADDR, ACCEL_AND_MAG);

    // Activate the data device
//    AGActive(AG_SLAVE_ADDR);

//    AGGetData(AG_SLAVE_ADDR, ACCEL_DATA, &tAccelData );
//    AGGetData(AG_SLAVE_ADDR, MAG_DATA, &tMagData );

    // ***********************Print values for testing feedback
//    printf("\r\nAccelerometer X:%d Y:%d Z:%d",tAccelData.x,tAccelData.y,tAccelData.z);
//    printf("\r\nMagnetometer  X:%d Y:%d Z:%d",tMagData.x,tMagData.y,tMagData.z);
    // ***********************Print values for testing feedback
    
    printf("\r\n");
    return 0;
}
