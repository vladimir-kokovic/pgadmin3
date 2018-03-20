/*-------------------------------------------------------------------------
 *
 * keywords.h
 *	  lexical token lookup for reserved words in postgres SQL
 *
 *
 * Portions Copyright (c) 1996-2006, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/parser/keywords.h,v 1.21 2006/03/05 15:58:57 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */

///////////////////////////////////////////////////////////////////////////
//
// pgAdmin note: This file is based on src/include/parser/keywords.h from
//               PostgreSQL, but with the token code entry removed.
//
//               This file is under the BSD licence, per PostgreSQL.
///////////////////////////////////////////////////////////////////////////

#ifndef KEYWORDS_PGADMIN_H
#define KEYWORDS_PGADMIN_H

/* Keyword categories --- should match lists in gram.y */
#define UNRESERVED_KEYWORD		0
#define COL_NAME_KEYWORD		1
#define TYPE_FUNC_NAME_KEYWORD	2
#define RESERVED_KEYWORD		3

typedef struct ScanKeyword_pgadmin
{
	const char *name;			/* in lower case */
	int		category;		/* see codes above */
} ScanKeyword_pgadmin;

extern const ScanKeyword_pgadmin *ScanKeywordLookup_pgadmin(const char *text);

extern const ScanKeyword_pgadmin ScanKeywords_pgadmin[];
extern const ScanKeyword_pgadmin ScanKeywordsExtra[];
extern const int NumScanKeywords_pgadmin;
extern const int NumScanKeywordsExtra;
#endif   /* KEYWORDS_PGADMIN_H */
