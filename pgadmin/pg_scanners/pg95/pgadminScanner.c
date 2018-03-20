
#include "postgres.h"
#include "pgadminScanner.h"
#undef YYTOKENTYPE
#include "parser/gram.h"
/*
#include <mcheck.h>
*/

int base_yylex(core_YYSTYPE *lvalp, YYLTYPE *llocp, core_yyscan_t yyscanner)
{
	base_yy_extra_type *yyextra = pg_yyget_extra(yyscanner);
	int cur_token;
	int next_token;
	core_YYSTYPE cur_yylval;
	YYLTYPE cur_yylloc;

	/* Get next token --- we might already have it */
	if (yyextra->have_lookahead)
	{
		cur_token = yyextra->lookahead_token;
		*lvalp = yyextra->lookahead_yylval;
		*llocp = yyextra->lookahead_yylloc;
		yyextra->have_lookahead = false;
	}
	else
		cur_token = core_yylex(lvalp, llocp, yyscanner);

	/* Do we need to look ahead for a possible multiword token? */
	switch (cur_token)
	{
#undef VERVER
#ifdef XVER
#if XVER >= 0x83
#define VERVER
#endif
#endif
#ifdef VERVER
        case NOT:
			/* Replace NOT by NOT_LA if it's followed by BETWEEN, IN, etc */
            cur_yylval = *lvalp;
			cur_yylloc = *llocp;
			next_token = core_yylex(lvalp, llocp, yyscanner);
			switch (next_token)
			{
				case BETWEEN:
				case IN_P:
				case LIKE:
				case ILIKE:
				case SIMILAR:
					cur_token = NOT_LA;
					break;
                default:
					/* save the lookahead token for next time */
					yyextra->lookahead_token = next_token;
					yyextra->lookahead_yylval = *lvalp;
					yyextra->lookahead_yylloc = *llocp;
					yyextra->have_lookahead = true;
					/* and back up the output info to cur_token */
					*lvalp = cur_yylval;
					*llocp = cur_yylloc;
                    break;
			}
			break;

        case NULLS_P:
			/*
			 * NULLS FIRST and NULLS LAST must be reduced to one token
			 */
			cur_yylval = *lvalp;
			cur_yylloc = *llocp;
			next_token = core_yylex(lvalp, llocp, yyscanner);
			switch (next_token)
			{
				case FIRST_P:
				case LAST_P:
					cur_token = NULLS_LA;
					break;
                default:
					/* save the lookahead token for next time */
					yyextra->lookahead_token = next_token;
					yyextra->lookahead_yylval = *lvalp;
					yyextra->lookahead_yylloc = *llocp;
					yyextra->have_lookahead = true;
					/* and back up the output info to cur_token */
					*lvalp = cur_yylval;
					*llocp = cur_yylloc;
                    break;
			}
			break;
#endif
		case WITH:
			/*
			 * WITH TIME must be reduced to one token
			 */
			cur_yylval = *lvalp;
			cur_yylloc = *llocp;
			next_token = core_yylex(lvalp, llocp, yyscanner);
			switch (next_token)
			{
				case TIME:
                case ORDINALITY:
#undef VERVER
#ifdef XVER
#if XVER >= 0x84
#define VERVER
#endif
#endif
#ifdef VERVER
					cur_token = WITH_LA;
#else
					scanner_yyerror("WITH_TIME is valid from the PostgreSQL 8.4", yyscanner);
#endif
					break;
                default:
					/* save the lookahead token for next time */
					yyextra->lookahead_token = next_token;
					yyextra->lookahead_yylval = *lvalp;
					yyextra->lookahead_yylloc = *llocp;
					yyextra->have_lookahead = true;
					/* and back up the output info to cur_token */
					*lvalp = cur_yylval;
					*llocp = cur_yylloc;
                    break;
			}
			break;

		default:
			break;
	}

	return cur_token;
}

void printlist(const struct myScannerNode *head)
{
	struct myScannerNode *current;
	current = (struct myScannerNode *)head;
	if (current != NULL)
	{
		do
		{
			switch (current->data.token)
			{
				case ICONST:
				case PARAM:
					printf("LOC=%d TOKEN=%d INT=%d\n", current->data.loc, current->data.token, current->data.val.ival);
					break;
				default:
					if (current->data.token < 256)
						printf("LOC=%d TOKEN=%d TXT='%c'\n", current->data.loc, current->data.token, current->data.token);
					else
						printf("LOC=%d TOKEN=%d TXT='%s'\n", current->data.loc, current->data.token, current->data.val.str);
					break;
			}
			current = current->next;
		}
		while (current != head);
		printf("\n");
	} else
		printf("The list is empty\n");

}

