SHELL := /bin/bash

CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Wpedantic -I. -Igenerated -Igenerated/cliente-side -Igenerated/server-side -Icommon
LDFLAGS :=
LDLIBS := -lprotobuf -lpthread

PROTO_DIR := proto
GEN_DIR := generated

CLIENT_PROTO_DIR := $(PROTO_DIR)/cliente-side
SERVER_PROTO_DIR := $(PROTO_DIR)/server-side

PROTO_SRCS := \
  $(PROTO_DIR)/common.proto \
  $(CLIENT_PROTO_DIR)/register.proto \
  $(CLIENT_PROTO_DIR)/message_general.proto \
  $(CLIENT_PROTO_DIR)/message_dm.proto \
  $(CLIENT_PROTO_DIR)/change_status.proto \
  $(CLIENT_PROTO_DIR)/list_users.proto \
  $(CLIENT_PROTO_DIR)/get_user_info.proto \
  $(CLIENT_PROTO_DIR)/quit.proto \
  $(SERVER_PROTO_DIR)/all_users.proto \
  $(SERVER_PROTO_DIR)/for_dm.proto \
  $(SERVER_PROTO_DIR)/broadcast_messages.proto \
  $(SERVER_PROTO_DIR)/get_user_info_response.proto \
  $(SERVER_PROTO_DIR)/server_response.proto

PB_CC := $(patsubst $(PROTO_DIR)/%.proto,$(GEN_DIR)/%.pb.cc,$(PROTO_SRCS))
PB_O  := $(PB_CC:.cc=.o)

COMMON_SRCS := common/framing.cpp common/socket_utils.cpp common/protocol_io.cpp common/status_mapper.cpp
COMMON_O := $(COMMON_SRCS:.cpp=.o)

SERVER_SRCS := server/main.cpp server/chat_server.cpp server/user_registry.cpp server/session_handler.cpp
SERVER_O := $(SERVER_SRCS:.cpp=.o)

CLIENT_SRCS := client/main.cpp client/client_app.cpp client/input_loop.cpp client/receiver_loop.cpp
CLIENT_O := $(CLIENT_SRCS:.cpp=.o)

.PHONY: all clean protos server client

all: server client

$(GEN_DIR):
	mkdir -p "$(GEN_DIR)"

$(GEN_DIR)/%.pb.cc $(GEN_DIR)/%.pb.h: $(PROTO_DIR)/%.proto | $(GEN_DIR)
	mkdir -p "$(dir $@)"
	protoc -I "$(PROTO_DIR)" --cpp_out="$(GEN_DIR)" "$<"

protos: $(PB_CC)

server: protos $(PB_O) $(COMMON_O) $(SERVER_O)
	$(CXX) $(CXXFLAGS) -o server/server $(PB_O) $(COMMON_O) $(SERVER_O) $(LDFLAGS) $(LDLIBS)

client: protos $(PB_O) $(COMMON_O) $(CLIENT_O)
	$(CXX) $(CXXFLAGS) -o client/client $(PB_O) $(COMMON_O) $(CLIENT_O) $(LDFLAGS) $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o "$@" "$<"

clean:
	rm -rf "$(GEN_DIR)" common/*.o server/*.o client/*.o server/server client/client

