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
    printf("Verifying connection");

    uint8_t ui8Data[1];

    // Get WHO_AM_I register, return should be 0x0C
    I2CGyroReceive(GYRO_SLAVE_ADDR, GYRO_WHO_AM_I, ui8Data, sizeof(ui8Data));
    if ( 0xD7 == ui8Data[0] )
    {
        printf(" ... FXAS21002C is alive!!!");
    }
    else
    {
        printf(" ... FXAS21002C is NOT alive.");
        printf("\r\n");
        return 0;
    }

    // Put the device into standby before changing register values
    GyroStandby(GYRO_SLAVE_ADDR);

    // Reset the Gyro
    printf("\r\nResetting device");
    GyroReset(GYRO_SLAVE_ADDR);

    sleep(1); //FIXME this should be replaced with a test to see if device is back
    printf(" ... Done");

    // ***********************Print values for testing feedback
    I2CGyroReceive(GYRO_SLAVE_ADDR, GYRO_CTRL_REG0, ui8Data, sizeof(ui8Data));
    printf("\r\nGYRO_CTRL_REG0=0x%02X",ui8Data[0]);
    I2CGyroReceive(GYRO_SLAVE_ADDR, GYRO_CTRL_REG1, ui8Data, sizeof(ui8Data));
    printf("\r\nGYRO_CTRL_REG1=0x%02X",ui8Data[0]);
    // ***********************Print values for testing feedback

    // Put the device into standby before changing register values
    GyroStandby(GYRO_SLAVE_ADDR);

    // Choose the range of the accelerometer (2000,1000,500,250 dps)
    GyroRange(GYRO_SLAVE_ADDR, GFSR_250PS);

    // Choose the output data rate (800 Hz, 400 Hz, 200 Hz, 100 Hz,
    //  50 Hz, 25 Hz, 12.5 Hz)
    GyroOutputDataRate(GYRO_SLAVE_ADDR, ODR_12_5HZ);

    // Activate the data device
    GyroActive(GYRO_SLAVE_ADDR);    

    sleep(1); //FIXME this should be replaced with a test to see if device is back


    // ***********************Print values for testing feedback
    I2CGyroReceive(GYRO_SLAVE_ADDR, GYRO_CTRL_REG0, ui8Data, sizeof(ui8Data));
    printf("\r\nGYRO_CTRL_REG0=0x%02X",ui8Data[0]);
    I2CGyroReceive(GYRO_SLAVE_ADDR, GYRO_CTRL_REG1, ui8Data, sizeof(ui8Data));
    printf("\r\nGYRO_CTRL_REG1=0x%02X",ui8Data[0]);
    // ***********************Print values for testing feedback

    while (1)
{
    GyroGetData(GYRO_SLAVE_ADDR, &tGyroData );  
    printf("\r\nX:%04X Y:%04X Z:%04X", tGyroData.x,tGyroData.y,tGyroData.z);
}

//      GyroSelfTest(GYRO_SLAVE_ADDR, 1);

    // Activate the data device
//    GyroReady(GYRO_SLAVE_ADDR);

    // Temperature
//    GyroTemp(GYRO_SLAVE_ADDR);

    // ***********************Print values for testing feedback
//    I2CGyroReceive(GYRO_SLAVE_ADDR, GYRO_CTRL_REG1, ui8Data, sizeof(ui8Data));
//    printf("\r\nGYRO_CTRL_REG1=0x%02X",ui8Data[0]);
    // ***********************Print values for testing feedback



    // ***********************Print values for testing feedback
//    printf("\r\nGyroscope X:%d Y:%d Z:%d",tGyroData.x,tGyroData.y,tGyroData.z);
    // ***********************Print values for testing feedback

    printf("\r\n");
    return 0;
}
