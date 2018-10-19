dir := lib/test

test += $(dir)/lib-add-attr
test += $(dir)/lib-del-attr
test += $(dir)/lib-init
test += $(dir)/lib-init-exists
test += $(dir)/lib-init-malloc-fail
test += $(dir)/lib-invalid-board-dotfile
test += $(dir)/lib-invalid-board-empty
test += $(dir)/lib-invalid-board-slash
test += $(dir)/lib-open
test += $(dir)/lib-open-dstrcat-fail
test += $(dir)/lib-open-dstrcpy-fail
test += $(dir)/lib-open-malloc-fail
test += $(dir)/lib-query-id
test += $(dir)/lib-seed-rng
test += $(dir)/lib-seed-zero
test += $(dir)/lib-version

cover += lib/cobalt lib/fsvm lib/random
