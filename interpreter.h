#include <iostream>
#include <vector>
#include <map>
#include <cmath>
#include "parser.h"
#include "draw.h"

typedef struct g_data {
    double left;
    double right;
    double top;
    double bottom;
    int detail;
    std::vector <colorf> colors;
    colorf currentcolor;
    unsigned int colorindex;
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
    bool receivedequals;
    bool receivedinequal;

    void getnextcolor();
    void evaluate(block*);
    void evaluate(statement*);
    double evaluate(expression*);
    double evaluate(sum*);
    double evaluate(term*);
    double evaluate(value*);
    void evaluate(explicitplot*);
    void evaluate(implicitplot*);

    interpreter (block*, std::map<std::string, dfuncd>, g_data);
    ~interpreter();
};
