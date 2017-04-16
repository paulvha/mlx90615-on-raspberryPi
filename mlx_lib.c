/* read/write an MLX90615 sensor with Raspberry-pi
 * 
 * Copyright (c) 2017 Paul van Haastrecht <paulvha@hotmail.com>
 *
 * mlx_lib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * mlx_lib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mlx_lib. If not, see <http://www.gnu.org/licenses/>.  
 * 
 * version 1.0 / paulvha / April 2017
 * 
 * initial version of program
 *   
 */
 
#include <bcm2835.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "mlx90615.h"

/* MLX commands */
#define MLX_EEPROM 	0x10	// EEPROM access
#define MLX_RAM		0x20	// RAM access
#define MLX_READ    0x1		// read instruction
#define MLX_WRITE   0x0		// write instruction
#define MLX_SLEEP 	0xc6	// sleep command

/* (default) MLX90615 slave address */
uint8_t slave_address_base = default_SLA;

/* in sleep mode */
int in_sleep_mode = 0;

// set for NO PEC check during read
int no_pec_check = 0;

/* apply Debug */
int DEBUG = 0;

/* CRC8 check to compare PEC (Packet Error Checking)
 * &param poly : x8+x2+x1+1
 * @param data : array to check
 * @param size : #bytes
 * 
 * return CRC value
 */
uint8_t crc8Msb(uint8_t poly, uint8_t * data, int size)
{
	uint8_t crc = 0x00;
	int bit;
	
	while (size--)
	{
		crc ^= *data++;
		
		for (bit = 0;  bit < 8; bit++)
		{
			if (crc & 0x80)  crc = crc << 1 ^ poly; 
			else crc <<= 1;
		}
	}
	return(crc);
}

/* handle power ON or OFF for MLX */

void mlx_power(int act)
{

	// set the power pin to output
	bcm2835_gpio_fsel(power_pin, BCM2835_GPIO_FSEL_OUTP); 
	
	if (act == ON) bcm2835_gpio_write(power_pin, HIGH);
	else 	bcm2835_gpio_write(power_pin, LOW);
	
}

/* make sure NOT to overwrite any MLX reserved bits in the
 * config register and only allow :
 * Bit 0 : PWM or SMB
 * Bit 1 : PWM frequency
 * Bit 2 : PWM output
 * 
 * Return :
 * -1 : Error else update value to write to config
 */

long valid_config(long val)
{
	long  result;
	
	if ((result = read_reg(CONFIG)) < 0)
	{
		printf(REDSTR,"can not read CONFIG register\n");
		return (-1);
	}

	// Bit 0 : PWM or SMB
	if (val & 0x1)	result |= 1 << 0;
	else result &= ~ (1 << 0);

	// Bit 1 : PWM frequency
	if (val & 0x2)	result |= 1 << 1;
	else result &= ~ (1 << 1);	
	
	// Bit 2 : PWM output	
	if (val & 0x4)	result |= 1 << 2;
	else result &= ~ (1 << 2);	
	
	return(result);
}

/* write MLX90615 location
 * 
 * @param reg : location to write
 * @param val : value to write
 * 
 * For the MLX90615 it is only possible to write to registers
 * however we keep this structure open for the future.. you never know :-)
 *
 * return : 0 = OK,  -1 = Error */

int write_mlx(char reg, long val)
{
	char wbuf[6]= {0};
	
	/* needed to calculate PEC */
	wbuf[0]=slave_address_base <<1 | MLX_WRITE;
	wbuf[1]= reg;
 	wbuf[2]= val & 0xff;						// LSB
	wbuf[3]=(val >> 8) & 0xff;					// MSB   
	wbuf[4]=crc8Msb(0x7, (uint8_t *) wbuf, 4);	// add PEC
	
	if (crc8Msb(0x7, (uint8_t *) wbuf, 5))
	{
		printf(REDSTR,"error in CRC check\n");
		return(-1);
	}
	
	/* While the slave_address is needed for the correct PEC calculation
	 * it should NOT be sent by user level, as the BCM2835 will do 
	 * that already automatically. (hence wbuf+1)
	 */
	 
    switch(bcm2835_i2c_write(wbuf+1,4))
    {
        case BCM2835_I2C_REASON_ERROR_NACK :
            if(DEBUG) printf(REDSTR,"DEBUG: write NACK error\n");
            return(-1);
            break;
        case BCM2835_I2C_REASON_ERROR_CLKT :
            if(DEBUG) printf(REDSTR,"DEBUG: write Clock stretch error\n");
            return(-1);
            break;
        case BCM2835_I2C_REASON_ERROR_DATA :
            if(DEBUG) printf(REDSTR,"DEBUG: not all data has been sent\n");
            return(-1);
            break;
    }
    
    /* Apperently the EEPROM needs a delay after write to handle/settle.
	 * This is undocumented, but needed !!
	 * 
	 * imperial research proved 50ms is needed at least for stable working, 
	 * for safety 100ms has bee choosen !! 
	 */
	 
    usleep(100000);
    return(0);
}

