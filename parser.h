#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include "tokenizer.h"

typedef struct g_data {
    double left;
    double right;
    double top;
    double bottom;
    int detail;
    } g_data;

class procedure
{
    public:
    procedure();
    procedure(int, std::vector <std::string>);
    int entrypoint;
    std::vector <std::string> args;
};

class parser
{
    public:
    g_data data;
    std::vector <token> tokens;
    int tindex;
    token t;
    token last;

    std::map<std::string, double> vars;
    std::map<std::string, dfuncd> funcs;
    std::map<std::string, procedure> procedures;
    std::map<int, int> jumps;
    std::vector <int> stack;
    void gettoken();
    parser(std::vector <token>);
    parser(std::vector <token>, std::map <std::string, dfuncd>);
    parser(std::vector <token>, std::map <std::string, dfuncd>, g_data data_);
    bool accept(token_type_enum);
    void expect(token_type_enum);
    double expression();
    double comp();
    double sum();
    double term();
    double value();
    double statement();
    double block();
    void explicitplot(std::string);
    void implicitplot(int);
    void jumpto(int);
    void skipblock();
    void skiptoend();
};



#endif // PARSER_H_INCLUDED
