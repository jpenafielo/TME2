all: main

CXX = g++
override CXXFLAGS += -g -Wno-everything

ARCH = $(shell getconf LONG_BIT)
ifeq ($(ARCH), 32)
  LIBPATH = ../../libxl-4.1.2/lib
else
  LIBPATH = ../../libxl-4.1.2/lib64
endif

SRCS = $(shell find . -name '.ccls-cache' -type d -prune -o -type f -name '*.cpp' -print | sed -e 's/ /\\ /g')
HEADERS = $(shell find . -name '.ccls-cache' -type d -prune -o -type f -name '*.h' -print)
INCLUDES = -I "./../../libxl-4.1.2/include_cpp/"

main: $(SRCS) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRCS) ${INCLUDES} -L ${LIBPATH} -lxl -Wl,-rpath,$(LIBPATH) ${LINK} -o "$@"

main-debug: $(SRCS) $(HEADERS)
	$(CXX) $(CXXFLAGS) -O0 $(SRCS) -o "$@"

clean:
	rm -f main main-debug