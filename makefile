TEST_DIR = tests
CC = clang
INCLUDEDIR = include
CFLAGS = -I$(INCLUDEDIR)
all: musicbox
TESTLDFLAGS = -lmpv
PRODLDFLAGS = -lmpv -lwiringPi
PRODSRC=$(wildcard *.c)
NOTESTSRC= buttons.c speedloop.c gpio.c
TESTSRC= $(wildcard $(TEST_DIR)/*.c) $(filter-out $(NOTESTSRC), $(PRODSRC)) 


TESTOBJS = $(TESTSRC:.c=.o)
PRODOBJS = $(PRODSRC:.c=.o)


musicbox: $(PRODOBJS)
	$(CC) $(PRODOBJS) -o musicbox $(PRODLDFLAGS) 


test: $(TESTOBJS)
	$(CC) $(TESTOBJS) -o test_musicbox $(TESTLDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
.PHONY: clean
clean:
	rm *.o test_musicbox musicbox tests/*.o
