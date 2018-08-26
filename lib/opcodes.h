/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#ifndef OPCODES_H_
#define OPCODES_H_


typedef enum {
	CO_DB_DIR_EXIST,
	CO_DB_DIR_CREATE,
	CO_DB_FILE_CREATE,
	CO_DB_FILE_RENAME,
	CO_DB_SYM_CREATE,
	CO_DB_SYM_DELETE,
	CO_DB_STR_CAT,
	CO_DB_STR_COPY,
	CO_DB_QUERY_LOAD,
	CO_DB_QUERY_READ,
	CO_DB_QUERY_MAP,
	CO_DB_QUERY_BOARD,
	CO_DB_NOP,
	CO_DB_ABORT,
	CO_DB_NUM_OPS
} co_opcode_t;

struct co_db_op {
	const char *data;
	size_t len;
	size_t str0;
	size_t str1;
	co_opcode_t type;
};

#define OP_DEXIST(path) \
	{ .type = CO_DB_DIR_EXIST, .str0 = path, }
#define OP_DCREAT(path) \
	{ .type = CO_DB_DIR_CREATE, .str0 = path, }
#define OP_FCREAT(path, val, size) \
	{ .type = CO_DB_FILE_CREATE, .str0 = path, .data = val, .len = size, }
#define OP_FRENAM(old, new) \
	{ .type = CO_DB_FILE_RENAME, .str0 = old, .str1 = new, }
#define OP_LCREAT(target, linkname) \
	{ .type = CO_DB_SYM_CREATE, .str0 = target, .str1 = linkname }
#define OP_LDEL(target, linkname) \
	{ .type = CO_DB_SYM_DELETE, .str0 = target, .str1 = linkname }
#define OP_SCAT(dst, src, size) \
	{ .type = CO_DB_STR_CAT, .str0 = dst, .data = src, .len = size, }
#define OP_SCPY(dst, src) \
	{ .type = CO_DB_STR_COPY, .str0 = dst, .str1 = src, }
#define OP_QLD(dir) \
	{ .type = CO_DB_QUERY_LOAD, .str0 = dir, }
#define OP_QRD \
	{ .type = CO_DB_QUERY_READ, }
#define OP_QMAP \
	{ .type = CO_DB_QUERY_MAP, }
#define OP_QBD \
	{ .type = CO_DB_QUERY_BOARD, }
#define OP_NOP \
	{ .type = CO_DB_NOP, }
#define OP_ABORT \
	{ .type = CO_DB_ABORT, }

/* macros providing some syntactic sugar for common operations */
#define MACRO_SCATL(dst, src) OP_SCAT(dst, src, strlen(src))


#endif /* end of include guard: OPCODES_H_ */

