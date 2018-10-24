/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <cobalt/cobalt.h>

#include "fsvm.h"
#include "list.h"

static struct fsvm_glob *fsvm_globalloc(struct fsvm *vm)
{
	struct fsvm_glob *g;
	size_t i;

	g = malloc(sizeof(*g));
	if (g == NULL)
		return g;

	for (i = 0; i < lengthof(g->reg); i++)
		dstrempty(&g->reg[i]);

	g->fd = -1;

	list_pushfront(&vm->globs, &g->le);

	return g;
}

void fsvm_globfree(struct fsvm_glob *glob)
{
	size_t i;

	assert(glob != NULL);

	for (i = 0; i < lengthof(glob->reg); i++)
		dstrclr(&glob->reg[i]);

	if (glob->fd != -1)
		close(glob->fd);

	free(glob);
}

static int fsvm_globname(struct fsvm *vm, struct fsvm_glob *g, const char *name,
		size_t len)
{
	assert(vm != NULL);
	assert(vm->index != NULL);
	assert(g != NULL);
	assert(g->fd == -1);
	assert(len < lengthof(g->name));

	memcpy(g->name, name, len);
	g->name[len] = '\0';

	assert(strlen(g->name) == len);
	assert(strcmp(g->name, name) == 0);

	g->fd = openat(dirfd(vm->index), g->name, O_RDONLY);
	if (g->fd == -1)
		return errno;
	else
		return CO_ENOERR;
}

static void fsvm_clearglobs(struct fsvm *vm)
{
	struct fsvm_glob *g;

	while (!list_isempty(&vm->globs)) {
		g = to_fsvm_glob(list_popfront(&vm->globs));
		fsvm_globfree(g);
	}

	assert(list_isempty(&vm->globs));
}

void fsvm_init(struct fsvm *vm)
{
	size_t i;

	assert(vm != NULL);

	/* registers are initialized to be empty */
	for (i = 0; i < FSVM_NUM_REGS; i++) {
		dstrempty(&vm->reg[i]);
		assert(dstrlen(&vm->reg[i]) == 0);
	}

	vm->index = NULL;
	list_init(&vm->globs);

	assert(list_isempty(&vm->globs));

	/* note that arguments are explicitly not initialized */
}

void fsvm_clear(struct fsvm *vm)
{
	size_t i;

	assert(vm != NULL);

	for (i = 0; i < FSVM_NUM_REGS; i++)
		dstrclr(&vm->reg[i]);

	if (vm->index != NULL)
		closedir(vm->index);

	fsvm_clearglobs(vm);
	assert(list_isempty(&vm->globs));
}

void fsvm_globs(struct fsvm *vm, struct list *globs)
{
	assert(vm != NULL);
	assert(globs != NULL);

	list_cat(globs, &vm->globs);
	list_init(&vm->globs);

	assert(list_isempty(&vm->globs));
}

void fsvm_ldarg(struct fsvm *vm, size_t argno, const char *argv, size_t arglen)
{
	assert(vm != NULL);
	assert(argno < FSVM_NUM_ARGS);
	assert(argv != NULL);

	vm->arg[argno].data = argv;
	vm->arg[argno].len  = arglen;
}

static int fsvm_run_dex(struct fsvm *vm, uint8_t path)
{
	int rc;

	assert(vm != NULL);
	assert(path < FSVM_NUM_REGS);

	rc = mkdir(dstr(&vm->reg[path]), 0755);
	if (rc == -1 && errno != EEXIST)
		return errno;
	else
		return CO_ENOERR;
}

static int fsvm_run_dmk(struct fsvm *vm, uint8_t path)
{
	int rc;

	assert(vm != NULL);
	assert(path < FSVM_NUM_REGS);

	rc = mkdir(dstr(&vm->reg[path]), 0755);
	if (rc == -1)
		return errno;
	else
		return CO_ENOERR;
}

static void fsvm_revert_dmk(struct fsvm *vm, uint8_t path)
{
	assert(vm != NULL);
	assert(path < FSVM_NUM_REGS);

	/* even if an error happens here, we can't respond to it */
	(void)rmdir(dstr(&vm->reg[path]));
}

static int fsvm_run_fmk(struct fsvm *vm, uint8_t path, uint8_t contents)
{
	ssize_t rc;
	int fd;
	int ret;

	assert(vm != NULL);
	assert(path < FSVM_NUM_REGS);
	assert(contents < FSVM_NUM_ARGS);

	fd = open(dstr(&vm->reg[path]), O_CREAT | O_EXCL | O_WRONLY, 0644);
	if (fd == -1)
		return errno;

	rc = write(fd, vm->arg[contents].data, vm->arg[contents].len);
	if (rc == -1)
		ret = errno;
	else if ((size_t)rc != vm->arg[contents].len)
		ret = CO_EINCOMPLETE;
	else
		ret = CO_ENOERR;

	close(fd);
	if (ret != CO_ENOERR)
		(void)unlink(dstr(&vm->reg[path]));

	return ret;
}

