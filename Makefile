CFLAGS = -std=c++17 -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

SierpinskiTriangle: *.cpp *.hpp
	g++ $(CFLAGS) -o SierpinskiTriangle *.cpp $(LDFLAGS)

.PHONY: test clean

test: SierpinskiTriangle
	./SierpinskiTriangle

clean:
	rm -f SierpinskiTriangle
