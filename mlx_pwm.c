/* read/write an MLX90615 sensor in PWM on Raspberry-pi
 * 
 * Copyright (c) 2017 Paul van Haastrecht <paulvha@hotmail.com>
 *
 * mlx is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * mlx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mlx. If not, see <http://www.gnu.org/licenses/>.  
 * 
 * version 1.0 / paulvha / April 2017
 * 
 * initial version of sample program
 */

#include <stdlib.h>
#include <bcm2835.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include "mlx90615.h"
#include <math.h>

// PWM changes in config
#define COMMS_BIT	0
#define FREQ_BIT	1
#define PWM_OUT_BIT 2

/* indicate PWM mode */
int pwm_mode = 0;

// PWM frequency requested
int	cur_freq = HIGH;

// PWM TMIN requested (default = 0 celcius)
long t_min = 0x355A;

// PWM T_range requested (default = 50 celcius)
long t_range = 0x9c4;

// indicate temperature to display
int cur_temp = TA;

/* detect whether MLX is in PWM mode and capture the start_high, stop_high and cycle_time
 * return values
 * 1 = PWM signal detected, values are stored
 * 0 = NO PWM signal detected.
 */ 

int detect_pwm( double * r_start_high, double * r_stop_high, double * r_cycle_time)
{
	double start_high = 0, stop_high = 0, cycle_time = 0;
    double start_loop;
	int	 first = 1;

	// set start time_out
	start_loop = (get_current() / 1000);

	do
	{
		// detect level
		if (bcm2835_gpio_lev(sda_pin))
		{
			// if first time detect high
			if (first)
			{
				// wait untill it goes low or timeout
				while ( bcm2835_gpio_lev(sda_pin) )
				{
					if ((get_current()/1000) - start_loop > 250)
						return (0);
				}
				// restart start time_out
				start_loop = get_current() /1000;

				first = 0;
			}
			// second time set start_high
			else if (start_high == 0)
				start_high = get_current();

			// determine cycle time
			else if (start_high != 0 && stop_high != 0)
				cycle_time = get_current() - start_high;
		}
		else // get stop high if start was set first
			if (start_high != 0 && stop_high == 0) 
				stop_high = get_current();
		
		//* time_out ?		
		if ((get_current()/1000) - start_loop > 250)
			return (0);
		
	} while (stop_high == 0 || start_high == 0 || cycle_time == 0);	
	
	*r_start_high = start_high;
	*r_stop_high = stop_high;
	*r_cycle_time = cycle_time;

	return(1);
}

/* discover the current frequency either from the variable (if not PWM mode)
 * or by detecting the signal and display the result
 */
void get_cur_freq( int d_col)
{
    double start_high = 0, stop_high = 0, cycle_time = 0;
		
	if (! pwm_mode)
	{
        if(cur_freq == HIGH) p_printf(d_col," (set to 1Khz)\n");
        else p_printf(d_col," (set to 10hz)\n");
        return;
    }	


	if ( detect_pwm(&start_high, &stop_high, &cycle_time))
		p_printf(d_col," (measured %4.1fHz)\n",(1000000/cycle_time));

	else
		p_printf(1," (?) (MLX does not seem to be in PWM mode.)\n");
}

/* set temperature to display in PWM mode
 * @param tmp : either TA for ambient temperature, or TO for object temperature */
void set_cur_temp(int temp)
{
	cur_temp = temp;
	
	if (pwm_mode)
	{
		p_printf(3, 
		"MLX is already in PWM mode.\n"
		"When displaying the temperature it will try to detect the temperature type\n"
		"that is selected & sent by the MLX. Your new request is sent to the MLX now\n");

		exit_pwm(0);
		enter_pwm(1);
	}
	
}

/* Menu to handle PWM mode 
 * @param set_pwm_value : 
 * 	1 = set MLX to the values coming from the variables in the program, 
 *  0 = try to read from the MLX.
 */
