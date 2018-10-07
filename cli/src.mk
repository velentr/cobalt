dir := cli

cli := cobalt

clirl := config_parse editstr_parse
climod := add argparse config editstr find gc help init main modify modules \
	show version $(clirl)

cliobj := $(addprefix $(dir)/,$(climod:=.o))
clidep := $(addprefix $(dir)/,$(climod:=.d))
clisrc := $(addprefix $(dir)/,$(clirl:=.c))

cgen += $(addprefix $(dir)/,$(clirl))
