
/* Compiler implementation of the D programming language
 * Copyright (c) 1999-2014 by Digital Mars
 * All Rights Reserved
 * written by Walter Bright
 * http://www.digitalmars.com
 * Distributed under the Boost Software License, Version 1.0.
 * http://www.boost.org/LICENSE_1_0.txt
 * https://github.com/D-Programming-Language/dmd/blob/master/src/expression.h
 */

#ifndef DMD_EXPRESSION_H
#define DMD_EXPRESSION_H

#include "mars.h"
#include "identifier.h"
#include "lexer.h"
#include "arraytypes.h"
#include "intrange.h"
#include "visitor.h"

class Type;
class TypeVector;
struct Scope;
class TupleDeclaration;
class VarDeclaration;
class FuncDeclaration;
class FuncLiteralDeclaration;
class Declaration;
class CtorDeclaration;
class NewDeclaration;
class Dsymbol;
class Import;
class Module;
class ScopeDsymbol;
class Expression;
class Declaration;
class AggregateDeclaration;
class StructDeclaration;
class TemplateInstance;
class TemplateDeclaration;
class ClassDeclaration;
struct HdrGenState;
class BinExp;
struct InterState;
struct Symbol;          // back end symbol
class OverloadSet;
class Initializer;
class StringExp;
class ArrayExp;
class SliceExp;

enum TOK;

// Back end
struct IRState;
struct dt_t;

#ifdef IN_GCC
typedef union tree_node elem;
#else
struct elem;
#endif

void initPrecedence();

Expression *resolveProperties(Scope *sc, Expression *e);
Expression *resolvePropertiesOnly(Scope *sc, Expression *e1);
void accessCheck(Loc loc, Scope *sc, Expression *e, Declaration *d);
Expression *build_overload(Loc loc, Scope *sc, Expression *ethis, Expression *earg, Dsymbol *d);
Dsymbol *search_function(ScopeDsymbol *ad, Identifier *funcid);
void argExpTypesToCBuffer(OutBuffer *buf, Expressions *arguments, HdrGenState *hgs);
void argsToCBuffer(OutBuffer *buf, Expressions *arguments, HdrGenState *hgs);
void expandTuples(Expressions *exps);
TupleDeclaration *isAliasThisTuple(Expression *e);
int expandAliasThisTuples(Expressions *exps, size_t starti = 0);
FuncDeclaration *hasThis(Scope *sc);
Expression *fromConstInitializer(int result, Expression *e);
bool arrayExpressionSemantic(Expressions *exps, Scope *sc);
int arrayExpressionCanThrow(Expressions *exps, FuncDeclaration *func, bool mustNotThrow);
TemplateDeclaration *getFuncTemplateDecl(Dsymbol *s);
Expression *valueNoDtor(Expression *e);
int modifyFieldVar(Loc loc, Scope *sc, VarDeclaration *var, Expression *e1);
Expression *resolveAliasThis(Scope *sc, Expression *e);
Expression *callCpCtor(Scope *sc, Expression *e);
Expression *resolveOpDollar(Scope *sc, ArrayExp *ae, Expression **pe0);
Expression *resolveOpDollar(Scope *sc, SliceExp *se, Expression **pe0);
Expression *integralPromotions(Expression *e, Scope *sc);
void discardValue(Expression *e);
bool isTrivialExp(Expression *e);

int isConst(Expression *e);
Expression *toDelegate(Expression *e, Scope *sc);
AggregateDeclaration *isAggregate(Type *t);
IntRange getIntRange(Expression *e);
bool isUnaArrayOp(TOK op);
bool isBinArrayOp(TOK op);
bool isBinAssignArrayOp(TOK op);
bool isArrayOpOperand(Expression *e);
Expression *arrayOp(BinExp *e, Scope *sc);
Expression *arrayOp(BinAssignExp *e, Scope *sc);
bool hasSideEffect(Expression *e);
bool canThrow(Expression *e, FuncDeclaration *func, bool mustNotThrow);
Expression *Expression_optimize(Expression *e, int result, bool keepLvalue);
dt_t **Expression_toDt(Expression *e, dt_t **pdt);
elem *toElem(Expression *e, IRState *irs);
MATCH implicitConvTo(Expression *e, Type *t);
Expression *implicitCastTo(Expression *e, Scope *sc, Type *t);
Expression *castTo(Expression *e, Scope *sc, Type *t);
void toCBuffer(Expression *e, OutBuffer *buf, HdrGenState *hgs);
Expression *ctfeInterpret(Expression *);
Expression *inlineCopy(Expression *e, Scope *sc);
Expression *op_overload(Expression *e, Scope *sc);
Type *toStaticArrayType(SliceExp *e);
Expression *scaleFactor(BinExp *be, Scope *sc);
Expression *typeCombine(BinExp *be, Scope *sc);
Expression *inferType(Expression *e, Type *t, int flag = 0);
Expression *semanticTraits(TraitsExp *e, Scope *sc);
Type *getIndirection(Type *t);

Expression *checkGC(Scope *sc, Expression *e);

/* Run CTFE on the expression, but allow the expression to be a TypeExp
 * or a tuple containing a TypeExp. (This is required by pragma(msg)).
 */
Expression *ctfeInterpretForPragmaMsg(Expression *e);

/* Interpreter: what form of return value expression is required?
 */
enum CtfeGoal
{   ctfeNeedRvalue,   // Must return an Rvalue
    ctfeNeedLvalue,   // Must return an Lvalue
    ctfeNeedAnyValue, // Can return either an Rvalue or an Lvalue
    ctfeNeedLvalueRef,// Must return a reference to an Lvalue (for ref types)
    ctfeNeedNothing   // The return value is not required
};

Expression *interpret(Expression *e, InterState *istate, CtfeGoal goal);

