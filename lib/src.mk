dir := lib

lib := libcobalt.so

libmod := cobalt db error random

libobj := $(addprefix $(dir)/,$(libmod:=.o))
libdep := $(addprefix $(dir)/,$(libmod:=.d))
