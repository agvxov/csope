/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "src/egrep.y"

/*===========================================================================
 Copyright (c) 1998-2000, The Santa Cruz Operation
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 *Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.

 *Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 *Neither name of The Santa Cruz Operation nor the names of its contributors
 may be used to endorse or promote products derived from this software
 without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS
 IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT falseT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT falseT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 DAMAGE.
 =========================================================================*/

/*
 * egrep -- fine lines containing a regular expression
 */
#line 45 "src/egrep.y"

#include "global.h"
#include <ctype.h>
#include <stdio.h>

#include <setjmp.h>	/* jmp_buf */

#define nextch()	(*input++)

#define MAXLIN 350
#define MAXPOS 4000
#define NCHARS 256
#define NSTATES 128
#define FINAL -1
static	char gotofn[NSTATES][NCHARS];
static	int state[NSTATES];
static	char out[NSTATES];
static	unsigned int line;
static	int name[MAXLIN];
static	unsigned int left[MAXLIN];
static	unsigned int right[MAXLIN];
static	unsigned int parent[MAXLIN];
static	int foll[MAXLIN];
static	int positions[MAXPOS];
static	char chars[MAXLIN];
static	int nxtpos;
static	int nxtchar;
static	int tmpstat[MAXLIN];
static	int initstat[MAXLIN];
static	int xstate;
static	int count;
static	int icount;
static	char *input;
static	long lnum;
static	int iflag;
static	jmp_buf	env;	/* setjmp/longjmp buffer */
static	char *message;	/* error message */

/* Internal prototypes: */
static	void cfoll(int v);
static	void cgotofn(void);
static	int cstate(int v);
static	int member(int symb, int set, int torf);
static	int notin(int n);
static	void synerror(void);
static	void overflo(void);
static	void add(int *array, int n);
static	void follow(unsigned int v);
static	int unary(int x, int d);
static	int node(int x, int l, int r);
static	unsigned int cclenter(int x);
static	unsigned int enter(int x);

static int yylex(void);
static int yyerror(char *);

#line 165 "y.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif


/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    CHAR = 258,                    /* CHAR  */
    DOT = 259,                     /* DOT  */
    CCL = 260,                     /* CCL  */
    NCCL = 261,                    /* NCCL  */
    OR = 262,                      /* OR  */
    CAT = 263,                     /* CAT  */
    STAR = 264,                    /* STAR  */
    PLUS = 265,                    /* PLUS  */
    QUEST = 266                    /* QUEST  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define CHAR 258
#define DOT 259
#define CCL 260
#define NCCL 261
#define OR 262
#define CAT 263
#define STAR 264
#define PLUS 265
#define QUEST 266

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);



/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_CHAR = 3,                       /* CHAR  */
  YYSYMBOL_DOT = 4,                        /* DOT  */
  YYSYMBOL_CCL = 5,                        /* CCL  */
  YYSYMBOL_NCCL = 6,                       /* NCCL  */
  YYSYMBOL_OR = 7,                         /* OR  */
  YYSYMBOL_CAT = 8,                        /* CAT  */
  YYSYMBOL_STAR = 9,                       /* STAR  */
  YYSYMBOL_PLUS = 10,                      /* PLUS  */
  YYSYMBOL_QUEST = 11,                     /* QUEST  */
  YYSYMBOL_12_ = 12,                       /* '('  */
  YYSYMBOL_13_ = 13,                       /* ')'  */
  YYSYMBOL_YYACCEPT = 14,                  /* $accept  */
  YYSYMBOL_s = 15,                         /* s  */
  YYSYMBOL_t = 16,                         /* t  */
  YYSYMBOL_b = 17,                         /* b  */
  YYSYMBOL_r = 18                          /* r  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  6
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   108

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  14
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  5
/* YYNRULES -- Number of rules.  */
#define YYNRULES  18
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  25

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   266


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      12,    13,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,   103,   103,   108,   110,   112,   114,   118,   121,   123,
     125,   127,   131,   133,   135,   137,   139,   141,   143
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "CHAR", "DOT", "CCL",
  "NCCL", "OR", "CAT", "STAR", "PLUS", "QUEST", "'('", "')'", "$accept",
  "s", "t", "b", "r", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-5)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-14)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int8 yypact[] =
{
       2,    -5,     3,    -5,     1,     1,    -5,    -5,    -5,    -5,
      -5,    -5,     1,    47,    60,    72,    86,    -5,    -5,    -5,
      19,    96,     1,    -5,    33
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       7,     7,     0,     2,     0,     0,     1,    18,     8,     9,
      10,    11,     0,     0,     0,     0,     0,    14,    15,    16,
       0,     0,     0,    17,     0
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
      -5,    -5,    -5,     9,    -4
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     2,     3,     4,    20
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      13,    14,     7,     6,     8,     9,    10,    11,    15,     1,
       5,     0,    24,    12,     0,     0,     0,    24,    24,   -13,
       7,     0,   -13,   -13,   -13,   -13,   -13,     0,    17,    18,
      19,   -13,   -13,   -12,     7,     0,     8,     9,    10,    11,
     -12,     0,    17,    18,    19,    12,   -12,    -3,     7,     0,
       8,     9,    10,    11,    16,     0,    17,    18,    19,    12,
      -5,     7,     0,     8,     9,    10,    11,    21,     0,    17,
      18,    19,    12,     7,     0,     8,     9,    10,    11,    22,
       0,    17,    18,    19,    12,    23,    -6,     7,     0,     8,
       9,    10,    11,     0,     0,     0,    -4,     7,    12,     8,
       9,    10,    11,     0,     0,     0,     0,     0,    12
};

