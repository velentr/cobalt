dir := cli

cli := cobalt

clirl := add_parse editstr_parse help_parse init_parse modify_parse \
	show_parse version_parse
climod := add editstr help init main modify modules show version $(clirl)

cliobj := $(addprefix $(dir)/,$(climod:=.o))
clidep := $(addprefix $(dir)/,$(climod:=.d))
clisrc := $(addprefix $(dir)/,$(clirl:=.c))

cgen += $(addprefix $(dir)/,$(clirl))
