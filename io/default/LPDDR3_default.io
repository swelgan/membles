# LPDDR3 IO power model
DQ_PER_STROBE=8 # Ratio between DQ and DQS
NUM_CMD_BIT=1   # CS pin
NUM_ADDR_BIT=10 # CA pins  
Vdd_IO=1.2      # Volt
C_LINE=3        # pF, capacitance on channel, POP package
C_MEM_DQ=2      # pF, capacitance on DRAM DQ/DQS pin
C_MEM_CMD=1.5   # pF, capacitance on DRAM CS pin
C_MEM_ADDR=1.5  # pF, capacitance on DRAM CA pin
C_MEM_CLK=1.5   # pF, capacitance on DRAM CK pin
C_CTRL_DQ=2     # pF, capacitance on SoC DQ/DQS pin
C_CTRL_CMD=1.5  # pF, capacitance on SoC CS pin
C_CTRL_ADDR=1.5 # pF, capacitance on SoC CA pin
C_CTRL_CLK=1.5  # pF, capacitance on SoC CK pin


