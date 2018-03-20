
#include "postgres.h"
#include "pgadminScanner.h"

bool assert_enabled = false;
char *pgadmin_scanner_last_error = NULL;
bool pgadmin_scanner_firstime = true;

/* GUC parameters */
bool quote_all_identifiers = false;

char *vk_vsprintf(const char *format, va_list argPtr)
{
	static char *s = NULL;
	int size = 1024, x = -1;
	while (x == -1)
	{
		if (s) free(s);
		if (NULL == (s = malloc(size + 1)))
		{
			s = strdup("");
			return s;
		}
#ifndef WIN32
		x = vsnprintf((char *) s, size, format, argPtr);
#else
		x = _vsnprintf((char *) s, size, format, argPtr);
#endif
		if ((x != -1) && ((x + 1) <= size))
		{
			if ((x + 1) < size) s = realloc(s, x + 1);
			if (s)
			{
				return s;
			}
			s = strdup("");
			return s;
		}
		size *= 2;
		x = -1;
	}
	s = strdup("");
	return s;
}

char *vk_sprintf(const char *format, ...)
{
	va_list argPtr;
	va_start(argPtr, format);
	char *x = vk_vsprintf(format, argPtr);
	va_end(argPtr);
	return x;
}

void set_pgadmin_scanner_last_error(const char *format, ...)
{
	va_list argPtr;
	va_start(argPtr, format);
	pgadmin_scanner_last_error = vk_vsprintf(format, argPtr);
	va_end(argPtr);
}

void set_pgadmin_scanner_last_error_va(const char *format, va_list argPtr)
{
	pgadmin_scanner_last_error = vk_vsprintf(format, argPtr);
}

bool
errstart(int elevel, const char *filename, int lineno,
		 const char *funcname, const char *domain)
{
	return true;
}

int
errmsg(const char *fmt, ...)
{
	va_list argPtr;
	va_start(argPtr, fmt);
	set_pgadmin_scanner_last_error_va("ERROR: %s\n", argPtr);
	va_end(argPtr);
	/*fprintf(stderr, "ERROR: %s\n", fmt);*/
	return 0; /* return value does not matter */
}

int
errdetail(const char *fmt, ...)
{
	set_pgadmin_scanner_last_error("DETAIL: %s\n", fmt);
	/*fprintf(stderr, "DETAIL: %s\n", fmt);*/
	return 0; /* return value does not matter */
}

int
errcode(int sqlerrcode)
{
	return 0; /* return value does not matter */
}

void
errfinish(int dummy, ...)
{
	/*exit(1);*/
}

int
errhint(const char *fmt, ...)
{
	set_pgadmin_scanner_last_error("HINT: %s\n", fmt);
	/*fprintf(stderr, "HINT: %s\n", fmt);*/
	return 0; /* return value does not matter */
}

int
errmsg_internal(const char *fmt, ...)
{
	set_pgadmin_scanner_last_error("ERROR: %s\n", fmt);
	/*fprintf(stderr, "ERROR: %s\n", fmt);*/
	return 0; /* return value does not matter */
}

int
errposition(int cursorpos)
{
	/*
		ErrorData  *edata = &errordata[errordata_stack_depth];

		/ * we don't bother incrementing recursion_depth * /
		CHECK_STACK_DEPTH();

		edata->cursorpos = cursorpos;
	 */

	return 0; /* return value does not matter */
}

void
elog_start(const char *filename, int lineno, const char *funcname)
{
}

void
elog_finish(int elevel, const char *fmt, ...)
{
	set_pgadmin_scanner_last_error("ERROR: %s\n", fmt);
	/*fprintf(stderr, "ERROR: %s\n", fmt);*/
	/*exit(1);*/
}

#if (PG_VERSION_NUM / 100) < 902
int
#else
void
#endif
ExceptionalCondition(const char *conditionName,
					 const char *errorType,
					 const char *fileName,
					 int lineNumber)
{
	if (!PointerIsValid(conditionName)
			|| !PointerIsValid(fileName)
			|| !PointerIsValid(errorType))
		write_stderr("TRAP: ExceptionalCondition: bad arguments\n");
	else
	{
		write_stderr("TRAP: %s(\"%s\", File: \"%s\", Line: %d)\n",
					 errorType, conditionName,
					 fileName, lineNumber);
	}

	/* Usually this shouldn't be needed, but make sure the msg went out */
	fflush(stderr);

#ifdef SLEEP_ON_ASSERT

	/*
	 * It would be nice to use pg_usleep() here, but only does 2000 sec or 33
	 * minutes, which seems too short.
	 */
	sleep(1000000);
#endif

	abort();
}

void
write_stderr(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	set_pgadmin_scanner_last_error_va(fmt, ap);
	va_end(ap);
}

struct myScannerNode* add(struct myScannerNode *head, const struct myScannerData *data)
{
	struct myScannerNode *tmp;

	if (head == NULL)
	{
		head = (struct myScannerNode *) malloc(sizeof (struct myScannerNode));
		if (head == NULL)
		{
			set_pgadmin_scanner_last_error("Add error1! Memory is not available\n");
			/*
						printf("Error! memory is not available\n");
						exit(0);
			 */
			return head;
		}
		head-> data = *data;
		head-> next = head;
	}
	else
	{
		tmp = head;

		while (tmp-> next != head)
			tmp = tmp-> next;
		tmp-> next = (struct myScannerNode *) malloc(sizeof (struct myScannerNode));
		if (tmp -> next == NULL)
		{
			/*
						printf("Error! memory is not available\n");
						exit(0);
			 */
			set_pgadmin_scanner_last_error("Add error2! Memory is not available\n");
			return NULL;
		}
		tmp = tmp-> next;
		tmp-> data = *data;
		tmp-> next = head;
	}
	return head;
}

