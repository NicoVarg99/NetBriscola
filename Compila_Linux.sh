#!/bin/bash

echo -e "\e[3;1mTutte le dipendenze sono soddisfatte..."
echo -e "Compilazione di: launcher.c\e[0m\e[3m"
gcc `pkg-config gtk+-2.0 --cflags` -Wall -o launcher src/launcher.c -lm `pkg-config gtk+-2.0 --libs`
echo -e "\e[3;1mCompilazione di: locale.c\e[0m\e[3m"
gcc `pkg-config gtk+-2.0 --cflags` -Wall -o briscola/locale src/locale.c -lm `pkg-config gtk+-2.0 --libs`
echo -e "\e[3;1mCompilazione di: client.c\e[0m\e[3m"
gcc `pkg-config gtk+-2.0 --cflags` -Wall -o briscola/client src/client.c -lm `pkg-config gtk+-2.0 --libs`
echo -e "\e[3;1mCompilazione di: popup.c\e[0m\e[3m"
gcc `pkg-config gtk+-2.0 --cflags` -Wall -o briscola/popup src/popup.c -lm `pkg-config gtk+-2.0 --libs`
echo -e "\e[3;1mCompilazione di: mainserver.c\e[0m\e[3m"
gcc `pkg-config gtk+-2.0 --cflags` -Wall -o briscola/mainserver src/mainserver.c -lm `pkg-config gtk+-2.0 --libs`
echo -e "\e[3;1mCompilazione di: gameserver.c\e[0m\e[3m"
gcc `pkg-config gtk+-2.0 --cflags` -Wall -o briscola/gameserver src/gameserver.c -lm `pkg-config gtk+-2.0 --libs`


echo -e "\e[3;1mCompilazione terminata.\e[0m"
