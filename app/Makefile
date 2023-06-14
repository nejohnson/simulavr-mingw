##############################################################################
##
##  Makefile for Simulavr under Mingw (tested on mingw64)
##
##  Copyright (C) 2023 Neil Johnson
##
##############################################################################

LIBSIMDIR = ../libsim

CLIBSIMSRC =  adcpin.cpp    \
		flash.cpp        \
		hwstack.cpp    \
		rwmem.cpp \
		application.cpp      \
		flashprog.cpp    \
		hwuart.cpp     \
		specialmem.cpp\
		avrdevice.cpp       \
		hardware.cpp     \
		hwusi.cpp      \
		spisink.cpp\
		avrerror.cpp        \
		helper.cpp       \
		hwwado.cpp     \
		spisrc.cpp\
		avrfactory.cpp      \
		hwacomp.cpp      \
		ioregs.cpp     \
		string2.cpp\
		avrmalloc.cpp       \
		hwad.cpp         \
		irqsystem.cpp  \
		systemclock.cpp\
		avrreadelf.cpp      \
		hweeprom.cpp     \
		memory.cpp     \
		traceval.cpp\
		avrsignature.cpp    \
		hwpinchange.cpp  \
		net.cpp        \
		decoder.cpp         \
		hwport.cpp       \
		pin.cpp\
		decoder_trace.cpp   \
		hwspi.cpp        \
		pinatport.cpp\
		externalirq.cpp     \
		hwsreg.cpp       \
		pinmon.cpp \
		hwtimer/timerprescaler.cpp \
		hwtimer/prescalermux.cpp\
		hwtimer/timerirq.cpp \
		hwtimer/icapturesrc.cpp \
        hwtimer/hwtimer.cpp
		
CLIBSIMSRC +=  attiny2313.cpp \
		attiny25_45_85.cpp  \
		at4433.cpp           \
		at8515.cpp           \
		at90canbase.cpp      \
		atmega128.cpp        \
		atmega1284abase.cpp  \
		atmega16_32.cpp      \
		atmega2560base.cpp   \
		atmega668base.cpp    \
		atmega8.cpp          \
		
LIBSIMOBJS = $(addprefix $(LIBSIMDIR)/,$(CLIBSIMSRC:.cpp=.o))

##############################################################################

CSRC = dumpargs.cpp main.cpp gdbserver.cpp \
		../libsimulavr/ui.cpp \
		../libsimulavr/mysocket.cpp

CFLAGS = -I./../include -I./../include/elfio -g

# Add platform-specific network module - manually edit for now...

ifneq ($(strip $(MSYSTEM)),)
	# MinGW/WIn32
	CSRC    += gdbserverwin32.cpp
	CFLAGS  += -DHAVE_SYS_MINGW
	LDFLAGS += -lWs2_32 
else
	# Unix
	CSRC    += gdbserverunix.cpp
endif

##############################################################################

OBJS   = $(CSRC:.cpp=.o)
OBJS  += $(LIBSIMOBJS)

##############################################################################

all: simulavr

%.o: %.cpp
	g++ $(CFLAGS) $< -c -o $@

simulavr: $(OBJS)
	g++ $(OBJS) $(LDFLAGS) -o simulavr

clean:
	rm -f $(OBJS)

##############################################################################
##############################################################################
