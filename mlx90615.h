/* header file for MLX90615 
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
 * For info only the register and RAM values from my MLX90615
 * 
 * Register 				  Ram dump of device
 * slave address: 0x5b		0 Melexis res :	0x39c4
 * PWM T range:	 0x9c3		1 Melexis res :	0xc1
 * Config register: 0x14d9	2 Melexis res :	0x60e
 * Emissitivy:	0x4000		3 Melexis res :	0x82f8
 * Melexis res:	0x6a67		4 Melexis res :	0x1a44
 * Melexis res:	0x355a		5 RAW IR data :	0x8036
 * Melexis res:	0x431c		6 Ta (ambient):	0x39c6
 * Melexis res:	0x2011		7 To (Object) :	0x39aa
 * Melexis res:	0x3d		8 Melexis res :	0x8004
 * Melexis res:	0x8011		9 Melexis res :	0x8003
 * Melexis res:	0x1d19		A Melexis res :	0x5ea
 * Melexis res:	0x269		B Melexis res :	0x1
 * Melexis res:	0x1a7a		C Melexis res :	0x5f6
 * Melexis res:	0x3a3c		D Melexis res :	0x24b
 * ID number:	0xc744		E Melexis res :	0x2f3
 * ID number:	0x64		F Melexis res :	0x3bc
*/


/* EEPROM registers */
#define PWMSA   0x0		// PWM T min / SMBus Slave address (SA)
#define PWMTR   0x1		// PWM T range
#define CONFIG  0x2		// config
#define EMMIS   0x3		// Emissivity
#define ID1   	0xE		// ID number
#define ID2     0xF		// ID number

/* RAM data */
#define RAWIR   0x5		// RAW IR data
#define TA		0x6		// Ambient Temperature
#define TO      0x7		// Object Temperature

/* default values */
#define default_SLA	0x5b
#define default_emissivity 0x4000
#define default_CONF	0x14d9	// at least on MY MLX90615
#define sda_pin RPI_V2_GPIO_P1_03
#define scl_pin RPI_V2_GPIO_P1_05

/*version number */
#define MLX_VERSION "1.0"

/* MLX POWER */
#define power_pin RPI_V2_GPIO_P1_07	// GPIO4
#define ON  1
#define OFF 0




/* display color */
#define REDSTR "\e[1;31m%s\e[00m"
#define GRNSTR "\e[1;92m%s\e[00m"
#define YLWSTR "\e[1;93m%s\e[00m"
#define BLUSTR "\e[1;34m%s\e[00m"


/** defined in mlx.c */

/* include details (where possible)*/
extern int detailed;

/* (default) MLX90615 slave address */
extern uint8_t slave_address_base;

/* overwrite default _slave address with -s option*/
extern uint8_t slave_address_base_req;


/** defined  in mlx_lib.c */

// enable /disable debug
extern int DEBUG;

/* MLX90615 slave address */
extern uint8_t slave_address_base;

/* in sleep mode ?*/
extern int in_sleep_mode;

// set for NO PEC check during read
extern int no_pec_check;


/** defined in mlx_pwm.c */
/* set to PWM mode */
extern int pwm_mode;

// PWM TMIN 
extern long t_min;

// PWM T_range
extern long t_range;

// indicate temperature to display
extern int cur_temp;

// PWM frequency requested
extern int cur_freq;


/************************/
/** routines in mlx */
/************************/

/* setup hardware  */
void hw_init();

/* set the BCM2835 ready for SMD communication and detect the
 * slave address of the single device connected */
void set_for_smb();

/* end the program correctly */
void close_out(int end);

/* get yes (1) or No (0) */
int yes_or_no(char *mess);

/* display usage / help information */
void usage(char *name);

/* catch signals to close out correctly */
void signal_handler(int sig_num);

/* setup signals */
void set_signals();

/* check for the number of devices on I2c
 * if only 1, it will set the slavebaseaddress.
 * Return the count or -1 in case of error */
int check_for_mlx();

/* get decimal input
 * When trying to get decimal number, it will continue to fail
 * if the number in the user level buffer is NOT decimal. So that has
 * to be removed from the user level buffer */
int get_dec_input();

/* read unit_id
 * @param disp : 1 = display, 0 = return in buf
 * @param buf : NULL or buffer to store ID ( minimal 7 positions) */
int get_unit_id(int disp, char *buf);

/* display temperature information
 * @param temp : object, ambient, raw */
int display_temp(int temp);

/* Display ram location content*/
int disp_all_ram();

/* Display all the MLX90615 registers.
 * return value 0 = ok, 1 = error */
int disp_all_reg();

/* Display registers content*/
void disp_cont_reg(char reg, long val);

/* Display detailed info about config register */
void disp_config();

/* display debug message
 * @param format : debug message to display and optional arguments
 *                 same as printf
 * @param level : priority : 1 = error, 0 = info 
 * info is only displayed if DEBUG had set for it*/
void p_printf (int level, char *format, ...);

/* set the MLX90615 slave address
 * @param sl_addr : if 0 new address will be asked
 * 
 * return:0 = OK, -1 = error */
int slv_addr_set();

/* read slave address
 * return address or 0 in case it can not be obtained */
uint8_t slv_addr_get_cur();

/* in case the MLX can not be discovered, advice to recover is provided */
int slv_recover();

/* set the MLX to sleep */
void set_sleep();

/* set new value for emissivity 
 * this can be directly manual (if emissivity is known)
 * or with lookup based on type/material */
void set_emis();

