dir := lib

lib := libcobalt.so

libmod := cobalt db error fsvm gc random

libobj := $(addprefix $(dir)/,$(libmod:=.o))
libdep := $(addprefix $(dir)/,$(libmod:=.d))
