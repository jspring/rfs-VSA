# Makefile
#
# Copyright (c) 2013   Regents of the University of California
#
include $(CAPATH_MK_DEFS)

PATH_LIBS += -lmxml

PATH_LDFLAGS += -L./lnx

#EXEC =  $(OBJDIR)/xml_parser $(OBJDIR)/sign_control $(OBJDIR)/clt_vars
EXEC =  $(OBJDIR)/xml_parser $(OBJDIR)/sign_control

all: 	$(OBJDIR) $(PATH_OBJS) $(EXEC)

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	\rm -f $(OBJDIR)/*.[oa] $(EXEC)
