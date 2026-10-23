CXX      := g++
CXXFLAGS := -std=c++17 -g -Wall -Wextra -pthread -Iinclude -MMD -MP
LDFLAGS  := -pthread

BUILD := build

COMMON_SRC := src/protocol.cpp
SERVER_SRC := server/main.cpp src/server.cpp src/filesystem.cpp \
              src/session.cpp src/persistence.cpp $(COMMON_SRC)
CLIENT_SRC := client/main.cpp $(COMMON_SRC)

SERVER_OBJ := $(SERVER_SRC:%.cpp=$(BUILD)/%.o)
CLIENT_OBJ := $(CLIENT_SRC:%.cpp=$(BUILD)/%.o)
DEPS       := $(SERVER_OBJ:.o=.d) $(CLIENT_OBJ:.o=.d)

.PHONY: all clean

all: fsserver fsclient

fsserver: $(SERVER_OBJ)
	$(CXX) $^ -o $@ $(LDFLAGS)

fsclient: $(CLIENT_OBJ)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(BUILD)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD) fsserver fsclient


LIB_SRC  := src/server.cpp src/filesystem.cpp src/session.cpp \
            src/persistence.cpp src/protocol.cpp
LIB_OBJ  := $(LIB_SRC:%.cpp=$(BUILD)/%.o)
TEST_SRC := $(wildcard tests/*.cpp)
TEST_BIN := $(TEST_SRC:tests/%.cpp=$(BUILD)/tests/%)
DEPS     += $(TEST_SRC:%.cpp=$(BUILD)/%.d)

tests: $(TEST_BIN)

$(BUILD)/tests/%: $(BUILD)/tests/%.o $(LIB_OBJ)
	$(CXX) $^ -o $@ $(LDFLAGS)

-include $(DEPS)
