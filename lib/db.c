/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cobalt/cobalt.h>
#include <cobalt/cobalt_error.h>

#include "db.h"
#include "opcodes.h"
#include "struct.h"
#include "util.h"

static int co_db_dir_exist_run(struct co_db *db, const struct co_db_op *op)
{
	int rc;

	assert(op->str0 < CO_DB_NSTR);

	rc = mkdir(dstr(&db->str[op->str0]), 0755);
	if (rc == -1 && errno != EEXIST)
		return errno;

	return CO_ENOERR;
}

static int co_db_dir_create_run(struct co_db *db, const struct co_db_op *op)
{
	int rc;

	assert(op->str0 < CO_DB_NSTR);

	rc = mkdir(dstr(&db->str[op->str0]), 0755);
	if (rc == -1)
		return errno;

	return CO_ENOERR;
}

static void co_db_dir_create_revert(struct co_db *db, const struct co_db_op *op)
{
	assert(op->str0 < CO_DB_NSTR);

	rmdir(dstr(&db->str[op->str0]));
}

static int co_db_file_create_run(struct co_db *db, const struct co_db_op *op)
{
	int fd;
	ssize_t rc;
	int ret;

	assert(op->str0 < CO_DB_NSTR);
	assert(op->data != NULL);

	fd = open(dstr(&db->str[op->str0]), O_CREAT | O_EXCL | O_WRONLY, 0644);
	if (fd == -1)
		return errno;

	rc = write(fd, op->data, op->len);
	if (rc == -1)
		ret = errno;
	else if ((size_t)rc != op->len)
		ret = CO_EINCOMPLETE;
	else
		ret = CO_ENOERR;

	close(fd);
	if (ret != CO_ENOERR)
		unlink(dstr(&db->str[op->str0]));

	return ret;
}

static void co_db_file_create_revert(struct co_db *db,
		const struct co_db_op *op)
{
	assert(op->str0 < CO_DB_NSTR);

	unlink(dstr(&db->str[op->str0]));
}

static int co_db_file_rename_run(struct co_db *db, const struct co_db_op *op)
{
	const char *oldpath;
	const char *newpath;
	int rc;

	assert(op->str0 < CO_DB_NSTR);
	assert(op->str1 < CO_DB_NSTR);

	oldpath = dstr(&db->str[op->str0]);
	newpath = dstr(&db->str[op->str1]);

	rc = rename(oldpath, newpath);
	if (rc == -1)
		return errno;
	else
		return CO_ENOERR;
}

static int co_db_sym_create_run(struct co_db *db, const struct co_db_op *op)
{
	const char *target;
	const char *linkname;
	int rc;

	assert(op->str0 < CO_DB_NSTR);
	assert(op->str1 < CO_DB_NSTR);

	target = dstr(&db->str[op->str0]);
	linkname = dstr(&db->str[op->str1]);

	rc = symlink(target, linkname);
	if (rc == -1)
		return errno;
	else
		return CO_ENOERR;
}

static void co_db_sym_create_revert(struct co_db *db, const struct co_db_op *op)
{
	const char *linkname;

	assert(op->str1 < CO_DB_NSTR);

	linkname = dstr(&db->str[op->str1]);
	unlink(linkname);
}

static int co_db_sym_delete_run(struct co_db *db, const struct co_db_op *op)
{
	const char *linkname;
	int rc;

	assert(op->str1 < CO_DB_NSTR);

	linkname = dstr(&db->str[op->str1]);
	rc = unlink(linkname);
	if (rc == -1)
		return errno;
	else
		return CO_ENOERR;
}

static void co_db_sym_delete_revert(struct co_db *db, const struct co_db_op *op)
{
	const char *target;
	const char *linkname;

	assert(op->str0 < CO_DB_NSTR);
	assert(op->str1 < CO_DB_NSTR);

	target = dstr(&db->str[op->str0]);
	linkname = dstr(&db->str[op->str1]);
	symlink(target, linkname);
}

