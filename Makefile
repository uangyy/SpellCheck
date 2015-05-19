SRC_PATH := ./src
INC_PATH := ./include
BIN_PATH := ./bin

$(BIN_PATH)/main:$(wildcard $(SRC_PATH)/*.cc)
	g++ -g -o $@ $^ -I$(INC_PATH) -pthread
