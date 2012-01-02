#include <iostream>
#include <vector>
#include <map>
#include "parser.h"

typedef union valuedata{
    double n;
    std::string var;
    expression* expr;
    procedurecall* proccall;
    functioncall* funccall;
    } valuedata;

class block;
class procedurecall;
class functioncall;
class expression;

class value
{
    token_type_enum type;
    bool negative;
    valuedata val;
    bool expd
    valuedata exponent;

}

class term
{
    bool fixed;
    double nvalue;
    std::vector <value*> values;
    std::vector <token_type_enum> operators;
}

class sum
{
    bool fixed;
    double nvalue;
    std::vector <term*> terms;
    std::vector <token_type_enum> operators;
}

class comparison
{
    bool fixed;
    double nvalue;
    sum *a;
    token_type_enum operator;
    sum *b;
}

class ifstatement
{
    expression* cond;
    block* ifblock;
    std::vector <ifstatement*> elseifs;
    block* elseblock;
}

class whilestatement
{
    expression* cond;
    block* whileblock;
}

class forstatement
{
    std::string id;
    int a;
    int b;
    block* forblock;
}

class defstatement
{
    std::string name;
    std::vector <std::string> args;
    block* entrypoint;
}

class procedurecall
{
    std::string name;
    std::vector <expression*> args;
}

class functioncall
{
    std::string name;
    expression* arg;
}

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
        } stat;
    statement (ifstatement*);
    statement (whilestatement*);
    statement (forstatement*);
    statement (procedurecall*);
    statement (functioncall*);
}

class block
{
    public:
    std::vector <statement*> statements;
    block(std::vector <statement*>);
}
