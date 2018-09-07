#
include ../make.in
OBJDIR = $(shell pwd)/src
LIBDIR = ./lib
INC= -I./inc
INC += -I../inc
INC += -I../sys_sche/inc
INC += -I../serial/inc
INC += -I../protocol/inc
INC += -I../SDL/include
INC += -I../SDL/src
INC += -I../SDL/src/thread
INC += -I../SDL/src/timer
INC += -I../SDL/src/thread/pthread
INC += -I../can/inc
CFLAGS = -g -ldl -lm $(INC)
OUTPUT = $(LIBDIR)/libtcp.a
OBJS =  tcp_comm.o sub_socket_comm.o cmd_handler.o
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
