dir := test

test += $(dir)/dstring-cat-heap-heap
test += $(dir)/dstring-cat-heap-heap-long
test += $(dir)/dstring-cat-stack-heap
test += $(dir)/dstring-cat-stack-stack
test += $(dir)/dstring-delete-heap
test += $(dir)/dstring-delete-static
test += $(dir)/dstring-empty
test += $(dir)/dstring-grow-heap-heap
test += $(dir)/dstring-grow-heap-none
test += $(dir)/dstring-grow-stack-none
test += $(dir)/dstring-grow-stack-heap
test += $(dir)/dstring-heap-to-heap-fail
test += $(dir)/dstring-malloc-fail
test += $(dir)/dstring-set
test += $(dir)/dstring-set-long
test += $(dir)/dstring-stack-to-heap-fail
test += $(dir)/dstring-use-after-clear

test += $(dir)/list-add-one
test += $(dir)/list-add-two
test += $(dir)/list-cat
test += $(dir)/list-cat-empty
test += $(dir)/list-check-empty
test += $(dir)/list-empty-iter
test += $(dir)/list-insert-three
test += $(dir)/list-iter
test += $(dir)/list-queue
test += $(dir)/list-stack-back
test += $(dir)/list-stack-front

testlib := $(dir)/testlib
LDFLAGS_test := -Wl,--wrap=malloc,--wrap=realloc,--wrap=mkdir,--wrap=open
LDFLAGS_test += -Wl,--wrap=write,--wrap=rename,--wrap=symlink,--wrap=unlink

cover += dstring list

