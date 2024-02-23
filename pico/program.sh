#!/bin/bash

openocd -f upload/cfg/picoprobe.cfg -f upload/cfg/rp2040.cfg -c "program build/SensorPodController.elf verify reset exit"
