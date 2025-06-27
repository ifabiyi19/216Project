Run commands:

To create the test.bin file: python3 make_test_bin.py 

To compile the simulator: gcc -std=c11 -O2 -o simulator simulator.c

To run the simulator: 

ARM instructions: ./simulator test_arm.bin ARM 
THUMB instructions: ./simulator test.bin THUMB  