static const yytype_int8 yycheck[] =
{
       4,     5,     1,     0,     3,     4,     5,     6,    12,     7,
       1,    -1,    16,    12,    -1,    -1,    -1,    21,    22,     0,
       1,    -1,     3,     4,     5,     6,     7,    -1,     9,    10,
      11,    12,    13,     0,     1,    -1,     3,     4,     5,     6,
       7,    -1,     9,    10,    11,    12,    13,     0,     1,    -1,
       3,     4,     5,     6,     7,    -1,     9,    10,    11,    12,
       0,     1,    -1,     3,     4,     5,     6,     7,    -1,     9,
      10,    11,    12,     1,    -1,     3,     4,     5,     6,     7,
      -1,     9,    10,    11,    12,    13,     0,     1,    -1,     3,
       4,     5,     6,    -1,    -1,    -1,     0,     1,    12,     3,
       4,     5,     6,    -1,    -1,    -1,    -1,    -1,    12
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     7,    15,    16,    17,    17,     0,     1,     3,     4,
       5,     6,    12,    18,    18,    18,     7,     9,    10,    11,
      18,     7,     7,    13,    18
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    14,    15,    16,    16,    16,    16,    17,    18,    18,
      18,    18,    18,    18,    18,    18,    18,    18,    18
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     2,     4,     3,     3,     0,     1,     1,
       1,     1,     3,     2,     2,     2,     2,     3,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* s: t  */
#line 104 "src/egrep.y"
                { unary(FINAL, yyvsp[0]);
		  line--;
		}
#line 1248 "y.tab.c"
    break;

  case 3: /* t: b r  */
#line 109 "src/egrep.y"
                { yyval = node(CAT, yyvsp[-1], yyvsp[0]); }
#line 1254 "y.tab.c"
    break;

  case 4: /* t: OR b r OR  */
#line 111 "src/egrep.y"
                { yyval = node(CAT, yyvsp[-2], yyvsp[-1]); }
#line 1260 "y.tab.c"
    break;

  case 5: /* t: OR b r  */
#line 113 "src/egrep.y"
                { yyval = node(CAT, yyvsp[-1], yyvsp[0]); }
#line 1266 "y.tab.c"
    break;

  case 6: /* t: b r OR  */
#line 115 "src/egrep.y"
                { yyval = node(CAT, yyvsp[-2], yyvsp[-1]); }
#line 1272 "y.tab.c"
    break;

  case 7: /* b: %empty  */
#line 118 "src/egrep.y"
                { yyval = enter(DOT);
		   yyval = unary(STAR, yyval); }
#line 1279 "y.tab.c"
    break;

  case 8: /* r: CHAR  */
#line 122 "src/egrep.y"
                { yyval = enter(yyvsp[0]); }
#line 1285 "y.tab.c"
    break;

  case 9: /* r: DOT  */
#line 124 "src/egrep.y"
                { yyval = enter(DOT); }
#line 1291 "y.tab.c"
    break;

  case 10: /* r: CCL  */
#line 126 "src/egrep.y"
                { yyval = cclenter(CCL); }
#line 1297 "y.tab.c"
    break;

  case 11: /* r: NCCL  */
#line 128 "src/egrep.y"
                { yyval = cclenter(NCCL); }
#line 1303 "y.tab.c"
    break;

  case 12: /* r: r OR r  */
#line 132 "src/egrep.y"
                { yyval = node(OR, yyvsp[-2], yyvsp[0]); }
#line 1309 "y.tab.c"
    break;

  case 13: /* r: r r  */
#line 134 "src/egrep.y"
                { yyval = node(CAT, yyvsp[-1], yyvsp[0]); }
#line 1315 "y.tab.c"
    break;

  case 14: /* r: r STAR  */
#line 136 "src/egrep.y"
                { yyval = unary(STAR, yyvsp[-1]); }
#line 1321 "y.tab.c"
    break;

  case 15: /* r: r PLUS  */
#line 138 "src/egrep.y"
                { yyval = unary(PLUS, yyvsp[-1]); }
#line 1327 "y.tab.c"
    break;

  case 16: /* r: r QUEST  */
#line 140 "src/egrep.y"
                { yyval = unary(QUEST, yyvsp[-1]); }
#line 1333 "y.tab.c"
    break;

  case 17: /* r: '(' r ')'  */
#line 142 "src/egrep.y"
                { yyval = yyvsp[-1]; }
#line 1339 "y.tab.c"
    break;


#line 1343 "y.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 146 "src/egrep.y"

static int
yyerror(char *s)
{
	message = s;
	longjmp(env, 1);
	return 1;		/* silence a warning */
}

static int
yylex(void)
{
    int cclcnt, x;
    char c, d;

    switch(c = nextch()) {
    case '|':
    case '\n':
	return (OR);
    case '*':
	return (STAR);
    case '+':
	return (PLUS);
    case '?':
	return (QUEST);
    case '(':
    case ')':
	return (c);
    case '.':
	return (DOT);
    case '\0':
	return (0);
    case '[':
	x = CCL;
	cclcnt = 0;
	count = nxtchar++;
	if ((c = nextch()) == '^') {
	    x = NCCL;
	    c = nextch();
	}
	do {
	    if (c == '\0')
		synerror();
	    if (   (c == '-')
		&& (cclcnt > 0)
		&& (chars[nxtchar-1] != 0)
	       ) {
		if ((d = nextch()) != 0) {
		    c = chars[nxtchar-1];
		    while ((unsigned int)c < (unsigned int)d) {
			if (nxtchar >= MAXLIN)
			    overflo();
			chars[nxtchar++] = ++c;
			cclcnt++;
		    }
		    continue;
		} /* if() */
	    } /* if() */
	    if (nxtchar >= MAXLIN)
		overflo();
	    chars[nxtchar++] = c;
	    cclcnt++;
	} while ((c = nextch()) != ']');
	chars[count] = cclcnt;
	return (x);
    case '\\':
	if ((c = nextch()) == '\0')
	    synerror();
	yylval = c;
	return (CHAR);
    case '$':
    case '^':
	c = '\n';
	yylval = c;
	return (CHAR);
    default:
	yylval = c;
	return (CHAR);
    }
}

static void
synerror(void)
{
    yyerror("Syntax error");
}

static unsigned int
enter(int x)
{
    if(line >= MAXLIN)
	overflo();
    name[line] = x;
    left[line] = 0;
    right[line] = 0;
    return(line++);
}

static unsigned int
cclenter(int x)
{
    unsigned int linno;

    linno = enter(x);
    right[linno] = count;
    return (linno);
}

static int
node(int x, int l, int r)
{
    if(line >= MAXLIN)
	overflo();
    name[line] = x;
    left[line] = l;
    right[line] = r;
    parent[l] = line;
    parent[r] = line;
    return(line++);
}

static int
unary(int x, int d)
{
    if(line >= MAXLIN)
	overflo();
    name[line] = x;
    left[line] = d;
    right[line] = 0;
    parent[d] = line;
    return(line++);
}

static void
overflo(void)
{
    yyerror("internal table overflow");
}

static void
cfoll(int v)
{
    unsigned int i;

    if (left[v] == 0) {
	count = 0;
	for (i = 1; i <= line; i++)
	    tmpstat[i] = 0;
	follow(v);
	add(foll, v);
    } else if (right[v] == 0)
	cfoll(left[v]);
    else {
	cfoll(left[v]);
	cfoll(right[v]);
    }
}

static void
cgotofn(void)
{
    unsigned int i, n, s;
    int c, k;
    char symbol[NCHARS];
    unsigned int j, l, pc, pos;
    unsigned int nc;
    int curpos;
    unsigned int num, number, newpos;

    count = 0;
    for (n=3; n<=line; n++)
	tmpstat[n] = 0;
    if (cstate(line-1)==0) {
	tmpstat[line] = 1;
	count++;
	out[0] = 1;
    }
    for (n=3; n<=line; n++)
	initstat[n] = tmpstat[n];
    count--;		/*leave out position 1 */
    icount = count;
    tmpstat[1] = 0;
    add(state, 0);
    n = 0;
    for (s = 0; s <= n; s++)  {
	if (out[s] == 1)
	    continue;
	for (i = 0; i < NCHARS; i++)
	    symbol[i] = 0;
	num = positions[state[s]];
	count = icount;
	for (i = 3; i <= line; i++)
	    tmpstat[i] = initstat[i];
	pos = state[s] + 1;
	for (i = 0; i < num; i++) {
	    curpos = positions[pos];
	    if ((c = name[curpos]) >= 0) {
		if (c < NCHARS) {
		    symbol[c] = 1;
		} else if (c == DOT) {
		    for (k = 0; k < NCHARS; k++)
			if (k != '\n')
			    symbol[k] = 1;
		} else if (c == CCL) {
		    nc = chars[right[curpos]];
		    pc = right[curpos] + 1;
		    for (j = 0; j < nc; j++)
			symbol[(unsigned char)chars[pc++]] = 1;
		} else if (c == NCCL) {
		    nc = chars[right[curpos]];
		    for (j = 0; j < NCHARS; j++) {
			pc = right[curpos] + 1;
			for (l = 0; l < nc; l++)
			    if (j==(unsigned char)chars[pc++])
				goto cont;
			if (j != '\n')
			    symbol[j] = 1;
		    cont:
			;
		    }
		}
	    }
	    pos++;
	} /* for (i) */
	for (c=0; c<NCHARS; c++) {
	    if (symbol[c] == 1) {
		/* nextstate(s,c) */
		count = icount;
		for (i=3; i <= line; i++)
		    tmpstat[i] = initstat[i];
		pos = state[s] + 1;
		for (i=0; i<num; i++) {
		    curpos = positions[pos];
		    if ((k = name[curpos]) >= 0)
			if ((k == c)
			    || (k == DOT)
			    || (k == CCL && member(c, right[curpos], 1))
			    || (k == NCCL && member(c, right[curpos], 0))
			    ) {
			    number = positions[foll[curpos]];
			    newpos = foll[curpos] + 1;
			    for (j = 0; j < number; j++) {
				if (tmpstat[positions[newpos]] != 1) {
				    tmpstat[positions[newpos]] = 1;
				    count++;
				}
				newpos++;
			    }
			}
		    pos++;
		} /* end nextstate */
		if (notin(n)) {
		    if (n >= NSTATES)
			overflo();
		    add(state, ++n);
		    if (tmpstat[line] == 1)
			out[n] = 1;
		    gotofn[s][c] = n;
		} else {
		    gotofn[s][c] = xstate;
		}
	    } /* if (symbol) */
	} /* for(c) */
    } /* for(s) */
}

static int
cstate(int v)
{
	int b;
	if (left[v] == 0) {
		if (tmpstat[v] != 1) {
			tmpstat[v] = 1;
			count++;
		}
		return(1);
	}
	else if (right[v] == 0) {
		if (cstate(left[v]) == 0) return (0);
		else if (name[v] == PLUS) return (1);
		else return (0);
	}
	else if (name[v] == CAT) {
		if (cstate(left[v]) == 0 && cstate(right[v]) == 0) return (0);
		else return (1);
	}
	else { /* name[v] == OR */
		b = cstate(right[v]);
		if (cstate(left[v]) == 0 || b == 0) return (0);
		else return (1);
	}
}

static int
member(int symb, int set, int torf)
{
    unsigned int i, num, pos;

    num = chars[set];
    pos = set + 1;
    for (i = 0; i < num; i++)
	if (symb == (unsigned char)(chars[pos++]))
	    return (torf);
    return (!torf);
}

static int
notin(int n)
{
	int i, j, pos;
	for (i=0; i<=n; i++) {
		if (positions[state[i]] == count) {
			pos = state[i] + 1;
			for (j=0; j < count; j++)
				if (tmpstat[positions[pos++]] != 1) goto nxt;
			xstate = i;
			return (0);
		}
		nxt: ;
	}
	return (1);
}

static void
add(int *array, int n)
{
    unsigned int i;

    if (nxtpos + count > MAXPOS)
	overflo();
    array[n] = nxtpos;
    positions[nxtpos++] = count;
    for (i=3; i <= line; i++) {
	if (tmpstat[i] == 1) {
	    positions[nxtpos++] = i;
	}
    }
}

static void
follow(unsigned int v)
{
    unsigned int p;

    if (v == line)
	return;
    p = parent[v];
    switch(name[p]) {
    case STAR:
    case PLUS:	cstate(v);
	follow(p);
	return;

    case OR:
    case QUEST:	follow(p);
	return;

    case CAT:
	if (v == left[p]) {
	    if (cstate(right[p]) == 0) {
		follow(p);
		return;
	    }
	} else
	    follow(p);
	return;
    case FINAL:
	if (tmpstat[line] != 1) {
	    tmpstat[line] = 1;
	    count++;
	}
	return;
    }
}

char *
egrepinit(char *egreppat)
{
    /* initialize the global data */
    memset(gotofn, 0, sizeof(gotofn));
    memset(state, 0, sizeof(state));
    memset(out, 0, sizeof(out));
    line = 1;
    memset(name, 0, sizeof(name));
    memset(left, 0, sizeof(left));
    memset(right, 0, sizeof(right));
    memset(parent, 0, sizeof(parent));
    memset(foll, 0, sizeof(foll));
    memset(positions, 0, sizeof(positions));
    memset(chars, 0, sizeof(chars));
    nxtpos = 0;
    nxtchar = 0;
    memset(tmpstat, 0, sizeof(tmpstat));
    memset(initstat, 0, sizeof(initstat));
    xstate = 0;
    count = 0;
    icount = 0;
    input = egreppat;
    message = NULL;
    if (setjmp(env) == 0) {
	yyparse();
	cfoll(line-1);
	cgotofn();
    }
    return(message);
}

static char buf[2 * BUFSIZ];
static const char *buf_end = buf + (sizeof(buf) / sizeof(*buf));

static size_t read_next_chunk(char **p, FILE *fptr)
{
    if (*p <= (buf + BUFSIZ)) {
        /* bwlow the middle, so enough space left for one entire BUFSIZ */
	return fread(*p, sizeof(**p), BUFSIZ, fptr);
    } else if (*p == buf_end) {
        /* exactly at end ... wrap around and use lower half */
	*p = buf;
	return fread(*p, sizeof(**p), BUFSIZ, fptr);
    }
    /* somewhere in second half, so do a limited read */
    return fread(*p, sizeof(**p), buf_end - *p, fptr);
}

int
egrep(char *file, FILE *output, char *format)
{
    char *p;
    unsigned int cstat;
    int ccount;
    char *nlp;
    unsigned int istat;
    int in_line;
    FILE *fptr;

    if ((fptr = myfopen(file, "r")) == NULL)
	return(-1);

    lnum = 1;
    p = buf;
    nlp = p;
    ccount = read_next_chunk(&p, fptr);

    if (ccount <= 0) {
	fclose(fptr);
	return(0);
    }
    in_line = 1;
    istat = cstat = (unsigned int) gotofn[0]['\n'];
    if (out[cstat])
	goto found;
    for (;;) {
	if (!iflag) {
	    /* all input chars made positive */
	    cstat = (unsigned int) gotofn[cstat][(unsigned char)*p];
	} else {
	    /* for -i option*/
	    cstat = (unsigned int) gotofn[cstat][tolower((unsigned char)*p)];
        }
	if (out[cstat]) {
	found:
	    for(;;) {
		if (*p++ == '\n') {
		    in_line = 0;
		succeed:
		    fprintf(output, format, file, lnum);
		    if (p <= nlp) {
			while (nlp < buf_end)
			    putc(*nlp++, output);
			nlp = buf;
		    }
		    while (nlp < p)
			putc(*nlp++, output);
		    lnum++;
		    nlp = p;
		    if (out[cstat = istat] == 0)
			goto brk2;
		} /* if (p++ == \n) */
	    cfound:
		if (--ccount <= 0) {
		    ccount = read_next_chunk(&p, fptr);
		    if (ccount <= 0) {
			if (in_line) {
			    in_line = 0;
			    goto succeed;
			}
                        fclose(fptr);
                        return(0);
		    }
		} /* if(ccount <= 0) */
		in_line = 1;
	    } /* for(ever) */
	} /* if(out[cstat]) */

	if (*p++ == '\n') {
	    in_line = 0;
	    lnum++;
	    nlp = p;
	    if (out[(cstat=istat)])
		goto cfound;
	}
    brk2:
	if (--ccount <= 0) {
	    ccount = read_next_chunk(&p, fptr);
	    if (ccount <= 0)
		break;
	}
	in_line = 1;
    }
    fclose(fptr);
    return(0);
}

void
egrepcaseless(int i)
{
	iflag = i;	/* simulate "egrep -i" */
}
