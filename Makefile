COMPILER=g++

CPPFLAGS=--std=c++17
tgflags=-I/usr/local/include -lTgBot -lboost_system -lssl -lcrypto -lpthread
mybotflags=

SOURCES=./code/telegram_bot.cpp
EXECUTABLE=telegram_bot

all:
	$(COMPILER) $(SOURCES) -o $(EXECUTABLE) $(CPPFLAGS) $(tgflags) $(mybotflags)
	./$(EXECUTABLE)