SRCS=randomize.cpp text_symbols.cpp main.cpp minimize.cpp jumppatching.cpp

CPPFLAGS = -g

OBJS = ${SRCS:.cpp=.o}
	
#.o.cpp:
#	g++ -g -c $<

all: $(OBJS)
	g++ -o MarlinStaticProcessor *.o

clean:
	rm *.o
	
