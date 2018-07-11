/******************************************************************
 * Visor Data Logger
 * Accelerometer and Magnetometer Example
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
#include "ag/fxos8700cq_proc.h"
#include "ag/fxos8700cq.h" 

// Define FXOS8700CQ I2C address, determined by PCB layout with pins SA0=1, SA1=0
#define AG_SLAVE_ADDR       0x1D


tRawData g_tAccelData;      // Accelerometer data
tRawData g_tMagData;        // Magnetometer data

//*****************************************************************************
// I2C Functions
//*****************************************************************************

// see fxas8700cq.c for accelerometer and magnetometer functions

int main()
{

    //*****************************************************************************
    // Main Code
    //*****************************************************************************

    // Clear and reset home screen
    printf("\033[2J\033[;H");
    printf("Verifying connection ");

    uint8_t ui32Data[1];

    // Get WHO_AM_I register, return should be 0xC7
    I2CAGReceive(AG_SLAVE_ADDR, AG_WHO_AM_I, ui32Data, sizeof(ui32Data));
    if ( 0xC7 == ui32Data[0] )
    {
        printf("\r\n... FXOS8700CQ is alive!!!");
    }
    else
    {
        printf("\r\n... FXOS8700CQ is NOT alive.");
        printf("\r\n");
        return 0;
    }

    // ***********************Print register values for testing feedback
    I2CAGReceive(AG_SLAVE_ADDR, AG_CTRL_REG1, ui32Data, sizeof(ui32Data));
    printf("\r\nAG_CTRL_REG1    = 0x%02x",ui32Data[0]);

    I2CAGReceive(AG_SLAVE_ADDR, AG_XYZ_DATA_CFG, ui32Data, sizeof(ui32Data));
    printf("\r\nAG_XYZ_DATA_CFG = 0x%02x",ui32Data[0]);

    I2CAGReceive(AG_SLAVE_ADDR, AG_M_CTRL_REG1, ui32Data, sizeof(ui32Data));
    printf("\r\nAG_M_CTRL_REG1  = 0x%02x",ui32Data[0]);
    // ***********************Print register values for testing feedback

    // Put the device into standby before changing register values
    AGStandby(AG_SLAVE_ADDR);

    // Choose the range of the accelerometer (±2G,±4G,±8G)
    AGAccelRange(AG_SLAVE_ADDR, AFSR_2G);

    // Choose the output data rate (800 Hz, 400 Hz, 200 Hz, 100 Hz,
    //  50 Hz, 12.5 Hz, 6.25 Hz, 1.56 Hz). Rate is cut in half when
    //  running in hybrid mode (accelerometer and magnetometer active)
    AGOutputDataRate(AG_SLAVE_ADDR, ODR_1_56HZ);

    // Choose if both the acclerometer and magnetometer will both be used
    //  IF BOTH ARE USED THAN OUTPUT DATA RATE IS SHARED.
    //  E.G. 100 HZ ODR MEANS ACCELEROMETER WILL SAMPLE AT 50 HZ
    //    AND MAGNETOMETER WILL SAMPLE AT 50 HZ
    AGHybridMode(AG_SLAVE_ADDR, ACCEL_AND_MAG);

    // Activate the data device
    AGActive(AG_SLAVE_ADDR);

    // ***********************Print register values for testing feedback
    I2CAGReceive(AG_SLAVE_ADDR, AG_CTRL_REG1, ui32Data, sizeof(ui32Data));
    printf("\r\nAG_CTRL_REG1    = 0x%02x",ui32Data[0]);

    I2CAGReceive(AG_SLAVE_ADDR, AG_XYZ_DATA_CFG, ui32Data, sizeof(ui32Data));
    printf("\r\nAG_XYZ_DATA_CFG = 0x%02x",ui32Data[0]);

    I2CAGReceive(AG_SLAVE_ADDR, AG_M_CTRL_REG1, ui32Data, sizeof(ui32Data));
    printf("\r\nAG_M_CTRL_REG1  = 0x%02x",ui32Data[0]);
    // ***********************Print register values for testing feedback

    while(1)
    {
        AGGetData(AG_SLAVE_ADDR, ACCEL_DATA, &g_tAccelData );
        AGGetData(AG_SLAVE_ADDR, MAG_DATA, &g_tMagData );

        printf("\r\nACCEL X:%d Y:%d Z:%d  MAG X:%d Y:%d Z:%d",
                   g_tAccelData.x,g_tAccelData.y,g_tAccelData.z,
                   g_tMagData.x,g_tMagData.y,g_tMagData.z);
        sleep(1);
    }
}