static void fsvm_revert_fmk(struct fsvm *vm, uint8_t path)
{
	assert(vm != NULL);
	assert(path < FSVM_NUM_REGS);

	(void)unlink(dstr(&vm->reg[path]));
}

static int fsvm_run_fmv(struct fsvm *vm, uint8_t old, uint8_t new)
{
	const char *oldpath;
	const char *newpath;
	int rc;

	assert(vm != NULL);
	assert(old < FSVM_NUM_REGS);
	assert(new < FSVM_NUM_REGS);

	oldpath = dstr(&vm->reg[old]);
	newpath = dstr(&vm->reg[new]);

	rc = rename(oldpath, newpath);
	if (rc == -1)
		return errno;
	else
		return CO_ENOERR;
}

static int fsvm_run_frm(struct fsvm *vm, uint8_t file)
{
	const char *filename;
	int rc;

	assert(vm != NULL);
	assert(file < FSVM_NUM_REGS);

	filename = dstr(&vm->reg[file]);
	rc = unlink(filename);
	if (rc == -1)
		return errno;
	else
		return CO_ENOERR;
}

static int fsvm_run_lnk(struct fsvm *vm, uint8_t targ, uint8_t name)
{
	const char *target;
	const char *linkname;
	int rc;

	assert(vm != NULL);
	assert(targ < FSVM_NUM_REGS);
	assert(name < FSVM_NUM_REGS);

	target = dstr(&vm->reg[targ]);
	linkname = dstr(&vm->reg[name]);

	rc = symlink(target, linkname);
	if (rc == -1)
		return errno;
	else
		return CO_ENOERR;
}

static void fsvm_revert_lnk(struct fsvm *vm, uint8_t name)
{
	const char *linkname;

	assert(vm != NULL);
	assert(name < FSVM_NUM_REGS);

	linkname = dstr(&vm->reg[name]);
	(void)unlink(linkname);
}

static int fsvm_run_dlnk(struct fsvm *vm, uint8_t name)
{
	const char *linkname;
	int rc;

	assert(vm != NULL);
	assert(name < FSVM_NUM_REGS);

	linkname = dstr(&vm->reg[name]);
	rc = unlink(linkname);
	if (rc == -1)
		return errno;
	else
		return CO_ENOERR;
}

static void fsvm_revert_dlnk(struct fsvm *vm, uint8_t targ, uint8_t name)
{
	const char *target;
	const char *linkname;

	assert(vm != NULL);
	assert(targ < FSVM_NUM_REGS);
	assert(name < FSVM_NUM_REGS);

	target = dstr(&vm->reg[targ]);
	linkname = dstr(&vm->reg[name]);

	/* symlink is declared with attribute warn_unused_result, but there is
	 * nothing we can do to handle an error here, since we are already
	 * trying to recover from an error; just ignore the error (and the GCC
	 * warning and move on
	 */
	ignore(symlink(target, linkname));
}

static int fsvm_run_rcat(struct fsvm *vm, uint8_t dst, uint8_t src)
{
	struct dstring *destination;
	struct dstring *source;

	assert(vm != NULL);
	assert(dst < FSVM_NUM_REGS);
	assert(src < FSVM_NUM_REGS);

	destination = &vm->reg[dst];
	source = &vm->reg[src];

	return dstrcatl(destination, dstr(source), dstrlen(source));
}

static void fsvm_revert_rcat(struct fsvm *vm, uint8_t dst, uint8_t src)
{
	struct dstring *destination;
	struct dstring *source;

	assert(vm != NULL);
	assert(dst < FSVM_NUM_REGS);
	assert(src < FSVM_NUM_REGS);

	destination = &vm->reg[dst];
	source = &vm->reg[src];

	dstrdel(destination, dstrlen(source));
}

static int fsvm_run_acat(struct fsvm *vm, uint8_t dst, uint8_t src)
{
	struct dstring *destination;
	struct fsvm_arg *source;

	assert(vm != NULL);
	assert(dst < FSVM_NUM_REGS);
	assert(src < FSVM_NUM_ARGS);

	destination = &vm->reg[dst];
	source = &vm->arg[src];

	return dstrcatl(destination, source->data, source->len);
}

