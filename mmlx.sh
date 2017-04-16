#!/bin/bash
# make mlx90615 executable
# version 1.0 / paulvha / April 2017

cc -Wall -o mlx mlx.c mlx_lib.c mlx_emiss.c mlx_pwm.c -lbcm2835 -lm
