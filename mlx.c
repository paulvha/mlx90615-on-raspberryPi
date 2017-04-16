/*
 *MLX90615 - infra read sensor
 *
 * ***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 ***************************************************************************
 *
 * This version of GPL is at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 ***************************************************************************
 *
 * version 1.0 / paulvha / April 2017
 * 
 *  initial version of program
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <stdarg.h>
#include "mlx90615.h"
#include <bcm2835.h>

/* include details (where possible)*/
int detailed = 0;

/* overwrite default _slave address with -s option*/
uint8_t slave_address_base_req = 0x0;

/* set for PWM menu request */
int pwm_menu = 0;


/* display debug message
 * @param format : debug message to display and optional arguments
 *                 same as printf
 * @param level :  1 = RED, 2 = GREEN, 3 = YELLOW 4 = blue
 * info is only displayed if DEBUG had set for it*/

void p_printf (int level, char *format, ...)
{
    char	*col;
    va_list arg;
    
    //allocate memory
    col = malloc(strlen(format) + 20);
    		
    switch(level)
    {
		case 1:
			sprintf(col,REDSTR, format);
			break;
		case 2:
			sprintf(col,GRNSTR, format);
			break;		
		case 3:
			sprintf(col,YLWSTR, format);
			break;		
		case 4:
			sprintf(col,BLUSTR, format);
			break;
		default:
			sprintf(col,format);
	}
    
    if (DEBUG || level)
    {
		va_start (arg, format);
		vfprintf (stdout, col, arg);
		va_end (arg);
    }
    
    // release memory
    free(col);
}

/* get decimal input
 * When trying to get decimal number, it will continue to fail
 * if the number in the user level buffer is NOT decimal. So that has
 * to be removed from the user level buffer */
 
int get_dec_input()
{
	int count, tmp;

	// just in case there is a lot still in the kernel pending
	fflush(stdin);
	
	while ((count = scanf("%d",&tmp)) != 1)
	{
		// in case of error
		if (count < 0)	return(-1);
		
		// get the blocking character and discard
		scanf("%*c");
	}

	return(tmp);
}

/* check for the number of devices on I2c
 * if only one found, it will set the slave_base_address.
 * Returns the count.
 */
 
int check_for_mlx() 
{
	int i, count=0;
	uint8_t sla_found=0x5b;
	uint8_t slave_address_old = slave_address_base;
	
	for (i = 1 ; i < 0x7f ; i++)
	{
	  	/* set slave address for MLX90615*/
		bcm2835_i2c_setSlaveAddress((char) i);

		/* set for PEC calculation */
		slave_address_base = (uint8_t) i;
		
	    // perform read 
	    if (read_reg(0) > 0)
	    {
            count++;
            sla_found = (uint8_t) i;
	    }
	}
	
	if (count == 1)
	{
		// if slave address provided on the command line
		if (slave_address_base_req)
		{
			if (sla_found != slave_address_base_req)
			{
				p_printf(1,"**************************************************************************\n");
				p_printf(1,"WARNING requested slave address (0x%x) is NOT the same as MLX found (0x%x)\n",
				slave_address_base_req, sla_found);
				p_printf(1,"**************************************************************************\n");
			}
		}

		slave_address_base=sla_found;
		bcm2835_i2c_setSlaveAddress(sla_found);
	}
	else
	{
		slave_address_base=slave_address_old;
		bcm2835_i2c_setSlaveAddress(slave_address_old);
	}	
	
	return(count);
}

/* read current slave address
 * 
 * return code:
 * 200 can not find device
 * 201 found more than one device
 * 202 can not read config (to check PWM)
 * 203 is in PWM mode (and thus no valid slave address)
 * else 0
 * 
 */

uint8_t slv_addr_get_cur()
{
	long 	reg;
	int 	ret;
	
	// find a valid slave address on I2C.
	ret = check_for_mlx();
	
	if (ret == 0)
	{
		p_printf(1,"No device found\n");
		return(200);
	}		
	else if (ret > 1)
	{
		p_printf(1,
		"Discovered %d devices\n"
		"For security reasons this program can only work with \n"
		"one device connected on the line.\n",ret);
		return(201);
	}
	
	// read config register
	if ((reg = read_reg(CONFIG)) < 0)
	{
		p_printf(1,"can not read config register\n");
		return(201);
	}

	// check for PWM mode
	if (! reg & 0x1)
	{
		p_printf(1,"MLX90615 is in PWM mode\n");
		return(203);
	}

	return(0);
}

/* in case the MLX can not be discovered, advice to recover is provided */

int slv_recover()
{
	char	answ;
	long 	ret;
	uint8_t	sla;
	
	p_printf(3,
	"\n###################################################################\n");
	p_printf(1,
	"Your MLX can not be discovered. Try to following steps first:\n"
	"1. Is your MLX in sleep mode. Try exit sleep (main menu 10, sub menu 2)\n" 
	"2. Check your connection is correct and re-run the discovery\n"
	"3. Remove the device wait 10 seconds, reconnect and try again.\n"
	"4. Remove the device, re-boot the Raspberry Pi and try again.\n");
	
	p_printf(3,
	"\n\nIf you have tried all 3 options, but it still fails we can try\n"
	"a next step to recover.\n");
	
	do
	{
		p_printf(2, "\nDo you want to try ? (Y or N)");
		
		do
		{
			scanf("%c",&answ); 
		} while(answ == 0xa);
		
		if (answ == 'n' || answ == 'N') return(-1);

	} while (answ != 'y' && answ != 'Y');
	
	// set BCM2835 to slave 0x0
	bcm2835_i2c_setSlaveAddress(0x0);
	
	// set for PEC calculation
	slave_address_base= 0x0;
	
	ret=read_reg(0x0);

	if (ret != -1)
	{
		p_printf(1,
		"The MLX is still responding to slave address 0x0,\n"
		"as all MLX will respond to that if connected correctly\n");
		
		if (slave_address_base_req)
		{
			sla = slave_address_base_req;
			p_printf(3,	"Do you want to reset to the requested slave address (0x%x)?\n", sla);
		}
		else
		{
			sla = default_SLA;
			p_printf(3,	"Do you want to reset to the default address (0x%x)?\n", sla);
		}
		
		p_printf(3,	
		"###################################################################\n"
		"WARNING this must only be done if there is ONLY ONE MLX connected !!\n"
		"###################################################################\n");
		
		do
		{
			p_printf(2, "\nDo you want to try ? (Y or N)");
			
			do
			{
				scanf("%c",&answ); 
			} while(answ == 0xa);
			
			if (answ == 'n' || answ == 'N') return(-1);
	
		} while (answ != 'Y' && answ != 'y');
		
		// write default slave address
		if (write_reg(PWMSA, (long) (sla & 0x7f)) == 0 )
		{
	        p_printf(2,"Slave address set to 0x%x\n", sla);
			return(0);
		}
	
		p_printf(1,"can not set new slave address\n");
		return(-1);
	}
	else
	{
		p_printf(1,
		"The MLX does not respond to slave address 0x0\n"
		"Double check the connection and repeat recovery steps.\n");
	}
	return(0);
}


/* set the MLX90615 slave address
 * 
 * This will be allowed if only ONE MLX90615 is connected because
 * all of them have the same default address and all react to 0x0.
 * 
 * This is to prevent setting the incorrect address to multiple and
 * create a potential chaos.
 * 
 * @param sl_addr : if 0 new address will be asked
 * 
 * return:0 = OK, -1 = error */

int slv_addr_set()
{
    int     addr;
    uint8_t	cur_sl_addr, new_sl_addr;
    char	buf_id[10];

    // get current slave address from the MLX90615
    cur_sl_addr = slv_addr_get_cur();
    
    if (cur_sl_addr == 200)	return(slv_recover());
    if (cur_sl_addr > 200)	return(-1);

	// check requested new slave addr provided on the command line
    if (slave_address_base_req)
    {
        if(slave_address_base_req > 0x7f || slave_address_base_req < 1)
        {
            p_printf(1, "invalid slave address provided : %x\n", slave_address_base_req);
            return(-1);
        }

        new_sl_addr = slave_address_base_req;
    }
    
    else
    {
        // read unit ID
        if(get_unit_id(0,buf_id) < 0) return(-1);
        
        p_printf(2,"current detected slave address is: 0x%x for MLX90615 with ID: %s\n", slave_address_base,buf_id);

        do
        {
            p_printf(3,"Provide new slave address (1 > address < 0xf7) \n(99 = return) ");
            scanf("%x",&addr);

            if (addr == 0x99) return(0);

        } while (addr > 0xf7 || addr < 1);
        
        new_sl_addr = addr;
    }

	// write updated slave address
	if (write_reg(PWMSA, (long) (new_sl_addr & 0x7f)) == 0 )
	{
        p_printf(2,"slave address set to 0x%x\n", slave_address_base);
		return(0);
	}
	
	p_printf (1,"can not set new slave address\n");
	return(-1);
}

