dir := cli

cli := cobalt

clirl := editstr_parse
climod := add argparse editstr find gc help init main modify modules show \
	version $(clirl)

cliobj := $(addprefix $(dir)/,$(climod:=.o))
clidep := $(addprefix $(dir)/,$(climod:=.d))
clisrc := $(addprefix $(dir)/,$(clirl:=.c))

cgen += $(addprefix $(dir)/,$(clirl))