static int co_db_str_cat_run(struct co_db *db, const struct co_db_op *op)
{
	assert(op->str0 < CO_DB_NSTR);
	assert(op->data != NULL);
	assert(op->len > 0);

	return dstrcatl(&db->str[op->str0], op->data, op->len);
}

static void co_db_str_cat_revert(struct co_db *db, const struct co_db_op *op)
{
	assert(op->str0 < CO_DB_NSTR);
	assert(op->len > 0);

	dstrdel(&db->str[op->str0], op->len);
}

static int co_db_str_copy_run(struct co_db *db, const struct co_db_op *op)
{
	struct dstring *dst, *src;

	assert(op->str0 < CO_DB_NSTR);
	assert(op->str1 < CO_DB_NSTR);

	dst = &db->str[op->str0];
	src = &db->str[op->str1];

	return dstrcatl(dst, dstr(src), dstrlen(src));
}

static void co_db_str_copy_revert(struct co_db *db, const struct co_db_op *op)
{
	struct dstring *dst, *src;

	assert(op->str0 < CO_DB_NSTR);
	assert(op->str1 < CO_DB_NSTR);

	dst = &db->str[op->str0];
	src = &db->str[op->str1];

	dstrdel(dst, dstrlen(src));
}

static int co_db_query_load_run(struct co_db *db, const struct co_db_op *op)
{
	const char *dirname;

	assert(op->str0 < CO_DB_NSTR);
	assert(list_isempty(&db->query));
	assert(db->qdir == NULL);

	dirname = dstr(&db->str[op->str0]);
	db->qdir = opendir(dirname);
	if (db->qdir == NULL)
		return errno;

	return CO_ENOERR;
}

static void co_db_query_load_revert(struct co_db *db)
{
	struct list_elem *le;
	struct cobalt_query *q;

	while (!list_isempty(&db->query)) {
		le = list_popfront(&db->query);
		q = containerof(le, struct cobalt_query, le);
		co_db_query_free(q);
	}
}

static int co_db_query_read_run(struct co_db *db)
{
	struct cobalt_query *q;
	struct dirent *obj;
	uint32_t id;

	assert(db->qdir != NULL);

	errno = 0;
	obj = readdir(db->qdir);
	/* error encountered; revert the call sequence */
	if (obj == NULL && errno != 0)
		return errno;
	/* we've finished reading query entries; reset the cursor to the start
	 * of the query set
	 */
	else if (obj == NULL) {
		db->cursor = list_head(&db->query);
		return CO_ENOERR;
	}

	/* if the id is 0, we found '..' or '.' or something; ignore it */
	id = strtoul(obj->d_name, NULL, 16);
	if (id != 0) {
		/* allocate the entry and add it to the set */
		q = malloc(sizeof(*q));
		if (q == NULL)
			return errno;

		q->id = id;
		q->data = NULL;
		q->len = 0;
		dstrempty(&q->board);
		list_pushfront(&db->query, &q->le);
	}

	/* we haven't read all dir entries yet */
	db->ip--;
	return CO_ENOERR;
}

static int co_db_query_map_run(struct co_db *db)
{
	char filname[sizeof("XXXXXXXX/data")];
	struct stat st;
	struct cobalt_query *q;
	int fd;
	int rc;

	/* check if we've finished mapping all queries */
	if (db->cursor == list_end(&db->query)) {
		db->cursor = list_head(&db->query);
		return CO_ENOERR;
	}

	q = containerof(db->cursor, struct cobalt_query, le);
	snprintf(filname, sizeof(filname), "%08x/data", q->id);
	fd = openat(dirfd(db->qdir), filname, O_RDONLY);
	if (fd == -1)
		return errno;

	rc = fstat(fd, &st);
	if (rc < 0) {
		close(fd);
		return errno;
	}

	if (st.st_size > 0) {
		q->len = st.st_size;
		q->data = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
		close(fd);
		if (q->data == MAP_FAILED) {
			q->data = NULL;
			return errno;
		}
	}

	db->cursor = list_next(db->cursor);
	/* we haven't mapped all dir entries yet */
	db->ip--;
	return CO_ENOERR;
}