#define WANTflags   1
#define WANTvalue   2
// Same as WANTvalue, but also expand variables as far as possible
#define WANTexpand  8

class Expression : public RootObject
{
public:
    Loc loc;                    // file location
    TOK op;                // handy to minimize use of dynamic_cast
    Type *type;                 // !=NULL means that semantic() has been run
    unsigned char size;         // # of bytes in Expression so we can copy() it
    unsigned char parens;       // if this is a parenthesized expression

    Expression(Loc loc, TOK op, int size);
    static void init();
    Expression *copy();
    virtual Expression *syntaxCopy();
    virtual Expression *semantic(Scope *sc);
    Expression *trySemantic(Scope *sc);

    // kludge for template.isExpression()
    int dyncast() { return DYNCAST_EXPRESSION; }

    void print();
    char *toChars();
    void error(const char *format, ...);
    void warning(const char *format, ...);
    void deprecation(const char *format, ...);
    virtual bool rvalue();

    // creates a single expression which is effectively (e1, e2)
    // this new expression does not necessarily need to have valid D source code representation,
    // for example, it may include declaration expressions
    static Expression *combine(Expression *e1, Expression *e2);
    static Expression *extractLast(Expression *e, Expression **pe0);
    static Expressions *arraySyntaxCopy(Expressions *exps);

    virtual dinteger_t toInteger();
    virtual uinteger_t toUInteger();
    virtual real_t toReal();
    virtual real_t toImaginary();
    virtual complex_t toComplex();
    virtual StringExp *toStringExp();
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs)
    {
        ::toCBuffer(this, buf, hgs);
    }
    virtual void toMangleBuffer(OutBuffer *buf);
    virtual int isLvalue();
    virtual Expression *toLvalue(Scope *sc, Expression *e);
    virtual Expression *modifiableLvalue(Scope *sc, Expression *e);
    Expression *implicitCastTo(Scope *sc, Type *t)
    {
        return ::implicitCastTo(this, sc, t);
    }
    MATCH implicitConvTo(Type *t)
    {
        return ::implicitConvTo(this, t);
    }
    Expression *castTo(Scope *sc, Type *t)
    {
        return ::castTo(this, sc, t);
    }
    virtual void checkEscape();
    virtual void checkEscapeRef();
    virtual Expression *resolveLoc(Loc loc, Scope *sc);
    void checkScalar();
    void checkNoBool();
    Expression *checkIntegral();
    Expression *checkArithmetic();
    Expression *checkReadModifyWrite(TOK rmwOp, Expression *exp = NULL);
    void checkDeprecated(Scope *sc, Dsymbol *s);
    void checkPurity(Scope *sc, FuncDeclaration *f);
    void checkPurity(Scope *sc, VarDeclaration *v);
    void checkSafety(Scope *sc, FuncDeclaration *f);
    void checkNogc(Scope *sc, FuncDeclaration *f);
    bool checkPostblit(Scope *sc, Type *t);
    virtual int checkModifiable(Scope *sc, int flag = 0);
    virtual Expression *checkToBoolean(Scope *sc);
    virtual Expression *addDtorHook(Scope *sc);
    Expression *checkToPointer();
    Expression *addressOf();
    Expression *deref();

    Expression *optimize(int result, bool keepLvalue = false)
    {
        return Expression_optimize(this, result, keepLvalue);
    }

    // Entry point for CTFE.
    // A compile-time result is required. Give an error if not possible
    Expression *ctfeInterpret()
    {
        return ::ctfeInterpret(this);
    }
    Expression *interpret(InterState *istate, CtfeGoal goal = ctfeNeedRvalue)
    {
        return ::interpret(this, istate, goal);
    }

    int isConst() { return ::isConst(this); }
    virtual int isBool(int result);
    Expression *op_overload(Scope *sc)
    {
        return ::op_overload(this, sc);
    }

    // Back end
    elem *toElem(IRState *irs) { return ::toElem(this, irs); }
    dt_t **toDt(dt_t **pdt) { return ::Expression_toDt(this, pdt); }
    virtual void accept(Visitor *v) { v->visit(this); }
};

class IntegerExp : public Expression
{
private:
    dinteger_t value;

public:
    IntegerExp(Loc loc, dinteger_t value, Type *type);
    IntegerExp(dinteger_t value);
    bool equals(RootObject *o);
    Expression *semantic(Scope *sc);
    char *toChars();
    dinteger_t toInteger();
    real_t toReal();
    real_t toImaginary();
    complex_t toComplex();
    int isBool(int result);
    void toMangleBuffer(OutBuffer *buf);
    Expression *toLvalue(Scope *sc, Expression *e);
    void accept(Visitor *v) { v->visit(this); }
    dinteger_t getInteger() { return value; }
    void setInteger(dinteger_t value);

private:
    void normalize();
};

class ErrorExp : public Expression
{
public:
    ErrorExp();

    Expression *toLvalue(Scope *sc, Expression *e);
    void accept(Visitor *v) { v->visit(this); }
};

class RealExp : public Expression
{
public:
    real_t value;

    RealExp(Loc loc, real_t value, Type *type);
    bool equals(RootObject *o);
    Expression *semantic(Scope *sc);
    char *toChars();
    dinteger_t toInteger();
    uinteger_t toUInteger();
    real_t toReal();
    real_t toImaginary();
    complex_t toComplex();
    int isBool(int result);
    void toMangleBuffer(OutBuffer *buf);
    void accept(Visitor *v) { v->visit(this); }
};

class ComplexExp : public Expression
{
public:
    complex_t value;

    ComplexExp(Loc loc, complex_t value, Type *type);
    bool equals(RootObject *o);
    Expression *semantic(Scope *sc);
    char *toChars();
    dinteger_t toInteger();
    uinteger_t toUInteger();
    real_t toReal();
    real_t toImaginary();
    complex_t toComplex();
    int isBool(int result);
    void toMangleBuffer(OutBuffer *buf);
    void accept(Visitor *v) { v->visit(this); }
};

