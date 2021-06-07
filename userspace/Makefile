include ./Make.defs
include ../Make.rules

TOPTARGETS=all clean
SUBDIRS=bootstrap lib

.PHONY: $(TOPTARGETS) $(SUBDIRS)

#all: $(SUBDIRS)

$(TOPTARGETS): $(SUBDIRS)

$(SUBDIRS):
	@echo "Entering $@"
	@$(MAKE) -C $@ $(MAKECMDGOALS)