static int co_db_run_opcode(struct co_db *db, const struct co_db_op * const op)
{
	int rc;

	assert(op->type < CO_DB_NUM_OPS);

	switch (op->type) {
	case CO_DB_DIR_EXIST:
		rc = co_db_dir_exist_run(db, op);
		break;
	case CO_DB_DIR_CREATE:
		rc = co_db_dir_create_run(db, op);
		break;
	case CO_DB_FILE_CREATE:
		rc = co_db_file_create_run(db, op);
		break;
	case CO_DB_FILE_RENAME:
		rc = co_db_file_rename_run(db, op);
		break;
	case CO_DB_SYM_CREATE:
		rc = co_db_sym_create_run(db, op);
		break;
	case CO_DB_SYM_DELETE:
		rc = co_db_sym_delete_run(db, op);
		break;
	case CO_DB_STR_CAT:
		rc = co_db_str_cat_run(db, op);
		break;
	case CO_DB_STR_COPY:
		rc = co_db_str_copy_run(db, op);
		break;
	case CO_DB_QUERY_LOAD:
		rc = co_db_query_load_run(db, op);
		break;
	case CO_DB_QUERY_READ:
		rc = co_db_query_read_run(db);
		break;
	case CO_DB_QUERY_MAP:
		rc = co_db_query_map_run(db);
		break;
	case CO_DB_ABORT:
		rc = CO_EABORT;
		break;
	default:
		rc = CO_ENOERR;
		break;
	}

	if (rc == CO_ENOERR)
		db->ip++;

	return rc;
}

static void co_db_revert_opcode(struct co_db *db,
		const struct co_db_op * const op)
{
	assert(op->type < CO_DB_NUM_OPS);

	switch (op->type) {
	case CO_DB_DIR_CREATE:
		co_db_dir_create_revert(db, op);
		break;
	case CO_DB_FILE_CREATE:
		co_db_file_create_revert(db, op);
		break;
	case CO_DB_SYM_CREATE:
		co_db_sym_create_revert(db, op);
		break;
	case CO_DB_SYM_DELETE:
		co_db_sym_delete_revert(db, op);
		break;
	case CO_DB_STR_CAT:
		co_db_str_cat_revert(db, op);
		break;
	case CO_DB_STR_COPY:
		co_db_str_copy_revert(db, op);
		break;
	case CO_DB_QUERY_LOAD:
		co_db_query_load_revert(db);
		break;
	case CO_DB_ABORT:
		assert(0);
		break;
	default:
		break;
	}
}

int co_db_run(struct co_db *db, const struct co_db_op * const ops, size_t nops)
{
	size_t i;
	int j;
	int rc;

	list_init(&db->query);

	/* run each op code, checking for errors */
	for (db->ip = 0; db->ip < nops;) {
		rc = co_db_run_opcode(db, ops + db->ip);
		if (rc != CO_ENOERR)
			break;
	}

	/* if an error is detected, reverse all ops that completed */
	if (rc != CO_ENOERR) {
		for (j = (int)db->ip - 1; j >= 0; j--)
			co_db_revert_opcode(db, ops + j);
	}

	/* make sure the strings are clear for the next transaction */
	for (i = 0; i < CO_DB_NSTR; i++)
		dstrclr(&db->str[i]);

	if (db->qdir != NULL) {
		closedir(db->qdir);
		db->qdir = NULL;
	}

	return rc;
}

void co_db_init(struct co_db *db)
{
	size_t i;

	assert(db != NULL);

	for (i = 0; i < CO_DB_NSTR; i++)
		dstrempty(&db->str[i]);

	db->qdir = NULL;
}

void co_db_free(struct co_db *db)
{
	assert(db != NULL);
	assert(db->qdir == NULL);
	(void)db;
}

void co_db_query_free(struct cobalt_query *q)
{
	dstrclr(&q->board);
	if (q->data != NULL)
		munmap(q->data, q->len);
	free(q);
}

