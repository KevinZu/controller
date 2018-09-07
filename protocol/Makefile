#
include ../make.in
OBJDIR = $(shell pwd)/src
LIBDIR = ./lib
INC= -I./inc
INC += -I../inc
INC += -I../tcp_comm/inc
INC+=-I../SDL/include
INC+=-I../SDL/src
INC+=-I../SDL/src/thread
INC+=-I../SDL/src/timer
INC+=-I../SDL/src/thread/pthread
INC += -I../config/inc
INC += -I../serial/inc
INC += -I../can/inc
INC+=-I../sys_sche/inc
CFLAGS = -g -ldl -lm $(INC)
OUTPUT = $(LIBDIR)/libproto.a
OBJS =  protocol.o esl_proto.o esl_attr.o round_robin_queue.o event.o func_hlist.o 
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
