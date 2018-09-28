CC ?= gcc
CFLAGS += -fpic -Wall -Wextra -std=gnu99 -Iinclude -I. -Ilib -fvisibility=hidden
DFLAGS += -MMD

LDFLAGS += -L.

RL ?= ragel
RLFLAGS += -C

A2X ?= a2x
DOCFLAGS += -f manpage -d manpage -L

GCOV ?= gcov

INSTALL ?= install
INSTALL_PROGRAM := $(INSTALL)
INSTALL_DATA := $(INSTALL) -m 644
INSTALL_DIR := $(INSTALL) -d
prefix ?= /usr/local
libdir ?= $(DESTDIR)$(prefix)/lib
bindir ?= $(DESTDIR)$(prefix)/bin
includedir ?= $(DESTDIR)$(prefix)/include/cobalt

# shared variables need immediate assignment since $(dir) will change between
# directories
cgen :=
test :=
cover :=
pubdoc :=
prvdoc :=

sub := cli doc lib lib/test test

include $(sub:=/src.mk)
include src.mk

testobj := $(test:=.o) $(testlib:=.o)
testdep := $(test:=.d) $(testlib:=.d)
testbin := $(test:=.test)
testres := $(test:=.result)

# turn off warnings for generated C files
$(foreach R,$(cgen),$(eval CFLAGS_$R := -w))

all: $(cli) $(lib) $(pubdoc)
	@echo finished building cobalt

install: $(cli) $(lib)
	$(INSTALL_DIR) $(bindir) $(libdir) $(includedir)
	$(INSTALL_PROGRAM) $(cli) $(bindir)/$(cli)
	$(INSTALL_PROGRAM) $(lib) $(libdir)/$(lib)
	$(INSTALL_DATA) include/cobalt/* $(includedir)/

uninstall:
	rm -f $(bindir)/$(cli) $(libdir)/$(lib) $(includedir)/*

# turn on valgrind only when specified
ifdef VALGRIND
vlgnd := -v
endif

test: $(testres)
	@echo all tests passed

retest: clean-test test

# add gcov CFLAGS to each tested module when the cover target is run
define COV
cover: CFLAGS_$S := -ftest-coverage -fprofile-arcs
endef
$(foreach S, $(cover),$(eval $(COV)))
# set LDFLAGS for coverage analysis
cover: LDFLAGS += -lgcov
# do clean before running coverage to ensure the proper CFLAGS and active when
# the objects are built
cover: clean test
	$(GCOV) $(cover)

doc: $(prvdoc) $(pubdoc)

-include $(dep) $(clidep) $(libdep) $(testdep)

$(cli): $(cliobj) $(obj) | $(lib)
	@echo "LD	$@"
	$(CC) $^ -lcobalt -o $@ $(LDFLAGS)

$(lib): $(libobj) $(obj)
	@echo "LIB	$@"
	$(CC) -shared $^ -o $@

%.c: %.rl
	@echo "RL	$*"
	$(RL) $(RLFLAGS) $< -o $@

%.o: %.c
	@echo "CC	$*"
	$(CC) $(CFLAGS) $(DFLAGS) $(CFLAGS_$*) -c $< -o $@

%.s: %.c
	@echo "ASM	$*"
	$(CC) $(CFLAGS) $(CFLAGS_$*) -S $< -o $@

%.test: %.o $(testlib:=.o) $(obj) $(libobj)
	@echo "LD	$*"
	$(CC) $^ -o $@ $(LDFLAGS_test) $(LDFLAGS)

%.result: %.test script/cotest
	@echo "TEST	$*"
	script/cotest -f .fixture/$* -r $(vlgnd) -o $@ $<

clean-test:
	rm -f $(testres)

clean: clean-test
	@echo cleaning...
	rm -f $(cliobj) $(clidep) $(clisrc) $(cli) $(lib) $(libobj) $(libdep) \
		$(obj) $(dep) $(testobj) $(testdep) $(testbin) \
	rm -f doc/*.[1-9]
	find -type f \( -name '*.gcda' -o -name '*.gcno' -o -name '*.gcov' \) \
		-exec rm {} +

# need the second expansion here to reference $@ in the prerequisite list
.SECONDEXPANSION:
$(pubdoc): $$@.txt $(docconf)
	@echo "DOC	$@"
	$(A2X) $(DOCFLAGS) $<

$(prvdoc): $$@.txt $(docconf)
	@echo "DOC*	$@"
	$(A2X) $(DOCFLAGS) $<

.PHONY: all clean clean-test cover doc install retest test uninstall

# disable automatic rules
.SUFFIXES:

.PRECIOUS: %.o %.c %.test

# print jobs if V is defined to any value
$(V).SILENT:
