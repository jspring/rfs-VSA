# Makefile
#
# Copyright (c) 2013   Regents of the University of California
#
include $(CAPATH_MK_DEFS)

PATH_LIBS += -lmxml

PATH_CFLAGS += -I../mxml-release-2.10 -I$(ATSC_INC_DIR) -Wall
PATH_LDFLAGS += -L./lnx -L../mxml-release-2.10
PATH_OBJS += $(OBJDIR)/resource.o $(OBJDIR)/nrutil2.o $(OBJDIR)/sign_lib.o
DATADIR = Out_Data

EXEC = $(OBJDIR)/sign_control $(OBJDIR)/clt_vars  $(OBJDIR)/xml_parser $(OBJDIR)/radar_xml_parser $(OBJDIR)/opt_vsa 

all: 	$(DATADIR) $(OBJDIR) $(PATH_OBJS) $(EXEC)

$(DATADIR):
	mkdir -p $(DATADIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	\rm -f $(OBJDIR)/*.[oa] $(EXEC)
