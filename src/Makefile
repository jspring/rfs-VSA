# Makefile
#
# Copyright (c) 2013   Regents of the University of California
#
include $(CAPATH_MK_DEFS)

PATH_LIBS += -lmxml

PATH_CFLAGS += -I../mxml-release-2.10 -I$(ATSC_INC_DIR) -Wall
PATH_LDFLAGS += -L./lnx
PATH_OBJS += $(OBJDIR)/resource.o $(OBJDIR)/nrutil2.o

#EXEC =  $(OBJDIR)/xml_parser $(OBJDIR)/sign_control $(OBJDIR)/clt_vars
EXEC =  $(OBJDIR)/xml_parser $(OBJDIR)/sign_control $(OBJDIR)/opt_vsa

all: 	$(OBJDIR) $(PATH_OBJS) $(EXEC)

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	\rm -f $(OBJDIR)/*.[oa] $(EXEC)
