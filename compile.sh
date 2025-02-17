export LD_LIBRARY_PATH=./lib
time g++ -o main main.cpp -I./include -L./lib -luvc -lpthread -lusb-1.0 -lraylib