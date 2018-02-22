#!/usr/bin/env python

import os, sys

for i in range(1,11):
    if(i != 8):
        cmd = "./switch.bin " + str(i) + " localhost " + str(5000)  + " & "
        # print cmd
        os.system(cmd)
        if (i % 2 == 0):
            os.system("sleep 1")