/* write MLX90615 register
 * This will NOT allow to write the Melexis factory calibration registers
 * also a check on the CONFIG definable bits is done
 * @param regaddr : register to write
 * @param val : value to write
 *
 * return : 0 = OK,  -1 = Error */

int write_reg(char reg, long val)
{	
	// check on allowable registers to write
	if (reg > 0x03)
	{
		p_printf(1, "NOT allowed to write to register %d\n",reg);
		return(-1);
	}
	
	// allow only for user definable bits in config
	if (reg == CONFIG)	if ((val = valid_config(val)) < 0)  return(-1);

	/* Before write operation to EEPROM, this needs to be erased
	 * by writing 0x0000 (pag 14 of MLX90615 document) */

	if (DEBUG) printf("DEBUG: First erase register %d\n",reg);

	// add register opcode
	if (write_mlx((reg | MLX_EEPROM), 0x0000) < 0) return(-1);
	
	/* if updating the SMBUS slave address, it is now set for 0x0 
	 * we need to take extra action to handle this change. 
	 * In case the write of the new slave value fails, the MLX
	 * can still be accessed on address 0x0.
	 *  
	 * Although if the write of the new value to the CONFIG has failed, 
	 * you will continue to get PEC read errors. In that case you will 
	 * have to disable PEC on read (-n on commandline). */
	 
	if (reg == PWMSA)
	{
		// set BCM2835 to slave 0x0
		bcm2835_i2c_setSlaveAddress(0x0);
	
		// set for PEC calculation
		slave_address_base = 0x0;
		
		if (DEBUG) printf("DEBUG: Write new slave address:  %lx\n", val);
		
		// now update with new address
		if (write_mlx((reg | MLX_EEPROM), val) < 0)	return(-1);

		// set BCM2835 to slave to new value
		bcm2835_i2c_setSlaveAddress(val & 0x7f);
	
		// set for PEC calculation
		slave_address_base = val & 0x7f;	
		
		return(0);
	}
	
	if (DEBUG) printf("DEBUG: Write new value %lx to register %d\n",val,reg);
	return(write_mlx((reg | MLX_EEPROM), val));
}

/* read a location from MLX90615
 * @param reg : location to read
 *
 * return value:
 * 	access error   : -1 
 *  read/PEC error : -2	
 * else location content
 *  
 */

long read_mlx(char loc)
{
	/* needed to calculate PEC later */
	char rbuf[6]= {0};	
	rbuf[0]=slave_address_base<<1 | MLX_WRITE;
	rbuf[1]=loc;
	rbuf[2]=slave_address_base<<1 | MLX_READ;
	char *data = &rbuf[3];

    // perform read with restart
    switch(bcm2835_i2c_read_register_rs(&loc, data, 3))
    {
        case BCM2835_I2C_REASON_ERROR_NACK :
            if(DEBUG) printf(REDSTR,"DEBUG: NACK error\n");
            return(-1);
            break;

        case BCM2835_I2C_REASON_ERROR_CLKT :
            if(DEBUG) printf(REDSTR,"DEBUG: Clock stretch error\n");
            return(-1);
            break;

        case BCM2835_I2C_REASON_ERROR_DATA :
            if(DEBUG) printf(REDSTR,"DEBUG: not all data has been read\n");
            return(-1);
            break;
    }
	
	// unless requested on the command line (-n) a PEC check is done on read
	if (no_pec_check == 0)
	{
		// check PEC
		if (crc8Msb(0x7, (uint8_t *) rbuf, 5)  != rbuf[5])
		{
			p_printf(1, "PEC error. expected: %x, based on data calculated: %x\n", rbuf[5], crc8Msb(0x7, (uint8_t *) rbuf, 5) );
			if (DEBUG) p_printf(1,"DEBUG:reg: %x  data received MSB %x, LSB %x\n", slave_address_base, (int) rbuf[4], (int) rbuf[3]);
			return(-2);
		}
	}

	return(rbuf[4]<<8 | rbuf[3]);
}