void destroylist(struct myScannerNode *head) {
	struct myScannerNode *current, *tmp;

	current = head->next;
	head->next = NULL;
	while (current != NULL)
	{
		tmp = current->next;
		switch (current->data.token)
		{
			case ICONST:
			case PARAM:
				break;
			default:
				if (current->data.token >= 256)
					free(current->data.val.str);
				break;
		}
    if (current != head)
    {
      free(current);
    }
		current = tmp;
	}
  free(head);
}

struct myScannerNode *scan_SQL_command(jmp_buf env, int version, const char *command)
{
	struct myScannerNode *head = NULL;
	core_yyscan_t (*scanner)(jmp_buf env, const char *, core_yy_extra_type *, const ScanKeyword *, int);
	int (*yylex)(core_YYSTYPE *, YYLTYPE *, core_yyscan_t);
	scanner = scanner_init_95;
	yylex = base_yylex_95;
	core_yyscan_t vkscanner;
	base_yy_extra_type vkextra;
	core_YYSTYPE yylval_param;
	YYLTYPE yylloc_param;
	int yyresult;
  int token1 = -1;

/*
  mtrace();
*/

	/* initialize the flex scanner */
	vkscanner = (*scanner)(env, command, &vkextra.core_yy_extra, ScanKeywords, NumScanKeywords);
	/* base_yylex() only needs this much initialization */
	vkextra.have_lookahead = false;
	/* Parse! */
	do
	{
		yyresult = (*yylex)(&yylval_param, &yylloc_param, vkscanner);
		if (yyresult)
		{
			struct myScannerData data;
			data.loc = yylloc_param;
			data.token = yyresult;
			switch (yyresult)
			{
				case ICONST:
				case PARAM:
					data.val.ival = yylval_param.ival;
					break;
				default:
					if (yyresult < 256)
						data.val.ival = yyresult;
					else
					{
						if (yyresult == SCONST)
						{
							int len = strlen(yylval_param.str);
							data.val.str = malloc(len + 2 + 1);
							strcpy(data.val.str, "'");
							strcat(data.val.str, yylval_param.str);
							strcat(data.val.str, "'");
						}
						else if (yyresult == TYPECAST)
						{
							data.val.str = malloc(2 + 1);
							strcpy(data.val.str, "::");
						}
            else if (yyresult == DOT_DOT)
						{
							data.val.str = malloc(2 + 1);
							strcpy(data.val.str, "..");
						}
						else if (yyresult == COLON_EQUALS)
						{
							data.val.str = malloc(2 + 1);
							strcpy(data.val.str, ":=");
						}
            else if (yyresult == EQUALS_GREATER)
						{
							data.val.str = malloc(2 + 1);
							strcpy(data.val.str, "=>");
						}
            else if (yyresult == LESS_EQUALS)
						{
							data.val.str = malloc(2 + 1);
							strcpy(data.val.str, "<=");
						}
            else if (yyresult == GREATER_EQUALS)
						{
							data.val.str = malloc(2 + 1);
							strcpy(data.val.str, ">=");
						}
            else if (yyresult == NOT_EQUALS)
						{
							data.val.str = malloc(2 + 1);
							strcpy(data.val.str, "!=");
            }
            else if (yyresult == FIRST_P)
            {
              if (token1 == NULLS_LA)
              {
                data.val.str = malloc(11 + 1);
                strcpy(data.val.str, "nulls first");
                token1 = -1;
              }
            }
            else if (yyresult == LAST_P)
            {
              if (token1 == NULLS_LA)
              {
                data.val.str = malloc(10 + 1);
                strcpy(data.val.str, "nulls last");
                token1 = -1;
              }
            }
            else if (yyresult == TIME)
            {
              if (token1 == WITH_LA)
              {
                data.val.str = malloc(9 + 1);
                strcpy(data.val.str, "with time");
                token1 = -1;
              }
              else
              {
                data.val.str = strdup(yylval_param.str);
              }
            }
            else if (yyresult == ORDINALITY)
            {
              if (token1 == WITH_LA)
              {
                data.val.str = malloc(15 + 1);
                strcpy(data.val.str, "with ordinality");
                token1 = -1;
              }
            }
            else if (yyresult == NULLS_LA)
            {
              token1 = yyresult;
            }
            else if (yyresult == WITH_LA)
            {
              token1 = yyresult;
            }
            else
            {
              data.val.str = strdup(yylval_param.str);
              token1 = -1;
            }
					}
					break;
			}
			head = add(head, &data);
		}
	}
	while (yyresult);
	/* Clean up (release memory) */
	scanner_finish(vkscanner);

/*
  muntrace();
*/

