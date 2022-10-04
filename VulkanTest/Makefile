include .env

CFLAGS = -std=c++17 -I. -I$(VULKAN_INCLUDE_PATH) -I$(TINYOBJ_PATH)
LDFLAGS = `pkg-config --static --libs glfw3` -lvulkan 

# spv files
vertSources = $(shell find ./shaders -type f -name "*.vert")
fragSources = $(shell find ./shaders -type f -name "*.frag")
vertObjFiles = $(patsubst %.vert, %.vert.spv, $(vertSources))
fragObjFiles = $(patsubst %.frag, %.frag.spv, $(fragSources))


TARGET = a.out
$(TARGET): $(vertObjFiles) $(fragObjFiles)
$(TARGET): *.cpp *.hpp
	g++ $(CFLAGS) -o $(TARGET) *.cpp $(LDFLAGS)

# shader target
%.spv: %
	$(GLSLC) $< -o $@

.PHONY: test clean

test: a.out
	./a.out

clean:
	rm -f a.out
	rm -f *.spv
