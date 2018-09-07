#
include ../make.in
OBJDIR = $(shell pwd)/src
LIBDIR = ./lib
INC= -I./inc
INC += -I../inc
INC += -I../protocol/inc
INC += -I../SDL/include
INC += -I../SDL/src
INC += -I../SDL/src/thread
INC += -I../SDL/src/timer
INC += -I../SDL/src/thread/pthread
INC += -I../tcp_comm/inc
INC += -I../serial/inc
INC += -I../can/inc
CFLAGS = -g -ldl -lm $(INC)
OUTPUT = $(LIBDIR)/libsche.a
OBJS =  sys_manager.o log_s.o independ_process.o
OBJS := $(OBJS:%.o=$(OBJDIR)/%.o)
#----------------------------------------
all: $(OUTPUT)

clean:
	@rm -f $(OBJDIR)/*.o $(OUTPUT)

distclean:
	@rm -f *~

#install:
#	@cp ./lib/libtel.a ../PLUM_APP/lib

#deinstall:
#	@rm -f ../PLUM_APP/lib/libtel.a
#----------------------------------------
$(OUTPUT): $(OBJS)
	@$(AR) cr $@ $(OBJS)

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INCS)
