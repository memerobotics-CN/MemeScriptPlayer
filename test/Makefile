CC = gcc
CFLAGS = -Wall -O3
LFLAGS = 

INCLUDES = -I. -I.. -I../user
LIBS =

SRCS = ScriptTest.c\
       ../MemeServoAPI/MemeServoAPI.c\
       ../user/ScriptProcessor.c

OBJS = $(addsuffix .o, $(basename $(SRCS)))

%.o: %.c
	$(CC) -c $(INCLUDES) -o $@ $< $(CFLAGS)

ScriptTest: $(OBJS)
	$(CC) -o $@ $^ $(LFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f ScriptTest *.o $(OBJS) *~
