
all: simulavr

CSRC = dumpargs.cpp main.cpp gdbserver.cpp ../libsimulavr/ui.cpp ../libsimulavr/mysocket.cpp

LIBSIMDIR = ../libsim

CLIBSIMSRC =  adcpin.cpp    \
		attiny2313.cpp \
		flash.cpp        \
		hwstack.cpp    \
		rwmem.cpp \
		application.cpp      \
		attiny25_45_85.cpp  \
		flashprog.cpp    \
		hwuart.cpp     \
		specialmem.cpp\
		at4433.cpp           \
		avrdevice.cpp       \
		hardware.cpp     \
		hwusi.cpp      \
		spisink.cpp\
		at8515.cpp           \
		avrerror.cpp        \
		helper.cpp       \
		hwwado.cpp     \
		spisrc.cpp\
		at90canbase.cpp      \
		avrfactory.cpp      \
		hwacomp.cpp      \
		ioregs.cpp     \
		string2.cpp\
		atmega128.cpp        \
		avrmalloc.cpp       \
		hwad.cpp         \
		irqsystem.cpp  \
		systemclock.cpp\
		atmega1284abase.cpp  \
		avrreadelf.cpp      \
		hweeprom.cpp     \
		memory.cpp     \
		traceval.cpp\
		atmega16_32.cpp      \
		avrsignature.cpp    \
		hwpinchange.cpp  \
		net.cpp        \
		atmega2560base.cpp   \
		decoder.cpp         \
		hwport.cpp       \
		pin.cpp\
		atmega668base.cpp    \
		decoder_trace.cpp   \
		hwspi.cpp        \
		pinatport.cpp\
		atmega8.cpp          \
		externalirq.cpp     \
		hwsreg.cpp       \
		pinmon.cpp \
		hwtimer/timerprescaler.cpp \
		hwtimer/prescalermux.cpp\
		hwtimer/timerirq.cpp \
		hwtimer/icapturesrc.cpp \
        hwtimer/hwtimer.cpp
		

CFLAGS= -I./../include -I./../include/elfio -DHAVE_SYS_MINGW -g

OBJS=$(CSRC:.cpp=.o)

LIBSIMOBJS=$(addprefix $(LIBSIMDIR)/,$(CLIBSIMSRC:.cpp=.o))

OBJS+=$(LIBSIMOBJS)

%.o: %.cpp
	g++ $(CFLAGS) $< -c -o $@

simulavr: $(OBJS)
	g++ $(OBJS) -lWs2_32 -o simulavr
	
clean:
	rm -f $(OBJS)