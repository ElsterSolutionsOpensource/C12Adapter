// File MRegexp.cpp
//
// Portions copyright(c) by Guy Gascoigne, Like Zafir and Henry Spencer.
// Used under permission according to copyright notes of the above persons.

// In case this isn't obvious from the later comments this is an ALTERED
// version of the software. If you like my changes then cool, but nearly
// all of the functionality here is derived from Henry Spencer's original work.
//
// This code should work correctly under both _SBCS and _UNICODE, I did
// start working on making it work with _MBCS but gave up after a while
// since I don't need this particular port and it's not going to be as
// straight forward as the other two.
//
// The problem stems from the compiled program being stored as TCHARS,
// the individual items need to be wide enough to hold whatever character
// is thrown at them, but currently they are accessed as an array of
// whatever size integral type is appropriate.  _MBCS would cause this
// to be char, but at times it would need to be larger.  This would
// require making the program be an array of short with the appropriate
// conversions used everywhere.  Certainly it's doable, but it's a pain.
// What's worse is that the current code will compile and run under _MBCS,
// only breaking when it gets wide characters thrown against it.
//
// I've marked at least one bit of code with #pragma messages, I may not
// get all of them, but they should be a start
//
// Guy Gascoigne - Piggford (ggp@bigfoot.com) Friday, February 27, 1998

// @(#)regexp.c   1.3 of 18 April 87
//
// Copyright (c) 1986 by University of Toronto.
// Written by Henry Spencer.  Not derived from licensed software.
//
// Permission is granted to anyone to use this software for any
// purpose on any computer system, and to redistribute it freely,
// subject to the following restrictions:
//
// 1. The author is not responsible for the consequences of use of
//    this software, no matter how awful, even if they arise
//    from defects in it.
//
// 2. The origin of this software must not be misrepresented, either
//    by explicit claim or by omission.
//
// 3. Altered versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// Beware that some of this code is subtly aware of the way operator
// precedence is structured in regular expressions.  Serious changes in
// regular-expression syntax might require a total rethink.

#include "MCOREExtern.h"
#include "MRegexp.h"

   // The first byte of the regexp internal "program" is actually this magic
   // number; the start node begins in the second byte.
   //
   const MChar MAGIC = ((MChar)'\234');

// The "internal use only" fields in regexp are present to pass info from
// compile to execute that permits the execute phase to run lots faster on
// simple cases.
//
// Regstart and m_reganch permit very fast decisions on suitable starting
// points for a match, cutting down the work a lot.  Regmust permits fast
// rejection of lines that cannot possibly match.  The m_regmust tests are
// costly enough that Compile() supplies a m_regmust only if the
// r.e. contains something potentially expensive (at present, the only
// such thing detected is * or + at the start of the r.e., which can
// involve a lot of backup).  Regmlen is supplied because the test in
// Match() needs it and Compile() is computing it anyway.

// Structure for regexp "program".  This is essentially a linear encoding
// of a nondeterministic finite-state machine (aka syntax charts or
// "railroad normal form" in parsing technology).  Each node is an opcode
// plus a "next" pointer, possibly plus an operand.  "Next" pointers of
// all nodes except BRANCH implement concatenation; a "next" pointer with
// a BRANCH on both ends of it is connecting two alternatives.  (Here we
// have one of the subtle syntax dependencies: an individual BRANCH (as
// opposed to a collection of them) is never concatenated with anything
// because of operator precedence.)  The operand of some types of node is
// a literal string; for others, it is a node leading into a sub-FSM.  In
// particular, the operand of a BRANCH node is the first node of the
// branch.  (NB this is *not* a tree structure: the tail of the branch
// connects to the thing following the set of BRANCHes.)  The opcodes
// are:

enum
{
// definition number    opnd? meaning

   END      =  0,    // no    End of program.
   BOL      =  1,    // no    Match beginning of line.
   EOL      =  2,    // no    Match end of line.
   ANY      =  3,    // no    Match any character.
   ANYOF    =  4,    // str   Match any of these.
   ANYBUT   =  5,    // str   Match any but one of these.
   BRANCH   =  6,    // node  Match this, or the next..\&.
   BACK     =  7,    // no    "next" ptr points backward.
   EXACTLY  =  8,    // str   Match this string.
   NOTHING  =  9,    // no    Match empty string.
   STAR     =  10,   // node  Match this 0 or more times.
   PLUS     =  11,   // node  Match this 1 or more times.
   WORDA    =  12,   // no    Match "" at wordchar, where prev is nonword
   WORDZ    =  13,   // no    Match "" at nonwordchar, where prev is word
   OPEN     =  20,   // no    Sub-RE starts here. OPEN+1 is number 1, etc.
   CLOSE    =  30    // no    Analogous to OPEN.
};

// Opcode notes:
//
// BRANCH   The set of branches constituting a single choice are hooked
//    together with their "next" pointers, since precedence prevents
//    anything being concatenated to any individual branch.  The
//    "next" pointer of the last BRANCH in a choice points to the
//    thing following the whole choice.  This is also where the
//    final "next" pointer of each individual branch points; each
//    branch starts with the operand node of a BRANCH node.
//
// BACK     Normal "next" pointers all implicitly point forward; BACK
//    exists to make loop structures possible.
//
// STAR,PLUS   '?', and complex '*' and '+', are implemented as circular
//    BRANCH structures using BACK.  Simple cases (one character
//    per match) are implemented with STAR and PLUS for speed
//    and to minimize recursive plunges.
//
// OPEN,CLOSE  ...are numbered at compile time.

