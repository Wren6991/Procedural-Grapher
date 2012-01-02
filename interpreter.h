#include <iostream>
#include <vector>
#include <map>
#include "parser.h"

typedef struct g_data {
    double left;
    double right;
    double top;
    double bottom;
    int detail;
    } g_data;

class interpreter
{
    public:
    block* program;
    std::vector <block*> stack;
    std::map<std::string, double> vars;
    std::map<std::string, dfuncd> funcs;
    std::map<std::string, procedure> procedures;
    g_data data;

    void evaluate(block*);
    void evaluate(statement*);
    double evaluate(expression);

    interpreter (block*);
};