/* Toggle detailed */
void toggle_detailed();

/* Toggle debug */
void toggle_debug();

/************************/
/** routines in mlx_lib */
/************************/

/* handle power ON or OFF for MLX */
void mlx_power(int act);

/* write MLX90615 register
 * This will NOT allow to write the Melexis factory calibration registers
 * also a check on the CONFIG definable bits is done
 * @param regaddr : register to write
 * @param val : value to write
 *
 * return : 0 = OK,  -1 = Error */

int write_reg(char reg, long val);

/* read a registerfrom MLX90615
 * @param reg :  register to read.
 * 
 * return value
 * error :  -1  
 * else reg content */
long read_reg(char reg);

/* read a ram location from MLX90615
 * @param ram :  ram location to read
 * return value
 * error :  -1  
 * else ram content */
long read_ram(char ram);

/* sent sleep command to MLX */
int	enter_sleep();

/* wakeup MLX 
 * Datasheet pag. 15 : 8.4.8.2 exit sleep mode :
 * a low pulse of > 8ms is needed on SCL
 * 
 * Measurement done with a oscilloscope shows the pulse has 
 * to at least 17ms. Next to that another 0.3 seconds = 300ms should
 * be waited before stable measurement.
 *  
 * To be save the time has been set to 1 second.
 * 
 * It is assumed that a V2 system is used for the pull down */
int wake_up();

/* set the BCM2835 for I2c communication */
int set_mlx_i2c();

/* CRC8 check to compare PEC (Packet Error Checking)
 * &param poly : x8+x2+x1+1
 * @param data : array to check
 * @param size : #bytes
 * 
 * return CRC value
 */
uint8_t crc8Msb(uint8_t poly, uint8_t * data, int size);

/* write MLX90615 location
 * 
 * @param reg : location to write
 * @param val : value to write
 * 
 * For the MLX90615 it is only possible to write to registers
 * however we keep this structure open for the future.. you never know :-)
 *
 * return : 0 = OK,  -1 = Error */

int write_mlx(char reg, long val);

/* do a power up reset */
void por();

/************************/
/** routines in mlx_emiss.c */
/************************/


/* enter emissivity value directly */
float read_emis_int();

/* will support finding the emissivity for a certain material 
 * return 99 if no selection, otherwise the selection index from the table */
float find_emis();

/* Find entries in the emissivity table
 * @param step : 
 * 	1 = select by type, 
 * 	2 = select material within type
 * 	3 = wildcard search
 * 
 * @param lookup : 
 * 	lookup value for type in step 2 
 * 	loopup value for wildcard in step 3
 *  
 * return: 
 * 	99 = level back 
 * -1  = no match found (step 3)
 *  or entry in table
 */
int	select_emiss(int step, char *lookup);

/**************************/
/** routines in mlx_pwm.c */
/**************************/

/* Menu to handle PWM mode 
 * @param set_pwm_value : 
 * 	1 = set MLX to the values coming from the variables in the program, 
 *  0 = try to read from the MLX.
 */
int set_pwm(int set_pwm_value);


/* detect whether MLX is in PWM mode and capture the start_high, stop_high and cycle_time
 * return values
 * 1 = in PWM
 * 0 = not PWM
 */ 
int detect_pwm( double * r_start_high, double * r_stop_high, double * r_cycle_time);

/* read the T_min, T-range and temperature type from an MLX in pwm_mode
 * return : 0 = OK, 1 = not OK */

int get_values_from_mlx();

/* discover the current frequency either from the variable (if not PWM mode)
 * or by detecting the signal and display the result*/
void get_cur_freq(int d_col);

/* will write the attributes and set the MLX in PWM mode 
 * @param set_value: 1 = set attribute values from program, 0 = only set PWM mode
 * return 0 = OK, -1 is error*/
int enter_pwm(int set_value);

/* reset to SMB mode
 * if ch_to_smb = 1 : it will reset the config register
 * and make the change permanent to SMB.
 * 
 * return value 0 = OK, -1 = error */
int exit_pwm(int ch_to_smb);

/* will set a new PWM frequency
 * return value : 0 = OK, -1 = error */
int set_pwm_freq();

/* display the temperature that was selected BEFORE it went in PWM mode
 *  
 * It is not possible to request what temperature was set when in PWM 
 * as such the variable that is set in the menu is used 
 * 
 * The frequency is automatically detected and independent of the menu setting
 * 
 * return value
 * -2 : not in PWM mode
 * -1 : time-out
 *  0 : OK
 */
int display_pwm_temp();

/* will set T_min or T_range for PWM mode */
void set_range();

/* set SCL pin high
 * this must be done to keep the MLX in PWM mode 
 * A check is done whether the MLX is providing PWM signal
 * 
 * @param por : 1 = to do a power up reset first
 * 
 * Return values
 * 1 = MLX is providing PWM
 * 0 = MLX is NOT providing PWM
 */
int set_SCL_high(int reset);

/* reset to SMB mode
 * if ch_to_smb = 1 : it will reset the config register
 * and make the change perminate to SMB.
 */
int exit_pwm(int ch_to_smb);

/* set config register for PWM mode 
 * @param PWM : 1 = set to PWM else to SMB 
 * @param set_value : 1 = overwrite PWM attributes */
int set_conf_reg (int pwm, int set_value);

/* check, calculate and set t_min */
int calc_to_t_min(float val);

/* check, calculate and set t_range */
int calc_to_t_range (float val);

/* return current time in useconds */
double get_current();
