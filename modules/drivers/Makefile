include ../Make.defs

TOPTARGETS=all clean
SUBDIRS=serial keyboard screen console net sys mouse

.PHONY: $(TOPTARGETS) $(SUBDIRS)

all: $(SUBDIRS)

$(TOPTARGETS): $(SUBDIRS)

$(SUBDIRS):
	@echo "Entering $@"
	@$(MAKE) -C $@ $(MAKECMDGOALS)
