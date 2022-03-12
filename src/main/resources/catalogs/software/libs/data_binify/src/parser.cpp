// A Bison parser, made by GNU Bison 3.0.5.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015, 2018 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.


// First part of user declarations.

#line 37 "parser.cpp" // lalr1.cc:406

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

#include "parser.hpp"

// User implementation prologue.

#line 51 "parser.cpp" // lalr1.cc:414
// Unqualified %code blocks.
#line 87 "parser.y" // lalr1.cc:415

    #include "scanner.h"

    #undef yylex
    #define yylex scanner->lex

#line 60 "parser.cpp" // lalr1.cc:415


#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (/*CONSTCOND*/ false)
# endif


// Suppress unused-variable warnings by "using" E.
#define YYUSE(E) ((void) (E))

// Enable debugging if requested.
#if YYDEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << std::endl;                  \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yystack_print_ ();                \
  } while (false)

#else // !YYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YYUSE (Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void> (0)
# define YY_STACK_PRINT()                static_cast<void> (0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)


namespace yy {
#line 146 "parser.cpp" // lalr1.cc:481

  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  parser::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr = "";
        char const *yyp = yystr;

        for (;;)
          switch (*++yyp)
            {
            case '\'':
            case ',':
              goto do_not_strip_quotes;

            case '\\':
              if (*++yyp != '\\')
                goto do_not_strip_quotes;
              // Fall through.
            default:
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }


  /// Build a parser object.
  parser::parser (yy::scanner* scanner_yyarg, binify::ParserOutput* cb_yyarg)
    :
#if YYDEBUG
      yydebug_ (false),
      yycdebug_ (&std::cerr),
#endif
      scanner (scanner_yyarg),
      cb (cb_yyarg)
  {}

  parser::~parser ()
  {}


  /*---------------.
  | Symbol types.  |
  `---------------*/

  parser::syntax_error::syntax_error (const location_type& l, const std::string& m)
    : std::runtime_error (m)
    , location (l)
  {}

  // basic_symbol.
  template <typename Base>
  parser::basic_symbol<Base>::basic_symbol ()
    : value ()
  {}

  template <typename Base>
  parser::basic_symbol<Base>::basic_symbol (const basic_symbol& other)
    : Base (other)
    , value ()
    , location (other.location)
  {
    switch (other.type_get ())
    {
      case 7: // Align
      case 8: // Blank
      case 9: // LittleEndian
      case 10: // BigEndian
      case 11: // AddressLen
      case 12: // Fixup
      case 13: // Type
      case 14: // AllowNan
      case 15: // AllowInfinity
        value.copy< binify::ast::Statement > (other.value);
        break;

      case 16: // U8
      case 17: // U16
      case 18: // U32
      case 19: // U64
      case 20: // S8
      case 21: // S16
      case 22: // S32
      case 23: // S64
      case 24: // Float
      case 25: // Double
      case 42: // type
        value.copy< binify::ast::Type > (other.value);
        break;

      case 4: // "double"
      case 45: // fpexp
        value.copy< double > (other.value);
        break;

      case 3: // "int64"
      case 44: // intexp
        value.copy< int64_t > (other.value);
        break;

      case 5: // "string"
      case 6: // "identifier"
        value.copy< std::string > (other.value);
        break;

      default:
        break;
    }

  }

  template <typename Base>
  parser::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, const semantic_type& v, const location_type& l)
    : Base (t)
    , value ()
    , location (l)
  {
    (void) v;
    switch (this->type_get ())
    {
      case 7: // Align
      case 8: // Blank
      case 9: // LittleEndian
      case 10: // BigEndian
      case 11: // AddressLen
      case 12: // Fixup
      case 13: // Type
      case 14: // AllowNan
      case 15: // AllowInfinity
        value.copy< binify::ast::Statement > (v);
        break;

      case 16: // U8
      case 17: // U16
      case 18: // U32
      case 19: // U64
      case 20: // S8
      case 21: // S16
      case 22: // S32
      case 23: // S64
      case 24: // Float
      case 25: // Double
      case 42: // type
        value.copy< binify::ast::Type > (v);
        break;

      case 4: // "double"
      case 45: // fpexp
        value.copy< double > (v);
        break;

      case 3: // "int64"
      case 44: // intexp
        value.copy< int64_t > (v);
        break;

      case 5: // "string"
      case 6: // "identifier"
        value.copy< std::string > (v);
        break;

      default:
        break;
    }
}


  // Implementation of basic_symbol constructor for each type.

  template <typename Base>
  parser::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, const location_type& l)
    : Base (t)
    , value ()
    , location (l)
  {}

  template <typename Base>
  parser::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, const binify::ast::Statement v, const location_type& l)
    : Base (t)
    , value (v)
    , location (l)
  {}

  template <typename Base>
  parser::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, const binify::ast::Type v, const location_type& l)
    : Base (t)
    , value (v)
    , location (l)
  {}

  template <typename Base>
  parser::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, const double v, const location_type& l)
    : Base (t)
    , value (v)
    , location (l)
  {}

  template <typename Base>
  parser::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, const int64_t v, const location_type& l)
    : Base (t)
    , value (v)
    , location (l)
  {}

  template <typename Base>
  parser::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, const std::string v, const location_type& l)
    : Base (t)
    , value (v)
    , location (l)
  {}


  template <typename Base>
  parser::basic_symbol<Base>::~basic_symbol ()
  {
    clear ();
  }

  template <typename Base>
  void
  parser::basic_symbol<Base>::clear ()
  {
    // User destructor.
    symbol_number_type yytype = this->type_get ();
    basic_symbol<Base>& yysym = *this;
    (void) yysym;
    switch (yytype)
    {
   default:
      break;
    }

    // Type destructor.
  switch (yytype)
    {
      case 7: // Align
      case 8: // Blank
      case 9: // LittleEndian
      case 10: // BigEndian
      case 11: // AddressLen
      case 12: // Fixup
      case 13: // Type
      case 14: // AllowNan
      case 15: // AllowInfinity
        value.template destroy< binify::ast::Statement > ();
        break;

      case 16: // U8
      case 17: // U16
      case 18: // U32
      case 19: // U64
      case 20: // S8
      case 21: // S16
      case 22: // S32
      case 23: // S64
      case 24: // Float
      case 25: // Double
      case 42: // type
        value.template destroy< binify::ast::Type > ();
        break;

      case 4: // "double"
      case 45: // fpexp
        value.template destroy< double > ();
        break;

      case 3: // "int64"
      case 44: // intexp
        value.template destroy< int64_t > ();
        break;

      case 5: // "string"
      case 6: // "identifier"
        value.template destroy< std::string > ();
        break;

      default:
        break;
    }

    Base::clear ();
  }

  template <typename Base>
  bool
  parser::basic_symbol<Base>::empty () const
  {
    return Base::type_get () == empty_symbol;
  }

  template <typename Base>
  void
  parser::basic_symbol<Base>::move (basic_symbol& s)
  {
    super_type::move (s);
    switch (this->type_get ())
    {
      case 7: // Align
      case 8: // Blank
      case 9: // LittleEndian
      case 10: // BigEndian
      case 11: // AddressLen
      case 12: // Fixup
      case 13: // Type
      case 14: // AllowNan
      case 15: // AllowInfinity
        value.move< binify::ast::Statement > (s.value);
        break;

      case 16: // U8
      case 17: // U16
      case 18: // U32
      case 19: // U64
      case 20: // S8
      case 21: // S16
      case 22: // S32
      case 23: // S64
      case 24: // Float
      case 25: // Double
      case 42: // type
        value.move< binify::ast::Type > (s.value);
        break;

      case 4: // "double"
      case 45: // fpexp
        value.move< double > (s.value);
        break;

      case 3: // "int64"
      case 44: // intexp
        value.move< int64_t > (s.value);
        break;

      case 5: // "string"
      case 6: // "identifier"
        value.move< std::string > (s.value);
        break;

      default:
        break;
    }

    location = s.location;
  }

  // by_type.
  parser::by_type::by_type ()
    : type (empty_symbol)
  {}

  parser::by_type::by_type (const by_type& other)
    : type (other.type)
  {}

  parser::by_type::by_type (token_type t)
    : type (yytranslate_ (t))
  {}

  void
  parser::by_type::clear ()
  {
    type = empty_symbol;
  }

  void
  parser::by_type::move (by_type& that)
  {
    type = that.type;
    that.clear ();
  }

  int
  parser::by_type::type_get () const
  {
    return type;
  }
  // Implementation of make_symbol for each symbol type.
  parser::symbol_type
  parser::make_END (const location_type& l)
  {
    return symbol_type (token::T_END, l);
  }

  parser::symbol_type
  parser::make_INTNUM (const int64_t& v, const location_type& l)
  {
    return symbol_type (token::T_INTNUM, v, l);
  }

  parser::symbol_type
  parser::make_FPNUM (const double& v, const location_type& l)
  {
    return symbol_type (token::T_FPNUM, v, l);
  }

  parser::symbol_type
  parser::make_STRING (const std::string& v, const location_type& l)
  {
    return symbol_type (token::T_STRING, v, l);
  }

  parser::symbol_type
  parser::make_IDENTIFIER (const std::string& v, const location_type& l)
  {
    return symbol_type (token::T_IDENTIFIER, v, l);
  }

  parser::symbol_type
  parser::make_Align (const binify::ast::Statement& v, const location_type& l)
  {
    return symbol_type (token::T_Align, v, l);
  }

  parser::symbol_type
  parser::make_Blank (const binify::ast::Statement& v, const location_type& l)
  {
    return symbol_type (token::T_Blank, v, l);
  }

  parser::symbol_type
  parser::make_LittleEndian (const binify::ast::Statement& v, const location_type& l)
  {
    return symbol_type (token::T_LittleEndian, v, l);
  }

  parser::symbol_type
  parser::make_BigEndian (const binify::ast::Statement& v, const location_type& l)
  {
    return symbol_type (token::T_BigEndian, v, l);
  }

  parser::symbol_type
  parser::make_AddressLen (const binify::ast::Statement& v, const location_type& l)
  {
    return symbol_type (token::T_AddressLen, v, l);
  }

  parser::symbol_type
  parser::make_Fixup (const binify::ast::Statement& v, const location_type& l)
  {
    return symbol_type (token::T_Fixup, v, l);
  }

  parser::symbol_type
  parser::make_Type (const binify::ast::Statement& v, const location_type& l)
  {
    return symbol_type (token::T_Type, v, l);
  }

  parser::symbol_type
  parser::make_AllowNan (const binify::ast::Statement& v, const location_type& l)
  {
    return symbol_type (token::T_AllowNan, v, l);
  }

  parser::symbol_type
  parser::make_AllowInfinity (const binify::ast::Statement& v, const location_type& l)
  {
    return symbol_type (token::T_AllowInfinity, v, l);
  }

  parser::symbol_type
  parser::make_U8 (const binify::ast::Type& v, const location_type& l)
  {
    return symbol_type (token::T_U8, v, l);
  }

  parser::symbol_type
  parser::make_U16 (const binify::ast::Type& v, const location_type& l)
  {
    return symbol_type (token::T_U16, v, l);
  }

  parser::symbol_type
  parser::make_U32 (const binify::ast::Type& v, const location_type& l)
  {
    return symbol_type (token::T_U32, v, l);
  }

  parser::symbol_type
  parser::make_U64 (const binify::ast::Type& v, const location_type& l)
  {
    return symbol_type (token::T_U64, v, l);
  }

  parser::symbol_type
  parser::make_S8 (const binify::ast::Type& v, const location_type& l)
  {
    return symbol_type (token::T_S8, v, l);
  }

  parser::symbol_type
  parser::make_S16 (const binify::ast::Type& v, const location_type& l)
  {
    return symbol_type (token::T_S16, v, l);
  }

  parser::symbol_type
  parser::make_S32 (const binify::ast::Type& v, const location_type& l)
  {
    return symbol_type (token::T_S32, v, l);
  }

  parser::symbol_type
  parser::make_S64 (const binify::ast::Type& v, const location_type& l)
  {
    return symbol_type (token::T_S64, v, l);
  }

  parser::symbol_type
  parser::make_Float (const binify::ast::Type& v, const location_type& l)
  {
    return symbol_type (token::T_Float, v, l);
  }

  parser::symbol_type
  parser::make_Double (const binify::ast::Type& v, const location_type& l)
  {
    return symbol_type (token::T_Double, v, l);
  }

  parser::symbol_type
  parser::make_NEG (const location_type& l)
  {
    return symbol_type (token::T_NEG, l);
  }



  // by_state.
  parser::by_state::by_state ()
    : state (empty_state)
  {}

  parser::by_state::by_state (const by_state& other)
    : state (other.state)
  {}

  void
  parser::by_state::clear ()
  {
    state = empty_state;
  }

  void
  parser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  parser::by_state::by_state (state_type s)
    : state (s)
  {}

  parser::symbol_number_type
  parser::by_state::type_get () const
  {
    if (state == empty_state)
      return empty_symbol;
    else
      return yystos_[state];
  }

  parser::stack_symbol_type::stack_symbol_type ()
  {}

  parser::stack_symbol_type::stack_symbol_type (const stack_symbol_type& that)
    : super_type (that.state, that.location)
  {
    switch (that.type_get ())
    {
      case 7: // Align
      case 8: // Blank
      case 9: // LittleEndian
      case 10: // BigEndian
      case 11: // AddressLen
      case 12: // Fixup
      case 13: // Type
      case 14: // AllowNan
      case 15: // AllowInfinity
        value.copy< binify::ast::Statement > (that.value);
        break;

      case 16: // U8
      case 17: // U16
      case 18: // U32
      case 19: // U64
      case 20: // S8
      case 21: // S16
      case 22: // S32
      case 23: // S64
      case 24: // Float
      case 25: // Double
      case 42: // type
        value.copy< binify::ast::Type > (that.value);
        break;

      case 4: // "double"
      case 45: // fpexp
        value.copy< double > (that.value);
        break;

      case 3: // "int64"
      case 44: // intexp
        value.copy< int64_t > (that.value);
        break;

      case 5: // "string"
      case 6: // "identifier"
        value.copy< std::string > (that.value);
        break;

      default:
        break;
    }

  }

  parser::stack_symbol_type::stack_symbol_type (state_type s, symbol_type& that)
    : super_type (s, that.location)
  {
    switch (that.type_get ())
    {
      case 7: // Align
      case 8: // Blank
      case 9: // LittleEndian
      case 10: // BigEndian
      case 11: // AddressLen
      case 12: // Fixup
      case 13: // Type
      case 14: // AllowNan
      case 15: // AllowInfinity
        value.move< binify::ast::Statement > (that.value);
        break;

      case 16: // U8
      case 17: // U16
      case 18: // U32
      case 19: // U64
      case 20: // S8
      case 21: // S16
      case 22: // S32
      case 23: // S64
      case 24: // Float
      case 25: // Double
      case 42: // type
        value.move< binify::ast::Type > (that.value);
        break;

      case 4: // "double"
      case 45: // fpexp
        value.move< double > (that.value);
        break;

      case 3: // "int64"
      case 44: // intexp
        value.move< int64_t > (that.value);
        break;

      case 5: // "string"
      case 6: // "identifier"
        value.move< std::string > (that.value);
        break;

      default:
        break;
    }

    // that is emptied.
    that.type = empty_symbol;
  }

  parser::stack_symbol_type&
  parser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    switch (that.type_get ())
    {
      case 7: // Align
      case 8: // Blank
      case 9: // LittleEndian
      case 10: // BigEndian
      case 11: // AddressLen
      case 12: // Fixup
      case 13: // Type
      case 14: // AllowNan
      case 15: // AllowInfinity
        value.copy< binify::ast::Statement > (that.value);
        break;

      case 16: // U8
      case 17: // U16
      case 18: // U32
      case 19: // U64
      case 20: // S8
      case 21: // S16
      case 22: // S32
      case 23: // S64
      case 24: // Float
      case 25: // Double
      case 42: // type
        value.copy< binify::ast::Type > (that.value);
        break;

      case 4: // "double"
      case 45: // fpexp
        value.copy< double > (that.value);
        break;

      case 3: // "int64"
      case 44: // intexp
        value.copy< int64_t > (that.value);
        break;

      case 5: // "string"
      case 6: // "identifier"
        value.copy< std::string > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    return *this;
  }


  template <typename Base>
  void
  parser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);
  }

