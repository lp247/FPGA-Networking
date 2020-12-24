open_project proj_eth_out -reset
set_top eth_out
add_files eth_out.cpp
add_files DataInputAnalyzer.cpp
add_files DataSender.cpp
add_files DataWordGenerator.cpp
add_files ETHPacketWordGenerator.cpp
add_files FCSWordGenerator.cpp
add_files IPPacketWordGenerator.cpp
add_files PayloadWordGenerator.cpp
add_files PreambleWordGenerator.cpp
add_files UDPPacketWordGenerator.cpp
add_files ../utils/checksums/Checksum.cpp
add_files ../utils/checksums/CRC32.cpp
add_files ../utils/axis_word.cpp
add_files -tb eth_out_test.cpp
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
export_design -format ip_catalog -flow impl -ipname eth_out -library eth -output ../../ip/eth_out -rtl verilog -vendor ME -version 1.0.0