// A node is one char of opcode followed by two chars of "next" pointer.
// "Next" pointers are stored as two 8-bit pieces, high order first.  The
// value is a positive offset from the opcode of the node containing it.
// An operand, if any, simply follows the node.  (Note that much of the
// code generation knows about this implicit relationship.)
//
// Using two bytes for the "next" pointer is vast overkill for most things,
// but allows patterns to get big without disasters.

// Flags to be passed up and down.
//
enum
{
   WORST    =  0,   // Worst case
   HASWIDTH =  1,   // Known never to match empty string
   SIMPLE   =  2,   // Simple enough to be STAR/PLUS operand
   SPSTART  =  4    // Starts with * or +
};

// All of the functions required to directly access the 'program'

inline MChar OP(MChars p)
   {
      return *p;
   }

inline MChars OPERAND(MChars p)
   {
      M_ENSURED_ASSERT(OP(p) == ANYOF || OP(p) == ANYBUT || OP(p) == EXACTLY ||
                       OP(p) == BRANCH || OP(p) == STAR || OP(p) == PLUS);
      return p + 5;
   }

inline unsigned OPERANDLEN(MChars p)
   {
      M_ENSURED_ASSERT(OP(p) == ANYOF || OP(p) == ANYBUT || OP(p) == EXACTLY);
      return ((unsigned)*(short*)(p + 3));
   }

inline MChars regnext(MChars p)
   {
      const short offset = *((short*)(p+1));
      if ( offset == 0 )
         return NULL;
      return (OP(p) == BACK) ? p - offset : p + offset;
   }

///////////////////////////////////////////////////////////////////////////////
// Compile / Validate the regular expression

class MRegCompilerBase
{
public:

   MRegCompilerBase(const MStdString& pattern)
   :
#ifdef M_USE_USTL
      regparse(const_cast<MStdString&>(pattern).data()),
      regparseEnd(const_cast<MStdString&>(pattern).data() + pattern.size()),
#else
      regparse(pattern.data()),
      regparseEnd(pattern.data() + pattern.size()),
#endif
      regnpar(1)
   {
   }

   virtual ~MRegCompilerBase()
   {
   }

   // Regular expression, i.e. main body or parenthesized thing
   //
   // Caller must absorb opening parenthesis.
   //
   // Combining parenthesis handling with the base level of regular expression
   // is a trifle forced, but the need to tie the tails of the branches to what
   // follows makes it hard to avoid.
   //
   MChars reg(int paren, int *flagp);

protected:

   // Input-scan pointer
   //
   MConstChars regparse;

   // End of the input pointer
   //
   MConstChars regparseEnd;

   // Count
   //
   int regnpar;

   // One alternative of an | operator
   //
   // Implements the concatenation operator.
   //
   MChars regbranch(int *flagp);

   // Something followed by possible [*+?]
   //
   // Note that the branching code sequences used for ? and the general cases
   // of * and + are somewhat optimized:  they use the same NOTHING node as
   // both the endmarker for their branch list and the body of the last branch.
   // It might seem that this node could be dispensed with entirely, but the
   // endmarker role is not redundant.
   //
   MChars regpiece(int *flagp);

   // The lowest level of compiling
   //
   // Optimization:  gobbles an entire sequence of ordinary characters so that
   // it can turn them into a single node, which is smaller to store and
   // faster to run.  Backslashed characters are exceptions, each becoming a
   // separate node; the code is simpler that way and it's not worth fixing.
   //
   MChars regatom(int *flagp);

   bool ISREPN(MChar c)
   {
      return c == '*' || c == '+' || c == '?';
   }

   // Emit a single character
   //
   virtual void regc(int c) = 0;

   // Create a new node in the current position
   //
   virtual MChars CreateRegNode(int op) = 0;

   // Insert an operator in front of already-emitted operand.
   // Means relocating the operand.
   //
   virtual void reginsert(MChar op, MChars opnd) = 0;

   // Set the next-pointer at the end of a node chain
   //
   virtual void regtail(MChars p, MChars val) = 0;

   // regtail on operand of first argument; nop if operandless
   //
   virtual void regoptail(MChars p, MChars val) = 0;

   // Set the length of the operand using the last pointer
   //
   virtual void SetLastOperandLength(MChars opStart) = 0;
};

MChars MRegCompilerBase::reg(int paren, int* flagp)
{
   MChars ret = NULL;
   MChars ender;
   int parno = 0;
   int flags;

   *flagp = HASWIDTH;   // Tentatively.
   if ( paren )
   {
      // Make an OPEN node.
      if ( regnpar >= MRegexp::NUMBER_OF_SUBEXPRESSIONS )
      {
         MException::Throw(M_CODE_STR(M_ERR_REGEXP_TOO_MANY_PARENTHESES, M_I("Regular expression has too many parentheses")));
         M_ENSURED_ASSERT(0);
      }
      parno = regnpar;
      regnpar++;
      ret = CreateRegNode(OPEN+parno);
   }

   // Pick up the branches, linking them together.
   MChars br = regbranch(&flags);
   if ( br == NULL )
      return NULL;
   if ( paren )
      regtail(ret, br); // OPEN -> first.
   else
      ret = br;
   *flagp &= ~(~flags & HASWIDTH); // Clear bit if bit 0.
   *flagp |= flags & SPSTART;
   while ( *regparse == '|' )
   {
      regparse++;
      br = regbranch(&flags);
      if ( br == NULL )
         return NULL;
      regtail(ret, br); // BRANCH -> BRANCH.
      *flagp &= ~(~flags & HASWIDTH);
      *flagp |= flags & SPSTART;
   }

   // Make a closing node, and hook it on the end
   ender = CreateRegNode((paren) ? CLOSE + parno : END);
   regtail(ret, ender);

   // Hook the tails of the branches to the closing node.
   for ( br = ret; br != NULL; br = regnext(br) )
      regoptail(br, ender);

   // Check for proper termination.
   if ( paren && *regparse++ != ')' )
   {
      MException::Throw(M_CODE_STR(M_ERR_REGEXP_UNTERMINATED_PARENTHESES, M_I("Regular expression has unterminated parentheses '('")));
      M_ENSURED_ASSERT(0);
   }
   else if ( !paren && regparse != regparseEnd )
   {
      M_ASSERT(*regparse == ')'); // INTERNAL ERROR JUNK
      MException::Throw(M_CODE_STR(M_ERR_REGEXP_UNMATCHED_PARENTHESES, M_I("Regular expression has unmatched parentheses ')'")));
      M_ENSURED_ASSERT(0);
   }
   return ret;
}

