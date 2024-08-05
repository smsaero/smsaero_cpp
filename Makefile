CXX = g++
CFLAGS = -Wall -std=c++11
INCLUDE = -Iinclude -I/usr/local/include -I/usr/include -I/opt/homebrew/include
LDFLAGS = -L/usr/local/lib -L/opt/homebrew/lib -I/usr/lib
LDLIBS = -lcurl

SOURCES = src/demo.cc src/smsaero.cc
OUT = bin/demo

all: clean build

build: $(SOURCES)
	@$(CXX) -o $(OUT) $(INCLUDE) $(LDFLAGS) $(CFLAGS) $(SOURCES) $(LDLIBS)

clean:
	@rm -rf ./bin/demo

docker-build-and-push:
	@docker buildx create --name smsaero_cpp --use || docker buildx use smsaero_cpp
	@docker buildx build --platform linux/amd64,linux/arm64 -t 'smsaero/smsaero_cpp:latest' . -f Dockerfile --push
	@docker buildx rm smsaero_cpp
