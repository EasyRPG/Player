#!/bin/sh
cd `dirname $0`
#RTP2K dir
export RPG2K_RTP_PATH=/usr/local/share/easyrpg/rtp2k
#RTP2K3 dir
export RPG2K3_RTP_PATH=/usr/local/share/easyrpg/rtp2k3
./EasyRPG
unset RPG2K_RTP_PATH
unset RPG2K3_RTP_PATH