class IdentifierExp : public Expression
{
public:
    Identifier *ident;
    Declaration *var;

    IdentifierExp(Loc loc, Identifier *ident);
    static IdentifierExp *create(Loc loc, Identifier *ident);
    Expression *semantic(Scope *sc);
    char *toChars();
    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *e);
    void accept(Visitor *v) { v->visit(this); }
};

class DollarExp : public IdentifierExp
{
public:
    DollarExp(Loc loc);
    void accept(Visitor *v) { v->visit(this); }
};

class DsymbolExp : public Expression
{
public:
    Dsymbol *s;
    bool hasOverloads;

    DsymbolExp(Loc loc, Dsymbol *s, bool hasOverloads = false);
    Expression *semantic(Scope *sc);
    char *toChars();
    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *e);
    void accept(Visitor *v) { v->visit(this); }
};

class ThisExp : public Expression
{
public:
    Declaration *var;

    ThisExp(Loc loc);
    Expression *semantic(Scope *sc);
    int isBool(int result);
    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *e);
    Expression *modifiableLvalue(Scope *sc, Expression *e);

    void accept(Visitor *v) { v->visit(this); }
};

class SuperExp : public ThisExp
{
public:
    SuperExp(Loc loc);
    Expression *semantic(Scope *sc);

    void accept(Visitor *v) { v->visit(this); }
};

class NullExp : public Expression
{
public:
    unsigned char committed;    // !=0 if type is committed

    NullExp(Loc loc, Type *t = NULL);
    bool equals(RootObject *o);
    Expression *semantic(Scope *sc);
    int isBool(int result);
    StringExp *toStringExp();
    void toMangleBuffer(OutBuffer *buf);
    void accept(Visitor *v) { v->visit(this); }
};

class StringExp : public Expression
{
public:
    void *string;       // char, wchar, or dchar data
    size_t len;         // number of chars, wchars, or dchars
    unsigned char sz;   // 1: char, 2: wchar, 4: dchar
    unsigned char committed;    // !=0 if type is committed
    utf8_t postfix;      // 'c', 'w', 'd'
    bool ownedByCtfe;   // true = created in CTFE

    StringExp(Loc loc, char *s);
    StringExp(Loc loc, void *s, size_t len);
    StringExp(Loc loc, void *s, size_t len, utf8_t postfix);
    static StringExp *create(Loc loc, char *s);
    //Expression *syntaxCopy();
    bool equals(RootObject *o);
    Expression *semantic(Scope *sc);
    size_t length();
    StringExp *toStringExp();
    StringExp *toUTF8(Scope *sc);
    int compare(RootObject *obj);
    int isBool(int result);
    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *e);
    Expression *modifiableLvalue(Scope *sc, Expression *e);
    unsigned charAt(uinteger_t i);
    void toMangleBuffer(OutBuffer *buf);
    void accept(Visitor *v) { v->visit(this); }
};

// Tuple

class TupleExp : public Expression
{
public:
    Expression *e0;     // side-effect part
    /* Tuple-field access may need to take out its side effect part.
     * For example:
     *      foo().tupleof
     * is rewritten as:
     *      (ref __tup = foo(); tuple(__tup.field0, __tup.field1, ...))
     * The declaration of temporary variable __tup will be stored in TupleExp::e0.
     */
    Expressions *exps;

    TupleExp(Loc loc, Expression *e0, Expressions *exps);
    TupleExp(Loc loc, Expressions *exps);
    TupleExp(Loc loc, TupleDeclaration *tup);
    Expression *syntaxCopy();
    bool equals(RootObject *o);
    Expression *semantic(Scope *sc);
    void checkEscape();

    void accept(Visitor *v) { v->visit(this); }
};

class ArrayLiteralExp : public Expression
{
public:
    Expressions *elements;
    bool ownedByCtfe;   // true = created in CTFE

    ArrayLiteralExp(Loc loc, Expressions *elements);
    ArrayLiteralExp(Loc loc, Expression *e);

    Expression *syntaxCopy();
    bool equals(RootObject *o);
    Expression *semantic(Scope *sc);
    int isBool(int result);
    StringExp *toStringExp();
    void toMangleBuffer(OutBuffer *buf);

    void accept(Visitor *v) { v->visit(this); }
};

class AssocArrayLiteralExp : public Expression
{
public:
    Expressions *keys;
    Expressions *values;
    bool ownedByCtfe;   // true = created in CTFE

    AssocArrayLiteralExp(Loc loc, Expressions *keys, Expressions *values);
    bool equals(RootObject *o);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    int isBool(int result);
    void toMangleBuffer(OutBuffer *buf);

    void accept(Visitor *v) { v->visit(this); }
};

// scrubReturnValue is running
#define stageScrub          0x1
// hasNonConstPointers is running
#define stageSearchPointers 0x2
// optimize is running
#define stageOptimize       0x4
// apply is running
#define stageApply          0x8
//inlineScan is running
#define stageInlineScan     0x10
// toCBuffer is running
#define stageToCBuffer      0x20

class StructLiteralExp : public Expression
{
public:
    StructDeclaration *sd;      // which aggregate this is for
    Expressions *elements;      // parallels sd->fields[] with
                                // NULL entries for fields to skip
    Type *stype;                // final type of result (can be different from sd's type)

    Symbol *sinit;              // if this is a defaultInitLiteral, this symbol contains the default initializer
    Symbol *sym;                // back end symbol to initialize with literal
    size_t soffset;             // offset from start of s
    int fillHoles;              // fill alignment 'holes' with zero
    bool ownedByCtfe;           // true = created in CTFE

    // pointer to the origin instance of the expression.
    // once a new expression is created, origin is set to 'this'.
    // anytime when an expression copy is created, 'origin' pointer is set to
    // 'origin' pointer value of the original expression.
    StructLiteralExp *origin;

    // those fields need to prevent a infinite recursion when one field of struct initialized with 'this' pointer.
    StructLiteralExp *inlinecopy;

    // anytime when recursive function is calling, 'stageflags' marks with bit flag of
    // current stage and unmarks before return from this function.
    // 'inlinecopy' uses similar 'stageflags' and from multiple evaluation 'doInline'
    // (with infinite recursion) of this expression.
    int stageflags;

    StructLiteralExp(Loc loc, StructDeclaration *sd, Expressions *elements, Type *stype = NULL);
    static StructLiteralExp *create(Loc loc, StructDeclaration *sd, void *elements, Type *stype = NULL);
    bool equals(RootObject *o);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    Expression *getField(Type *type, unsigned offset);
    int getFieldIndex(Type *type, unsigned offset);
    void toMangleBuffer(OutBuffer *buf);
    Expression *addDtorHook(Scope *sc);
    Symbol *toSymbol();

    void accept(Visitor *v) { v->visit(this); }
};

class DotIdExp;
DotIdExp *typeDotIdExp(Loc loc, Type *type, Identifier *ident);

class TypeExp : public Expression
{
public:
    TypeExp(Loc loc, Type *type);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    bool rvalue();
    void accept(Visitor *v) { v->visit(this); }
};

class ScopeExp : public Expression
{
public:
    ScopeDsymbol *sds;

    ScopeExp(Loc loc, ScopeDsymbol *sds);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    void accept(Visitor *v) { v->visit(this); }
};

class TemplateExp : public Expression
{
public:
    TemplateDeclaration *td;
    FuncDeclaration *fd;

    TemplateExp(Loc loc, TemplateDeclaration *td, FuncDeclaration *fd = NULL);
    bool rvalue();
    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *e);
    void accept(Visitor *v) { v->visit(this); }
};

class NewExp : public Expression
{
public:
    /* thisexp.new(newargs) newtype(arguments)
     */
    Expression *thisexp;        // if !NULL, 'this' for class being allocated
    Expressions *newargs;       // Array of Expression's to call new operator
    Type *newtype;
    Expressions *arguments;     // Array of Expression's

    CtorDeclaration *member;    // constructor function
    NewDeclaration *allocator;  // allocator function
    int onstack;                // allocate on stack

    NewExp(Loc loc, Expression *thisexp, Expressions *newargs,
        Type *newtype, Expressions *arguments);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);

    void accept(Visitor *v) { v->visit(this); }
};

class NewAnonClassExp : public Expression
{
public:
    /* thisexp.new(newargs) class baseclasses { } (arguments)
     */
    Expression *thisexp;        // if !NULL, 'this' for class being allocated
    Expressions *newargs;       // Array of Expression's to call new operator
    ClassDeclaration *cd;       // class being instantiated
    Expressions *arguments;     // Array of Expression's to call class constructor

    NewAnonClassExp(Loc loc, Expression *thisexp, Expressions *newargs,
        ClassDeclaration *cd, Expressions *arguments);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    void accept(Visitor *v) { v->visit(this); }
};

class SymbolExp : public Expression
{
public:
    Declaration *var;
    bool hasOverloads;

    SymbolExp(Loc loc, TOK op, int size, Declaration *var, bool hasOverloads);

    void accept(Visitor *v) { v->visit(this); }
};

// Offset from symbol

class SymOffExp : public SymbolExp
{
public:
    dinteger_t offset;

    SymOffExp(Loc loc, Declaration *var, dinteger_t offset, bool hasOverloads = false);
    Expression *semantic(Scope *sc);
    void checkEscape();
    int isBool(int result);

    void accept(Visitor *v) { v->visit(this); }
};

// Variable

class VarExp : public SymbolExp
{
public:
    VarExp(Loc loc, Declaration *var, bool hasOverloads = false);
    static VarExp *create(Loc loc, Declaration *var, bool hasOverloads = false);
    bool equals(RootObject *o);
    Expression *semantic(Scope *sc);
    char *toChars();
    void checkEscape();
    void checkEscapeRef();
    int checkModifiable(Scope *sc, int flag);
    bool checkReadModifyWrite();
    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *e);
    Expression *modifiableLvalue(Scope *sc, Expression *e);

    void accept(Visitor *v) { v->visit(this); }
};

// Overload Set

class OverExp : public Expression
{
public:
    OverloadSet *vars;

    OverExp(Loc loc, OverloadSet *s);
    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *e);
    void accept(Visitor *v) { v->visit(this); }
};

// Function/Delegate literal

class FuncExp : public Expression
{
public:
    FuncLiteralDeclaration *fd;
    TemplateDeclaration *td;
    TOK tok;

    FuncExp(Loc loc, FuncLiteralDeclaration *fd, TemplateDeclaration *td = NULL);
    bool rvalue();
    void genIdent(Scope *sc);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    Expression *semantic(Scope *sc, Expressions *arguments);
    MATCH matchType(Type *to, Scope *sc, FuncExp **pfe, int flag = 0);
    char *toChars();

    void accept(Visitor *v) { v->visit(this); }
};

// Declaration of a symbol

// D grammar allows declarations only as statements. However in AST representation
// it can be part of any expression. This is used, for example, during internal
// syntax re-writes to inject hidden symbols.
class DeclarationExp : public Expression
{
public:
    Dsymbol *declaration;

    DeclarationExp(Loc loc, Dsymbol *declaration);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);

    void accept(Visitor *v) { v->visit(this); }
};

class TypeidExp : public Expression
{
public:
    RootObject *obj;

