dir := lib/test

test += $(dir)/lib-init
test += $(dir)/lib-init-exists
test += $(dir)/lib-init-malloc-fail
test += $(dir)/lib-open
test += $(dir)/lib-open-dstrcat-fail
test += $(dir)/lib-open-dstrcpy-fail
test += $(dir)/lib-open-malloc-fail

cover += lib/cobalt lib/fsvm