MChars MRegCompilerBase::regbranch(int *flagp)
{
   *flagp = WORST;            // Tentatively.

   int flags = 0;
   MChars ret = CreateRegNode(BRANCH);
   MChars chain = NULL;
   while ( regparse != regparseEnd )
   {
      int c = *regparse;
      if ( c == '|' || c == ')' )
         break;
      MChars latest = regpiece(&flags);
      if ( latest == NULL )
         return NULL;
      *flagp |= flags&HASWIDTH;
      if ( chain == NULL )      // First piece.
         *flagp |= flags&SPSTART;
      else
         regtail(chain, latest);
      chain = latest;
   }
   if ( chain == NULL )         // Loop ran zero times.
      CreateRegNode(NOTHING);
   return ret;
}

MChars MRegCompilerBase::regpiece(int *flagp)
{
   int flags;
   MChars ret = regatom(&flags);
   if ( ret == NULL )
      return NULL;

   MChar op = *regparse;
   if ( !ISREPN(op) )
   {
      *flagp = flags;
      return ret;
   }

   MChars next;
   if ( !(flags & HASWIDTH) && op != (MChar)'?' )
   {
      MException::Throw(M_CODE_STR(M_ERR_REGEXP_OP_COULD_BE_EMPTY, M_I("Regular expression operand '*+' could be empty")));
      M_ENSURED_ASSERT(0);
   }
   switch ( op )
   {
      case (MChar)'*': *flagp = WORST|SPSTART;           break;
      case (MChar)'+': *flagp = WORST|SPSTART|HASWIDTH;  break;
      case (MChar)'?': *flagp = WORST;                   break;
   }

   if ( op == (MChar)'*' && (flags&SIMPLE) )
      reginsert(STAR, ret);
   else if ( op == (MChar)'*' )
   {
      // Emit x* as (x&|), where & means "self".
      reginsert(BRANCH, ret);          // Either x
      regoptail(ret, CreateRegNode(BACK));      // and loop
      regoptail(ret, ret);          // back
      regtail(ret, CreateRegNode(BRANCH));      // or
      regtail(ret, CreateRegNode(NOTHING));     // null.
   }
   else if ( op == (MChar)'+' && (flags & SIMPLE) )
      reginsert(PLUS, ret);
   else if ( op == (MChar)'+' )
   {
      // Emit x+ as x(&|), where & means "self".
      next = CreateRegNode(BRANCH);          // Either
      regtail(ret, next);
      regtail(CreateRegNode(BACK), ret);     // loop back
      regtail(next, CreateRegNode(BRANCH));     // or
      regtail(ret, CreateRegNode(NOTHING));     // null.
   }
   else if ( op == (MChar)'?' )
   {
      // Emit x? as (x|)
      reginsert(BRANCH, ret);          // Either x
      regtail(ret, CreateRegNode(BRANCH));      // or
      next = CreateRegNode(NOTHING);         // null.
      regtail(ret, next);
      regoptail(ret, next);
   }
   regparse++;
   if ( ISREPN(*regparse) )
   {
      MException::Throw(M_CODE_STR(M_ERR_REGEXP_NESTED_OP, M_I("Regular expression has nested '*?+'")));
      M_ENSURED_ASSERT(0);
   }
   return ret;
}

