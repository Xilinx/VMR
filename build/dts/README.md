# Instruction

## apply vck5000.dtsi on top of system.dts for vck5000 
    echo "/include/ "vck5000.dtsi" >> system.dts
    dtc -I dts -O dtb -f system.dts -o system.dtb

## apply v70.dtsi on top of system.dts for v70
    echo "/include/ "v70.dtsi" >> system.dts
    dtc -I dts -O dtb -f system.dts -o system.dtb
