#!/bin/bash

rm -R briscola/*
mkdir briscola/srv
rm launcher

gcc -o briscola/mainserver src/mainserver.c 
gcc -o briscola/gameserver src/gameserver.c