    TypeidExp(Loc loc, RootObject *obj);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    void accept(Visitor *v) { v->visit(this); }
};

class TraitsExp : public Expression
{
public:
    Identifier *ident;
    Objects *args;

    TraitsExp(Loc loc, Identifier *ident, Objects *args);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    void accept(Visitor *v) { v->visit(this); }
};

class HaltExp : public Expression
{
public:
    HaltExp(Loc loc);
    Expression *semantic(Scope *sc);

    void accept(Visitor *v) { v->visit(this); }
};

class IsExp : public Expression
{
public:
    /* is(targ id tok tspec)
     * is(targ id == tok2)
     */
    Type *targ;
    Identifier *id;     // can be NULL
    TOK tok;       // ':' or '=='
    Type *tspec;        // can be NULL
    TOK tok2;      // 'struct', 'union', 'typedef', etc.
    TemplateParameters *parameters;

    IsExp(Loc loc, Type *targ, Identifier *id, TOK tok, Type *tspec,
        TOK tok2, TemplateParameters *parameters);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    void accept(Visitor *v) { v->visit(this); }
};

/****************************************************************/

class UnaExp : public Expression
{
public:
    Expression *e1;
    Type *att1; // Save alias this type to detect recursion

    UnaExp(Loc loc, TOK op, int size, Expression *e1);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc) = 0;
    Expression *unaSemantic(Scope *sc);
    Expression *resolveLoc(Loc loc, Scope *sc);

    void accept(Visitor *v) { v->visit(this); }
};

typedef Expression *(*fp_t)(Type *, Expression *, Expression *);
typedef int (*fp2_t)(Loc loc, TOK, Expression *, Expression *);

class BinExp : public Expression
{
public:
    Expression *e1;
    Expression *e2;

    Type *att1; // Save alias this type to detect recursion
    Type *att2; // Save alias this type to detect recursion

    BinExp(Loc loc, TOK op, int size, Expression *e1, Expression *e2);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc) = 0;
    Expression *binSemantic(Scope *sc);
    Expression *binSemanticProp(Scope *sc);
    Expression *checkComplexOpAssign(Scope *sc);
    int isunsigned();
    Expression *incompatibleTypes();

    Expression *reorderSettingAAElem(Scope *sc);

    void accept(Visitor *v) { v->visit(this); }
};

class BinAssignExp : public BinExp
{
public:
    BinAssignExp(Loc loc, TOK op, int size, Expression *e1, Expression *e2)
        : BinExp(loc, op, size, e1, e2)
    {
    }

    Expression *semantic(Scope *sc);

    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *ex);
    Expression *modifiableLvalue(Scope *sc, Expression *e);
    void accept(Visitor *v) { v->visit(this); }
};

/****************************************************************/

class CompileExp : public UnaExp
{
public:
    CompileExp(Loc loc, Expression *e);
    Expression *semantic(Scope *sc);
    void accept(Visitor *v) { v->visit(this); }
};

class FileExp : public UnaExp
{
public:
    FileExp(Loc loc, Expression *e);
    Expression *semantic(Scope *sc);
    void accept(Visitor *v) { v->visit(this); }
};

class AssertExp : public UnaExp
{
public:
    Expression *msg;

    AssertExp(Loc loc, Expression *e, Expression *msg = NULL);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);

    void accept(Visitor *v) { v->visit(this); }
};

class DotIdExp : public UnaExp
{
public:
    Identifier *ident;

    DotIdExp(Loc loc, Expression *e, Identifier *ident);
    static DotIdExp *create(Loc loc, Expression *e, Identifier *ident);
    Expression *semantic(Scope *sc);
    Expression *semanticX(Scope *sc);
    Expression *semanticY(Scope *sc, int flag);
    void accept(Visitor *v) { v->visit(this); }
};

class DotTemplateExp : public UnaExp
{
public:
    TemplateDeclaration *td;

    DotTemplateExp(Loc loc, Expression *e, TemplateDeclaration *td);
    Expression *semantic(Scope *sc);
    void accept(Visitor *v) { v->visit(this); }
};

class DotVarExp : public UnaExp
{
public:
    Declaration *var;
    bool hasOverloads;

    DotVarExp(Loc loc, Expression *e, Declaration *var, bool hasOverloads = false);
    Expression *semantic(Scope *sc);
    int checkModifiable(Scope *sc, int flag);
    bool checkReadModifyWrite();
    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *e);
    Expression *modifiableLvalue(Scope *sc, Expression *e);
    void accept(Visitor *v) { v->visit(this); }
};

class DotTemplateInstanceExp : public UnaExp
{
public:
    TemplateInstance *ti;

    DotTemplateInstanceExp(Loc loc, Expression *e, Identifier *name, Objects *tiargs);
    DotTemplateInstanceExp(Loc loc, Expression *e, TemplateInstance *ti);
    Expression *syntaxCopy();
    bool findTempDecl(Scope *sc);
    Expression *semantic(Scope *sc);
    Expression *semanticY(Scope *sc, int flag);
    void accept(Visitor *v) { v->visit(this); }
};

class DelegateExp : public UnaExp
{
public:
    FuncDeclaration *func;
    bool hasOverloads;

    DelegateExp(Loc loc, Expression *e, FuncDeclaration *func, bool hasOverloads = false);
    Expression *semantic(Scope *sc);

    void accept(Visitor *v) { v->visit(this); }
};

class DotTypeExp : public UnaExp
{
public:
    Dsymbol *sym;               // symbol that represents a type

    DotTypeExp(Loc loc, Expression *e, Dsymbol *sym);
    Expression *semantic(Scope *sc);
    void accept(Visitor *v) { v->visit(this); }
};

class CallExp : public UnaExp
{
public:
    Expressions *arguments;     // function arguments
    FuncDeclaration *f;         // symbol to call

