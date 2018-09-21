dir := lib

lib := libcobalt.so

libmod := cobalt error fsvm gc random

libobj := $(addprefix $(dir)/,$(libmod:=.o))
libdep := $(addprefix $(dir)/,$(libmod:=.d))
