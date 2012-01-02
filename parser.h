#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include "tokenizer.h"


class block;
class procedurecall;
class functioncall;
class expression;

class procedure
{
    public:
    std::vector <std::string> args;
    block* blk;
};

class value
{
    public:
    token_type_enum type;
    bool negative;

    double n;
    std::string var;
    expression* expr;
    procedurecall* proccall;
    functioncall* funccall;

    bool expd;
    value* exponent;
};

class term
{
    public:
    bool fixed;
    double nvalue;
    std::vector <value*> values;
    std::vector <token_type_enum> operators;
};

class sum
{
    public:
    bool fixed;
    double nvalue;
    std::vector <term*> terms;
    std::vector <token_type_enum> operators;
};

class comparison
{
    public:
    bool fixed;
    double nvalue;
    sum *a;
    token_type_enum oper;
    sum *b;
};

class expression
{
    public:
    comparison* a;
};

class ifstatement
{
    public:
    expression* cond;
    block* ifblock;
    std::vector <ifstatement*> elseifs;
    bool haselse;
    block* elseblock;
};

class whilestatement
{
    public:
    expression* cond;
    block* whileblock;
};

class forstatement
{
    public:
    std::string id;
    expression* a;
    expression* b;
    block* forblock;
};

class defstatement
{
    public:
    std::string name;
    std::vector <std::string> args;
    block* entrypoint;
};

class procedurecall
{
    public:
    std::string name;
    std::vector <expression*> args;
};

class functioncall
{
    public:
    std::string name;
    expression* arg;
};

class assignment
{
    public:
    std::string id;
    expression* rvalue;
};

class explicitplot
{
    public:
    std::string rangevar;
    expression* expr;
};

class implicitplot
{
    public:
    expression* exp;
};

class statement
{
    public:
    token_type_enum type;
    union {
        ifstatement* ifstat;
        whilestatement* whilestat;
        forstatement* forstat;
        defstatement* defstat;
        procedurecall* procstat;
        functioncall* funcstat;
        assignment* assignstat;
        explicitplot* expplot;
        implicitplot* impplot;
        } stat;
};

class block
{
    public:
    std::vector <statement*> statements;
};

class parser
{
    public:
    std::vector <token> tokens;
    int tindex;
    token t;
    token last;

    std::map<std::string, dfuncd> funcs;
    void gettoken();
    parser(std::vector <token>);
    parser(std::vector <token>, std::map <std::string, dfuncd>);
    bool accept(token_type_enum);
    void expect(token_type_enum);
    expression* expr();
    comparison* comp();
    sum* sm();
    term* trm();
    value* val();
    statement* stat();
    block* blk();
    explicitplot* expplot();
    implicitplot* impplot();
};



#endif // PARSER_H_INCLUDED
