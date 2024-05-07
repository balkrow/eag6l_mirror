#!/bin/bash

branch_name=$(git symbolic-ref --short HEAD)

./compilation_root/$branch_name/sim64_DX/appDemo -i ./sim_cfg/aldrin3_A0_wm.ini