/* read a register from the MLX90615 */
long read_reg(char reg)
{
	if ((reg > 0x0f) || (reg < 0))
    {
        printf(REDSTR,"invalid register\n");
        return(-1);
    }
	
	return(read_mlx(reg | MLX_EEPROM));	// add register opcode
}

/* read a ram location from the MLX90615 */
long read_ram(char ram)
{
	if ((ram > 0x0f) || (ram < 0))
    {
        printf(REDSTR,"invalid RAM location\n");
        return(-1);
    }
	return(read_mlx(ram | MLX_RAM));	// add ram opcode
}

/* sent sleep command to MLX 
 * Datasheet pag. 15 : 8.4.8.1 enter sleep mode :
 * write the sleep command */
int	enter_sleep()
{
	char wbuf[3]= {0};
	
	/* needed to calculate PEC */
	wbuf[0]= slave_address_base <<1 | MLX_WRITE;
	wbuf[1]= MLX_SLEEP;
	wbuf[2]= crc8Msb(0x7, (uint8_t *) wbuf, 2);	// add PEC
	
	if (crc8Msb(0x7, (uint8_t *) wbuf, 3))
	{
		printf ("error in CRC check\n");
		return(-1);
	}
	
	/* While the slave_address is needed for the correct PEC calculation
	 * it should NOT be sent by the user level, as the BCM2835 will do 
	 * that already automatically. (hence wbuf+1)
	 */
	 
    switch(bcm2835_i2c_write(wbuf+1,2))
    {
        case BCM2835_I2C_REASON_ERROR_NACK :
            if(DEBUG) printf(REDSTR,"DEBUG: write NACK error\n");
            return(-1);
            break;
        case BCM2835_I2C_REASON_ERROR_CLKT :
            if(DEBUG) printf(REDSTR,"DEBUG: write Clock stretch error\n");
            return(-1);
            break;
        case BCM2835_I2C_REASON_ERROR_DATA :
            if(DEBUG) printf(REDSTR,"DEBUG: not all data has been sent\n");
            return(-1);
            break;
    }
    return(0);
}

/* set the BCM2835 for I2c communication 
 * return 0 = ok, -1 = error*/
int set_mlx_i2c()
{
    // will select I2C channel 0 or 1 depending on board reversion.
    if (!bcm2835_i2c_begin()){
        printf(REDSTR,"Can't setup i2c pin!\n");
        return(-1);
    }
    
    /* set BSC speed to 100Khz*/
    bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_2500);

    /* set slave address for MLX90615*/
	bcm2835_i2c_setSlaveAddress(slave_address_base);
	
	return(0);
}

/* wakeup MLX or restore from PWM to SMB communication 
 * Datasheet pag. 15 : 8.4.8.2 exit sleep mode :
 * a low pulse of > 8ms is needed on SCL
 * 
 * Measurement done with a oscilloscope shows the pulse has 
 * to be at least 17ms. (Next to that another 0.3 seconds = 300ms should
 * be waited before stable measurement (in case of wake-up MLX))
 *  
 * To be save the time has been set to 50ms.
 * 
 * It is assumed that a V2 system is used for the SCL / pull down */
 
int wake_up()
{
	// reset pins
    bcm2835_i2c_end();
		
	// set the SCL pin to output and low
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_05, BCM2835_GPIO_FSEL_OUTP); 
	bcm2835_gpio_write(RPI_V2_GPIO_P1_05, LOW);
	
	// wait for wake_up and recovery 
	usleep(50000);
	
	return(set_mlx_i2c());
}

/* do a power on reset */
void por()
{
	// turn the power to MLX off.
    mlx_power(OFF);
    
    // wait one second
	sleep(1);
	
	// turn the power to MLX on.
    mlx_power(ON); 
}
