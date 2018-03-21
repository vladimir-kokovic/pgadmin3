/*
 * File:   vladimir_kokovic.h
 * Created on January 29, 2012, 6:24 AM
 */

#ifndef PGADMINSCANNER_H
#define	PGADMINSCANNER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <setjmp.h>

#ifdef PGADMIN_SCANNER
#include "postgres.h"
#else
#include "server/postgres_fe.h"
#include "parser/keywords_pgadmin.h"
#undef _
#undef PACKAGE_NAME
#undef PACKAGE_BUGREPORT
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef Abs
#endif
#include "parser/scanner.h"

/* additional token for $foo$*/
#define DOLCONST 999

/* Entry points in parser/scan.l */

extern int base_yylex_95(core_YYSTYPE *lvalp, YYLTYPE *llocp, core_yyscan_t yyscanner);
extern core_yyscan_t scanner_init_95(jmp_buf env, const char *str,
		core_yy_extra_type *yyext,
			 const ScanKeyword *keywords,
			 int num_keywords);
extern void scanner_finish_95(core_yyscan_t yyscanner);
extern int core_yylex_95(core_YYSTYPE *lvalp, YYLTYPE *llocp, core_yyscan_t yyscanner);
extern int	scanner_errposition_95(int location, core_yyscan_t yyscanner);
extern void scanner_yyerror_95(const char *message, core_yyscan_t yyscanner);

#define printlist					printlist_95
#define destroylist					destroylist_95

#define core_yy_create_buffer		core_yy_create_buffer_95
#define core_yyrealloc				core_yyrealloc_95
#define core_yyrestart				core_yyrestart_95
#define core_yy_switch_to_buffer	core_yy_switch_to_buffer_95
#define core_yyalloc				core_yyalloc_95
#define core_yy_delete_buffer		core_yy_delete_buffer_95
#define core_yyfree					core_yyfree_95
#define core_yy_flush_buffer		core_yy_flush_buffer_95
#define core_yypush_buffer_state	core_yypush_buffer_state_95
#define core_yypop_buffer_state		core_yypop_buffer_state_95
#define core_yy_scan_buffer			core_yy_scan_buffer_95
#define core_yy_scan_string			core_yy_scan_string_95
#define core_yy_scan_bytes			core_yy_scan_bytes_95
#define core_yyget_extra			core_yyget_extra_95
#define core_yyget_lineno			core_yyget_lineno_95
#define core_yyget_column			core_yyget_column_95
#define core_yyget_in				core_yyget_in_95
#define core_yyget_out				core_yyget_out_95
#define core_yyget_leng				core_yyget_leng_95
#define core_yyget_text				core_yyget_text_95
#define core_yyset_extra			core_yyset_extra_95
#define core_yyset_lineno			core_yyset_lineno_95
#define core_yyset_column			core_yyset_column_95
#define core_yyset_in				core_yyset_in_95
#define core_yyset_out				core_yyset_out_95
#define core_yyget_debug			core_yyget_debug_95
#define core_yyset_debug			core_yyset_debug_95
#define core_yyget_lval				core_yyget_lval_95
#define core_yyset_lval				core_yyset_lval_95
#define core_yyget_lloc				core_yyget_lloc_95
#define core_yyset_lloc				core_yyset_lloc_95
#define core_yylex_init				core_yylex_init_95
#define core_yylex_init_extra		core_yylex_init_extra_95
#define core_yylex_destroy			core_yylex_destroy_95
/*
#define backslash_quote				backslash_quote_95
#define escape_string_warning		escape_string_warning_95
#define standard_conforming_strings standard_conforming_strings_95
*/
#define base_yylex					base_yylex_95
#define scanner_init				scanner_init_95
#define scanner_finish				scanner_finish_95
#define core_yylex					core_yylex_95
#define scanner_errposition			scanner_errposition_95
#define scanner_yyerror				scanner_yyerror_95

extern core_yyscan_t scanner_init(jmp_buf env, const char *str,
		core_yy_extra_type *yyext,
			 const ScanKeyword *keywords,
			 int num_keywords);
extern void scanner_finish(core_yyscan_t yyscanner);
extern int core_yylex(core_YYSTYPE *lvalp, YYLTYPE *llocp, core_yyscan_t yyscanner);
extern int	scanner_errposition(int location, core_yyscan_t yyscanner);
extern void scanner_yyerror(const char *message, core_yyscan_t yyscanner);

#undef ereport
#define ereport(elevel, rest)

#undef palloc
#define palloc malloc

#define pfree free

#define repalloc realloc

#undef pstrdup
#define pstrdup strdup

typedef enum
{
	BACKSLASH_QUOTE_OFF,
	BACKSLASH_QUOTE_ON,
	BACKSLASH_QUOTE_SAFE_ENCODING
}	BackslashQuoteType;

typedef enum JoinType
{
	/*
	 * The canonical kinds of joins according to the SQL JOIN syntax. Only
	 * these codes can appear in parser output (e.g., JoinExpr nodes).
	 */
	JOIN_INNER,					/* matching tuple pairs only */
	JOIN_LEFT,					/* pairs + unmatched LHS tuples */
	JOIN_FULL,					/* pairs + unmatched LHS + unmatched RHS */
	JOIN_RIGHT,					/* pairs + unmatched RHS tuples */

	/*
	 * Semijoins and anti-semijoins (as defined in relational theory) do not
	 * appear in the SQL JOIN syntax, but there are standard idioms for
	 * representing them (e.g., using EXISTS).	The planner recognizes these
	 * cases and converts them to joins.  So the planner and executor must
	 * support these codes.  NOTE: in JOIN_SEMI output, it is unspecified
	 * which matching RHS row is joined to.  In JOIN_ANTI output, the row is
	 * guaranteed to be null-extended.
	 */
	JOIN_SEMI,					/* 1 copy of each LHS row that has match(es) */
	JOIN_ANTI,					/* 1 copy of each LHS row that has no match */

	/*
	 * These codes are used internally in the planner, but are not supported
	 * by the executor (nor, indeed, by most of the planner).
	 */
	JOIN_UNIQUE_OUTER,			/* LHS path must be made unique */
	JOIN_UNIQUE_INNER			/* RHS path must be made unique */

	/*
	 * We might need additional join types someday.
	 */
} JoinType;

typedef enum DropBehavior
{
	DROP_RESTRICT,				/* drop fails if any dependent objects */
	DROP_CASCADE				/* remove dependent objects too */
} DropBehavior;

typedef enum OnCommitAction
{
	ONCOMMIT_NOOP,				/* No ON COMMIT clause (do nothing) */
	ONCOMMIT_PRESERVE_ROWS,		/* ON COMMIT PRESERVE ROWS (do nothing) */
	ONCOMMIT_DELETE_ROWS,		/* ON COMMIT DELETE ROWS */
	ONCOMMIT_DROP				/* ON COMMIT DROP */
} OnCommitAction;

typedef enum NodeTag
{
	T_Invalid = 0,

	/*
	 * TAGS FOR EXECUTOR NODES (execnodes.h)
	 */
	T_IndexInfo = 10,
	T_ExprContext,
	T_ProjectionInfo,
	T_JunkFilter,
	T_ResultRelInfo,
	T_EState,
	T_TupleTableSlot,

	/*
	 * TAGS FOR PLAN NODES (plannodes.h)
	 */
	T_Plan = 100,
	T_Result,
	T_ModifyTable,
	T_Append,
	T_MergeAppend,
	T_RecursiveUnion,
	T_BitmapAnd,
	T_BitmapOr,
	T_Scan,
	T_SeqScan,
	T_IndexScan,
	T_BitmapIndexScan,
	T_BitmapHeapScan,
	T_TidScan,
	T_SubqueryScan,
	T_FunctionScan,
	T_ValuesScan,
	T_CteScan,
	T_WorkTableScan,
	T_ForeignScan,
	T_FdwPlan,
	T_Join,
	T_NestLoop,
	T_MergeJoin,
	T_HashJoin,
	T_Material,
	T_Sort,
	T_Group,
	T_Agg,
	T_WindowAgg,
	T_Unique,
	T_Hash,
	T_SetOp,
	T_LockRows,
	T_Limit,
	/* these aren't subclasses of Plan: */
	T_NestLoopParam,
	T_PlanRowMark,
	T_PlanInvalItem,

	/*
	 * TAGS FOR PLAN STATE NODES (execnodes.h)
	 *
	 * These should correspond one-to-one with Plan node types.
	 */
	T_PlanState = 200,
	T_ResultState,
	T_ModifyTableState,
	T_AppendState,
	T_MergeAppendState,
	T_RecursiveUnionState,
	T_BitmapAndState,
	T_BitmapOrState,
	T_ScanState,
	T_SeqScanState,
	T_IndexScanState,
	T_BitmapIndexScanState,
	T_BitmapHeapScanState,
	T_TidScanState,
	T_SubqueryScanState,
	T_FunctionScanState,
	T_ValuesScanState,
	T_CteScanState,
	T_WorkTableScanState,
	T_ForeignScanState,
	T_JoinState,
	T_NestLoopState,
	T_MergeJoinState,
	T_HashJoinState,
	T_MaterialState,
	T_SortState,
	T_GroupState,
	T_AggState,
	T_WindowAggState,
	T_UniqueState,
	T_HashState,
	T_SetOpState,
	T_LockRowsState,
	T_LimitState,

	/*
	 * TAGS FOR PRIMITIVE NODES (primnodes.h)
	 */
	T_Alias = 300,
	T_RangeVar,
	T_Expr,
	T_Var,
	T_Const,
	T_Param,
	T_Aggref,
	T_WindowFunc,
	T_ArrayRef,
	T_FuncExpr,
	T_NamedArgExpr,
	T_OpExpr,
	T_DistinctExpr,
	T_NullIfExpr,
	T_ScalarArrayOpExpr,
	T_BoolExpr,
	T_SubLink,
	T_SubPlan,
	T_AlternativeSubPlan,
	T_FieldSelect,
	T_FieldStore,
	T_RelabelType,
	T_CoerceViaIO,
	T_ArrayCoerceExpr,
	T_ConvertRowtypeExpr,
	T_CollateExpr,
	T_CaseExpr,
	T_CaseWhen,
	T_CaseTestExpr,
	T_ArrayExpr,
	T_RowExpr,
	T_RowCompareExpr,
	T_CoalesceExpr,
	T_MinMaxExpr,
	T_XmlExpr,
	T_NullTest,
	T_BooleanTest,
	T_CoerceToDomain,
	T_CoerceToDomainValue,
	T_SetToDefault,
	T_CurrentOfExpr,
	T_TargetEntry,
	T_RangeTblRef,
	T_JoinExpr,
	T_FromExpr,
	T_IntoClause,

	/*
	 * TAGS FOR EXPRESSION STATE NODES (execnodes.h)
	 *
	 * These correspond (not always one-for-one) to primitive nodes derived
	 * from Expr.
	 */
	T_ExprState = 400,
	T_GenericExprState,
	T_AggrefExprState,
	T_WindowFuncExprState,
	T_ArrayRefExprState,
	T_FuncExprState,
	T_ScalarArrayOpExprState,
	T_BoolExprState,
	T_SubPlanState,
	T_AlternativeSubPlanState,
	T_FieldSelectState,
	T_FieldStoreState,
	T_CoerceViaIOState,
	T_ArrayCoerceExprState,
	T_ConvertRowtypeExprState,
	T_CaseExprState,
	T_CaseWhenState,
	T_ArrayExprState,
	T_RowExprState,
	T_RowCompareExprState,
	T_CoalesceExprState,
	T_MinMaxExprState,
	T_XmlExprState,
	T_NullTestState,
	T_CoerceToDomainState,
	T_DomainConstraintState,

	/*
	 * TAGS FOR PLANNER NODES (relation.h)
	 */
	T_PlannerInfo = 500,
	T_PlannerGlobal,
	T_RelOptInfo,
	T_IndexOptInfo,
	T_Path,
	T_IndexPath,
	T_BitmapHeapPath,
	T_BitmapAndPath,
	T_BitmapOrPath,
	T_NestPath,
	T_MergePath,
	T_HashPath,
	T_TidPath,
	T_ForeignPath,
	T_AppendPath,
	T_MergeAppendPath,
	T_ResultPath,
	T_MaterialPath,
	T_UniquePath,
	T_EquivalenceClass,
	T_EquivalenceMember,
	T_PathKey,
	T_RestrictInfo,
	T_InnerIndexscanInfo,
	T_PlaceHolderVar,
	T_SpecialJoinInfo,
	T_AppendRelInfo,
	T_PlaceHolderInfo,
	T_MinMaxAggInfo,
	T_PlannerParamItem,

	/*
	 * TAGS FOR MEMORY NODES (memnodes.h)
	 */
	T_MemoryContext = 600,
	T_AllocSetContext,

	/*
	 * TAGS FOR VALUE NODES (value.h)
	 */
	T_Value = 650,
	T_Integer,
	T_Float,
	T_String,
	T_BitString,
	T_Null,

	/*
	 * TAGS FOR LIST NODES (pg_list.h)
	 */
	T_List,
	T_IntList,
	T_OidList,

	/*
	 * TAGS FOR STATEMENT NODES (mostly in parsenodes.h)
	 */
	T_Query = 700,
	T_PlannedStmt,
	T_InsertStmt,
	T_DeleteStmt,
	T_UpdateStmt,
	T_SelectStmt,
	T_AlterTableStmt,
	T_AlterTableCmd,
	T_AlterDomainStmt,
	T_SetOperationStmt,
	T_GrantStmt,
	T_GrantRoleStmt,
	T_AlterDefaultPrivilegesStmt,
	T_ClosePortalStmt,
	T_ClusterStmt,
	T_CopyStmt,
	T_CreateStmt,
	T_DefineStmt,
	T_DropStmt,
	T_TruncateStmt,
	T_CommentStmt,
	T_FetchStmt,
	T_IndexStmt,
	T_CreateFunctionStmt,
	T_AlterFunctionStmt,
	T_RemoveFuncStmt,
	T_DoStmt,
	T_RenameStmt,
	T_RuleStmt,
	T_NotifyStmt,
	T_ListenStmt,
	T_UnlistenStmt,
	T_TransactionStmt,
	T_ViewStmt,
	T_LoadStmt,
	T_CreateDomainStmt,
	T_CreatedbStmt,
	T_DropdbStmt,
	T_VacuumStmt,
	T_ExplainStmt,
	T_CreateSeqStmt,
	T_AlterSeqStmt,
	T_VariableSetStmt,
	T_VariableShowStmt,
	T_DiscardStmt,
	T_CreateTrigStmt,
	T_DropPropertyStmt,
	T_CreatePLangStmt,
	T_DropPLangStmt,
	T_CreateRoleStmt,
	T_AlterRoleStmt,
	T_DropRoleStmt,
	T_LockStmt,
	T_ConstraintsSetStmt,
	T_ReindexStmt,
	T_CheckPointStmt,
	T_CreateSchemaStmt,
	T_AlterDatabaseStmt,
	T_AlterDatabaseSetStmt,
	T_AlterRoleSetStmt,
	T_CreateConversionStmt,
	T_CreateCastStmt,
	T_DropCastStmt,
	T_CreateOpClassStmt,
	T_CreateOpFamilyStmt,
	T_AlterOpFamilyStmt,
	T_RemoveOpClassStmt,
	T_RemoveOpFamilyStmt,
	T_PrepareStmt,
	T_ExecuteStmt,
	T_DeallocateStmt,
	T_DeclareCursorStmt,
	T_CreateTableSpaceStmt,
	T_DropTableSpaceStmt,
	T_AlterObjectSchemaStmt,
	T_AlterOwnerStmt,
	T_DropOwnedStmt,
	T_ReassignOwnedStmt,
	T_CompositeTypeStmt,
	T_CreateEnumStmt,
	T_AlterEnumStmt,
	T_AlterTSDictionaryStmt,
	T_AlterTSConfigurationStmt,
	T_CreateFdwStmt,
	T_AlterFdwStmt,
	T_DropFdwStmt,
	T_CreateForeignServerStmt,
	T_AlterForeignServerStmt,
	T_DropForeignServerStmt,
	T_CreateUserMappingStmt,
	T_AlterUserMappingStmt,
	T_DropUserMappingStmt,
	T_AlterTableSpaceOptionsStmt,
	T_SecLabelStmt,
	T_CreateForeignTableStmt,
	T_CreateExtensionStmt,
	T_AlterExtensionStmt,
	T_AlterExtensionContentsStmt,

	/*
	 * TAGS FOR PARSE TREE NODES (parsenodes.h)
	 */
	T_A_Expr = 900,
	T_ColumnRef,
	T_ParamRef,
	T_A_Const,
	T_FuncCall,
	T_A_Star,
	T_A_Indices,
	T_A_Indirection,
	T_A_ArrayExpr,
	T_ResTarget,
	T_TypeCast,
	T_CollateClause,
	T_SortBy,
	T_WindowDef,
	T_RangeSubselect,
	T_RangeFunction,
	T_TypeName,
	T_ColumnDef,
	T_IndexElem,
	T_Constraint,
	T_DefElem,
	T_RangeTblEntry,
	T_SortGroupClause,
	T_WindowClause,
	T_PrivGrantee,
	T_FuncWithArgs,
	T_AccessPriv,
	T_CreateOpClassItem,
	T_InhRelation,
	T_FunctionParameter,
	T_LockingClause,
	T_RowMarkClause,
	T_XmlSerialize,
	T_WithClause,
	T_CommonTableExpr,

	/*
	 * TAGS FOR REPLICATION GRAMMAR PARSE NODES (replnodes.h)
	 */
	T_IdentifySystemCmd,
	T_BaseBackupCmd,
	T_StartReplicationCmd,

	/*
	 * TAGS FOR RANDOM OTHER STUFF
	 *
	 * These are objects that aren't part of parse/plan/execute node tree
	 * structures, but we give them NodeTags anyway for identification
	 * purposes (usually because they are involved in APIs where we want to
	 * pass multiple object types through the same pointer).
	 */
	T_TriggerData = 950,		/* in commands/trigger.h */
	T_ReturnSetInfo,			/* in nodes/execnodes.h */
	T_WindowObjectData,			/* private in nodeWindowAgg.c */
	T_TIDBitmap,				/* in nodes/tidbitmap.h */
	T_InlineCodeBlock,			/* in nodes/parsenodes.h */
	T_FdwRoutine				/* in foreign/fdwapi.h */
} NodeTag;

/*
 * The first field of a node of any type is guaranteed to be the NodeTag.
 * Hence the type of any node can be gotten by casting it to Node. Declaring
 * a variable to be of Node * (instead of void *) can also facilitate
 * debugging.
 */
typedef struct Node
{
	NodeTag		type;
} Node;

#define nodeTag(nodeptr)		(((Node*)(nodeptr))->type)

typedef struct ListCell ListCell;

typedef struct List
{
	NodeTag		type;			/* T_List, T_IntList, or T_OidList */
	int			length;
	ListCell   *head;
	ListCell   *tail;
} List;

struct ListCell
{
	union
	{
		void	   *ptr_value;
		int			int_value;
		Oid			oid_value;
	}			data;
	ListCell   *next;
};

typedef struct Value
{
	NodeTag		type;			/* tag appropriately (eg. T_String) */
	union ValUnion
	{
		long		ival;		/* machine integer */
		char	   *str;		/* string */
	}			val;
} Value;

typedef enum ObjectType
{
	OBJECT_AGGREGATE,
	OBJECT_ATTRIBUTE,			/* type's attribute, when distinct from column */
	OBJECT_CAST,
	OBJECT_COLUMN,
	OBJECT_CONSTRAINT,
	OBJECT_COLLATION,
	OBJECT_CONVERSION,
	OBJECT_DATABASE,
	OBJECT_DOMAIN,
	OBJECT_EXTENSION,
	OBJECT_FDW,
	OBJECT_FOREIGN_SERVER,
	OBJECT_FOREIGN_TABLE,
	OBJECT_FUNCTION,
	OBJECT_INDEX,
	OBJECT_LANGUAGE,
	OBJECT_LARGEOBJECT,
	OBJECT_OPCLASS,
	OBJECT_OPERATOR,
	OBJECT_OPFAMILY,
	OBJECT_ROLE,
	OBJECT_RULE,
	OBJECT_SCHEMA,
	OBJECT_SEQUENCE,
	OBJECT_TABLE,
	OBJECT_TABLESPACE,
	OBJECT_TRIGGER,
	OBJECT_TSCONFIGURATION,
	OBJECT_TSDICTIONARY,
	OBJECT_TSPARSER,
	OBJECT_TSTEMPLATE,
	OBJECT_TYPE,
	OBJECT_VIEW
} ObjectType;

typedef struct TypeName
{
	NodeTag		type;
	List	   *names;			/* qualified name (list of Value strings) */
	Oid			typeOid;		/* type identified by OID */
	bool		setof;			/* is a set? */
	bool		pct_type;		/* %TYPE specified? */
	List	   *typmods;		/* type modifier expression(s) */
	int32		typemod;		/* prespecified type modifier */
	List	   *arrayBounds;	/* array bounds */
	int			location;		/* token location, or -1 if unknown */
} TypeName;

typedef enum FunctionParameterMode
{
	/* the assigned enum values appear in pg_proc, don't change 'em! */
	FUNC_PARAM_IN = 'i',		/* input only */
	FUNC_PARAM_OUT = 'o',		/* output only */
	FUNC_PARAM_INOUT = 'b',		/* both */
	FUNC_PARAM_VARIADIC = 'v',	/* variadic (always input) */
	FUNC_PARAM_TABLE = 't'		/* table function output column */
} FunctionParameterMode;

typedef struct FunctionParameter
{
	NodeTag		type;
	char	   *name;			/* parameter name, or NULL if not given */
	TypeName   *argType;		/* TypeName for parameter type */
	FunctionParameterMode mode; /* IN/OUT/etc */
	Node	   *defexpr;		/* raw default expr, or NULL if not given */
} FunctionParameter;

typedef struct FuncWithArgs
{
	NodeTag		type;
	List	   *funcname;		/* qualified name of function */
	List	   *funcargs;		/* list of Typename nodes */
} FuncWithArgs;

typedef enum DefElemAction
{
	DEFELEM_UNSPEC,				/* no action given */
	DEFELEM_SET,
	DEFELEM_ADD,
	DEFELEM_DROP
} DefElemAction;

typedef struct DefElem
{
	NodeTag		type;
	char	   *defnamespace;	/* NULL if unqualified name */
	char	   *defname;
	Node	   *arg;			/* a (Value *) or a (TypeName *) */
	DefElemAction defaction;	/* unspecified action, or SET/ADD/DROP */
} DefElem;

/* Sort ordering options for ORDER BY and CREATE INDEX */
typedef enum SortByDir
{
	SORTBY_DEFAULT,
	SORTBY_ASC,
	SORTBY_DESC,
	SORTBY_USING				/* not allowed in CREATE INDEX ... */
} SortByDir;

typedef enum SortByNulls
{
	SORTBY_NULLS_DEFAULT,
	SORTBY_NULLS_FIRST,
	SORTBY_NULLS_LAST
} SortByNulls;

typedef struct SortBy
{
	NodeTag		type;
	Node	   *node;			/* expression to sort on */
	SortByDir	sortby_dir;		/* ASC/DESC/USING/default */
	SortByNulls sortby_nulls;	/* NULLS FIRST/LAST */
	List	   *useOp;			/* name of op to use, if SORTBY_USING */
	int			location;		/* operator location, or -1 if none/unknown */
} SortBy;

typedef struct WindowDef
{
	NodeTag		type;
	char	   *name;			/* window's own name */
	char	   *refname;		/* referenced window name, if any */
	List	   *partitionClause;	/* PARTITION BY expression list */
	List	   *orderClause;	/* ORDER BY (list of SortBy) */
	int			frameOptions;	/* frame_clause options, see below */
	Node	   *startOffset;	/* expression for starting bound, if any */
	Node	   *endOffset;		/* expression for ending bound, if any */
	int			location;		/* parse location, or -1 if none/unknown */
} WindowDef;

typedef struct Alias
{
	NodeTag		type;
	char	   *aliasname;		/* aliased rel name (never qualified) */
	List	   *colnames;		/* optional list of column aliases */
} Alias;

typedef struct JoinExpr
{
	NodeTag		type;
	JoinType	jointype;		/* type of join */
	bool		isNatural;		/* Natural join? Will need to shape table */
	Node	   *larg;			/* left subtree */
	Node	   *rarg;			/* right subtree */
	List	   *usingClause;	/* USING clause, if any (list of String) */
	Node	   *quals;			/* qualifiers on join, if any */
	Alias	   *alias;			/* user-written alias clause, if any */
	int			rtindex;		/* RT index assigned for join, or 0 */
} JoinExpr;

typedef struct IndexElem
{
	NodeTag		type;
	char	   *name;			/* name of attribute to index, or NULL */
	Node	   *expr;			/* expression to index, or NULL */
	char	   *indexcolname;	/* name for index column; NULL = default */
	List	   *collation;		/* name of collation; NIL = default */
	List	   *opclass;		/* name of desired opclass; NIL = default */
	SortByDir	ordering;		/* ASC/DESC/default */
	SortByNulls nulls_ordering; /* FIRST/LAST/default */
} IndexElem;

typedef enum InhOption
{
	INH_NO,						/* Do NOT scan child tables */
	INH_YES,					/* DO scan child tables */
	INH_DEFAULT					/* Use current SQL_inheritance option */
} InhOption;

typedef struct RangeVar
{
	NodeTag		type;
	char	   *catalogname;	/* the catalog (database) name, or NULL */
	char	   *schemaname;		/* the schema name, or NULL */
	char	   *relname;		/* the relation/sequence name */
	InhOption	inhOpt;			/* expand rel by inheritance? recursively act
								 * on children? */
	char		relpersistence; /* see RELPERSISTENCE_* in pg_class.h */
	Alias	   *alias;			/* table alias & optional column aliases */
	int			location;		/* token location, or -1 if unknown */
} RangeVar;

typedef struct IntoClause
{
	NodeTag		type;

	RangeVar   *rel;			/* target relation name */
	List	   *colNames;		/* column names to assign, or NIL */
	List	   *options;		/* options from WITH clause */
	OnCommitAction onCommit;	/* what do we do at COMMIT? */
	char	   *tableSpaceName; /* table space to use, or NULL */
} IntoClause;

typedef struct WithClause
{
	NodeTag		type;
	List	   *ctes;			/* list of CommonTableExprs */
	bool		recursive;		/* true = WITH RECURSIVE */
	int			location;		/* token location, or -1 if unknown */
} WithClause;

typedef struct A_Indices
{
	NodeTag		type;
	Node	   *lidx;			/* NULL if it's a single subscript */
	Node	   *uidx;
} A_Indices;

typedef struct ResTarget
{
	NodeTag		type;
	char	   *name;			/* column name or NULL */
	List	   *indirection;	/* subscripts, field names, and '*', or NIL */
	Node	   *val;			/* the value expression to compute or assign */
	int			location;		/* token location, or -1 if unknown */
} ResTarget;

typedef struct AccessPriv
{
	NodeTag		type;
	char	   *priv_name;		/* string name of privilege */
	List	   *cols;			/* list of Value strings */
} AccessPriv;

typedef struct InsertStmt
{
	NodeTag		type;
	RangeVar   *relation;		/* relation to insert into */
	List	   *cols;			/* optional: names of the target columns */
	Node	   *selectStmt;		/* the source SELECT/VALUES, or NULL */
	List	   *returningList;	/* list of expressions to return */
	WithClause *withClause;		/* WITH clause */
} InsertStmt;

typedef enum
{
	VAR_SET_VALUE,				/* SET var = value */
	VAR_SET_DEFAULT,			/* SET var TO DEFAULT */
	VAR_SET_CURRENT,			/* SET var FROM CURRENT */
	VAR_SET_MULTI,				/* special case for SET TRANSACTION ... */
	VAR_RESET,					/* RESET var */
	VAR_RESET_ALL				/* RESET ALL */
} VariableSetKind;

typedef struct VariableSetStmt
{
	NodeTag		type;
	VariableSetKind kind;
	char	   *name;			/* variable to be set */
	List	   *args;			/* List of A_Const nodes */
	bool		is_local;		/* SET LOCAL? */
} VariableSetStmt;

typedef enum GrantObjectType
{
	ACL_OBJECT_RELATION,		/* table, view */
	ACL_OBJECT_SEQUENCE,		/* sequence */
	ACL_OBJECT_DATABASE,		/* database */
	ACL_OBJECT_FUNCTION,		/* function */
	ACL_OBJECT_LANGUAGE,		/* procedural language */
	ACL_OBJECT_NAMESPACE,		/* namespace */
	ACL_OBJECT_TABLESPACE		/* tablespace */
} GrantObjectType;

/* 8.3 This is only used internally in gram.y. */
typedef struct PrivTarget
{
	NodeTag		type;
	GrantObjectType objtype;
	List	   *objs;
} PrivTarget;

/* 8.1 */
typedef enum ContainsOids
{
	MUST_HAVE_OIDS,				/* WITH OIDS explicitely specified */
	MUST_NOT_HAVE_OIDS,			/* WITHOUT OIDS explicitely specified */
	DEFAULT_OIDS				/* neither specified; use the default, which
								 * is the value of the default_with_oids GUC
								 * var */
} ContainsOids;

/* 7.4
 * FastList is an optimization for building large lists.  The conventional
 * way to build a list is repeated lappend() operations, but that is O(N^2)
 * in the number of list items, which gets tedious for large lists.
 *
 * Note: there are some hacks in gram.y that rely on the head pointer (the
 * value-as-list) being the first field.
 */
typedef struct FastList
{
	List	   *head;
	List	   *tail;
} FastList;

/* 7.4
 * ColumnRef - specifies a reference to a column, or possibly a whole tuple
 *
 *		The "fields" list must be nonempty; its last component may be "*"
 *		instead of a field name.  Subscripts are optional.
 */
typedef struct ColumnRef
{
	NodeTag		type;
	List	   *fields;			/* field names (list of Value strings) */
	List	   *indirection;	/* subscripts (list of A_Indices) */
} ColumnRef;

/* 7.3
 * SortGroupBy - for ORDER BY clause
 */
typedef struct SortGroupBy
{
	NodeTag		type;
	List	   *useOp;			/* operator to use */
	Node	   *node;			/* Expression  */
} SortGroupBy;

/* 7.2
 * ParamNo - specifies a parameter reference
 */
typedef struct ParamNo
{
	NodeTag		type;
	int			number;			/* the number of the parameter */
	TypeName   *typename_vk;	/* the typecast */
	List	   *indirection;	/* array references */
} ParamNo;

/* 7.2
 * Attr -
 *	  specifies an Attribute (ie. a Column); could have nested dots or
 *	  array references.
 *
 */
typedef struct Attr
{
	NodeTag		type;
	char	   *relname;		/* name of relation (can be "*") */
	ParamNo    *paramNo;		/* or a parameter */
	List	   *attrs;			/* attributes (possibly nested); list of
								 * Values (strings) */
	List	   *indirection;	/* array refs (list of A_Indices') */
} Attr;

/* 7.2
 * Ident -
 *	  an identifier (could be an attribute or a relation name). Depending
 *	  on the context at transformStmt time, the identifier is treated as
 *	  either a relation name (in which case, isRel will be set) or an
 *	  attribute (in which case, it will be transformed into an Attr).
 */
typedef struct Ident
{
	NodeTag		type;
	char	   *name;			/* its name */
	List	   *indirection;	/* array references */
	bool		isRel;			/* is this a relation or a column? */
} Ident;

/* ----------------------
 *		Create Version Statement 7.2
 * ----------------------
 */
typedef struct VersionStmt
{
	NodeTag		type;
	char	   *relname;		/* the new relation */
	int			direction;		/* FORWARD | BACKWARD */
	char	   *fromRelname;	/* relation to create a version */
	char	   *date;			/* date of the snapshot */
} VersionStmt;

/* ----------------------
 *		Create {Operator|Type|Aggregate} Statement 7.2
 * ----------------------
 */
typedef struct DefineStmt
{
	NodeTag		type;
	int			defType;		/* OPERATOR|P_TYPE|AGGREGATE */
	char	   *defname;
	List	   *definition;		/* a list of DefElem */
} DefineStmt;

/* 7.2
 * CmdType -
 *	  enums for type of operation represented by a Query
 *
 * ??? could have put this in parsenodes.h but many files not in the
 *	  optimizer also need this...
 */
typedef enum CmdType
{
	CMD_UNKNOWN,
	CMD_SELECT,					/* select stmt (formerly retrieve) */
	CMD_UPDATE,					/* update stmt (formerly replace) */
	CMD_INSERT,					/* insert stmt (formerly append) */
	CMD_DELETE,
	CMD_UTILITY,				/* cmds like create, destroy, copy,
								 * vacuum, etc. */
	CMD_NOTHING					/* dummy command for instead nothing rules
								 * with qual */
} CmdType;

/* ----------------------
 *		Create Rule Statement 7.2
 * ----------------------
 */
typedef struct RuleStmt
{
	NodeTag		type;
	char	   *rulename;		/* name of the rule */
	Node	   *whereClause;	/* qualifications */
	CmdType		event;			/* RETRIEVE */
	struct Attr *object;		/* object affected */
	bool		instead;		/* is a 'do instead'? */
	List	   *actions;		/* the action statements */
} RuleStmt;

/*
 * The YY_EXTRA data that a flex scanner allows us to pass around.	Private
 * state needed for raw parsing/lexing goes here.
 */
typedef struct base_yy_extra_type
{
	/*
	 * Fields used by the core scanner.
	 */
	core_yy_extra_type core_yy_extra;

	/*
	 * State variables for base_yylex().
	 */
	bool		have_lookahead; /* is lookahead info valid? */
	int			lookahead_token;	/* one-token lookahead */
	core_YYSTYPE lookahead_yylval;		/* yylval for lookahead token */
	YYLTYPE		lookahead_yylloc;		/* yylloc for lookahead token */

	/*
	 * State variables that belong to the grammar.
	 */
	List	   *parsetree;		/* final parse result is delivered here */
} base_yy_extra_type;

/*
 * In principle we should use yyget_extra() to fetch the yyextra field
 * from a yyscanner struct.  However, flex always puts that field first,
 * and this is sufficiently performance-critical to make it seem worth
 * cheating a bit to use an inline macro.
 */
#define pg_yyget_extra(yyscanner) (*((base_yy_extra_type **) (yyscanner)))

extern bool errstart(int elevel, const char *filename, int lineno, const char *funcname, const char *domain);
extern int errmsg(const char *fmt,...);
extern int errdetail(const char *fmt,...);
extern int errcode(int sqlerrcode);
extern void errfinish(int dummy,...);
extern int errhint(const char *fmt,...);
extern int errmsg_internal(const char *fmt,...);
extern int errposition(int cursorpos);
extern void elog_start(const char *filename, int lineno, const char *funcname);
extern void elog_finish(int elevel, const char *fmt,...);
extern
#if (PG_VERSION_NUM / 100) < 902
int
#else
void
#endif
ExceptionalCondition(const char *conditionName, const char *errorType, const char *fileName, int lineNumber)
#if (PG_VERSION_NUM / 100) >= 902
#ifdef __GNUC__		/* GNU cc */
 __attribute__((noreturn))
#endif
#endif
;
extern void write_stderr(const char *fmt,...);

extern int base_yylex(core_YYSTYPE *lvalp, YYLTYPE *llocp, core_yyscan_t yyscanner);

struct myScannerData {
	int loc;
	int token;
	core_YYSTYPE val;
};

struct myScannerNode {
	struct myScannerData data;
	struct myScannerNode *next;
};

extern struct myScannerNode* add(struct myScannerNode *head, const struct myScannerData *data);
extern void printlist(const struct myScannerNode *head);
extern void destroylist(struct myScannerNode *head);

extern bool pgadmin_scanner_firstime;

extern struct myScannerNode *scan_SQL_command(jmp_buf env, int version, const char *command);
extern void addToken(char **result, const char *token, bool appendblank);
extern char *quote_identifier(const char *ident);
extern char *quote_qualified_identifier(const char *qualifier, const char *ident);

extern struct myScannerNode *get_all_commands(const struct myScannerNode *head);
extern char *parse_rule_sql(
	const struct myScannerNode *head, const char *rulename, const char *targetschema, const char *tablename);
extern char *parse_trigger_sql(const struct myScannerNode *head, const char *targetschema);
extern int qualified_identifier_dot_pos(const struct myScannerNode *head);

extern char *vk_sprintf(const char *format, ...);
extern char *vk_vsprintf(const char *format, va_list argPtr);

extern void set_pgadmin_scanner_last_error(const char *format, ...);
extern void set_pgadmin_scanner_last_error_va(const char *format, va_list argPtr);

extern char *pgadmin_scanner_last_error;

extern struct myScannerNode *scanSqlCpp(const char *command);
extern struct myScannerNode *getAllCommandsCpp(const struct myScannerNode *head);
extern char *parseRuleCpp(
	const struct myScannerNode *head, const char *rulename, const char *targetschema, const char *tablename);
extern char *parseTriggerCpp(const struct myScannerNode *head, const char *targetschema);
extern void destroylistCpp(struct myScannerNode *head);
extern int qualifiedIdentifierDotPosCpp(const char *identifier);

#ifdef	__cplusplus
}
#endif

#endif	/* PGADMINSCANNER_H */
