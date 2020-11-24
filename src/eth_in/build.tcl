open_project proj_eth_in -reset
set_top eth_in
add_files eth_in.cpp
add_files DataBundler.cpp
add_files DataGate.cpp
add_files DataSpotter.cpp
add_files EthDataHandler.cpp
add_files FCSValidator.cpp
add_files IPPacketHandler.cpp
add_files UDPPacketHandler.cpp
add_files ../utils/checksums/Checksum.cpp
add_files ../utils/checksums/CRC32.cpp
add_files ../utils/axis_word.cpp
add_files -tb eth_in_test.cpp
add_files -tb ../utils/test/Frame.cpp
add_files -tb ../utils/test/ETHPacket.cpp
add_files -tb ../utils/test/IPPacket.cpp
add_files -tb ../utils/test/UDPPacket.cpp
add_files -tb ../utils/test/calculate_checksum.cpp
add_files -tb ../utils/Addresses.cpp
open_solution "solution1"
set_part {xc7a100tcsg324-1}
create_clock -period 20 -name default
set_clock_uncertainty 1
config_rtl -module_auto_prefix -reset all -reset_level high
csim_design
csynth_design
cosim_design -rtl verilog -tool xsim
export_design -format ip_catalog -flow impl -ipname eth_in -library eth -output ../../ip/eth_in -rtl verilog -vendor ME -version 1.0.0
