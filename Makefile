# Pixel Breakout - build with:  make        run with:  ./pixel_breakout
# Needs:  sudo apt install build-essential libglfw3-dev libgl-dev

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra
override CPPFLAGS += -Iinclude
override LDLIBS += -lglfw -lGL

SRC := $(wildcard src/*.cpp)
OBJ := $(SRC:src/%.cpp=build/%.o)

pixel_breakout: $(OBJ)
	$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)

build/%.o: src/%.cpp $(wildcard include/*.h) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

build:
	mkdir -p build

# Animation evidence: 150 frames (5 s at 30 fps) of the autopilot playing.
frames: pixel_breakout
	mkdir -p frames
	./pixel_breakout --frames frames 150

clean:
	rm -rf build pixel_breakout

.PHONY: frames clean
