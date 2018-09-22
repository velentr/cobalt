/*
 * Copyright (C) 2018 Brian Kubisiak <brian@kubisiak.com>
 */

#ifndef FSVM_OPS_H_
#define FSVM_OPS_H_


enum {
	FSVM_DEX,	/* ensure directory existence */
	FSVM_DMK,	/* make a new directory */
	FSVM_FMK,	/* make a new file */
	FSVM_FMV,	/* move a file */
	FSVM_LNK,	/* create a symbolic link */
	FSVM_DLNK,	/* delete a symbolic link */
	FSVM_RCAT,	/* concatenate from a register */
	FSVM_ACAT,	/* concatenate from an argument */
	FSVM_LD,	/* load a directory as an index */
	FSVM_GLOB,	/* glob from the current index */
	FSVM_RD,	/* read a single entry from the current index */
	FSVM_MAP,	/* map relative to the glob */
	FSVM_RDLNK,	/* read a link relative to the glob */
	FSVM_ABORT,	/* abort execution */
	FSVM_NOP,	/* do nothing */

	FSVM_NUM_OPS
};

#define OP_DEX(path)		{ .type = FSVM_DEX, .x = path, }
#define OP_DMK(path)		{ .type = FSVM_DMK, .x = path, }
#define OP_FMK(path, contents)	{ .type = FSVM_FMK, .x = path, .y = contents, }
#define OP_FMV(old, new)	{ .type = FSVM_FMV, .x = old, .y = new, }
#define OP_LNK(target, link)	{ .type = FSVM_LNK, .x = target, .y = link, }
#define OP_DLNK(target, link)	{ .type = FSVM_DLNK, .x = target, .y = link, }
#define OP_RCAT(dst, src)	{ .type = FSVM_RCAT, .x = dst, .y = src, }
#define OP_ACAT(dst, src)	{ .type = FSVM_ACAT, .x = dst, .y = src, }
#define OP_LD(path)		{ .type = FSVM_LD, .x = path, }
#define OP_GLOB			{ .type = FSVM_GLOB, }
#define OP_RD(name)		{ .type = FSVM_RD, .x = name, }
#define OP_MAP(dst, name)	{ .type = FSVM_MAP, .x = dst, .y = name, }
#define OP_RDLNK(dst, name)	{ .type = FSVM_RDLNK, .x = dst, .y = name, }
#define OP_ABORT		{ .type = FSVM_ABORT, }
#define OP_NOP			{ .type = FSVM_NOP, }


#endif /* end of include guard: FSVM_OPS_H_ */

