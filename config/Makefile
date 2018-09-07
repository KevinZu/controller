#
include ../make.in
OBJDIR = $(shell pwd)/src
LIBDIR = ./lib
INC= -I./inc
INC += -I../inc
#INC+=-I../SDL/include
CFLAGS = -g -ldl -lm $(INC)
OUTPUT = $(LIBDIR)/libcfg.a
OBJS =  config.o inirw.o
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