    CallExp(Loc loc, Expression *e, Expressions *exps);
    CallExp(Loc loc, Expression *e);
    CallExp(Loc loc, Expression *e, Expression *earg1);
    CallExp(Loc loc, Expression *e, Expression *earg1, Expression *earg2);

    static CallExp *create(Loc loc, Expression *e, Expressions *exps);
    static CallExp *create(Loc loc, Expression *e);
    static CallExp *create(Loc loc, Expression *e, Expression *earg1);

    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *e);
    Expression *addDtorHook(Scope *sc);

    void accept(Visitor *v) { v->visit(this); }
};

class AddrExp : public UnaExp
{
public:
    AddrExp(Loc loc, Expression *e);
    Expression *semantic(Scope *sc);
    void checkEscape();
    void accept(Visitor *v) { v->visit(this); }
};

class PtrExp : public UnaExp
{
public:
    PtrExp(Loc loc, Expression *e);
    PtrExp(Loc loc, Expression *e, Type *t);
    Expression *semantic(Scope *sc);
    void checkEscapeRef();
    int checkModifiable(Scope *sc, int flag);
    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *e);
    Expression *modifiableLvalue(Scope *sc, Expression *e);

    void accept(Visitor *v) { v->visit(this); }
};

class NegExp : public UnaExp
{
public:
    NegExp(Loc loc, Expression *e);
    Expression *semantic(Scope *sc);

    void accept(Visitor *v) { v->visit(this); }
};

class UAddExp : public UnaExp
{
public:
    UAddExp(Loc loc, Expression *e);
    Expression *semantic(Scope *sc);

    void accept(Visitor *v) { v->visit(this); }
};

class ComExp : public UnaExp
{
public:
    ComExp(Loc loc, Expression *e);
    Expression *semantic(Scope *sc);

    void accept(Visitor *v) { v->visit(this); }
};

class NotExp : public UnaExp
{
public:
    NotExp(Loc loc, Expression *e);
    Expression *semantic(Scope *sc);
    void accept(Visitor *v) { v->visit(this); }
};

class BoolExp : public UnaExp
{
public:
    BoolExp(Loc loc, Expression *e, Type *type);
    Expression *semantic(Scope *sc);
    void accept(Visitor *v) { v->visit(this); }
};

class DeleteExp : public UnaExp
{
public:
    DeleteExp(Loc loc, Expression *e);
    Expression *semantic(Scope *sc);
    Expression *checkToBoolean(Scope *sc);
    void accept(Visitor *v) { v->visit(this); }
};

class CastExp : public UnaExp
{
public:
    // Possible to cast to one type while painting to another type
    Type *to;                   // type to cast to
    unsigned char mod;          // MODxxxxx

    CastExp(Loc loc, Expression *e, Type *t);
    CastExp(Loc loc, Expression *e, unsigned char mod);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    void checkEscape();

    void accept(Visitor *v) { v->visit(this); }
};

class VectorExp : public UnaExp
{
public:
    TypeVector *to;             // the target vector type before semantic()
    unsigned dim;               // number of elements in the vector

    VectorExp(Loc loc, Expression *e, Type *t);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    void accept(Visitor *v) { v->visit(this); }
};

class SliceExp : public UnaExp
{
public:
    Expression *upr;            // NULL if implicit 0
    Expression *lwr;            // NULL if implicit [length - 1]
    VarDeclaration *lengthVar;

    SliceExp(Loc loc, Expression *e1, Expression *lwr, Expression *upr);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    void checkEscape();
    void checkEscapeRef();
    int checkModifiable(Scope *sc, int flag);
    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *e);
    Expression *modifiableLvalue(Scope *sc, Expression *e);
    int isBool(int result);

    void accept(Visitor *v) { v->visit(this); }
};

class ArrayLengthExp : public UnaExp
{
public:
    ArrayLengthExp(Loc loc, Expression *e1);
    Expression *semantic(Scope *sc);

    static Expression *rewriteOpAssign(BinExp *exp);
    void accept(Visitor *v) { v->visit(this); }
};

class IntervalExp : public Expression
{
public:
    Expression *lwr;
    Expression *upr;

    IntervalExp(Loc loc, Expression *lwr, Expression *upr);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    void accept(Visitor *v) { v->visit(this); }
};

class DelegatePtrExp : public UnaExp
{
public:
    DelegatePtrExp(Loc loc, Expression *e1);
    Expression *semantic(Scope *sc);
    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *e);
    void accept(Visitor *v) { v->visit(this); }
};

class DelegateFuncptrExp : public UnaExp
{
public:
    DelegateFuncptrExp(Loc loc, Expression *e1);
    Expression *semantic(Scope *sc);
    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *e);
    void accept(Visitor *v) { v->visit(this); }
};

// e1[a0,a1,a2,a3,...]

class ArrayExp : public UnaExp
{
public:
    Expressions *arguments;             // Array of Expression's
    size_t currentDimension;            // for opDollar
    VarDeclaration *lengthVar;

    ArrayExp(Loc loc, Expression *e1, Expressions *arguments);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *e);

    void accept(Visitor *v) { v->visit(this); }
};

/****************************************************************/

class DotExp : public BinExp
{
public:
    DotExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);
    void accept(Visitor *v) { v->visit(this); }
};

class CommaExp : public BinExp
{
public:
    CommaExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);
    void checkEscape();
    void checkEscapeRef();
    int checkModifiable(Scope *sc, int flag);
    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *e);
    Expression *modifiableLvalue(Scope *sc, Expression *e);
    int isBool(int result);
    Expression *addDtorHook(Scope *sc);
    void accept(Visitor *v) { v->visit(this); }
};

class IndexExp : public BinExp
{
public:
    VarDeclaration *lengthVar;
    int modifiable;
    bool skipboundscheck;

