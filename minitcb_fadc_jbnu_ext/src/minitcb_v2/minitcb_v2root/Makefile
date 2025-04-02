ROOTCINT = rootcling
NOTICE_LIB = $(NKHOME)/lib
NOTICE_INC = $(NKHOME)/include

INSTALL_LIB = $(NKHOME)/lib
INSTALL_INC = $(NKHOME)/include

RC     := root-config
ifeq ($(shell which $(RC) 2>&1 | sed -ne "s@.*/$(RC)@$(RC)@p"),$(RC))
MKARCH := $(wildcard $(shell $(RC) --etcdir)/Makefile.arch)
endif
ifneq ($(MKARCH),)
include $(MKARCH)
else
ROOTSYS = ..
include $(ROOTSYS)/etc/Makefile.arch
endif
-include ../MyConfig.mk

#------------------------------------------------------------------------------

SrcSuf = cc
HdrSuf = h
ObjSuf = o
DllSuf = so
OutPutOpt = -o #keep the space after the option "-o"

CXXFLAGS += -I. -I$(LIBUSB_INC) -I$(NOTICE_INC)
ifneq ($(DEBUG),)
CXXFLAGS += -g
endif

#LDFLAGS += -L$(LIB_nkusb) -lnkusb -shared -Wall,-soname
SOFLAGS += -Wall
LDFLAGS = $(SOFLAGS)
#LDLIBS =  -L$(LIBUSB_LIB) -lusb-1.0 -L$(NOTICE_LIB)
LDLIBS =  -L$(NOTICE_LIB) -lNoticeMINITCB_V2

SOURCE = NoticeMINITCB_V2ROOT.$(SrcSuf) NoticeMINITCB_V2ROOTDict.$(SrcSuf)
INCLUDE = NoticeMINITCB_V2ROOT.$(HdrSuf) LinkDef.h
OBJECT = $(SOURCE:.cc=.o)
LIBRARY = libNoticeMINITCB_V2ROOT.$(DllSuf)

all: $(LIBRARY)
clean:
	@rm -rf $(LIBRARY) $(OBJECT) *Dict.$(SrcSuf) *Dict.$(HdrSuf)

install: $(LIBRARY) NoticeMINITCB_V2ROOT.h
	#/bin/rm -f $(INSTALL_LIB)/$<
	#/bin/rm -f $(INSTALL_INC)/NoticeMINITCB_V2ROOT.h
	install *.pcm $(INSTALL_LIB)
	install $< $(INSTALL_LIB)
	install NoticeMINITCB_V2ROOT.h $(INSTALL_INC)

$(LIBRARY): $(OBJECT)
	$(LD) $(LDFLAGS) $^ $(LDLIBS) $(OutPutOpt) $@

NoticeMINITCB_V2ROOTDict.$(SrcSuf): $(INCLUDE) LinkDef.h
	@echo "Generating dictionary $@..."
	$(ROOTCINT) -f $@ -c -I$(NOTICE_INC) $^

#dependicies
$(OBJECT):$(INCLUDE)
