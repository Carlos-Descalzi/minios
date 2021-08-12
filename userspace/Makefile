include ./Make.defs

TOPTARGETS=all clean
SUBDIRS=bootstrap lib bin tcpip

.PHONY: $(TOPTARGETS) $(SUBDIRS)

all: $(SUBDIRS)

$(TOPTARGETS): $(SUBDIRS)

$(SUBDIRS):
	@echo "Entering $@"
	@$(MAKE) -C $@ $(MAKECMDGOALS)