MChars MRegCompilerBase::regatom(int* flagp)
{
   MChars ret;
   int flags;

   *flagp = WORST;      // Tentatively.
   switch ( *regparse++ )
   {
      // FIXME: these chars only have meaning at beg/end of pat?
      case '^':
         ret = CreateRegNode(BOL);
         break;
      case '$':
         ret = CreateRegNode(EOL);
         break;
      case '.':
         ret = CreateRegNode(ANY);
         *flagp |= HASWIDTH|SIMPLE;
         break;
      case '[':
      {
         int c;
         if ( *regparse == '^' )
         {
            regparse++;
            ret = CreateRegNode(ANYBUT);
         }
         else
            ret = CreateRegNode(ANYOF);
         if ( (c = *regparse) == ']' || c == '-' )
         {
            regc(c);
            regparse++;
         }
         while ( regparse != regparseEnd )
         {
            c = *regparse++;
            if ( c == ']' )
               break;
            if ( c != '-' )
               regc(c);
            else if ( (c = *regparse) == ']' || regparse == regparseEnd )
               regc('-');
            else
            {
               int range = (MChar)*(regparse-2);
               int rangeEnd = (MChar)c;
               if ( range > rangeEnd )
               {
                  MException::Throw(M_CODE_STR(M_ERR_REGEXP_INVALID_RANGE, M_I("Regular expression has invalid range within '[]'")));
                  M_ENSURED_ASSERT(0);
               }
               for ( range++; range <= rangeEnd; ++range )
                  regc(range);
               regparse++;
            }
         }
         if ( c != ']' )
         {
            MException::Throw(M_CODE_STR(M_ERR_REGEXP_UNMATCHED_BRACE, M_I("Regular expression has unmatched '[]'")));
            M_ENSURED_ASSERT(0);
         }
         SetLastOperandLength(ret);
         *flagp |= HASWIDTH|SIMPLE;
         break;
      }
      case '(':
         ret = reg(1, &flags);
         if ( ret == NULL )
            return NULL;
         *flagp |= flags & (HASWIDTH | SPSTART);
         break;
      case '|':
      case ')':
         M_ENSURED_ASSERT(0); // INTERNAL UNEXPECTED CHAR, supposed to be caught earlier
      case '?':
      case '+':
      case '*':
         MException::Throw(M_CODE_STR(M_ERR_REGEXP_OP_FOLLOWS_NOTHING, M_I("Regular expression has '?', '+' or '*' that follows nothing")));
         M_ENSURED_ASSERT(0);
      case '\\':
         switch ( *regparse++ )
         {
            case '<':
               ret = CreateRegNode(WORDA);
               break;
            case '>':
               ret = CreateRegNode(WORDZ);
               break;
            // Someday handle \1, \2, ...
            default:
               // Handle general quoted chars in exact-match routine
               goto de_fault;
         }
         break;
   de_fault:
   default:
      // Encode a string of characters to be matched exactly.
      //
      // This is a bit tricky due to quoted chars and due to
      // '*', '+', and '?' taking the SINGLE char previous
      // as their operand.
      //
      // On entry, the char at regparse[-1] is going to go
      // into the string, no matter what it is.  (It could be
      // following a \ if we are entered from the '\' case.)
      //
      // Basic idea is to pick up a good char in  ch  and
      // examine the next char.  If it's *+? then we twiddle.
      // If it's \ then we frozzle.  If it's other magic char
      // we push  ch  and terminate the string.  If none of the
      // above, we push  ch  on the string and go around again.
      //
      //  regprev  is used to remember where "the current char"
      // starts in the string, if due to a *+? we need to back
      // up and put the current char in a separate, 1-char, string.
      // When  regprev  is NULL,  ch  is the only char in the
      // string; this is used in *+? handling, and in setting
      // flags |= SIMPLE at the end.
      {
         MConstChars regprev;
         MChar ch;

         regparse--;       // Look at cur char
         ret = CreateRegNode(EXACTLY);
         for ( regprev = 0 ; regparse != regparseEnd ; )
         {
            ch = *regparse++; // Get current char
            switch ( *regparse )
            { // look at next one
            default:
               regc(ch);   // Add cur to string
               break;
            case '.': case '[': case '(':
            case ')': case '|': case '\n':
            case '$': case '^':
            // FIXME, $ and ^ should not always be magic
            magic:
               regc(ch);   // dump cur char
               goto done;  // and we are done
            case '?': case '+': case '*':
               if ( !regprev )  // If just ch in str,
                  goto magic;   //    use it
               // End mult-char string one early
               regparse = regprev; // Back up parse
               goto done;
            case '\\':
               regc(ch);   // Cur char OK
               if ( regparse + 1 == regparseEnd ) // end-backslash not allowed
               {
                  MException::Throw(M_CODE_STR(M_ERR_REGEXP_TRAILING_ESC, M_I("Regular expression has trailing '\\'")));
                  M_ENSURED_ASSERT(0);
               }
               switch ( regparse[1] )
               { // Look after '\'
               case '<':
               case '>':     // Someday also handle \1, \2, ...
                  goto done; // Not quoted
               default:
                  // Backup point is \, scan point is after it
                  regprev = regparse;
                  regparse++;
                  continue;   // NOT break
               }
            }
            regprev = regparse;  // Set backup point
         }
      done:
         SetLastOperandLength(ret);
         *flagp |= HASWIDTH;
         if ( !regprev )     /* One char? */
            *flagp |= SIMPLE;
      }
      break;
   }
   return ret;
}

// First pass over the expression, testing for validity and returning the program size
//
class MRegValidator : public MRegCompilerBase
{
public:

   MRegValidator(const MStdString& parse)
   :
      MRegCompilerBase(parse),
      regsize(0)
   {
      regc(MAGIC);
      regdummy[0] = NOTHING;
      regdummy[1] = regdummy[2] = regdummy[3] = regdummy[4] = 0;
   }

public:
   int regsize;         // Code size

protected:
   MChar regdummy[5];   // NOTHING, 0 next ptr
   virtual MChars CreateRegNode(int) { regsize += 5; return regdummy; }
   virtual void regc(int) { regsize++; }
   virtual void reginsert(MChar, MChars) { regsize += 5; }
   virtual void regtail(MChars, MChars) { }
   virtual void regoptail(MChars, MChars) { }
   virtual void SetLastOperandLength(MChars) {}
};

class MRegCompiler : public MRegCompilerBase
{
public:
   MRegCompiler(const MStdString& parse, MChars prog)
   :
      MRegCompilerBase(parse),
      regcode(prog)
   {
      regc(MAGIC);
   }

private:

   MChars regcode;

protected:

   virtual void regc(int b)
   {
      *regcode++ = (MChar)b;
   }

   virtual MChars CreateRegNode(int op);
   virtual void reginsert(MChar op, MChars opnd);
   virtual void regtail(MChars p, MChars val);
   virtual void regoptail(MChars p, MChars val);
   virtual void SetLastOperandLength(MChars opStart);
};

