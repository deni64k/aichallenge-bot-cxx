CC=g++
CXX=g++
CFLAGS=-O3 -funroll-loops -c -std=c++0x -DDEBUG
LDFLAGS=-O3 -lm
SOURCES=Bot.cpp MyBot.cpp State.cpp kdtree.c
OBJECTS=$(addsuffix .o, $(basename ${SOURCES}))
EXECUTABLE=MyBot

#Uncomment the following to enable debugging
#CFLAGS+=-g -DDEBUG # -DBOOST_ENABLE_ASSERT_HANDLER
CFLAGS+=-DBOOST_DISABLE_ASSERTS

all: $(EXECUTABLE)

.deps:
	$(CXX) $(CFLAGS) -E -MM *.cpp > $@

include .deps

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@

%.o: %.cpp
	$(CXX) $(CFLAGS) $< -o $@

clean:
	-rm -f ${EXECUTABLE} ${OBJECTS} *.d

zip:
	-rm MyBot.zip
	dir=$$(basename $$(pwd)); \
	(cd ..; \
	 zip -9 -i '*.h' -i '*.hpp' -i "$${dir}/Bot.cpp" -i "$${dir}/MyBot.cpp" -i "$${dir}/State.cpp" -i "$${dir}/kdtree.c" -i "$${dir}/Makefile" -r -9 "$${dir}/MyBot.zip" "$${dir}"; \
	 )

.PHONY: all clean zip

# ~/src/boost_1_52_0 $ dist/bin/bcp current_function scoped_ptr multi_array ~/src/aichallenge-bot/boost-1.52.0
