# this is a LPDDR3-1600 configuration

# memory type, e.g DDR3, LPDDR2, LPDDR3, ...
MEM_TYPE=LPDDR3

# number of banks per physical channel
NUM_BANK=8

# number of rows per bank
NUM_ROW=16384

# number of columns per bank
NUM_COL=1024

# device width, unit: bit
DEVICE_WIDTH=32

# clock period, unit: nanosecond
tCK=1.25

# refresh period, unit: nanosecond
tREFI=3900

# burst length
BL=8 

# DRAM timing parameters
RL=12 
WL=6            # set A
AL=0
tCCD=4
tRTP=7.5ns,4
tRCD=18ns,3
tRPpb=18ns,3
tRPab=21ns,3
tRAS=42ns,3
tWR=15ns,4 
tWTR=7.5ns,4
tRRD=10ns,2
tFAW=50ns,8
tDQSCK=4        # average number 
tRFCab=130ns    # 4Gb device
tRFCpb=60ns     # 4Gb device
tCMD=1          # one cycle per command

# supply voltage
Vdd=1.8
Vdd_2=1.2

# IDD model
IDD_MODEL=default

# I/O model
IO_MODEL=default