MChars MRegCompiler::CreateRegNode(int op)
{
   MChars const ret = regcode;
   MChars ptr = ret;
   *ptr++ = (MChar)op;
   *ptr++ = '\0';    // Null next pointer
   *ptr++ = '\0';
   *ptr++ = '\0';    // length of the operand
   *ptr++ = '\0';
   regcode = ptr;
   return ret;
}

void MRegCompiler::reginsert(MChar op, MChars opnd)
{
   memmove(opnd + 5, opnd, (size_t)((regcode - opnd) * sizeof(MChar)));
   regcode += 5;
   MChars place = opnd;     // Op node, where operand used to be.
   *place++ = op;
   *place++ = '\0';
   *place++ = '\0';
   *place++ = '\0';
   *place = '\0';
}

void MRegCompiler::regtail(MChars p, MChars val)
{
   MChars scan;
   MChars temp;
   for ( scan = p; (temp = regnext(scan)) != NULL; scan = temp )   // Find last node.
      continue;
   *((short*)(scan + 1)) = (short)((OP(scan) == BACK) ? scan - val : val - scan);
}

void MRegCompiler::regoptail(MChars p, MChars val)
{
   if ( OP(p) == BRANCH )   // "Operandless" and "op != BRANCH" are synonymous in practice
      regtail(OPERAND(p), val);
}

void MRegCompiler::SetLastOperandLength(MChars opStart)
{
   int len = M_64_CAST(int, regcode - opStart - 5);
   M_ASSERT(len >= 0 && len < 0x7FF0);
   *((short*)(opStart + 3)) = (short)len;
}

// Executor and matcher
//
class MRegExecutor
{
   MConstChars reginput;  // String-input pointer.
   MConstChars regbol;    // Beginning of input, for ^ check.
   MConstChars regeol;    // Beginning of input, for ^ check.
   MChars* regstartp;     // Pointer to m_startp array.
   MChars* regendp;       // Ditto for m_endp.
   MRegexp* owner;

public:

   MRegExecutor(MRegexp* p, const MStdString& s)
   :
      reginput(NULL),
#ifdef M_USE_USTL
      regbol(const_cast<MChars>(const_cast<MStdString&>(s).data())),
      regeol(const_cast<MChars>(const_cast<MStdString&>(s).data() + s.size())),
#else
      regbol(const_cast<MChars>(s.data())),
      regeol(const_cast<MChars>(s.data() + s.size())),
#endif
      regstartp(p->m_startp),
      regendp(p->m_endp),
      owner(p)
   {
   }

   // Try match at specific point
   //
   bool regtry(MConstChars);

   // Main matching routine.
   //
   // Conceptually the strategy is simple:  check to see whether the current
   // node matches, call self recursively to see whether the rest matches,
   // and then act accordingly.  In practice we make some effort to avoid
   // recursion, in particular by going through "ordinary" nodes (that don't
   // need to know whether the rest of the match failed) by a loop instead of
   // by recursion.
   //
   bool regmatch(MConstChars prog);

   // Report how many times something simple would match
   //
   size_t regrepeat(MChars node);
};

bool MRegExecutor::regtry(MConstChars str)
{
   reginput = const_cast<MChars>(str);
   memset(owner->m_startp, 0, sizeof(owner->m_startp));
   memset(owner->m_endp,   0, sizeof(owner->m_endp));
   if ( regmatch(owner->m_program + 1) )
   {
      M_ASSERT(owner->m_count <= MRegexp::NUMBER_OF_SUBEXPRESSIONS);
      owner->m_startp[0] = const_cast<MChars>(str);
      owner->m_endp[0] = const_cast<MChars>(reginput);
      return true;
   }
   return false;
}

bool MRegExecutor::regmatch(MConstChars prog)
{
   MChars next;  // Next node.
   for ( MChars scan = const_cast<MChars>(prog); scan != NULL; scan = next )
   {
      next = regnext(scan);
      switch ( OP(scan) )
      {
         case BOL:
            if ( reginput != regbol )
               return false;
            break;
         case EOL:
            if ( reginput != regeol )
               return false;
            break;
         case WORDA: // Must be looking at a letter, digit, or _
            if ( !isalnum(*reginput) && *reginput != '_' )
               return false;
            // Prev must be BOL or nonword
            if ( reginput > regbol && (isalnum(reginput[-1]) || reginput[-1] == '_') )
               return false;
            break;
         case WORDZ: // Must be looking at non letter, digit, or _
            if ( isalnum(*reginput) || *reginput == '_' )
               return false;
            // We don't care what the previous char was
            break;
         case ANY:
            if ( reginput == regeol )
               return false;
            reginput++;
            break;
         case EXACTLY:
            {
            int len = OPERANDLEN(scan);
            if ( len > regeol - reginput || // even length is bigger
                 memcmp(OPERAND(scan), reginput, len * sizeof(MChar)) != 0 )
               return false;
            reginput += len;
            break;
            }
         case ANYOF:
            {
            if ( reginput == regeol )
               return false;
            MChars p = OPERAND(scan);
            MChars pEnd = p + OPERANDLEN(scan);
            for ( ;; ++p )
            {
               if ( p == pEnd )
                  return false; // not found a match
               if ( *p == *reginput )
                  break; // found a match, continue
            }
            reginput++;
            break;
            }
         case ANYBUT:
            {
            if ( reginput == regeol )
               return false;
            MChars p = OPERAND(scan);
            MChars pEnd = p + OPERANDLEN(scan);
            for ( ; p != pEnd; ++p )
               if ( *p == *reginput )
                  return false;
            reginput++;
            break;
            }
         case NOTHING:
            break;
         case BACK:
            break;
         case OPEN+1: case OPEN+2: case OPEN+3:
         case OPEN+4: case OPEN+5: case OPEN+6:
         case OPEN+7: case OPEN+8: case OPEN+9:
            {
            const int no = OP(scan) - OPEN;
            MConstChars input = reginput;
            if ( regmatch(next) )
            {  // Don't set m_startp if some later invocation of the same parentheses already has
               if ( regstartp[no] == NULL )
                  regstartp[no] = const_cast<MChars>(input);
               return true;
            }
            return false;
            }
         case CLOSE+1: case CLOSE+2: case CLOSE+3:
         case CLOSE+4: case CLOSE+5: case CLOSE+6:
         case CLOSE+7: case CLOSE+8: case CLOSE+9:
            {
            const int no = OP(scan) - CLOSE;
            MConstChars input = reginput;
            if ( regmatch(next) )
            {  // Don't set m_endp if some later invocation of the same parentheses already has
               if ( regendp[no] == NULL )
               {
                  ++ owner->m_count;
                  regendp[no] = const_cast<MChars>(input);
               }
               return true;
            }
            return false;
            }
         case BRANCH:
            {
            MConstChars save = reginput;
            if ( OP(next) != BRANCH )    // No choice.
               next = OPERAND(scan);     // Avoid recursion.
            else
            {
               while ( OP(scan) == BRANCH )
               {
                  if ( regmatch(OPERAND(scan)) )
                     return true;
                  reginput = save;
                  scan = regnext(scan);
               }
               return false;
            }
            break;
            }
         case STAR:
         case PLUS:
            {
            MChar nextChar = 0;
            bool nextCharExists = (OP(next) == EXACTLY);
            if ( nextCharExists )
               nextChar = *OPERAND(next);
            size_t no;
            MConstChars save = reginput;
            const size_t min = (OP(scan) == STAR) ? 0 : 1;

            for ( no = regrepeat(OPERAND(scan)) + 1; no > min; no-- )
            {
               reginput = save + no - 1;
               // If it could work, try it.
               if ( !nextCharExists || *reginput == nextChar )
                  if ( regmatch(next) )
                     return true;
            }
            return false;
            }
         case END:
            return true;   // Success!
      }
   }
   M_ASSERT(0); // CORRUPTED POINTERS
   return false;
}

size_t MRegExecutor::regrepeat(MChars node)
{
   switch ( OP(node) )
   {
   case ANY:
      return regeol - reginput;
   case EXACTLY:
      {
      MChar ch = *OPERAND(node);
      size_t count = 0;
      for ( MConstChars scan = reginput; scan != regeol && *scan == ch; ++scan )
         count++;
      return count;
      }
   case ANYOF: // cannot use strspn, as it can have \0
      {
      MChar* set = OPERAND(node);
      MChar* setEnd = set + OPERANDLEN(node);
      size_t count = 0;
      for ( MConstChars scan = reginput; scan != regeol; ++scan )
      {
         for ( MChar* s = set; ; ++s )
         {
            if ( s == setEnd )
               return count; // end by nonmatching character
            if ( *s == *scan )
               break; // found a match
         }
         count++;
      }
      return count; // return by end of input
      }
   case ANYBUT: // cannot use strspn, as it can have \0
      {
      MChar* set = OPERAND(node);
      MChar* setEnd = set + OPERANDLEN(node);
      size_t count = 0;
      for ( MConstChars scan = reginput; scan != regeol; ++scan )
      {
         for ( MChar* s = set; ; ++s )
         {
            if ( s == setEnd )
               return count; // end by nonmatching character
            if ( *s != *scan )
               break; // found a match
         }
         count++;
      }
      return count; // return by end of input
      }
   default:    // Oh dear.  Called inappropriately.
      break;
   }
   M_ASSERT(0); // BAD REGREPEAT
   return(0);  // Best compromise.
}

   #if !M_NO_REFLECTION

      /// Constructor of the regular expression that takes an expression as standard string.
      ///
      /// \param exp
      ///     Regular expression
      ///
      /// \param caseInsensitive
      ///     Whether the match shall be case insensitive, false by default.
      ///
      /// \pre The expression has to correspond to the valid syntax definition
      /// as presented in the header of the file. Otherwise MERegexp is thrown
      /// with the type and string that corresponds to the error.
      ///
      static MRegexp* DoNew2(const MStdString& exp, bool caseInsensitive)
      {
         return M_NEW MRegexp(exp, caseInsensitive);
      }

      /// Constructor of the regular expression that takes an expression as standard string.
      ///
      /// The match will be case sensitive.
      ///
      /// \param exp
      ///     Regular expression
      ///
      /// \pre The expression has to correspond to the valid syntax definition
      /// as presented in the header of the file. Otherwise MERegexp is thrown
      /// with the type and string that corresponds to the error.
      ///
      static MRegexp* DoNew1(const MStdString& exp)
      {
         return DoNew2(exp, false);
      }

      /// Default constructor
      ///
      static MObject* DoNew0()
      {
         return M_NEW MRegexp;
      }

      /// Do a case sensitive match using the given regular expression and string without creating regexp object
      ///
      /// \param regexp
      ///    Regular expression to match
      ///
      /// \param str
      ///    String in which the regular expression shall be matched.
      ///
      /// \see Match - non-static version of this call
      ///
      static bool DoStaticMatch2(const MStdString& regexp, const MStdString& str)
      {
         return MRegexp::StaticMatch(regexp.c_str(), str);
      }

   #endif

M_START_PROPERTIES(Regexp)
   M_OBJECT_PROPERTY_READONLY_INT        (Regexp, Count)
   M_OBJECT_PROPERTY_READONLY_BOOL_EXACT (Regexp, IsCompiled)
   M_OBJECT_PROPERTY_READONLY_STRING     (Regexp, Pattern,                        ST_constMStdStringA_X)
M_START_METHODS(Regexp)
   M_OBJECT_SERVICE                      (Regexp, Match,                          ST_bool_X_constMStdStringA)
   M_OBJECT_SERVICE                      (Regexp, Item,                           ST_MStdString_X_int)
   M_OBJECT_SERVICE                      (Regexp, GetItemStart,                   ST_int_X_int)
   M_OBJECT_SERVICE                      (Regexp, GetItemLength,                  ST_int_X_int)
   M_OBJECT_SERVICE                      (Regexp, GetReplaceString,               ST_MStdString_X_constMStdStringA)
   M_OBJECT_SERVICE_OVERLOADED           (Regexp, Compile, Compile,            2, ST_X_constMStdStringA_bool)
   M_OBJECT_SERVICE_OVERLOADED           (Regexp, Compile, DoCompile1,         1, ST_X_constMStdStringA)  // SWIG_HIDE
   M_CLASS_FRIEND_SERVICE_OVERLOADED     (Regexp, New, DoNew0,                 0, ST_MObjectP_S)
   M_CLASS_FRIEND_SERVICE_OVERLOADED     (Regexp, New, DoNew1,                 1, ST_MObjectP_S_constMStdStringA)
   M_CLASS_FRIEND_SERVICE_OVERLOADED     (Regexp, New, DoNew2,                 2, ST_MObjectP_S_constMStdStringA_bool)
   M_OBJECT_SERVICE                      (Regexp, Clear,                          ST_X)
   M_OBJECT_SERVICE                      (Regexp, CheckIsCompiled,                ST_X)
   M_CLASS_FRIEND_SERVICE_OVERLOADED     (Regexp, StaticMatch, DoStaticMatch2, 2, ST_bool_S_constMStdStringA_constMStdStringA)  // SWIG_HIDE default parameter is done by StaticMatch
   M_CLASS_SERVICE_OVERLOADED            (Regexp, StaticMatch, StaticMatch,    3, ST_bool_S_MConstChars_constMStdStringA_bool)
M_END_CLASS(Regexp, Object)

MRegexp::MRegexp()
:
   m_pattern(),
   m_str(),
   m_regstart('\0'),
   m_regmust(NULL),
   m_program(NULL),
   m_regmlen(0),
   m_count(0),
   m_regstartExists(false),
   m_reganch(false)
{
   memset(m_startp, 0, sizeof(m_startp));
   memset(m_endp, 0, sizeof(m_endp));
}

MRegexp::MRegexp(const MStdString& exp, bool ignoreCase)
:
   m_program(NULL)
{
   memset(m_startp, 0, sizeof(m_startp));
   memset(m_endp, 0, sizeof(m_endp));
   Compile(exp, ignoreCase);
}

MRegexp::MRegexp(MConstChars exp, bool ignoreCase)
:
   m_program(NULL)
{
   memset(m_startp, 0, sizeof(m_startp));
   memset(m_endp, 0, sizeof(m_endp));
   Compile(exp, ignoreCase);
}

MRegexp::MRegexp(const MRegexp& r)
:
   m_program(NULL)
{
   memset(m_startp, 0, sizeof(m_startp));
   memset(m_endp, 0, sizeof(m_endp));
   Compile(r.m_pattern); // respects case sensitivity within the pattern.
}

MRegexp::~MRegexp()
{
   Clear();
}

void MRegexp::Clear()
{
   delete [] m_program;
   m_program = NULL;
   m_str.clear();
   m_pattern.clear();
}