/* set the BCM2835 ready for SMD communication and detect the
 * slave address of the single device connected 
 */
void set_for_smb()
{
    int	mlx_count;
    
	// if new slave address was provided on command line (-s)
	if (slave_address_base_req) slave_address_base = slave_address_base_req;
	
	/* do a wake-up in case the MLX was in sleep
	 * or it is in PWM mde and set it to I2c*/
	
	if(wake_up() < 0)
	{
		p_printf(1,"reset to I2C communication failed\n");
		close_out(-1);
	}
		
	// find the current slave_address for the connected MLX
	if ((mlx_count = check_for_mlx()) != 1)
	{
		 if (mlx_count > 1)
			p_printf(1,"Detected %d slave decives i2c line. BE CAREFULL \n");
		else
			p_printf(1,"Detected NO MLX90615\n");
	}
	else
		p_printf(2,"Slave device detected with address 0x%x\n",slave_address_base);
		
}

/* setup hardware  */
void hw_init() 
{
    
    if (!bcm2835_init()) {
        p_printf(1,"Can't init bcm2835!\n");
        exit(1);
    }
    
    // turn the power to MLX on.
    mlx_power(ON);
    
    // if starting for PWM mode -p
    if (pwm_mode)	set_SCL_high(0);
    
    // if NO PWM signal was detected in set_SCL_high(), pwm_mode was reset
    // hence another check
    if (! pwm_mode) set_for_smb();
}

/* end the program correctly */
void close_out(int end)
{
    if (in_sleep_mode)
    {
		p_printf(1, "The MLX is exiting sleep mode before closing program.\n");
		wake_up();
    }
    
    // turn the power to MLX on.
    mlx_power(OFF);
    
    // reset pins
    if (pwm_mode)
		bcm2835_gpio_fsel(RPI_V2_GPIO_P1_05, BCM2835_GPIO_FSEL_INPT); 
    else
		bcm2835_i2c_end();	
	
    // release BCM2835 library
    bcm2835_close();

    // exit with return code
    exit(end);
}

/* display information about the emissivity register
 * @param disp : whether or not to display the content
 * 
 * returns value of emissivity register or -1 in case of error*/
long disp_emis(int disp)
{
	long reg;
		
	if ((reg = read_reg(EMMIS)) < 0)
	{
		p_printf(1,"can not read Emmissivity register \n");
		return(-1);
	}
	
	if(disp)
		p_printf(2,"current emmissivity register value is 0x%lx and thus emmissivity is %1.2f\n", reg, (double) reg/16384);
	
	return(reg);
}
	

/* Display all the MLX90615 registers.
 * return value 0 = ok, 1 = error */

int disp_all_reg()
{
    int	i;
    long reg;
    
    p_printf(3,"\nAll registers\n");
    
    for (i = 0; i < 16; i++)
    {
		
		if ((reg= read_reg(i)) < 0)
		{
			p_printf(1,"can not read register %d\n",i);
			return(-1);
		}
		// display content of register
		disp_cont_reg(i, reg);
	}
	
	printf("\n\n");
	
	return(0);
}

/* Display registers and display info */
void disp_cont_reg(char reg, long val)
{
	long result;
	static	int unit_id = 0;		// display unit_id once in detailed
	
	switch(reg)
	{
		case 0  : 
			if ((result = read_reg(CONFIG)) < 0)
			{
				p_printf(1,"can not read CONFIG register\n");
				return;
			}
			if (result & 0x1)
				p_printf(2,"slave address:\t0x%x\n", val & 0xff);
			else
				p_printf(2, "PWM T min\t0x%lx\n", val);
			
			break;
		case 1  : 
			p_printf(2,"PWM T range:\t0x%lx\n", val);
			break;
		case 2  : 
			if (detailed) disp_config();
			else p_printf(2,"config:\t\t0x%lx\n", val);
			break;
		case 3  : 
			if (detailed) disp_emis(1);
			else p_printf(2,"Emissitivy:\t0x%lx\n", val);
			break;
		case 4  :
		case 5  : 
		case 6  : 
		case 7  : 
		case 8  : 
		case 9  : 
		case 10 : 
		case 11 : 
		case 12 : 
		case 13 : 
			p_printf(2,"Melexis res:\t0x%lx\n", val);
			break;
		case 14 :
		case 15 :
			if (detailed) 
			{
				if (unit_id++ == 0) get_unit_id(1, NULL);
				else unit_id = 0;
			}
			else p_printf(2,"ID number:\t0x%04lx\n", val);
			break;
	}
}

