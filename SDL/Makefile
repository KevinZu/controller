#
include ../make.in
OBJDIR1 = $(shell pwd)/src
OBJDIR2 = $(shell pwd)/src/timer
OBJDIR3 = $(shell pwd)/src/timer/unix
OBJDIR4 = $(shell pwd)/src/thread
OBJDIR5 = $(shell pwd)/src/thread/pthread
OBJDIR6 = $(shell pwd)/src/net

LIBDIR = ./lib
INC= -I./include
INC+=-I$(shell pwd)/src -I$(shell pwd)/src/timer -I$(shell pwd)/src/thread -I$(shell pwd)/src/thread/pthread

CFLAGS = -g -ldl -lm $(INC)
OUTPUT = $(LIBDIR)/libsdl.a

OBJS1 = SDL_error.o

OBJS2 = SDL_timer.o

OBJS3 =  SDL_systimer.o

OBJS4 = SDL_thread.o

OBJS5 = SDL_syscond.o SDL_sysmutex.o SDL_syssem.o SDL_systhread.o

OBJS6 = SDLnet.o SDLnetselect.o SDLnetTCP.o SDLnetUDP.o

OBJS1 := $(OBJS1:%.o=$(OBJDIR1)/%.o)

OBJS2 := $(OBJS2:%.o=$(OBJDIR2)/%.o)

OBJS3 := $(OBJS3:%.o=$(OBJDIR3)/%.o)

OBJS4 := $(OBJS4:%.o=$(OBJDIR4)/%.o)

OBJS5 := $(OBJS5:%.o=$(OBJDIR5)/%.o)

OBJS6 := $(OBJS6:%.o=$(OBJDIR6)/%.o)

#----------------------------------------
all: $(OUTPUT)

clean:
	@rm -f $(OUTPUT)
	@rm -f $(OBJS1)
	@rm -f $(OBJS2)
	@rm -f $(OBJS3)
	@rm -f $(OBJS4)
	@rm -f $(OBJS5)
	@rm -f $(OBJS6)

distclean:
	@rm -f *~

#----------------------------------------
$(OUTPUT): $(OBJS1) $(OBJS2) $(OBJS3) $(OBJS4) $(OBJS5) $(OBJS6)
	@$(AR) cr $@ $(OBJS1) $(OBJS2) $(OBJS3) $(OBJS4) $(OBJS5) $(OBJS6)

$(OBJDIR1)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INCS)

$(OBJDIR2)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INCS)

$(OBJDIR3)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INCS)

$(OBJDIR4)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INCS)

$(OBJDIR5)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INCS)

$(OBJDIR6)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INCS)



