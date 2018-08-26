dir := lib/test

test += $(dir)/op-abort
test += $(dir)/op-dir-create
test += $(dir)/op-dir-create-fail
test += $(dir)/op-dir-create-revert
test += $(dir)/op-dir-exist
test += $(dir)/op-dir-exist-fail
test += $(dir)/op-dir-exist-pre
test += $(dir)/op-dir-exist-revert
test += $(dir)/op-file-create
test += $(dir)/op-file-create-open-fail
test += $(dir)/op-file-create-revert
test += $(dir)/op-file-create-write-fail
test += $(dir)/op-file-create-write-partial
test += $(dir)/op-file-rename
test += $(dir)/op-file-rename-fail
test += $(dir)/op-nop
test += $(dir)/op-str-cat-fail
test += $(dir)/op-str-cat-revert
test += $(dir)/op-str-copy
test += $(dir)/op-str-copy-fail
test += $(dir)/op-str-copy-revert
test += $(dir)/op-sym-create
test += $(dir)/op-sym-create-fail
test += $(dir)/op-sym-create-revert
test += $(dir)/op-sym-delete
test += $(dir)/op-sym-delete-fail
test += $(dir)/op-sym-delete-revert

cover += lib/db

