#!/bin/bash
# Creates the EasyRPG App banner

bannertool makecwav -i ../cia_builder/assets/audio.wav -o banner/banner.bcwav
3dstool -c -t banner -f banner.bin --banner-dir banner