void addToken(char **result, const char *token, bool appendblank)
{
	int lenr = (*result) ? strlen(*result) : 0;
	int lenb = strlen(token);
	char *x = malloc(lenr + lenb + 2);
	if (*result)
		strcpy(x, *result);
	else
		strcpy(x, "");
	if (!pgadmin_scanner_firstime && appendblank)
		strcat(x, " ");
	else
		pgadmin_scanner_firstime = false;
	strcat(x, token);
	if (*result)
		free(*result);
	*result = x;
}

/*
 * quote_identifier			- Quote an identifier only if needed
 *
 * When quotes are needed, we palloc the required space; slightly
 * space-wasteful but well worth it for notational simplicity.
 */
char *quote_identifier(const char *ident)
{
	/*
	 * Can avoid quoting if ident starts with a lowercase letter or underscore
	 * and contains only lowercase letters, digits, and underscores, *and* is
	 * not any SQL keyword.  Otherwise, supply quotes.
	 */
	int nquotes = 0;
	bool safe;
	const char *ptr;
	char *result;
	char *optr;

	/*
	 * would like to use <ctype.h> macros here, but they might yield unwanted
	 * locale-specific results...
	 */
	safe = ((ident[0] >= 'a' && ident[0] <= 'z') || ident[0] == '_');

	for (ptr = ident; *ptr; ptr++)
	{
		char ch = *ptr;

		if ((ch >= 'a' && ch <= 'z') ||
				(ch >= '0' && ch <= '9') ||
				(ch == '_'))
		{
			/* okay */
		}
		else
		{
			safe = false;
			if (ch == '"')
				nquotes++;
		}
	}

	if (quote_all_identifiers)
		safe = false;

	if (safe)
	{
		/*
		 * Check for keyword.  We quote keywords except for unreserved ones.
		 * (In some cases we could avoid quoting a col_name or type_func_name
		 * keyword, but it seems much harder than it's worth to tell that.)
		 *
		 * Note: ScanKeywordLookup() does case-insensitive comparison, but
		 * that's fine, since we already know we have all-lower-case.
		 */
		const ScanKeyword *keyword = ScanKeywordLookup(ident,
													   ScanKeywords,
													   NumScanKeywords);

		if (keyword != NULL && keyword->category != UNRESERVED_KEYWORD)
			safe = false;
	}

	if (safe)
	{
		result = strdup(ident);
		return result; /* no change needed */
	}

	result = (char *) palloc(strlen(ident) + nquotes + 2 + 1);

	optr = result;
	*optr++ = '"';
	for (ptr = ident; *ptr; ptr++)
	{
		char ch = *ptr;

		if (ch == '"')
			*optr++ = '"';
		*optr++ = ch;
	}
	*optr++ = '"';
	*optr = '\0';

	return result;
}

/*
 * quote_qualified_identifier	- Quote a possibly-qualified identifier
 *
 * Return a name of the form qualifier.ident, or just ident if qualifier
 * is NULL, quoting each component if necessary.  The result is palloc'd.
 */
char *quote_qualified_identifier(const char *qualifier, const char *ident)
{
	char *qual = NULL, *identifier, *result;

	if (qualifier)
	{
		qual = quote_identifier(qualifier);
	}

	identifier = quote_identifier(ident);
	if (!qual)
		return identifier;

	result = malloc(strlen(qual) + 1 + strlen(identifier) + 1);
	strcpy(result, qual);
	strcat(result, ".");
	strcat(result, identifier);
	free(qual);
	free(identifier);

	return result;
}

struct myScannerNode *scanSqlCpp(const char *command)
{
	struct myScannerNode *head = NULL;
	jmp_buf env;
	/*
	 * setjmp() return 0 if returning directly, and nonzero when returning from longjmp using the saved context.
	 */
	int rc = setjmp(env);
	if (rc == 0)
	{
		head = scan_SQL_command(env, 95, command);
	}

	return head;
}

char *parseRuleCpp(
const struct myScannerNode *head, const char *rulename, const char *targetschema, const char *tablename)
{
	char *result = parse_rule_sql(head, rulename, targetschema, tablename);

	return result;
}

char *parseTriggerCpp(const struct myScannerNode *head, const char *targetschema)
{
	char *result = parse_trigger_sql(head, targetschema);

	return result;
}

extern void destroylist_95(struct myScannerNode *head);

void destroylistCpp(struct myScannerNode *head)
{
	destroylist_95(head);
}

int qualifiedIdentifierDotPosCpp(const char *identifier)
{
	int pos = -1;
	struct myScannerNode *head = scanSqlCpp(identifier);
	if (head)
	{
		pos = qualified_identifier_dot_pos(head);
		destroylist_95(head);
	}

	return pos;
}

struct myScannerNode *getAllCommandsCpp(const struct myScannerNode *head)
{
	struct myScannerNode *result = get_all_commands(head);

	return result;
}