	return head;
}

char *parse_rule_sql(const struct myScannerNode *head, const char *rulename, const char *targetschema, const char *tablename)
{
	char *result = NULL;
	bool to_token = false;
	struct myScannerNode *current, *prevnode = NULL, *nextnode = NULL;
	pgadmin_scanner_firstime = true;
	current = (struct myScannerNode *)head;
	if (current != NULL)
	{
		do
		{
			switch (current->data.token)
			{
				case ICONST:
				case PARAM:
				{
					char token[20];
					sprintf(token, "%d", current->data.val.ival);
					addToken(&result, token, true);
					break;
				}
				case RULE:
				{
					addToken(&result, current->data.val.str, true);
					nextnode = current->next;
					char *x = quote_identifier(rulename);
					addToken(&result, x, true);
					free(x);
					current = nextnode;
					break;
				}
				case TO:
				{
					addToken(&result, "to", true);
					to_token = true;
					break;
				}
				default:
				{
					if (current->data.token < 256)
					{
						char token[20];
						if (current->data.token == '.')
						{
							nextnode = current->next;
							if (to_token)
							{
								char *x = quote_qualified_identifier(targetschema, tablename);
								addToken(&result, x, true);
								free(x);
								current = nextnode;
								to_token = false;
							}
							else if (prevnode->data.token == IDENT && nextnode->data.token == IDENT)
							{
								char *x = quote_qualified_identifier(prevnode->data.val.str, nextnode->data.val.str);
								addToken(&result, x, true);
								free(x);
								current = nextnode;
							}
							else
							{
								addToken(&result, ".", true);
							}
						}
						else
						{
							sprintf(token, "%c", current->data.token);
							addToken(&result, token, true);
						}
					}
					else
					{
						if (current->data.token == IDENT)
						{
							nextnode = current->next;
							if (nextnode->data.token != '.')
							{
								char *x = quote_identifier(current->data.val.str);
								addToken(&result, x, true);
								free(x);
							}
						}
						else
						{
							addToken(&result, current->data.val.str, true);
						}
					}
					break;
				}
			}
			prevnode = current;
			current = current->next;
		}
		while (current != head);

	}
	return result;
}

char *parse_trigger_sql(const struct myScannerNode *head, const char *targetschema)
{
	char *result = NULL;
	bool on_token = false;
	struct myScannerNode *current, *prevnode = NULL, *nextnode = NULL;
	pgadmin_scanner_firstime = true;
	current = (struct myScannerNode *)head;
	if (current != NULL)
	{
		do
		{
			switch (current->data.token)
			{
				case ICONST:
				case PARAM:
				{
					char token[20];
					sprintf(token, "%d", current->data.val.ival);
					addToken(&result, token, true);
					break;
				}
				case TRIGGER:
				{
					addToken(&result, current->data.val.str, true);
					nextnode = current->next;
					char *x = quote_identifier(nextnode->data.val.str);
					addToken(&result, x, true);
					free(x);
					current = nextnode;
					break;
				}
				case ON:
				{
					addToken(&result, "on", true);
					on_token = true;
					break;
				}
				default:
				{
					if (current->data.token < 256)
					{
						char token[20];
						if (current->data.token == '.')
						{
							nextnode = current->next;
							if (on_token)
							{
								char *x = quote_qualified_identifier(targetschema, nextnode->data.val.str);
								addToken(&result, x, true);
								free(x);
								current = nextnode;
								on_token = false;
							}
							else if (prevnode->data.token == IDENT && nextnode->data.token == IDENT)
							{
								char *x = quote_qualified_identifier(prevnode->data.val.str, nextnode->data.val.str);
								addToken(&result, x, true);
								free(x);
								current = nextnode;
							}
							else
							{
								addToken(&result, ".", true);
							}
						}
						else
						{
							sprintf(token, "%c", current->data.token);
							addToken(&result, token, true);
						}
					}
					else
					{
						if (current->data.token == IDENT)
						{
							nextnode = current->next;
							if (nextnode->data.token != '.')
							{
								char *x = quote_identifier(current->data.val.str);
								addToken(&result, x, true);
								free(x);
							}
						}
						else
						{
							addToken(&result, current->data.val.str, true);
						}
					}
					break;
				}
			}
			prevnode = current;
			current = current->next;
		}
		while (current != head);

	}
	return result;
}

