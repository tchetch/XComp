CC=gcc
LD=$(CC)
RM=rm
CFLAGS=--std=c99 
LIBS=-lpthread
RM=rm

SRCS=sys.c xcomp.c xcomp_main.c
OBJS=${SRCS:.c=.o}
EXE=xcomp

.SUFFIXES:
.SUFFIXES: .o .c

.c.o :
	$(CC) $(CFLAGS) -c $<

$(EXE): $(OBJS)
	$(LD) $(LIBS) -o $@ $(OBJS)

all: $(EXE)

clean:
	-$(RM) -f $(EXE) $(OBJS)