int set_pwm(int set_pwm_value)
{
	int tmp=0;
	int d_col = 3 ;		// display color (yellow)
	double trash, cycle_time;
	static	int	read_values = 1;
	
	if (in_sleep_mode)
	{
		p_printf(1,"MLX is in sleep mode\n");
		return(0);
	}
	
	do
    {
        // check whether or not in PWM signal
        if ( detect_pwm(&trash, &trash, &cycle_time))
        {
			p_printf (2, "\nMLX90615 IS PROVIDING A PWM SIGNAL\n");
		
			// if in PWM mode get current stored values (only once)
			if (read_values)
			{		
				/* if requested on command line, first store the current
				 * values in the MLX90615 */
				if (set_pwm_value)
				{
						exit_pwm(0);
						enter_pwm(1);
				}
				
				if (get_values_from_mlx())
				{
					d_col = 3;		// display color (yellow)
					p_printf (1, "\nCould not read values from MLX90615\n");
				}
				else
				{
					// set cur_freq. 1khz ~ 1024 ms.
					if (cycle_time < 1100)	cur_freq=HIGH;
					else	cur_freq= LOW;
					
					d_col = 4;		// display color (blue)
					read_values = 0;
				}
			}
        }
        else
        {
			p_printf (1, "\nMLX90615 DOES NOT PROVIDE A PWM SIGNAL\n");
			read_values = 1;
        }
        
        p_printf(3,"\nWhat do you want to do ? \n\n");
        
        p_printf(2, "1	Change PWM frequency");
        get_cur_freq(d_col);
        
        p_printf(2,"2	Change PWM temperature minimum or range");
        p_printf(d_col," (Min %1.2f / Range %1.2f / max %1.2f)\n",
        (float) round((t_min-(50 * 273.15))/50), (float) (t_range / 50), 
        (float) round((t_min-(50 * 273.15))/50) +t_range/50);
        
        p_printf(2,"3	Set to display object temperature");
        if(cur_temp == TO) p_printf(d_col," (selected)\n");
        else printf("\n");
        
        p_printf(2,"4	Set to display ambient temperature");
        if(cur_temp == TA) p_printf(d_col," (selected)\n");
        else printf("\n");       
        
        if (pwm_mode)	p_printf(d_col, "	ALL ready in PWM mode\n");
        else
        {
			p_printf(2, "5	Enter PWM mode (only overwrite t_min)\n");
			p_printf(2, "6	Enter PWM mode & reset all parameters to current.\n");
		}
        
        p_printf(2,
        "7	Exit PWM mode\n"
        "8 	Display selected temperature\n"
        "12	Toggle to show details (where possible)");
  
        if(detailed) p_printf(1," (enabled)");
        
        p_printf(2,"\n13	Toggle debug messages");
        if(DEBUG) p_printf(1," (enabled)");
        
        p_printf(3,"\n\n99	to return. ");

        tmp = get_dec_input();

		if (tmp == -1 ) tmp = 99;

        switch(tmp)
        {

        case 1:
			set_pwm_freq();
            break;
        case 2:
			set_range();
            break;
        case 3:
			set_cur_temp(TO);
            break;
        case 4:
            set_cur_temp(TA);
            break;
        case 5:
			enter_pwm(0);
            break;
        case 6:
			enter_pwm(1);
            break;
        case 7:
			read_values = 1;		// read values on next entry
			return(exit_pwm(1));
			break;
        case 8:
			display_pwm_temp();
			break;
        case 12:
			toggle_detailed();
            break;
        case 13:
            toggle_debug();
            break;
        case 99:
            read_values = 1;		// read values on next entry
            break;
            
        default:

            printf(REDSTR,"invalid entry, try again\n");
            break;
        }

    } while (tmp != 99);
	
	return(0);
}

/* check, calculate and set t_min */
int calc_to_t_min(float val)
{
	if (val < -40 )
	{
		p_printf(1,"can not go below -40C\n");
		return(-1);
	}
	else if (val + (float) (t_range/50) > 115  )
	{
		p_printf(1,"can not go above 115C (min = %1.2f, range = %1.2f)\n",val, (float) (t_range/50));
		return(-1);
	}
	else
	{
		t_min = (long) round(50 * (273.15 + val ) + 0.1);
		if (DEBUG) printf("t_min = %lx val = %f\n", t_min, val);
	}
	return(0);
}