/*
 * read unit_id
 * @param disp : 1 = display, 0 = return in buf
 * @param buf : NULL or buffer to store ID ( minimal 7 positions)
 */
int get_unit_id(int disp, char *buf)
{
	long id_high, id_low;

	if ((id_low = read_reg(ID1)) < 0)
	{
		p_printf(1,"can not read ID low register\n");
		return(-1);
	}

	if ((id_high = read_reg(ID2)) < 0)
	{
		p_printf(1,"can not read ID high register\n");
		return(-1);
	}
	
	// if display is requested
	if (disp)
		p_printf(2,"\nUnit Id:\t%lx%lx\n",id_high, id_low);
	else
		sprintf(buf,"%lx%lx\n", id_high, id_low);
	
	return(0);
}

/* display configuration register */

void disp_config()
{
	long result; 
	int	tmp;

	if ((result = read_reg(CONFIG)) < 0)
	{
		p_printf(1,"can not read CONFIG register\n");
		return;
	}
	
	if (detailed)
	{
		p_printf(2,"Config register: 0x%lx\n\n", result);
	
		for (tmp = 15; tmp > -1; tmp--) printf("%d ", tmp);
		
		printf("\n");
		
		printf("%d  ", (result & 0x08000) ? 1:0);
		printf("%d  ", (result & 0x04000) ? 1:0);
		printf("%d  ", (result & 0x02000) ? 1:0);
		printf("%d  ", (result & 0x01000) ? 1:0);	
		printf("%d  ", (result & 0x0800) ? 1:0);
		printf("%d  ", (result & 0x0400) ? 1:0);
		printf("%d ", (result & 0x0200) ? 1:0);
		printf("%d ", (result & 0x0100) ? 1:0);	
		printf("%d ", (result & 0x080) ? 1:0);
		printf("%d ", (result & 0x040) ? 1:0);
		printf("%d ", (result & 0x020) ? 1:0);
		printf("%d ", (result & 0x010) ? 1:0);
		printf("%d ", (result & 0x08) ? 1:0);
		printf("%d ", (result & 0x04) ? 1:0);
		printf("%d ", (result & 0x02) ? 1:0);
		printf("%d ", (result & 0x01) ? 1:0);
	
		printf("\n\n");
	}
	
	p_printf(1, "\nUser definable parameters\n");
	
	if (result & 0x1)
		p_printf(3, "\tCommunication type is SMBus (%d)\n", (result & 0x01) ? 1:0);
	else
	{ 
		p_printf(3, "\tCommunication type is PWM (%d)\n", (result & 0x01) ? 1:0);
		
		if (result & 0x2)
			p_printf(3, "\tPWM frequency is 10kHZ (%d)\n", (result & 0x02) ? 1:0);
		else
			p_printf(3, "\tPWM frequency is 1kHZ (%d)\n", (result & 0x02) ? 1:0);
		
		if (result & 0x4)
			p_printf(3, "\tPWM output is Ta (ambient)(%d)\n", (result & 0x04) ? 1:0);
		else
			p_printf(3, "\tPWM output is To (object) (%d)\n", (result & 0x04) ? 1:0);
	}

	if (result & 0x800)
		p_printf(3, "\tInternal shunt regulator enabled (%d)\n", (result & 0x800) ? 1:0);
	else
		p_printf(3, "\tInternal shunt regulator disabled (%d)\n", (result & 0x800) ? 1:0);

	p_printf(1, "\nMLX reserved parameters\n");
	p_printf(3, "\tInternal RC Oscillator Trimming: 0x%x\n", (result>>3) & 0x3f);
	
	tmp = (int) (result >>9) & 0x3;
	
	if (tmp == 0)
		p_printf(3, "\tGain is bypassed (%d)\n", tmp);
	else if (tmp == 1)
		p_printf(3, "\tGain = 10 (%d)\n", tmp);
	else 
		p_printf(3, "\tGain = 40 (%d)\n", tmp);
		
	tmp = (int) result >>12 & 0x7;
	
	if (tmp == 0)
		p_printf(1, "\tForbidden combination (%d)\n", tmp);
	else if (tmp == 1)
		p_printf(3, "\tIIR (2) a1=1, b1=0 (%d)\n", tmp);
	else if (tmp == 2) 
		p_printf(3, "\tIIR (3) a1=0.5, b1=0.5 (%d)\n", tmp);
	else if (tmp == 3)
		p_printf(3, "\tIIR (4) a1=0.333(3), b1=0.666(6) (%d)\n", tmp);	
	else if (tmp == 4)
		p_printf(3, "\tIIR (5) a1=0.25, b1=0.75 (%d)\n", tmp);	
	else if (tmp == 5)
		p_printf(3, "\tIIR (6) a1=0.2, b1=0.8 (%d)\n", tmp);
	else if (tmp == 6)
		p_printf(3, "\tIIR (7) a1=0.166(6), b1=0.833(3)\n");		
	else if (tmp == 7)
		p_printf(3, "\tIIR (8) a1=0.14286, b1=0.87514 (%d)\n", tmp);	
			
}

