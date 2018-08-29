CC ?= gcc
CFLAGS += -fpic -Wall -Wextra -std=gnu99 -Iinclude -I. -Ilib -fvisibility=hidden
DFLAGS += -MMD

LDFLAGS += -L.

RL ?= ragel
RLFLAGS += -C

A2X ?= a2x
DOCFLAGS += -f manpage -d manpage -L

GCOV ?= gcov

# shared variables need immediate assignment since $(dir) will change between
# directories
cgen :=
test :=
cover :=
doc :=

sub := cli doc lib lib/test test

include $(sub:=/src.mk)
include src.mk

testobj := $(test:=.o) $(testlib:=.o)
testdep := $(test:=.d)
testbin := $(test:=.test)

# turn off warnings for generated C files
$(foreach R,$(cgen),$(eval CFLAGS_$R := -w))

all: $(cli) $(lib)

# turn on valgrind only when specified
ifdef VALGRIND
vlgnd := --valgrind
endif

test: $(testbin)
	./script/testbench.pl $(vlgnd) $(testbin)

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

doc: $(doc)

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

clean:
	@echo cleaning...
	rm -f $(cliobj) $(clidep) $(clisrc) $(cli) $(lib) $(libobj) $(libdep) \
		$(obj) $(dep) $(testobj) $(testdep) $(testbin) $(doc)
	find -type f \( -name '*.gcda' -o -name '*.gcno' -o -name '*.gcov' \) \
		-exec rm {} +

# need the second expansion here to reference $@ in the prerequisite list
.SECONDEXPANSION:
$(doc): $$@.txt $(docconf)
	@echo "DOC	$@"
	$(A2X) $(DOCFLAGS) $<

.PHONY: all clean cover doc test

# disable automatic rules
.SUFFIXES:

.PRECIOUS: %.o %.c

# print jobs if V is defined to any value
$(V).SILENT:
