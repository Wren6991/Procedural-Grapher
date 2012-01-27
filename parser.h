#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include "tokenizer.h"


class block;
class procedurecall;
class functioncall;
class expression;

typedef enum value_type
{
    val_nil = 0,
    val_number,
    val_string,
    val_procedure,
    val_array,
    val_func
} value_type;

class procedure;

class tagged_value
{
    public:
    value_type type;
    union
    {
        double n;
        int str;    //index in interpreter's string table
        procedure* proc;
        int arr;
    } val;
    tagged_value();
    tagged_value(double);
    tagged_value(procedure*);
    bool operator<(const tagged_value&) const;
};


class value;

class arrayinitializer
{
    public:
    std::vector<expression*> explist;
};

class arrayitem
{
    public:
    value* array;
    expression* index;
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
    arrayinitializer* arrinit;
    arrayitem* arritem;

    bool expd;
    value* b;
    ~value();
};

class term
{
    public:
    bool fixed;
    double nvalue;
    std::vector <value*> values;
    std::vector <token_type_enum> operators;
    ~term();
};

class sum
{
    public:
    bool fixed;
    double nvalue;
    std::vector <term*> terms;
    std::vector <token_type_enum> operators;
    ~sum();
};

class comparison
{
    public:
    bool fixed;
    double nvalue;
    sum *a;
    token_type_enum oper;
    sum *b;
    ~comparison();
};

class expression
{
    public:
    bool fixed;
    double nvalue;
    std::vector <comparison*> comparisons;
    std::vector <token_type_enum> operators;
    ~expression();
};

class ifstatement
{
    public:
    expression* cond;
    block* ifblock;
    std::vector <ifstatement*> elseifs;
    bool haselse;
    block* elseblock;
    ~ifstatement();
};

class whilestatement
{
    public:
    expression* cond;
    block* whileblock;
    ~whilestatement();
};

class forstatement
{
    public:
    std::string id;
    expression* a;
    expression* b;
    block* forblock;
    ~forstatement();
};

class defstatement
{
    public:
    std::string name;
    std::vector <std::string> args;
    block* entrypoint;
    ~defstatement();
};

class procedurecall
{
    public:
    std::string name;
    std::vector <expression*> args;
    ~procedurecall();
};

class functioncall
{
    public:
    int id;
    std::vector<expression*> args;
    ~functioncall();
};

class assg_lvalue
{
    public:
    bool isarray;
    std::string str;
    arrayitem* ai;
    assg_lvalue();
    assg_lvalue(std::string);
};

class assignment
{
    public:
    assg_lvalue lvalue;
    expression* rvalue;
    bool ismultiple;
    std::vector <assg_lvalue> extra_lvalues;
    std::vector <expression*> extra_rvalues;
    ~assignment();
};

class explicitplot
{
    public:
    std::string rangevar;
    expression* expr;
    ~explicitplot();
};

class implicitplot
{
    public:
    bool haseq;
    bool hasineq;
    expression* expr;
    ~implicitplot();
};

class parametricplot
{
    public:
    std::string parname;
    bool givenfrom;
    expression* from;
    bool givento;
    expression* to;
    bool givenstep;
    expression* step;
    std::vector <assignment*> assignments;
    ~parametricplot();
};

class returnstatement
{
    public:
    expression* expr;
    ~returnstatement();
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
        returnstatement* returnstat;
        parametricplot* parplot;
        } stat;
    ~statement();
};

class block
{
    public:
    std::vector <statement*> statements;
    ~block();
};

class parser
{
    public:
    std::vector <token> tokens;
    unsigned int tindex;
    token t;
    token last;
    bool accepted_equals;
    bool accepted_ineq;     //for parsing implicit plots :)

    std::map<std::string, int> funcnames;
    void gettoken();
    parser();
    parser(std::vector <token>);
    parser(std::vector <token>, std::map <std::string, dfuncd>);
    bool accept(token_type_enum);
    void expect(token_type_enum);
    void jumpto(int);
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
