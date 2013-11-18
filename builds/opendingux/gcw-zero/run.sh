#!/bin/sh
cd `dirname $0`
#RTP2K dir
export RPG2K_RTP_PATH=$HOME/.easyrpg/rtp2k
#RTP2K3 dir
export RPG2K3_RTP_PATH=$HOME/.easyrpg/rtp2k3
export RPG_GAME_PATH=$(dirname $1)
./EasyRPG
unset RPG2K_RTP_PATH
unset RPG2K3_RTP_PATH
unset RPG_GAME_PATH