    IndexExp(Loc loc, Expression *e1, Expression *e2);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    int checkModifiable(Scope *sc, int flag);
    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *e);
    Expression *modifiableLvalue(Scope *sc, Expression *e);

    void accept(Visitor *v) { v->visit(this); }
};

/* For both i++ and i--
 */
class PostExp : public BinExp
{
public:
    PostExp(TOK op, Loc loc, Expression *e);
    Expression *semantic(Scope *sc);
    void accept(Visitor *v) { v->visit(this); }
};

/* For both ++i and --i
 */
class PreExp : public UnaExp
{
public:
    PreExp(TOK op, Loc loc, Expression *e);
    Expression *semantic(Scope *sc);
    void accept(Visitor *v) { v->visit(this); }
};

class AssignExp : public BinExp
{
public:
    int ismemset;       // !=0 if setting the contents of an array

    AssignExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);
    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *ex);
    Expression *checkToBoolean(Scope *sc);

    void accept(Visitor *v) { v->visit(this); }
};

class ConstructExp : public AssignExp
{
public:
    ConstructExp(Loc loc, Expression *e1, Expression *e2);
    void accept(Visitor *v) { v->visit(this); }
};

class BlitExp : public AssignExp
{
public:
    BlitExp(Loc loc, Expression *e1, Expression *e2);
    void accept(Visitor *v) { v->visit(this); }
};

class AddAssignExp : public BinAssignExp
{
public:
    AddAssignExp(Loc loc, Expression *e1, Expression *e2);
    void accept(Visitor *v) { v->visit(this); }
};

class MinAssignExp : public BinAssignExp
{
public:
    MinAssignExp(Loc loc, Expression *e1, Expression *e2);
    void accept(Visitor *v) { v->visit(this); }
};

class MulAssignExp : public BinAssignExp
{
public:
    MulAssignExp(Loc loc, Expression *e1, Expression *e2);
    void accept(Visitor *v) { v->visit(this); }
};

class DivAssignExp : public BinAssignExp
{
public:
    DivAssignExp(Loc loc, Expression *e1, Expression *e2);
    void accept(Visitor *v) { v->visit(this); }
};

class ModAssignExp : public BinAssignExp
{
public:
    ModAssignExp(Loc loc, Expression *e1, Expression *e2);
    void accept(Visitor *v) { v->visit(this); }
};

class AndAssignExp : public BinAssignExp
{
public:
    AndAssignExp(Loc loc, Expression *e1, Expression *e2);
    void accept(Visitor *v) { v->visit(this); }
};

class OrAssignExp : public BinAssignExp
{
public:
    OrAssignExp(Loc loc, Expression *e1, Expression *e2);
    void accept(Visitor *v) { v->visit(this); }
};

class XorAssignExp : public BinAssignExp
{
public:
    XorAssignExp(Loc loc, Expression *e1, Expression *e2);
    void accept(Visitor *v) { v->visit(this); }
};

class PowAssignExp : public BinAssignExp
{
public:
    PowAssignExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);
    void accept(Visitor *v) { v->visit(this); }
};

class ShlAssignExp : public BinAssignExp
{
public:
    ShlAssignExp(Loc loc, Expression *e1, Expression *e2);
    void accept(Visitor *v) { v->visit(this); }
};

class ShrAssignExp : public BinAssignExp
{
public:
    ShrAssignExp(Loc loc, Expression *e1, Expression *e2);
    void accept(Visitor *v) { v->visit(this); }
};

class UshrAssignExp : public BinAssignExp
{
public:
    UshrAssignExp(Loc loc, Expression *e1, Expression *e2);
    void accept(Visitor *v) { v->visit(this); }
};

class CatAssignExp : public BinAssignExp
{
public:
    CatAssignExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);
    void accept(Visitor *v) { v->visit(this); }
};

class AddExp : public BinExp
{
public:
    AddExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);

    void accept(Visitor *v) { v->visit(this); }
};

class MinExp : public BinExp
{
public:
    MinExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);

    void accept(Visitor *v) { v->visit(this); }
};

class CatExp : public BinExp
{
public:
    CatExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);

    void accept(Visitor *v) { v->visit(this); }
};

class MulExp : public BinExp
{
public:
    MulExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);

    void accept(Visitor *v) { v->visit(this); }
};

class DivExp : public BinExp
{
public:
    DivExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);

    void accept(Visitor *v) { v->visit(this); }
};

class ModExp : public BinExp
{
public:
    ModExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);

    void accept(Visitor *v) { v->visit(this); }
};

class PowExp : public BinExp
{
public:
    PowExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);

    void accept(Visitor *v) { v->visit(this); }
};

class ShlExp : public BinExp
{
public:
    ShlExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);

    void accept(Visitor *v) { v->visit(this); }
};

class ShrExp : public BinExp
{
public:
    ShrExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);

    void accept(Visitor *v) { v->visit(this); }
};

class UshrExp : public BinExp
{
public:
    UshrExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);

    void accept(Visitor *v) { v->visit(this); }
};

class AndExp : public BinExp
{
public:
    AndExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);

    void accept(Visitor *v) { v->visit(this); }
};

class OrExp : public BinExp
{
public:
    OrExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);

    void accept(Visitor *v) { v->visit(this); }
};

class XorExp : public BinExp
{
public:
    XorExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);

    void accept(Visitor *v) { v->visit(this); }
};

class OrOrExp : public BinExp
{
public:
    OrOrExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);
    Expression *checkToBoolean(Scope *sc);
    void accept(Visitor *v) { v->visit(this); }
};

class AndAndExp : public BinExp
{
public:
    AndAndExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);
    Expression *checkToBoolean(Scope *sc);
    void accept(Visitor *v) { v->visit(this); }
};

class CmpExp : public BinExp
{
public:
    CmpExp(TOK op, Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);

