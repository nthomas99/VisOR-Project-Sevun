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
    tRawData tGyroData;      // Gyroscope data

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
//    GyroStandby(GYRO_SLAVE_ADDR);

    // Reset the Gyro
//    GyroReset(GYRO_SLAVE_ADDR);

      GyroSelfTest(GYRO_SLAVE_ADDR, 1);

    // Activate the data device
//    GyroReady(GYRO_SLAVE_ADDR);

    // Activate the data device
//    GyroActive(GYRO_SLAVE_ADDR);

    // Temperature
//    GyroTemp(GYRO_SLAVE_ADDR);

    // ***********************Print values for testing feedback
    I2CGyroReceive(GYRO_SLAVE_ADDR, GYRO_CTRL_REG1, ui32Data, sizeof(ui32Data));
    printf("\r\nGYRO_CTRL_REG1=0x%02X",ui32Data[0]);
    // ***********************Print values for testing feedback

    GyroGetData(GYRO_SLAVE_ADDR, &tGyroData );

    // ***********************Print values for testing feedback
    printf("\r\nGyroscope X:%d Y:%d Z:%d",tGyroData.x,tGyroData.y,tGyroData.z);
    // ***********************Print values for testing feedback

    printf("\r\n");
    return 0;
}