/* display temperature information
 * @param temp : TO=object, TA = ambient, RAWIR = raw
 * 
 * it is in celcius. if Fahrenheit is needed : (celcius *1.8) + 32
 */
int display_temp(int temp)
{
	long ram;
	
	// check for correct request
	if (temp != TO && temp != TA && temp != RAWIR)
	{
		p_printf(1,"Invalid argument %d\n", temp);
		return(-1);
	}
	
	// read the location
	if ((ram = read_ram(temp)) < 0)
	{
		p_printf(1,"can not read ram location %d\n",temp);
		return(-1);
	}	

	if (temp == TA )
		p_printf(2,"Ambient temperature is %2.2fC\n", ((ram & 0x7fff) * 0.02) - 273.15);
	
	else if (temp == TO )
		p_printf(2,"Object temperature is %2.2fC\n",  ((ram & 0x7fff)* 0.02) - 273.15);
	
	else
		p_printf(2,"Raw IR data sign %c, magnitude: 0x%04lx\n",(ram & 0x08000) ? '+':'-', ram & 0x7fff);

	return(0);
}

int disp_all_ram()
{
	int	i;
    long ram;
    
    p_printf(3,"\nAll ram locations\n");
    
    for (i = 0; i < 16; i++)
    {
		
		if ((ram = read_ram(i)) < 0)
		{
			p_printf(1,"can not read ram location %d\n",i);
			return(-1);
		}
		
		switch(i)
		{
			case 5  : 
				p_printf(2,"RAW IR data :\t0x%x\n", ram);
				if (detailed) display_temp(RAWIR);
				break;
			
			case 6  : 
				p_printf(2,"Ta (ambient):\t0x%lx\n", ram);
				if (detailed) display_temp(TA);
				break;
			
			case 7  : 
				p_printf(2,"To (object) :\t0x%lx\n", ram);
				if (detailed) display_temp(TO);
				break;
			
			default :
				p_printf(2,"Melexis res :\t0x%lx\n", ram);
				break;
		}
	}
	
	printf("\n");
	
	return(0);
}

/* set new value for emissivity 
 * this can be directly manual (if emissivity is known)
 * or with lookup based on type/material
 */

void set_emis()
{
	long  	emiss;
	int	 	answ;
	float	new_emiss;
	
	// read & display current level
	if ((emiss = disp_emis(1)) < 0) return;
  
    do
	{
		p_printf(3,"Do you want to: \n");
		p_printf(2,"1) lookup emissivity values based on type/material\n2) input directly\n(99 = return) ");
		answ = get_dec_input();

		if (answ == 99 || answ == -1) return;
		else if (answ == 1)	new_emiss = find_emis();
		else if (answ == 2) new_emiss = read_emis_int();
		else
		{
			 p_printf(1,"Invalid answer : %d\n", answ);
			answ = 0;
		}
	} while (answ == 0);

	// write updated new emissivity level
	if (write_reg(EMMIS, (long) (16384 * new_emiss)) != 0 )
		p_printf (1,"can not set new emissivity level 0x%lx\n", (long) (16384 * new_emiss));
	
	return;	
}