    void accept(Visitor *v) { v->visit(this); }
};

class InExp : public BinExp
{
public:
    InExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);

    void accept(Visitor *v) { v->visit(this); }
};

class RemoveExp : public BinExp
{
public:
    RemoveExp(Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);
    void accept(Visitor *v) { v->visit(this); }
};

// == and !=

class EqualExp : public BinExp
{
public:
    EqualExp(TOK op, Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);

    void accept(Visitor *v) { v->visit(this); }
};

// is and !is

class IdentityExp : public BinExp
{
public:
    IdentityExp(TOK op, Loc loc, Expression *e1, Expression *e2);
    Expression *semantic(Scope *sc);
    void accept(Visitor *v) { v->visit(this); }
};

/****************************************************************/

class CondExp : public BinExp
{
public:
    Expression *econd;

    CondExp(Loc loc, Expression *econd, Expression *e1, Expression *e2);
    Expression *syntaxCopy();
    Expression *semantic(Scope *sc);
    void checkEscape();
    void checkEscapeRef();
    int checkModifiable(Scope *sc, int flag);
    int isLvalue();
    Expression *toLvalue(Scope *sc, Expression *e);
    Expression *modifiableLvalue(Scope *sc, Expression *e);
    Expression *checkToBoolean(Scope *sc);

    void accept(Visitor *v) { v->visit(this); }
};

/****************************************************************/

class DefaultInitExp : public Expression
{
public:
    TOK subop;             // which of the derived classes this is

    DefaultInitExp(Loc loc, TOK subop, int size);
    void accept(Visitor *v) { v->visit(this); }
};

class FileInitExp : public DefaultInitExp
{
public:
    FileInitExp(Loc loc);
    Expression *semantic(Scope *sc);
    Expression *resolveLoc(Loc loc, Scope *sc);
    void accept(Visitor *v) { v->visit(this); }
};

class LineInitExp : public DefaultInitExp
{
public:
    LineInitExp(Loc loc);
    Expression *semantic(Scope *sc);
    Expression *resolveLoc(Loc loc, Scope *sc);
    void accept(Visitor *v) { v->visit(this); }
};

class ModuleInitExp : public DefaultInitExp
{
public:
    ModuleInitExp(Loc loc);
    Expression *semantic(Scope *sc);
    Expression *resolveLoc(Loc loc, Scope *sc);
    void accept(Visitor *v) { v->visit(this); }
};

class FuncInitExp : public DefaultInitExp
{
public:
    FuncInitExp(Loc loc);
    Expression *semantic(Scope *sc);
    Expression *resolveLoc(Loc loc, Scope *sc);
    void accept(Visitor *v) { v->visit(this); }
};

class PrettyFuncInitExp : public DefaultInitExp
{
public:
    PrettyFuncInitExp(Loc loc);
    Expression *semantic(Scope *sc);
    Expression *resolveLoc(Loc loc, Scope *sc);
    void accept(Visitor *v) { v->visit(this); }
};

/****************************************************************/

/* Special values used by the interpreter
 */
extern Expression *EXP_CANT_INTERPRET;
extern Expression *EXP_CONTINUE_INTERPRET;
extern Expression *EXP_BREAK_INTERPRET;
extern Expression *EXP_GOTO_INTERPRET;
extern Expression *EXP_VOID_INTERPRET;

Expression *expType(Type *type, Expression *e);

Expression *Neg(Type *type, Expression *e1);
Expression *Com(Type *type, Expression *e1);
Expression *Not(Type *type, Expression *e1);
Expression *Bool(Type *type, Expression *e1);
Expression *Cast(Type *type, Type *to, Expression *e1);
Expression *ArrayLength(Type *type, Expression *e1);
Expression *Ptr(Type *type, Expression *e1);

Expression *Add(Type *type, Expression *e1, Expression *e2);
Expression *Min(Type *type, Expression *e1, Expression *e2);
Expression *Mul(Type *type, Expression *e1, Expression *e2);
Expression *Div(Type *type, Expression *e1, Expression *e2);
Expression *Mod(Type *type, Expression *e1, Expression *e2);
Expression *Pow(Type *type, Expression *e1, Expression *e2);
Expression *Shl(Type *type, Expression *e1, Expression *e2);
Expression *Shr(Type *type, Expression *e1, Expression *e2);
Expression *Ushr(Type *type, Expression *e1, Expression *e2);
Expression *And(Type *type, Expression *e1, Expression *e2);
Expression *Or(Type *type, Expression *e1, Expression *e2);
Expression *Xor(Type *type, Expression *e1, Expression *e2);
Expression *Index(Type *type, Expression *e1, Expression *e2);
Expression *Cat(Type *type, Expression *e1, Expression *e2);

Expression *Equal(TOK op, Type *type, Expression *e1, Expression *e2);
Expression *Cmp(TOK op, Type *type, Expression *e1, Expression *e2);
Expression *Identity(TOK op, Type *type, Expression *e1, Expression *e2);

Expression *Slice(Type *type, Expression *e1, Expression *lwr, Expression *upr);

// Const-folding functions used by CTFE

void sliceAssignArrayLiteralFromString(ArrayLiteralExp *existingAE, StringExp *newval, size_t firstIndex);
void sliceAssignStringFromArrayLiteral(StringExp *existingSE, ArrayLiteralExp *newae, size_t firstIndex);
void sliceAssignStringFromString(StringExp *existingSE, StringExp *newstr, size_t firstIndex);

int sliceCmpStringWithString(StringExp *se1, StringExp *se2, size_t lo1, size_t lo2, size_t len);
int sliceCmpStringWithArray(StringExp *se1, ArrayLiteralExp *ae2, size_t lo1, size_t lo2, size_t len);


#endif /* DMD_EXPRESSION_H */