struct myScannerNode *get_all_commands(const struct myScannerNode *head)
{
	struct myScannerNode *newhead = NULL;
	char *result = NULL;
	struct myScannerNode *current, *prevnode = NULL;
	int offset = 0;
	pgadmin_scanner_firstime = true;
	current = (struct myScannerNode *)head;
	if (current != NULL)
	{
		do
		{
			switch (current->data.token)
			{
				case ICONST:
				case PARAM:
				{
					char token[20];
					sprintf(token, "%d", current->data.val.ival);
					addToken(&result, token, true);
					break;
				}
				default:
				{
					if (current->data.token < 256)
					{
						char token[20];
						if (current->data.token == ';')
						{
							struct myScannerData data;
							data.loc = offset;
							data.token = IDENT;
							addToken(&result, ";", false);
							data.val.str = strdup(result);
							newhead = add(newhead, &data);
							free(result);
							result = NULL;
							pgadmin_scanner_firstime = true;
							offset = current->data.loc + 1;
						}
						else if (current->data.token == '.' || current->data.token == ',' ||
								current->data.token == '(' || current->data.token == ')' ||
								current->data.token == '[' || current->data.token == ']' ||
								current->data.token == '{' || current->data.token == '}')
						{
							sprintf(token, "%c", current->data.token);
							addToken(&result, token, false);
						}
						else
						{
							sprintf(token, "%c", current->data.token);
							addToken(&result, token, true);
						}
					}
					else
					{
						if (current->data.token == IDENT)
						{
							char *x = quote_identifier(current->data.val.str);
							addToken(&result, x, (prevnode &&
									(prevnode->data.token == '.' || prevnode->data.token == '(')
									) ? false : true);
							free(x);
						}
						else
						{
							if (current->data.token == SCONST)
							{
								if (result)
								{
									int cnt = 0;
									int len = strlen(current->data.val.str);
									int i;
									for (i = 1; i < (len - 1); i++)
									{
										char c = current->data.val.str[i];
										if (c == '\'')
											cnt++;
									}
									if (cnt > 0)
									{
										int lenr = strlen(result);
										int lenx = len + cnt + cnt + 1;
										char *x = malloc(lenr + lenx);
										char *y = x;
										for (i = 0; i < lenr; i++)
										{
											*y++ = result[i];
										}
										*y++ = '\'';
										for (i = 1; i < (len - 1); i++)
										{
											char c = current->data.val.str[i];
											if (c == '\'')
												*y++ = c;
											*y++ = c;
										}
										*y++ = '\'';
										*y++ = 0;
										free(result);
										result = x;
									}
									else
										addToken(&result, current->data.val.str, true);
								}
								else
								{
									int cnt = 0;
									int len = strlen(current->data.val.str);
									int i;
									for (i = 1; i < (len - 1); i++)
									{
										char c = current->data.val.str[i];
										if (c == '\'')
											cnt++;
									}
									if (cnt > 0)
									{
										int lenx = len + cnt + cnt + 1;
										char *x = malloc(lenx);
										char *y = x;
										*y++ = '\'';
										for (i = 1; i < (len - 1); i++)
										{
											char c = current->data.val.str[i];
											if (c == '\'')
												*y++ = c;
											*y++ = c;
										}
										*y++ = '\'';
										*y++ = 0;
										result = x;
									}
									else
										addToken(&result, current->data.val.str, true);
								}
							}
							else
							{
								addToken(&result, current->data.val.str,
										 (current->data.token == TYPECAST) ? false : true);
							}
						}
					}
					break;
				}
			}
			prevnode = current;
			current = current->next;
		}
		while (current != head);
	}
	if (result)
	{
		struct myScannerData data;
		data.loc = offset;
		data.token = IDENT;
		data.val.str = strdup(result);
		newhead = add(newhead, &data);
		free(result);
	}
	return newhead;
}

int qualified_identifier_dot_pos(const struct myScannerNode *head)
{
	int pos = -1;
	struct myScannerNode *current, *prevnode = NULL, *nextnode = NULL;
	pgadmin_scanner_firstime = true;
	current = (struct myScannerNode *)head;
	if (current != NULL)
	{
		do
		{
			switch (current->data.token)
			{
				case ICONST:
				case PARAM:
					break;
				default:
				{
					if (current->data.token < 256)
					{
						if (current->data.token == '.')
						{
							nextnode = current->next;
							if (prevnode->data.token == IDENT && nextnode->data.token == IDENT)
							{
								pos = current->data.loc;
								return pos;
							}
						}
					}
					break;
				}
			}
			prevnode = current;
			current = current->next;
		}
		while (current != head);
	}
	return pos;
}
