#!/bin/sh
SOURCE="$1"
DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"

if [ ! -d "$HOME/.easyrpg" ]; then
mkdir $HOME/.easyrpg
	if [ ! -d "$HOME/.easyrpg/rtp2k" ]; then
	mkdir $HOME/.easyrpg/rtp2k
	fi
	if [ ! -d "$HOME/.easyrpg/rtp2k3" ]; then
	mkdir $HOME/.easyrpg/rtp2k3
	fi
fi


#RTP2K dir
export RPG2K_RTP_PATH=$HOME/.easyrpg/rtp2k
#RTP2K3 dir
export RPG2K3_RTP_PATH=$HOME/.easyrpg/rtp2k3

./EasyRPG --project-path "$DIR" --fullscreen


unset RPG2K_RTP_PATH
unset RPG2K3_RTP_PATH