/* set the MLX to sleep */

void set_sleep()
{
	int answ;
	
	p_printf(3,	"\nDo you want to:\n1) enter sleep mode\n2) exit sleep mode\n(99 = return) ");
	
	do
	{
		p_printf(3, "? ");
		
		answ = get_dec_input();

		if (answ == 99 || answ == -1) return;
		
		else if (answ == 1) 
		{
			if(enter_sleep() < 0)
				p_printf(1,"could not set sleep mode\n");
			else
			{
				p_printf(2,"sleep mode entered\n");
				// if still in sleep mode, a wake-up is done in close_out()
				in_sleep_mode = 1;
			}
			
			return;
		}
		else if (answ == 2) 
		{
			if (wake_up() < 0)
				p_printf(1,"could not wakeup\n");
			else
			{
				p_printf(2,"exit sleep mode\n");
				// reset still in sleep mode
				in_sleep_mode = 0;
			}
			
			return;
		}
		
	} while (1);

}


/* catch signals to close out correctly */
void signal_handler(int sig_num)
{
	switch(sig_num)
	{
		case SIGKILL:
		case SIGABRT:
		case SIGINT:
		case SIGTERM:
			printf("\nStopping MLX90615 reader.\n");
			close_out(2);
			break;
		default:
			printf("\nneglecting signal %d.\n",sig_num);
	}
}

/* setup signals */
void set_signals()
{
	struct sigaction act;
	
	memset(&act, 0x0,sizeof(act));
	act.sa_handler = &signal_handler;
	sigemptyset(&act.sa_mask);
	
	sigaction(SIGTERM,&act, NULL);
	sigaction(SIGINT,&act, NULL);
	sigaction(SIGABRT,&act, NULL);
	sigaction(SIGSEGV,&act, NULL);
	sigaction(SIGKILL,&act, NULL);
}

/* get yes (1) or No (0) */
int yes_or_no(char *mess)
{
	char	answ;
	
	while (1)
	{
		p_printf(3, mess);
		
		scanf("%c",&answ); 
		
		if (answ == 'n' || answ == 'N') return(0);
		if (answ == 'y' || answ == 'Y') return(1);
	} 
}

/* This is a potential way to recover the MLX by writting the config register
 * follow the text below.
 * 
 * THIS ROUTINE CAN CAUSE UNREPARABLE DAMAGE TO THE MLX AND SHOULD ONLY BE TRIED 
 * AS A LAST RESORT WHEN NOTHING ELSE HELPED. THERE IS NO GUARANTY, NO SUPPORT, NO DAMAGE PAYMENT
 * NOTHING.... YOU ARE ON YOUR OWN...
 * 
 * BUT IN MY CASE IT DID RECOVER MY MLX :-)
 */	
