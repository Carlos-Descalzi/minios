include ../Make.defs

TOPTARGETS=all clean
SUBDIRS=sys

.PHONY: $(TOPTARGETS) $(SUBDIRS)

all: $(SUBDIRS)

$(TOPTARGETS): $(SUBDIRS)

$(SUBDIRS):
	@echo "Entering $@"
	@$(MAKE) -C $@ $(MAKECMDGOALS)
