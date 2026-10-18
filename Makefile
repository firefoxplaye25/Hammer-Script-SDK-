CXX ?= C:/msys64/ucrt64/bin/g++.exe
CXXFLAGS ?= -std=c++17 -O3 -Wall -static -Iinclude

SRCS = src/token.cpp \
       src/lexer.cpp \
       src/diagnostics.cpp \
       src/value.cpp \
       src/environment.cpp \
       src/stdlib.cpp \
       src/hamfs.cpp \
       src/asock.cpp \
       src/hmripa.cpp \
       src/parser.cpp \
       src/interpreter.cpp \
       src/compiler.cpp \
       src/module_manager.cpp \
       src/modules/hamhook.cpp \
       src/modules/hamc.cpp \
       src/main.cpp

TARGET = bin/hammer.exe

all: $(TARGET)

$(TARGET): $(SRCS)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(SRCS) -lwininet -lws2_32 -o $(TARGET)
	@echo "Built Hammer Script successfully: $(TARGET)"

clean:
	rm -rf bin/*.exe bin/*.o

.PHONY: all clean
