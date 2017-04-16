# version 1.0	initial version

Copyright (c) 2017 Paul van Haastrecht <paulvha@hotmail.com>


## Background
The MLX90615 is like an Infra_Red ( I.R.) temperature sensor. Having seen a commercial IR temperature meter, I wanted to better understand how it works. 

The MLX90615 is available at low cost as part of the GROOVE family from Seed-Studio. 
There is a great, detailed, data-sheet available on-line that provide a lot of insights. 
I was, and I am, impressed by the possibilities and flexibility of the sensor.

I saw a number of libraries and programs available for the Arduino, but I did not find a good programs for the Raspberry Pi. 
Hence I to created a program that makes use of the complete capabilities.

My focus of interest is the area ”where hardware meets software” and the program developed is focused on exactly that. 
The learnings have been documented in the MLX90615.odt and provides good reading on some the errors in de data-sheet.

It does not have a fancy interface, but it does work and is able to explore the complete capabilities of the MLX90615. 
A next step could be the expansion with a graphical interface.

 
## Software installation


Make yourself superuser : sudo bash

BCM2835 library
Install latest from BCM2835 from : http://www.airspayce.com/mikem/bcm2835/

1. cd /home/pi
2. wget http://www.airspayce.com/mikem/bcm2835/bcm2835-1.50.tar.gz
3. tar -zxf bcm2835-1.50.tar.gz		// 1.50 was version number at the time of writing
4. cd bcm2835-1.50
5. ./configure
6. sudo make check
7. sudo make install

Install the MLX utility
1. cd /home/pi
2. tar -xzvf mlx90615.1.tar.gz
3. cd  mlx
4. ./mmlx.sh

To run the software you MUST be root/super user given the Linux permission: sudo ./mlx