// Compile - compile a regular expression into internal code.
//
// We can't allocate space until we know how big the compiled form will
// be, but we can't compile it (and thus know how big it is) until we've
// got a place to put the code.  So we cheat: we compile it twice, once
// with code generation turned off and size counting turned on, and once
// "for real".  This also means that we don't allocate space until we are
// sure that the thing really will compile successfully, and we never
// have to move the code and thus invalidate pointers into it.
//
void MRegexp::Compile(const MStdString& exp, bool ignoreCase)
{
   Clear();

   m_regstartExists = false;
   m_regstart = '\0';
   m_reganch = false;
   m_regmust = NULL;
   m_regmlen = 0;
   if ( ignoreCase )
   {
      // copy in to out making every top level character a [Aa] set
      m_pattern.clear();
      bool inRange = false;
      MStdString::const_iterator it = exp.begin();
      MStdString::const_iterator itEnd = exp.end();
      for ( ; it < itEnd; ++it )
      {
         MChar c = *it;
         if ( c == '[' )
            inRange = true;
         if ( c == ']' )
            inRange = false;
         if ( !inRange && m_isalpha(c) )
         {
            m_pattern += '[';
            m_pattern += (MChar)m_toupper(c);
            m_pattern += (MChar)m_tolower(c);
            m_pattern += ']';
         }
         else
            m_pattern += c;
      }
   }
   else
      m_pattern = exp;

   // First pass: determine size, legality.
   MRegValidator tester(m_pattern);

   int flags;
   if ( tester.reg(0, &flags) != NULL )
   {
      // Small enough for pointer-storage convention?
      if ( tester.regsize >= 0x7FFFL )
      {
         MException::Throw(M_CODE_STR(M_ERR_REGEXP_TOO_BIG, M_I("Regular expression is too big")));
         M_ENSURED_ASSERT(0);
      }

      m_program = M_NEW MChar [ tester.regsize ]; // program size

      MRegCompiler comp(m_pattern, m_program);
      // Second pass: emit code.
      if ( comp.reg(0, &flags) != NULL )
      {
         MChars scan = m_program + 1;       // First BRANCH.
         if ( OP(regnext(scan)) == END )
         {     // Only one top-level choice.
            scan = OPERAND(scan);

            // Starting-point info.
            if ( OP(scan) == EXACTLY )
            {
               m_regstartExists = true;
               m_regstart = *OPERAND(scan);
            }
            else if ( OP(scan) == BOL )
               m_reganch = true;

            // If there's something expensive in the r.e., find the
            // longest literal string that must appear and make it the
            // m_regmust.  Resolve ties in favor of later strings, since
            // the m_regstart check works with the beginning of the r.e.
            // and avoiding duplication strengthens checking.  Not a
            // strong reason, but sufficient in the absence of others.

            if ( flags & SPSTART )
            {
               MChars longest = NULL;
               size_t len = 0;

               for ( ; scan != NULL; scan = regnext(scan) )
                  if ( OP(scan) == EXACTLY && OPERANDLEN(scan) >= len )
                  {
                     longest = OPERAND(scan);
                     len = OPERANDLEN(scan);
                  }
               m_regmust = longest;
               m_regmlen = (int)len;
            }
         }
      }
   }
   m_count = 0;
}

bool MRegexp::Match(const MStdString& s)
{
   CheckIsCompiled();
   M_ASSERT(*m_program == MAGIC);
   m_count = 0;
   m_str = s;
   MConstChars strPtr = m_str.c_str();

#ifdef M_USE_USTL
   // If there is a "must appear" string, look for it.
   if ( m_regmust != NULL )
   {
      size_t pos = m_str.find(m_regmust, 0);
      if ( pos == MStdString::npos || pos > static_cast<size_t>(m_regmlen) )
         return false;
   }
#else
   // If there is a "must appear" string, look for it.
   if ( m_regmust != NULL && m_str.find(m_regmust, 0, m_regmlen) == MStdString::npos )
      return false;
#endif

   // Simplest case: anchored match need be tried only once.
   MRegExecutor executor(this, m_str);
   if ( m_reganch )
      return executor.regtry(strPtr);

   // Messy cases: unanchored match
   if ( m_regstartExists )  // We know what MChar it must start with
   {
      for ( MStdString::size_type i = 0; (i = m_str.find(m_regstart, i)) != MStdString::npos; ++i )
         if ( executor.regtry(strPtr + i) )
            return true;
   }
   else   // We don't -- general case.
   {
      MStdString::size_type strSize = m_str.size();
      for ( MStdString::size_type i = 0; i <= strSize; ++i ) // NOTE: use <=, try one after last!
         if ( executor.regtry(strPtr + i) )
            return true;
   }
   return false;
}

bool MRegexp::StaticMatch(MConstChars regexp, const MStdString& s, bool caseInsensitive)
{
   MRegexp re(regexp, caseInsensitive);
   return re.Match(s);
}

MStdString MRegexp::GetReplaceString(const MStdString& replaceExp) const
{
   CheckIsCompiled();
   M_ASSERT(*m_program == MAGIC);
   MStdString buf;
   MStdString::const_iterator it = replaceExp.begin();
   MStdString::const_iterator itEnd = replaceExp.end();
   while ( it != itEnd ) // zero characters can be within the string
   {
      MChar c = *it++;
      int no;
      if ( c == MChar('&') )
         no = 0;
      else if ( c == MChar('\\') && m_isdigit(*it) )
         no = *it++ - MChar('0');
      else
         no = -1;
      if ( no < 0 )
      {  // Ordinary character.
         if ( c == MChar('\\') && (it != itEnd) && (*it == MChar('\\') || *it == MChar('&')) )
            c = *it++; // backslashed '\\' or '\&'
         buf += c;
      }
      else if ( m_startp[no] != NULL && m_endp[no] != NULL && m_endp[no] > m_startp[no] )
      {  // Get tagged expression
         MStdString::size_type len = m_endp[no] - m_startp[no];
         buf.append(m_startp[no], len);
      }
   }
   return buf;
}

int MRegexp::GetCount() const
{
   CheckIsCompiled();
   return m_count;
}

int MRegexp::GetItemStart(int i) const
{
   CheckIsCompiled();
   MEIndexOutOfRange::Check(0, m_count, i); // here the index can take values up to m_count, not up to m_count - 1
   return M_64_CAST(int, m_startp[i] - m_str.c_str());
}

int MRegexp::GetItemLength(int i) const
{
   CheckIsCompiled();
   MEIndexOutOfRange::Check(0, m_count, i); // here the index can take values up to m_count, not up to m_count - 1
   return M_64_CAST(int, m_endp[i] - m_startp[i]);
}

MStdString MRegexp::Item(int i) const
{
   int len = GetItemLength(i); // this checks range, whether it is compiled
   return MStdString(m_startp[i], len);
}

void MRegexp::CheckIsCompiled() const
{
   if ( !IsCompiled() )
   {
      MException::Throw(M_CODE_STR(M_ERR_REGEXP_IS_NOT_COMPILED, M_I("Regular expression is not compiled")));
      M_ENSURED_ASSERT(0);
   }
}