static void fsvm_revert_acat(struct fsvm *vm, uint8_t dst, uint8_t src)
{
	struct dstring *destination;
	struct fsvm_arg *source;

	assert(vm != NULL);
	assert(dst < FSVM_NUM_REGS);
	assert(src < FSVM_NUM_ARGS);

	destination = &vm->reg[dst];
	source = &vm->arg[src];

	dstrdel(destination, source->len);
}

static int fsvm_run_ld(struct fsvm *vm, uint8_t path)
{
	const char *dirname;

	assert(vm != NULL);
	assert(path < FSVM_NUM_REGS);
	assert(vm->index == NULL);

	dirname = dstr(&vm->reg[path]);
	vm->index = opendir(dirname);
	if (vm->index == NULL)
		return errno;
	else
		return CO_ENOERR;
}

static void fsvm_revert_ld(struct fsvm *vm)
{
	fsvm_clearglobs(vm);
	assert(list_isempty(&vm->globs));

	closedir(vm->index);
	vm->index = NULL;
}

static int fsvm_run_glob(struct fsvm *vm)
{
	struct fsvm_glob *g;
	struct dirent *obj;
	size_t len;
	int rc;

	assert(vm != NULL);
	assert(vm->index != NULL);

	for (;;) {
		errno = 0;
		obj = readdir(vm->index);
		if (obj == NULL && errno != 0)
			return errno;
		else if (obj == NULL)
			return CO_ENOERR;
		/* ignore hidden files; make sure to skip at least . and .. */
		else if (obj->d_name[0] == '.')
			continue;

		len = strlen(obj->d_name);
		if (len >= lengthof(g->name))
			return CO_ELIMIT;

		g = fsvm_globalloc(vm);
		if (g == NULL)
			return errno;

		rc = fsvm_globname(vm, g, obj->d_name, len);
		if (rc != CO_ENOERR)
			return rc;
	}
}

static int fsvm_run_rd(struct fsvm *vm, uint8_t name)
{
	struct fsvm_glob *g;

	assert(vm != NULL);
	assert(name < FSVM_NUM_ARGS);
	assert(vm->arg[name].len < lengthof(g->name));

	g = fsvm_globalloc(vm);
	if (g == NULL)
		return errno;

	return fsvm_globname(vm, g, vm->arg[name].data, vm->arg[name].len);
}

static int fsvm_run_gmap(struct fsvm *vm, uint8_t reg, uint8_t name)
{
	struct list_elem *le;
	struct fsvm_glob *g;
	const char *map;
	size_t len;
	int rc = CO_ENOERR;

	assert(vm != NULL);
	assert(reg < FSVM_GLOB_NUM_REGS);
	assert(name < FSVM_NUM_REGS);

	list_foreach(le, &vm->globs) {
		g = to_fsvm_glob(le);

		rc = fmapat(g->fd, dstr(&vm->reg[name]), &map, &len);

		/* if the file is empty, then we are trying to append 0 bytes of
		 * data to 'reg'; this is just a no-op, so we can skip the rest
		 * of this iteration
		 */
		if (rc == ECANCELED && len == 0)
			continue;
		else if (rc != 0)
			break;

		rc = dstrcatl(&g->reg[reg], map, len);
		funmap(map, len);
		if (rc < 0)
			break;

		rc = CO_ENOERR;
	}

	return rc;
}

static int fsvm_run_grdlnk(struct fsvm *vm, uint8_t reg, uint8_t name)
{
	/* include an extra byte for a canary */
	char buf[FSVM_MAX_LNKTARG + 1];
	struct list_elem *le;
	struct fsvm_glob *g;
	ssize_t len;
	int rc;

	assert(vm != NULL);
	assert(reg < FSVM_GLOB_NUM_REGS);
	assert(name < FSVM_NUM_REGS);

	list_foreach(le, &vm->globs) {
		g = to_fsvm_glob(le);
		len = readlinkat(g->fd, dstr(&vm->reg[name]), buf, sizeof(buf));
		if (len < 0)
			return errno;
		else if ((size_t)len == sizeof(buf))
			return CO_ELIMIT;

		rc = dstrcatl(&g->reg[reg], buf, len);
		if (rc < 0)
			return rc;
	}

	return CO_ENOERR;
}

static int fsvm_run_rrdlnk(struct fsvm *vm, uint8_t dst, uint8_t name)
{
	char buf[FSVM_MAX_LNKTARG + 1];
	ssize_t len;
	int rc;

	assert(vm != NULL);
	assert(dst < FSVM_NUM_REGS);
	assert(name < FSVM_NUM_REGS);

	len = readlink(dstr(&vm->reg[name]), buf, sizeof(buf));
	if (len < 0)
		return errno;
	else if ((size_t)len == sizeof(buf))
		return CO_ELIMIT;

	rc = dstrcatl(&vm->reg[dst], buf, len);
	if (rc < 0)
		return rc;
	else
		return CO_ENOERR;
}

