all:
	g++ ./code/json.cpp ./code/telegram_bot.cpp -o telegram_bot --std=c++17 -I/usr/local/include -lTgBot -lboost_system -lssl -lcrypto -lpthread -lcurl
	./telegram_bot