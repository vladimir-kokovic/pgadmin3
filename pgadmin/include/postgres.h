//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
//
// Copyright (C) 2002 - 2016, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// postgres.h - dummy include
//
//////////////////////////////////////////////////////////////////////////

#if !defined(PG_VERSION_NUM)
#include <pg_config.h>
#endif

#if PG_VERSION_NUM >= 90600
#include <server/postgres.h>
#ifndef endof
#define endof(array)	(&array[lengthof(array)])
#endif
#else
//#if defined(PG_VERSION_NUM)
//#define msg "PG_VERSION_NUM= " ## #PG_VERSION_NUM
//#else
//#define msg "PG_VERSION_NUM is not defined "
//#endif
//#pragma message msg
#include <string.h>
#define lengthof(array) (sizeof (array) / sizeof ((array)[0]))
#define endof(array)	(&array[lengthof(array)])
// to suppress much stuff in parse.h
#define YYTOKENTYPE
#define YYSTYPE int
#define NAMEDATALEN 32
#endif