static int fsvm_run_one(struct fsvm *vm, const struct fsvm_op *op)
{
	int rc;

	assert(vm != NULL);
	assert(op != NULL);
	assert(op->type < FSVM_NUM_OPS);

	switch (op->type) {
	case FSVM_DEX:
		rc = fsvm_run_dex(vm, op->x);
		break;
	case FSVM_DMK:
		rc = fsvm_run_dmk(vm, op->x);
		break;
	case FSVM_FMK:
		rc = fsvm_run_fmk(vm, op->x, op->y);
		break;
	case FSVM_FMV:
		rc = fsvm_run_fmv(vm, op->x, op->y);
		break;
	case FSVM_FRM:
		rc =fsvm_run_frm(vm, op->x);
		break;
	case FSVM_LNK:
		rc = fsvm_run_lnk(vm, op->x, op->y);
		break;
	case FSVM_DLNK:
		rc = fsvm_run_dlnk(vm, op->y);
		break;
	case FSVM_RCAT:
		rc = fsvm_run_rcat(vm, op->x, op->y);
		break;
	case FSVM_ACAT:
		rc = fsvm_run_acat(vm, op->x, op->y);
		break;
	case FSVM_LD:
		rc = fsvm_run_ld(vm, op->x);
		break;
	case FSVM_GLOB:
		rc = fsvm_run_glob(vm);
		break;
	case FSVM_RD:
		rc = fsvm_run_rd(vm, op->x);
		break;
	case FSVM_GMAP:
		rc = fsvm_run_gmap(vm, op->x, op->y);
		break;
	case FSVM_GRDLNK:
		rc = fsvm_run_grdlnk(vm, op->x, op->y);
		break;
	case FSVM_RRDLNK:
		rc = fsvm_run_rrdlnk(vm, op->x, op->y);
		break;
	case FSVM_ABORT:
		rc = CO_EABORT;
		break;
	case FSVM_NOP:
		rc = 0;
		break;
	default:
		/* need to set rc here to avoid some compiler warnings; this
		 * should be indicative of a bug, so assert as well
		 */
		rc = EINVAL;
		assert(0);
		break;
	}

	return rc;
}

static void fsvm_revert_one(struct fsvm *vm, const struct fsvm_op *op)
{
	assert(vm != NULL);
	assert(op != NULL);
	assert(op->type < FSVM_NUM_OPS);
	assert(op->type != FSVM_ABORT);
	assert(op->type != FSVM_FMV);
	assert(op->type != FSVM_FRM);

	switch (op->type) {
	case FSVM_DMK:
		fsvm_revert_dmk(vm, op->x);
		break;
	case FSVM_FMK:
		fsvm_revert_fmk(vm, op->x);
		break;
	case FSVM_LNK:
		fsvm_revert_lnk(vm, op->y);
		break;
	case FSVM_DLNK:
		fsvm_revert_dlnk(vm, op->x, op->y);
		break;
	case FSVM_RCAT:
		fsvm_revert_rcat(vm, op->x, op->y);
		break;
	case FSVM_ACAT:
		fsvm_revert_acat(vm, op->x, op->y);
		break;
	case FSVM_LD:
		fsvm_revert_ld(vm);
		break;
	case FSVM_GLOB:		/* fallthrough */
	case FSVM_RD:		/* fallthrough */
	case FSVM_GMAP:		/* fallthrough */
	case FSVM_GRDLNK:	/* fallthrough */
	case FSVM_RRDLNK:	/* fallthrough */
	case FSVM_DEX:		/* fallthrough */
	case FSVM_NOP:
		break;
	default:
		assert(0);
		break;
	}
}

int fsvm_run(struct fsvm *vm, const struct fsvm_op *op, size_t nops)
{
	size_t i;
	ssize_t j;
	int rc;

	assert(vm != NULL);
	assert(op != NULL);

	for (i = 0; i < nops; i++) {
		/* an op may only be fmv if it is the last operation */
		assert(op[i].type != FSVM_FMV || i == nops - 1);

		rc = fsvm_run_one(vm, &op[i]);
		if (rc != CO_ENOERR)
			break;
	}
	assert((rc == CO_ENOERR && i == nops) || i < nops);
	vm->ip = i;

	if (rc != CO_ENOERR) {
		for (j = i - 1; j >= 0; j--)
			fsvm_revert_one(vm, op + j);
	}

	return rc;
}

