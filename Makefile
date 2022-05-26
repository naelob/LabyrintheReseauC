# COLORS
_BLACK		= "\033[30m"
_RED		= "\033[31m"
_GREEN		= "\033[32m"
_YELLOW		= "\033[33m"
_BLUE		= "\033[34m"
_VIOLET		= "\033[35m"
_CYAN		= "\033[36m"
_WHITE		= "\033[37m"
_END		= "\033[0m"
_CLEAR		= "\033[2K"
_HIDE_CURS	= "\033[?25l"
_SHOW_CURS	= "\033[?25h"
_UP			= "\033[A"
_CUT		= "\033[k"

.PHONY: all
.SILENT:

all: serveur client
	echo $(_CLEAR)$(_YELLOW)"Compilation du serveur..."
	echo $(_GREEN)"Serveur pret !"
	echo $(_YELLOW)"Compilation du client ..."
	echo $(_GREEN)"Client pret !" 
	echo $(_WHITE)"./Serveur <port> puis java client <ip> <port> pour tester !"$(_END)

serveur : serverNew.c
	gcc -g -o Serveur serverNew.c -lpthread

client : Client.java
	javac *.java

clean : 
	echo $(_CYAN)"Effacement des executable ..."
	echo $(_GREEN)"ok"$(_END)
	rm -f ./client1 ./serveur ./client2 

re:
	$(MAKE) -s clean
	$(MAKE) -s