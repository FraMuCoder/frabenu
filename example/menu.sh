#!/bin/bash

# use this stop fbi if it is used for splash screen
#sudo killall fbi

# shorter reaction for ESC
export ESCDELAY=100

# select default
selection=1

# use this if you want to restart menu after any selection but abort ([ESC] or [Q])
#while [ 1 ]
#   do

   ./frabenu -s2 -d$selection 3x2 menu_%x_%y.png

   selection=$?

   case $selection in
   0) clear;
      echo "You have select $selection => abort";
      exit;
      ;;
   1) clear;
      echo "You have select $selection => menu_1_1.png";
      ;;
   2) clear;
      echo "You have select $selection => menu_2_1.png";
      ;;
   3) clear;
      echo "You have select $selection => menu_3_1.png";
      ;;
   4) clear;
      echo "You have select $selection => menu_1_2.png";
      ;;
   5) clear;
      echo "You have select $selection => menu_2_2.png";
      ;;
   6) clear;
      echo "You have select $selection => menu_3_2.png";
      ;;
   255) clear;
      echo "You have select $selection => error";
      exit
      ;;
   *) clear;
      echo "You have select $selection => this should not happen";
      exit
      ;;
   esac

#done
