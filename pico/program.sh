#!/bin/bash

ELF_FILE="build/SensorPodController.elf"

#openocd -f upload/cfg/cmsis-dap.cfg -f upload/cfg/rp2040.cfg -c "program build/SensorPodController.elf verify reset exit"

openocd -f upload/cfg/cmsis-dap.cfg -c "adapter speed 5000" -f upload/cfg/rp2040.cfg -c "program ${ELF_FILE} verify reset exit" -s tcl
