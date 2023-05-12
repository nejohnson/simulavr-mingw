
all: simulavr

CSRC = dumpargs.cpp main.cpp gdbserver.cpp ./libsimulavr/ui.cpp ../libsimulavr/mysocket.cpp

CFLAGS= -I./../include -DHAVE_SYS_MINGW

OBJS=$(CSRC:.cpp=.o)

%.o: %.cpp
	g++ $(CFLAGS) $< -c -o $@

simulavr: $(OBJS)
	ld $(OBJS) -o simulavr
	