/* check, calculate and set t_range */
int calc_to_t_range (float val)
{
	if (val < 0)
	{
		p_printf(1,"can not set a negative range\n");
		return(-1);
	}				
	else if (val + (float) (t_min-(50 * 273.15))/50 > 115  )
	{
		p_printf(1,"can not go above 115C (min = %1.2f, range = %1.2f)\n", (float) (t_min-(50 * 273.15))/50, val );
		return(-1);
	}
	else
	{
		t_range = (long) 50 * val;
		if (DEBUG) printf("t_range = %lx\n", t_range);
	}
	return(0);
}

/* set SCL pin high
 * this must be done to keep the MLX in PWM mode 
 * A check is done whether the MLX is providing PWM signal
 * 
 * @param por : 1 = to do a power up reset first
 * 
 * Return values
 * 0 = OK, MLX is providing PWM
 * 1 = MLX is NOT providing PWM
 */
int set_SCL_high(int reset)
{
	double trash;
	
	// if requested perform a power up reset
	if (reset) por();
	else 
	{
		// disable i2C (already done by por)
		bcm2835_i2c_end();
	}
	
	// set the SCL pin to output and high
	// bcm2835_gpio_write(scl_pin, HIGH);
	bcm2835_gpio_fsel(scl_pin, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_write(scl_pin, HIGH);
	
	// set SDA for input
	bcm2835_gpio_fsel(sda_pin, BCM2835_GPIO_FSEL_INPT);
	
	// indicate that comms is in PWM
	slave_address_base= 0xff;
	
	// check whether or not in PWM signal
    if (detect_pwm(&trash, &trash, &trash))
	{
		// indicate it is in PWM
		pwm_mode = 1;
		return(0);
	}
	else
	{
		pwm_mode = 0;
		return(1);
	}
}

/* set config register for PWM mode 
 * @param PWM : 1 = set to PWM else to SMB 
 * @param set_value : 1 = overwrite PWM attributes */
int set_conf_reg (int pwm, int set_value)
{
	long result;

	// read config register
	if ((result = read_reg(CONFIG)) < 0)
	{
		p_printf(1,"can not read CONFIG register\n");
		return(-1);
	}

	if (pwm)	
	{
		if (set_value)
		{	// set frequency
			if (cur_freq == HIGH)	result &= ~ (1 << FREQ_BIT);
			else		result |= 1 << FREQ_BIT;
		
			// set temperature to display
			if (cur_temp == TO)	result &= ~ (1 << PWM_OUT_BIT);
			else		result |= 1 << PWM_OUT_BIT;
		}
		
		// set comms bit as PWM
		result &= ~ (1 << COMMS_BIT);
	}
	else // set comms bit as SMB
		result |= 1 << COMMS_BIT;
	
	// write config register
	if (write_reg(CONFIG, result) < 0 )
	{
        if (pwm) p_printf(1,"can not update to PWM mode\n");
		else p_printf(1,"can not update to SMB mode\n");
		return(-1);
	}
		
	return(0);
}

/* will write the attributes and set the MLX in PWM mode 
 * @param set_value: 1 = set attribute values from program, 0 = only set PWM mode
 * return 0 = OK, -1 is error*/
 
int enter_pwm(int set_value)
{
	// if already in PWM mode
	if (pwm_mode) 	return(0);
	
	// if communication is set for PWM
	if (slave_address_base == 0xff) set_for_smb();
	
	// overwrite other current settings ?
	if (set_value)
	{
		// set for PWM + attributes
		if (set_conf_reg(1, 1) < 0) return(-1);	
		
		// set range
		if (write_reg(PWMTR, t_range) < 0 )
		{
	        p_printf(1,"can not set temperature range \n");
			return(-1);
		}
	}
	else
		// set for PWM
		if (set_conf_reg(1, 0) < 0) return(-1);	
	
	/* set T_min.
	 * This will overwrite the Slave address, and thus the
	 * MLX can only be accessed with slave address 0x0
	 */
	if (write_reg(PWMSA, t_min) < 0 )
	{
        p_printf(1,"can not set minimum temperature\n");
		return(-1);
	}
	
	// pull SCL high + Power up reset to enter PWM
	if (set_SCL_high(1))
	{
		p_printf(1, "MLX could NOT be set to PWM mode\n");
		return(-1);
	}
	
	p_printf(2, "MLX has been set to PWM mode\n");
	
	return(0);
}

/* reset to SMB mode
 * if ch_to_smb = 1 : it will reset the config register
 * and make the change permanent to SMB.
 * 
 * return value 0 = OK, -1 = error
 */
int exit_pwm(int ch_to_smb)
{
	uint8_t sla;
	
	// set for PEC calculation and re-init
	slave_address_base= 0x0;
	
	/* do SMB Request condition
	 * datasheet, pag 16 :
	 * > 39ms low. Hence wake-up call
	 *  (including enable I2C communication)
	 */
	if(wake_up() < 0)
	{
		p_printf(1,"reset to I2C communication failed\n");
		return(-1);
	}
	
	// indicate it is not in PWM
	pwm_mode = 0;
	
	// reset request config to SMB and default slave address
	if(ch_to_smb)
	{		
		// reset to SMB in config register
		if (set_conf_reg(0,0) < 0) return (-1);
	
		// if a new slave address was provided on command line else default
		if (slave_address_base_req) sla = slave_address_base_req;
		else sla = default_SLA;
		
		// write new slave address
		if (write_reg(PWMSA, (long) (sla & 0x7f)) == 0 )
		{
	        p_printf(2,"Slave address set to 0x%x\n", sla & 0x7f);
	        
		    // set for PEC calculation and re-init
			slave_address_base = sla & 0x7f;
			
			/* set slave address for MLX90615*/
			bcm2835_i2c_setSlaveAddress(sla & 0x7f);
		}
		else
		{
			p_printf(1,"can not set slave address: 0x%x\n", sla & 0x7f);
			return(-1);
		}
	}
	
	return(0);
}

/* will set a new PWM frequency
 * return value : 0 = OK, -1 = error
 */
int set_pwm_freq()
{
	int	answ;
	
	// check for PWM mode
	if (pwm_mode)
	{
		p_printf(2, 
		"MLX is already in PWM mode\n"
		"When displaying temperature, the frequency is automatically detected.\n"
		"Your change can be applied if you want.\n");
	}
	
	do
	{		 
		// ask for frequency rate
		p_printf(3,"\nDo you want to set the frequency to\n1) 10HZ\n2) 1Khz\n(99 = return) ");

		answ = get_dec_input();

		if (answ == 99 || answ == -1) return(0);
	}
	while (answ != 1 && answ != 2);
	
	if (answ == 1) 	cur_freq = LOW;
	else if (answ == 2) cur_freq = HIGH;
	
	// if already in PWM, re-apply to MLX
	if (pwm_mode) 
	{
		exit_pwm(0);
		return(enter_pwm(1));
	}

	return(0);
}

/* will set T_min or T_range for PWM mode */
void set_range()
{
	int	answ;
	float val;
	
	// check for PWM mode
	if (pwm_mode)
	{
		p_printf(2, 
		"MLX is already in PWM mode and when displaying temperature, the minimum\n"
		"temperature and range are automatically detected.\nYour change can be applied if you want.\n");
	}

	do
	{		 
		// ask for frequency rate
		p_printf(3,"\nDo you want to set\n1) minimum temperature (T_min)\n2) range on top (T_range)\n(99 = return) ");

		answ = get_dec_input();

		if (answ == 99 || answ == -1) return;
	}
	while (answ != 1 && answ != 2);
	
	if (answ == 1) 	
	{
		p_printf(3,"T_min is currently set to 0x%lx or %1.2fC\n",t_min, round((t_min-(50 * 273.15))/50));
	
		do
		{		 
			p_printf(2,"Enter new minimum temperature in celcius (99 = return) ");
			scanf("%f",&val);

			if (val == 99) return;
			
		} while (calc_to_t_min(val)< 0);
	}
	else if (answ == 2)
	{
		p_printf(3,"T_range is currently set to 0x%lx or %1.2fC\n",t_range, (float) t_range/50);
		do
		{		 
			p_printf(2,"Enter new range temperature in celcius (99 = return) ");
			scanf("%f",&val);
		
			if (val == 99) return;

		} while (calc_to_t_range(val)< 0);

	}
	
	// if already in PWM, re-apply to MLX
	if (pwm_mode) 
	{
		exit_pwm(0);
		enter_pwm(1);
	}
}

/* return current time in useconds */
double get_current()
{
	struct	timeval	tv;

	gettimeofday(&tv,NULL);
	
	return((tv.tv_sec * 1000000) + tv.tv_usec);
}

/* read the T_min, T-range and temperature type from an MLX in pwm_mode
 * return : 0 = OK, 1 = error
 */

int get_values_from_mlx()
{
	long result;
	
	if (! pwm_mode ) return(1);
	
	// set for PEC calculation and re-init
	slave_address_base= 0x0;
	
	// switch back to SMB communication
	if(wake_up() < 0)
	{
		p_printf(1,"reset to I2C communication failed\n");
		return(1);
	}

	// indicate MLX is PWM mode anymore
	pwm_mode = 0;
	
	// read T-Min register
	if ((result = read_reg(PWMSA)) < 0)
	{
		p_printf(1,"can not read T_min register\n");
		return(1);
	}
	t_min = result;	
	
	// read T-range register
	if ((result = read_reg(PWMTR)) < 0)
	{
		p_printf(1,"can not read T_range register\n");
		return(1);
	}
	t_range = result;	

	// read config and obtain the temperature type
	if ((result = read_reg(CONFIG)) < 0)
	{
		p_printf(1,"can not read CONFIG register\n");
		return(1);
	}
	if (result & 0x04) cur_temp = TA;
	else cur_temp = TO;
	
	// pull SCL high + Power up reset to enter PWM
	return(set_SCL_high(1));
}

/* display the temperature that was selected BEFORE it went in PWM mode
 *  
 * The frequency is automatically detected and independent of the menu setting
 * 
 * return value
 * -2 : not in PWM mode
 * -1 : time-out
 *  0 : OK
 */
int display_pwm_temp()
{
	double start_high = 0, stop_high = 0, cycle_time = 0;
	float temp;
	float duty;
	
	// check for PWM mode
	if ( ! pwm_mode)
	{
		p_printf(1, "MLX is not in PWM mode\n");
		return(-2);
	}
	
	if (detect_pwm(&start_high, &stop_high, &cycle_time))
	{
		// calculate duty
		duty = (stop_high - start_high) / cycle_time;

		/* the first 0.125 are always high and need to be subtracted
		 * page 17 and 18 of the datasheet explain
		 */
			
		temp = 2 * (duty - 0.125) * t_range/50 + (t_min-(50 * 273.15))/50;
		
		if (cur_temp == TA)
			p_printf(3,"Ambient temperature : %1.2fC\n",temp);
		else
			p_printf(3,"Object temperature is : %1.2fC\n",temp);
	}
	else
	{
		p_printf(1,"MLX does not seem to be in PWM mode.\n");
		return(-1);	
	}
	// display warning messages
	if (duty > 0.6)
		p_printf(1,
		"The displayed temperature is extremely close to the max %1.2f\n"
		"It might even be that the real temperature is HIGHER and you should\n"
		"increase the minimum temperature and/or increase the range.\n"
		"(option 2 of PWM menu).\n",
		(float) round((t_min-(50 * 273.15))/50) +t_range/50);

	else if (duty < 0.15)
		p_printf(1,
		"The displayed temperature is extremely close to the minimum %1.2f\n"
		"It might even be that the real temperature is LOWER and you should\n"
		"decrease the minimum temperature (option 2 of PWM menu).\n",
		(float) round((t_min-(50 * 273.15))/50));	
	
	return(0);
}