#if YYDEBUG
  template <typename Base>
  void
  parser::yy_print_ (std::ostream& yyo,
                                     const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YYUSE (yyoutput);
    symbol_number_type yytype = yysym.type_get ();
    // Avoid a (spurious) G++ 4.8 warning about "array subscript is
    // below array bounds".
    if (yysym.empty ())
      std::abort ();
    yyo << (yytype < yyntokens_ ? "token" : "nterm")
        << ' ' << yytname_[yytype] << " ("
        << yysym.location << ": ";
    YYUSE (yytype);
    yyo << ')';
  }
#endif

  void
  parser::yypush_ (const char* m, state_type s, symbol_type& sym)
  {
    stack_symbol_type t (s, sym);
    yypush_ (m, t);
  }

  void
  parser::yypush_ (const char* m, stack_symbol_type& s)
  {
    if (m)
      YY_SYMBOL_PRINT (m, s);
    yystack_.push (s);
  }

  void
  parser::yypop_ (unsigned n)
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  parser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  parser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  parser::debug_level_type
  parser::debug_level () const
  {
    return yydebug_;
  }

  void
  parser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // YYDEBUG

  parser::state_type
  parser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - yyntokens_] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - yyntokens_];
  }

  bool
  parser::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  parser::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  parser::parse ()
  {
    // State.
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The locations where the error started and ended.
    stack_symbol_type yyerror_range[3];

    /// The return value of parse ().
    int yyresult;

    // FIXME: This shoud be completely indented.  It is not yet to
    // avoid gratuitous conflicts when merging into the master branch.
    try
      {
    YYCDEBUG << "Starting parse" << std::endl;


    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, yyla);

    // A new symbol was pushed on the stack.
  yynewstate:
    YYCDEBUG << "Entering state " << yystack_[0].state << std::endl;

    // Accept?
    if (yystack_[0].state == yyfinal_)
      goto yyacceptlab;

    goto yybackup;

    // Backup.
  yybackup:

    // Try to take a decision without lookahead.
    yyn = yypact_[yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token: ";
        try
          {
            yyla.type = yytranslate_ (yylex (&yyla.value, &yyla.location));
          }
        catch (const syntax_error& yyexc)
          {
            error (yyexc);
            goto yyerrlab1;
          }
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.type_get ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.type_get ())
      goto yydefault;

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", yyn, yyla);
    goto yynewstate;

  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;

  /*-----------------------------.
  | yyreduce -- Do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_ (yystack_[yylen].state, yyr1_[yyn]);
      /* Variants are always initialized to an empty instance of the
         correct type. The default '$$ = $1' action is NOT applied
         when using variants.  */
      switch (yyr1_[yyn])
    {
      case 7: // Align
      case 8: // Blank
      case 9: // LittleEndian
      case 10: // BigEndian
      case 11: // AddressLen
      case 12: // Fixup
      case 13: // Type
      case 14: // AllowNan
      case 15: // AllowInfinity
        yylhs.value.build< binify::ast::Statement > ();
        break;

      case 16: // U8
      case 17: // U16
      case 18: // U32
      case 19: // U64
      case 20: // S8
      case 21: // S16
      case 22: // S32
      case 23: // S64
      case 24: // Float
      case 25: // Double
      case 42: // type
        yylhs.value.build< binify::ast::Type > ();
        break;

      case 4: // "double"
      case 45: // fpexp
        yylhs.value.build< double > ();
        break;

      case 3: // "int64"
      case 44: // intexp
        yylhs.value.build< int64_t > ();
        break;

      case 5: // "string"
      case 6: // "identifier"
        yylhs.value.build< std::string > ();
        break;

      default:
        break;
    }


      // Default location.
      {
        slice<stack_symbol_type, stack_type> slice (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, slice, yylen);
        yyerror_range[1].location = yylhs.location;
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
      try
        {
          switch (yyn)
            {
  case 8:
#line 111 "parser.y" // lalr1.cc:856
    { yylhs.value.as< binify::ast::Type > ()=binify::ast::Type::U8; }
#line 1165 "parser.cpp" // lalr1.cc:856
    break;

  case 9:
#line 112 "parser.y" // lalr1.cc:856
    { yylhs.value.as< binify::ast::Type > ()=binify::ast::Type::U16; }
#line 1171 "parser.cpp" // lalr1.cc:856
    break;

  case 10:
#line 113 "parser.y" // lalr1.cc:856
    { yylhs.value.as< binify::ast::Type > ()=binify::ast::Type::U32; }
#line 1177 "parser.cpp" // lalr1.cc:856
    break;

  case 11:
#line 114 "parser.y" // lalr1.cc:856
    { yylhs.value.as< binify::ast::Type > ()=binify::ast::Type::U64; }
#line 1183 "parser.cpp" // lalr1.cc:856
    break;

  case 12:
#line 115 "parser.y" // lalr1.cc:856
    { yylhs.value.as< binify::ast::Type > ()=binify::ast::Type::S8; }
#line 1189 "parser.cpp" // lalr1.cc:856
    break;

  case 13:
#line 116 "parser.y" // lalr1.cc:856
    { yylhs.value.as< binify::ast::Type > ()=binify::ast::Type::S16; }
#line 1195 "parser.cpp" // lalr1.cc:856
    break;

  case 14:
#line 117 "parser.y" // lalr1.cc:856
    { yylhs.value.as< binify::ast::Type > ()=binify::ast::Type::S32; }
#line 1201 "parser.cpp" // lalr1.cc:856
    break;

  case 15:
#line 118 "parser.y" // lalr1.cc:856
    { yylhs.value.as< binify::ast::Type > ()=binify::ast::Type::S64; }
#line 1207 "parser.cpp" // lalr1.cc:856
    break;

  case 16:
#line 119 "parser.y" // lalr1.cc:856
    { yylhs.value.as< binify::ast::Type > ()=binify::ast::Type::Float; }
#line 1213 "parser.cpp" // lalr1.cc:856
    break;

  case 17:
#line 120 "parser.y" // lalr1.cc:856
    { yylhs.value.as< binify::ast::Type > ()=binify::ast::Type::Double; }
#line 1219 "parser.cpp" // lalr1.cc:856
    break;

  case 18:
#line 123 "parser.y" // lalr1.cc:856
    { cb->U8( yystack_[0].value.as< int64_t > () ); }
#line 1225 "parser.cpp" // lalr1.cc:856
    break;

  case 19:
#line 124 "parser.y" // lalr1.cc:856
    { cb->U16( yystack_[0].value.as< int64_t > () ); }
#line 1231 "parser.cpp" // lalr1.cc:856
    break;

  case 20:
#line 125 "parser.y" // lalr1.cc:856
    { cb->U32( yystack_[0].value.as< int64_t > () ); }
#line 1237 "parser.cpp" // lalr1.cc:856
    break;

  case 21:
#line 126 "parser.y" // lalr1.cc:856
    { cb->U64( yystack_[0].value.as< int64_t > () ); }
#line 1243 "parser.cpp" // lalr1.cc:856
    break;

  case 22:
#line 127 "parser.y" // lalr1.cc:856
    { cb->S8( yystack_[0].value.as< int64_t > () ); }
#line 1249 "parser.cpp" // lalr1.cc:856
    break;

  case 23:
#line 128 "parser.y" // lalr1.cc:856
    { cb->S16( yystack_[0].value.as< int64_t > () ); }
#line 1255 "parser.cpp" // lalr1.cc:856
    break;

  case 24:
#line 129 "parser.y" // lalr1.cc:856
    { cb->S32( yystack_[0].value.as< int64_t > () ); }
#line 1261 "parser.cpp" // lalr1.cc:856
    break;

  case 25:
#line 130 "parser.y" // lalr1.cc:856
    { cb->S64( yystack_[0].value.as< int64_t > () ); }
#line 1267 "parser.cpp" // lalr1.cc:856
    break;

  case 26:
#line 131 "parser.y" // lalr1.cc:856
    { cb->Float( yystack_[0].value.as< double > () ); }
#line 1273 "parser.cpp" // lalr1.cc:856
    break;

  case 27:
#line 132 "parser.y" // lalr1.cc:856
    { cb->Double( yystack_[0].value.as< double > () ); }
#line 1279 "parser.cpp" // lalr1.cc:856
    break;

  case 28:
#line 133 "parser.y" // lalr1.cc:856
    { cb->IntDefault( yystack_[0].value.as< int64_t > () ); }
#line 1285 "parser.cpp" // lalr1.cc:856
    break;

  case 29:
#line 134 "parser.y" // lalr1.cc:856
    { cb->FloatDefault( yystack_[0].value.as< double > () ); }
#line 1291 "parser.cpp" // lalr1.cc:856
    break;

  case 30:
#line 135 "parser.y" // lalr1.cc:856
    { cb->String( yystack_[0].value.as< std::string > () ); }
#line 1297 "parser.cpp" // lalr1.cc:856
    break;

  case 31:
#line 136 "parser.y" // lalr1.cc:856
    { cb->Align( yystack_[0].value.as< int64_t > () ); }
#line 1303 "parser.cpp" // lalr1.cc:856
    break;

  case 32:
#line 137 "parser.y" // lalr1.cc:856
    { cb->Blank( yystack_[0].value.as< int64_t > () ); }
#line 1309 "parser.cpp" // lalr1.cc:856
    break;

  case 33:
#line 138 "parser.y" // lalr1.cc:856
    { cb->AllowNan( yystack_[0].value.as< int64_t > () ); }
#line 1315 "parser.cpp" // lalr1.cc:856
    break;

  case 34:
#line 139 "parser.y" // lalr1.cc:856
    { cb->AllowInfinity( yystack_[0].value.as< int64_t > () ); }
#line 1321 "parser.cpp" // lalr1.cc:856
    break;

  case 35:
#line 140 "parser.y" // lalr1.cc:856
    { cb->SetAddressLen( yystack_[0].value.as< int64_t > () ); }
#line 1327 "parser.cpp" // lalr1.cc:856
    break;

  case 36:
#line 141 "parser.y" // lalr1.cc:856
    { cb->Fixup( yystack_[0].value.as< int64_t > () ); }
#line 1333 "parser.cpp" // lalr1.cc:856
    break;

  case 37:
#line 142 "parser.y" // lalr1.cc:856
    { cb->SetDefaultType( yystack_[0].value.as< binify::ast::Type > () ); }
#line 1339 "parser.cpp" // lalr1.cc:856
    break;

  case 38:
#line 143 "parser.y" // lalr1.cc:856
    { cb->SetByteOrder( binify::ast::Statement::LittleEndian ); }
#line 1345 "parser.cpp" // lalr1.cc:856
    break;

  case 39:
#line 144 "parser.y" // lalr1.cc:856
    { cb->SetByteOrder( binify::ast::Statement::BigEndian ); }
#line 1351 "parser.cpp" // lalr1.cc:856
    break;

  case 40:
#line 145 "parser.y" // lalr1.cc:856
    { cb->SetPass0Symbol( yystack_[2].value.as< std::string > (), yystack_[0].value.as< int64_t > () ); }
#line 1357 "parser.cpp" // lalr1.cc:856
    break;

  case 41:
#line 146 "parser.y" // lalr1.cc:856
    { cb->SetSymbolToOffset( yystack_[1].value.as< std::string > () ); }
#line 1363 "parser.cpp" // lalr1.cc:856
    break;

  case 42:
#line 147 "parser.y" // lalr1.cc:856
    { cb->SetSymbol( yystack_[2].value.as< std::string > (), yystack_[0].value.as< int64_t > () ); }
#line 1369 "parser.cpp" // lalr1.cc:856
    break;

  case 43:
#line 151 "parser.y" // lalr1.cc:856
    { yylhs.value.as< int64_t > ()=yystack_[0].value.as< int64_t > (); }
#line 1375 "parser.cpp" // lalr1.cc:856
    break;

  case 44:
#line 152 "parser.y" // lalr1.cc:856
    { yylhs.value.as< int64_t > ()=yystack_[2].value.as< int64_t > ()-yystack_[0].value.as< int64_t > (); }
#line 1381 "parser.cpp" // lalr1.cc:856
    break;

  case 45:
#line 153 "parser.y" // lalr1.cc:856
    { yylhs.value.as< int64_t > ()=yystack_[2].value.as< int64_t > ()+yystack_[0].value.as< int64_t > (); }
#line 1387 "parser.cpp" // lalr1.cc:856
    break;

  case 46:
#line 154 "parser.y" // lalr1.cc:856
    { yylhs.value.as< int64_t > ()=yystack_[1].value.as< int64_t > (); }
#line 1393 "parser.cpp" // lalr1.cc:856
    break;

  case 47:
#line 155 "parser.y" // lalr1.cc:856
    { yylhs.value.as< int64_t > ()=yystack_[2].value.as< int64_t > ()|yystack_[0].value.as< int64_t > (); }
#line 1399 "parser.cpp" // lalr1.cc:856
    break;

  case 48:
#line 156 "parser.y" // lalr1.cc:856
    { yylhs.value.as< int64_t > () = cb->LookupSymbol( yystack_[0].value.as< std::string > () ); }
#line 1405 "parser.cpp" // lalr1.cc:856
    break;

  case 49:
#line 159 "parser.y" // lalr1.cc:856
    { yylhs.value.as< double > ()=yystack_[0].value.as< double > (); }
#line 1411 "parser.cpp" // lalr1.cc:856
    break;

  case 50:
#line 160 "parser.y" // lalr1.cc:856
    { yylhs.value.as< double > ()=yystack_[2].value.as< double > ()+yystack_[0].value.as< double > (); }
#line 1417 "parser.cpp" // lalr1.cc:856
    break;

  case 51:
#line 161 "parser.y" // lalr1.cc:856
    { yylhs.value.as< double > ()=yystack_[2].value.as< double > ()-yystack_[0].value.as< double > (); }
#line 1423 "parser.cpp" // lalr1.cc:856
    break;

  case 52:
#line 162 "parser.y" // lalr1.cc:856
    { yylhs.value.as< double > ()=yystack_[2].value.as< double > ()*yystack_[0].value.as< double > (); }
#line 1429 "parser.cpp" // lalr1.cc:856
    break;

  case 53:
#line 163 "parser.y" // lalr1.cc:856
    { yylhs.value.as< double > ()=yystack_[2].value.as< double > ()/yystack_[0].value.as< double > (); }
#line 1435 "parser.cpp" // lalr1.cc:856
    break;

  case 54:
#line 164 "parser.y" // lalr1.cc:856
    { yylhs.value.as< double > ()=-yystack_[0].value.as< double > (); }
#line 1441 "parser.cpp" // lalr1.cc:856
    break;

  case 55:
#line 165 "parser.y" // lalr1.cc:856
    { yylhs.value.as< double > ()=yystack_[1].value.as< double > (); }
#line 1447 "parser.cpp" // lalr1.cc:856
    break;


#line 1451 "parser.cpp" // lalr1.cc:856
            default:
              break;
            }
        }
      catch (const syntax_error& yyexc)
        {
          error (yyexc);
          YYERROR;
        }
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;
      YY_STACK_PRINT ();

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, yylhs);
    }
    goto yynewstate;

  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        error (yyla.location, yysyntax_error_ (yystack_[0].state, yyla));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.type_get () == yyeof_)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:

    /* Pacify compilers like GCC when the user code never invokes
       YYERROR and the label yyerrorlab therefore never appears in user
       code.  */
    if (false)
      goto yyerrorlab;
    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    goto yyerrlab1;

  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    {
      stack_symbol_type error_token;
      for (;;)
        {
          yyn = yypact_[yystack_[0].state];
          if (!yy_pact_value_is_default_ (yyn))
            {
              yyn += yyterror_;
              if (0 <= yyn && yyn <= yylast_ && yycheck_[yyn] == yyterror_)
                {
                  yyn = yytable_[yyn];
                  if (0 < yyn)
                    break;
                }
            }

          // Pop the current state because it cannot handle the error token.
          if (yystack_.size () == 1)
            YYABORT;

          yyerror_range[1].location = yystack_[0].location;
          yy_destroy_ ("Error: popping", yystack_[0]);
          yypop_ ();
          YY_STACK_PRINT ();
        }

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      // Shift the error token.
      error_token.state = yyn;
      yypush_ ("Shifting", error_token);
    }
    goto yynewstate;

    // Accept.
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;

    // Abort.
  yyabortlab:
    yyresult = 1;
    goto yyreturn;

  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack"
                 << std::endl;
        // Do not try to display the values of the reclaimed symbols,
        // as their printer might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
  }

  void
  parser::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what ());
  }

  // Generate an error message.
  std::string
  parser::yysyntax_error_ (state_type yystate, const symbol_type& yyla) const
  {
    // Number of reported tokens (one for the "unexpected", one per
    // "expected").
    size_t yycount = 0;
    // Its maximum.
    enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
    // Arguments of yyformat.
    char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];

    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yyla) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yyla.  (However, yyla is currently not documented for users.)
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state
         merging (from LALR or IELR) and default reductions corrupt the
         expected token list.  However, the list is correct for
         canonical LR with one exception: it will still contain any
         token that will not be accepted due to an error action in a
         later state.
    */
    if (!yyla.empty ())
      {
        int yytoken = yyla.type_get ();
        yyarg[yycount++] = yytname_[yytoken];
        int yyn = yypact_[yystate];
        if (!yy_pact_value_is_default_ (yyn))
          {
            /* Start YYX at -YYN if negative to avoid negative indexes in
               YYCHECK.  In other words, skip the first -YYN actions for
               this state because they are default actions.  */
            int yyxbegin = yyn < 0 ? -yyn : 0;
            // Stay within bounds of both yycheck and yytname.
            int yychecklim = yylast_ - yyn + 1;
            int yyxend = yychecklim < yyntokens_ ? yychecklim : yyntokens_;
            for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
              if (yycheck_[yyx + yyn] == yyx && yyx != yyterror_
                  && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
                {
                  if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                    {
                      yycount = 1;
                      break;
                    }
                  else
                    yyarg[yycount++] = yytname_[yyx];
                }
          }
      }

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
      default: // Avoid compiler warnings.
        YYCASE_ (0, YY_("syntax error"));
        YYCASE_ (1, YY_("syntax error, unexpected %s"));
        YYCASE_ (2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_ (3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_ (4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_ (5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    std::string yyres;
    // Argument number.
    size_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += yytnamerr_ (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const signed char parser::yypact_ninf_ = -32;

  const signed char parser::yytable_ninf_ = -1;

  const short int
  parser::yypact_[] =
  {
     -32,    91,   -32,   -32,   -32,   -32,   -18,     2,     2,   -32,
     -32,     2,     2,   128,     2,     2,    -3,    -4,   112,   -32,
     -32,    -6,   -32,   -17,   113,     2,   -32,   -32,     2,   -17,
     -17,   -17,   -17,   -32,   -32,   -32,   -32,   -32,   -32,   -32,
     -32,   -32,   -32,   -32,   -17,   -17,    -3,   -32,   -21,   -20,
     -12,    -5,    24,    26,    28,    31,    38,    39,    40,     0,
      81,    23,   -32,    33,     2,     2,     2,    -3,    -3,    -3,
      -3,   -17,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    -3,    -3,   -32,   -32,   -32,   -16,   -16,   -32,    25,
      25,   -32,   -32,   -17,   -17,   -17,   -17,   -17,   -17,   -17,
     -17,   -17,   113,   113
  };

  const unsigned char
  parser::yydefact_[] =
  {
       2,     0,     1,    43,    49,    30,    48,     0,     0,    38,
      39,     0,     0,     0,     0,     0,     0,     0,     0,     4,
       3,     0,     6,    28,    29,     0,    41,    48,     0,    31,
      32,    35,    36,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    37,    33,    34,     0,    54,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     5,     0,     0,     0,     0,     0,     0,     0,
       0,    42,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    46,    55,     7,    44,    45,    47,    51,
      50,    52,    53,    40,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27
  };

  const signed char
  parser::yypgoto_[] =
  {
     -32,   -32,   -32,   -32,   -32,   -31,    10,    -1
  };

  const signed char
  parser::yydefgoto_[] =
  {
      -1,     1,    20,    21,    43,    22,    60,    61
  };

  const unsigned char
  parser::yytable_[] =
  {
      24,     4,    48,     3,     4,     3,    27,    72,    27,    64,
      65,    23,    73,    66,    66,    47,    25,    29,    30,    26,
      74,    31,    32,    16,    44,    45,    16,    75,    46,    62,
      63,    59,    85,    28,     0,    71,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    67,
      68,    69,    70,    69,    70,    84,    76,     0,    77,    16,
      78,    17,    24,    79,    18,     0,    89,    90,    91,    92,
      80,    81,    82,    23,    86,    87,    88,     0,     0,     0,
     102,   103,    93,    94,    95,    96,    97,    98,    99,   100,
     101,     2,     0,     0,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    64,    65,     0,
       0,    66,     0,    83,     0,     3,     4,    16,    27,    17,
       0,     0,    18,     0,     0,     0,    19,     0,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    16,    67,
      68,    69,    70,    59,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42
  };

  const signed char
  parser::yycheck_[] =
  {
       1,     4,     6,     3,     4,     3,     6,    28,     6,    26,
      27,     1,    32,    30,    30,    16,    34,     7,     8,    37,
      32,    11,    12,    26,    14,    15,    26,    32,    31,    35,
      36,    31,    63,    31,    -1,    25,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    26,
      27,    28,    29,    28,    29,    32,    32,    -1,    32,    26,
      32,    28,    63,    32,    31,    -1,    67,    68,    69,    70,
      32,    32,    32,    63,    64,    65,    66,    -1,    -1,    -1,
      81,    82,    72,    73,    74,    75,    76,    77,    78,    79,
      80,     0,    -1,    -1,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    26,    27,    -1,
      -1,    30,    -1,    32,    -1,     3,     4,    26,     6,    28,
      -1,    -1,    31,    -1,    -1,    -1,    35,    -1,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    26,
      27,    28,    29,    31,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25
  };

  const unsigned char
  parser::yystos_[] =
  {
       0,    39,     0,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    26,    28,    31,    35,
      40,    41,    43,    44,    45,    34,    37,     6,    31,    44,
      44,    44,    44,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    42,    44,    44,    31,    45,     6,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    31,
      44,    45,    35,    36,    26,    27,    30,    26,    27,    28,
      29,    44,    28,    32,    32,    32,    32,    32,    32,    32,
      32,    32,    32,    32,    32,    43,    44,    44,    44,    45,
      45,    45,    45,    44,    44,    44,    44,    44,    44,    44,
      44,    44,    45,    45
  };

  const unsigned char
  parser::yyr1_[] =
  {
       0,    38,    39,    39,    40,    40,    41,    41,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    44,    44,    44,    44,    44,    44,    45,
      45,    45,    45,    45,    45,    45
  };

  const unsigned char
  parser::yyr2_[] =
  {
       0,     2,     0,     2,     1,     2,     1,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     1,     1,
       1,     2,     2,     2,     2,     2,     2,     2,     1,     1,
       4,     2,     3,     1,     3,     3,     3,     3,     1,     1,
       3,     3,     3,     3,     2,     3
  };



  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a yyntokens_, nonterminals.
  const char*
  const parser::yytname_[] =
  {
  "\"end of file\"", "error", "$undefined", "\"int64\"", "\"double\"",
  "\"string\"", "\"identifier\"", "Align", "Blank", "LittleEndian",
  "BigEndian", "AddressLen", "Fixup", "Type", "AllowNan", "AllowInfinity",
  "U8", "U16", "U32", "U64", "S8", "S16", "S32", "S64", "Float", "Double",
  "'-'", "'+'", "'*'", "'/'", "'|'", "'('", "')'", "NEG", "'='", "'\\n'",
  "','", "':'", "$accept", "input", "line", "run", "type", "item",
  "intexp", "fpexp", YY_NULLPTR
  };

#if YYDEBUG
  const unsigned char
  parser::yyrline_[] =
  {
       0,    97,    97,    98,   101,   102,   106,   107,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   151,   152,   153,   154,   155,   156,   159,
     160,   161,   162,   163,   164,   165
  };

  // Print the state stack on the debug stream.
  void
  parser::yystack_print_ ()
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << i->state;
    *yycdebug_ << std::endl;
  }

  // Report on the debug stream that the rule \a yyrule is going to be reduced.
  void
  parser::yy_reduce_print_ (int yyrule)
  {
    unsigned yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):" << std::endl;
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG

  // Symbol number corresponding to token number t.
  parser::token_number_type
  parser::yytranslate_ (int t)
  {
    static
    const token_number_type
    translate_table[] =
    {
     0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      35,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      31,    32,    28,    27,    36,    26,     2,    29,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    37,     2,
       2,    34,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    30,     2,     2,     2,     2,     2,
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
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    33
    };
    const unsigned user_token_number_max_ = 281;
    const token_number_type undef_token_ = 2;

    if (static_cast<int> (t) <= yyeof_)
      return yyeof_;
    else if (static_cast<unsigned> (t) <= user_token_number_max_)
      return translate_table[t];
    else
      return undef_token_;
  }


} // yy
#line 1942 "parser.cpp" // lalr1.cc:1164
#line 171 "parser.y" // lalr1.cc:1165


void yy::parser::error(const parser::location_type& l, const std::string& m)
{
    throw yy::parser::syntax_error(l, m);
}