void config_recover()
{
	unsigned long restore_val = default_CONF;
	long cur_conf; 

	p_printf(1,
	"You are about to do a VERY TRICKY action: reset the COMPLETE config register value !\n"
	"This might be necessary in case a write of a new value to the config register had failed\n"
	"and you want to restore to a default value.\n\n"
	"A number of programmed safety checks are by-passed by applying this routine that can cause\n"
	"the MLX to be DAMAGED FOR EVER. CONTINUING THIS ROUTINE MEANS YOU ARE ON YOUR OWN.\n");
	
	if (yes_or_no("Are you sure you want to continue ?  (Y or N) ") == 0) 	return;
	
	p_printf(1,
	"\nThis can only be done is the correct slave address has been set to access the MLX\n"
	"Follow the menu options for that first. You might get PEC errors when reading trying \n"
	"read a register on the MLX. First try that you can read the registers and/or RAM with \n"
	"the option -n : 'no PEC check on read' on the command line. If you can read, we can try to reset\n\n"
	"PLEASE BE AWARE THAT OVERWRITING WITH THE WRONG VALUE, IT CAN CHANGE THE RESULTS OF THE MLX \n"
	"AND CAN CAUSE UNREPAIRABLE DAMAGE.\n" );
	
	if (yes_or_no("Are you sure you want to continue ?  (Y or N) ") == 0) 	return;
		
	p_printf(1,"\nyou are a brave person !. Lets try to read first the config register\n\n");
	
	// no PWM mode (just to be sure)
	pwm_mode=0;
    
    // set DEBUG
    DEBUG = 1;
    
    // set detailed
    detailed = 1;
    
    // set for NO pec_check on read
    no_pec_check = 1;
    
    // do hardware init
    hw_init();
		
	// try to read without pec check & display content
	if ((cur_conf = read_reg(CONFIG)) != -1)	disp_config();

	else
		p_printf(1,"Not able to read the config register. A recovery is highly questionable !\n");
		
	if (yes_or_no("\nAre you sure you want to continue ?  (Y or N) ") == 0) 	return;
	
	p_printf(2, "current default value to restore is 0x%lx\n", restore_val);
	
	if (yes_or_no("Do you want to use this value?  (Y or N) ") == 0) 
	{	
		p_printf(3, "Please provide a different value to use. There is NO CHECK ON VALID INPUT : ");
		
		do
		{
			scanf ("%lx", &restore_val);
		} while(restore_val == 0xa);
	}
	
	p_printf(1,
	"\n***************************************\n"
	"Going to write 0x%lx to config_register\n"
	"***************************************\n\n",restore_val);
	
	if (yes_or_no("Are you REALLY SURE you want to continue ?  (Y or N) ") == 0) 	return;
	
	p_printf(2, "OK.. lets try...First erase config register\n");

	// add register opcode
	if (write_mlx((CONFIG | 0x10), 0x0000) < 0)
	{
		p_printf(1, " Sorry.. but can not erase the config register. Stopping now.\n");
		return;
	}
		
	p_printf(2, "Step 1 worked out .. now trying to write new content\n");	

	// add register opcode
	if (write_mlx((CONFIG | 0x10), restore_val) < 0)
		p_printf(1, "Sorry.. but can not write the new content the config register\n");
	else
		p_printf(2, "Maybe this is your lucky day.. it seemed to have worked.\n");	
}

/* display usage / help information */
void usage(char *name)
{
	p_printf(2,
	"Version: %s \n"
	"Copyright (c)  2017 Paul van Haastrecht\n\n", MLX_VERSION);
	
	p_printf(3,"%s [-m #] [-r #] [-h] ..... \n\n"
		
		"PWM options :\n"
		"-p, 	start in PWM menu & try to read attributes from MLX\n"
		"-P, 	start in PWM menu & overwrite attributes in the MLX\n"
		"-m, 	set minimum temperature for PWM (in Celsius)\n"
		"-r,	set range temperature for PWM (in Celsius)\n"
		"-a,	set for ambient temperature\n"
		"-o,	set for object temperature\n"
		"-l,	set for low frequency (10hz)\n"
		"-h,	set for high frequency (1khz)\n"
		
		"\nSMB options :\n"
		"-s,	slave address to use\n"
		"-n,	no PEC check on read\n"
		
		"\nGeneral options\n"
		"-t,	enable debug tracking\n"
		"-d,	enable detailed display\n"
		"-H,	display this help text\n"
		
		"\nSpecial options\n"
		"-C,	recovery of config register. (CAREFULL !!!)\n", name);
}

/* Toggle detailed */
void toggle_detailed()
{
	if (detailed) 
		{
			p_printf(2,"details off\n");
			detailed = 0;
		}
		else 
		{
			p_printf(2,"details on\n");
			detailed = 1;
		}
}

/* Toggle debug */
void toggle_debug()
{
	if (DEBUG) 
		{
			p_printf(2,"DEBUG messages off\n");
			DEBUG = 0;
		}
		else
		{
			p_printf(2,"DEBUG messages on\n");
			DEBUG = 1;
		}
}

