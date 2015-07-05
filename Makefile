all: main.cc connector.h fase1.cpp
	clang++ main.cc -o g1 -std=c++14 -g -Wall -Wextra

connector.h:
	git clone https://github.com/snipstruis/strats-connector
	cp strats-connector/connector.h .
	rm strats-connector -Rf