int main(int argc, char *argv[])
{
    int tmp=0, c;
    int set_pwm_value = 0;

	// catch signals
	set_signals();

	while (1)
	{
		c = getopt(argc, argv,"-aolhpm:r:s:ntCPdH");

		if (c == -1)	break;
			
		switch (c) {

			case 'P':	// set to PWM mode with command line /default values
				set_pwm_value = 1;
			case 'p':	// set to PWM mode & read MLX attributes
				pwm_mode = 1;
				pwm_menu = 1;
				p_printf(2,"PWM menu has been requested \n");
				break;
			case 'a':	// set ambient temperature PWM
				cur_temp=TA;
				break;

			case 'o':	// set object temperature PWM
				cur_temp=TO;
				break;
			
			case 'l':	// set low frequency PWM
				cur_freq=LOW;
				break;
			case 'h':	// set high frequency PWM
				cur_freq=HIGH;
				break;

			case 'm':	// set t_min for PWM
				if (calc_to_t_min(strtod(optarg,NULL))< 0)
				{
					p_printf(1,"can not set T_minimum\n");
					exit(1);
				}
				break;
			case 'r':	// set t_range for PWM
				if (calc_to_t_range(strtod(optarg, NULL))< 0)
				{
					p_printf(1,"can not set T_range\n");
					exit(1);
				}
				break;	
						
			case 'n':	// No PEC check on read
				no_pec_check = 1;
				break;
		
			case 's':	// set slave_address
				slave_address_base_req = (uint8_t) strtol(optarg, NULL,16);

				if (slave_address_base_req> 0xf7 || slave_address_base_req < 0)
				{
					p_printf(1,"Invalid slave address provided %x\n", slave_address_base_req);
					exit(1);
				}
				break;

			case 'd':	// enable detailed
				detailed = 1;
				break;
							
			case 't':	// debug tracking
				DEBUG = 1;
				break;								

			case 'C':	// CONFIG RECOVER
				config_recover();
				close_out(0);
				break;	
			
			case 'H':
				usage(argv[0]);
				exit(0);
				break;				
			default:
				p_printf(1,"Invalid argument %c. try %s -H\n",c,argv[0]);
				exit(-1);
				break;
		}
	}
	
	if (geteuid() != 0){
        p_printf(1,"Must be run as root.\n");
        exit(-1);
    }
	
    // do hardware init
    hw_init();
    
    // if PWM menu was requested on command line
    if (pwm_menu)	set_pwm(set_pwm_value);
    else
    {
		// get manual input
	    do
	    {
	        p_printf(3,"\nWhat do you want to do ? \n\n");
	        p_printf(2,
	        "1	Change slave address\n"
	        "2	Display ambient temperature information\n"
	        "3	Display object temperature information\n"
	        "4	Display RAW-IR data\n"
	        "5	Display the configuration register MLX90615\n"
	        "6	Display ALL registers of MLX90615\n"
			"7	Display ALL RAM locations of MLX90615\n"
			"8	Display unit ID\n"
	        "9	Show PWM menu\n"
	        "10	Enter / exit sleep mode");
	        if (in_sleep_mode) p_printf(1," (in sleep mode)");
	        
	        p_printf(2,"\n11	set emissivity\n");
	        
	        p_printf(2,"12	Toggle to show details (where possible)");
	        if(detailed) p_printf(1," (enabled)");
	        
	        p_printf(2,"\n13	Toggle debug messages");
	        if(DEBUG) p_printf(1," (enabled)");
	        
	        p_printf(3, "\n\n99  exit program ");
			
	        tmp = get_dec_input();

			if (tmp < 0)	tmp = 99;

	        switch(tmp)
	        {
	        case 1:
	            slv_addr_set();
	            break;
	        case 2:
				display_temp(TA);
	            break;
	        case 3:
				display_temp(TO);
	            break;
	        case 4:
				display_temp(RAWIR);
	            break;
	        case 5:
	            disp_config();
	            break;
	        case 6:
	            disp_all_reg();
	            break;
	        case 7:
	            disp_all_ram();
	            break;
	        case 8:
				get_unit_id(1, NULL);
				break;
	        case 9:
	            set_SCL_high(1);	// try to enable PWM
	            set_pwm(0);
	            set_for_smb();		// back to SMB mode
	            break;
	        case 10:
	            set_sleep();
	            break;
	        case 11:
	            set_emis();
	            break;
	        case 12:
				toggle_detailed();
	            break;
	        case 13:
				toggle_debug();
	            break;
	        case 99:
	            break;
	        default:
	            p_printf(1,"invalid entry, try again\n");
	            break;
	        }

	    } while (tmp != 99);
	}
	
    //close cleanly
    close_out(1);
    
    exit(0);
}
