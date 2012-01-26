#include <iostream>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
#include "parser.h"
#include "draw.h"

class error
{
    public:
    std::string errstring;
    error(std::string);
};

typedef struct g_data {
    double left;
    double right;
    double top;
    double bottom;
    double front;
    double back;
    double yaw;
    double pitch;
    double mousex;
    double mousey;
    double time;
    double dt;
    bool is3d;
    int detail;
    std::vector <colorf> colors;
    colorf currentcolor;
    unsigned int colorindex;
    } g_data;

class arglist_member
{
    public:
    tagged_value v;
    arglist_member* next;
    arglist_member();
    ~arglist_member();
};

class procedure
{
    public:
    std::vector <std::string> args;
    block* entrypoint;
    procedure();
    procedure(std::vector <std::string>, block*);
    ~procedure();
};

class interpreter
{
    public:
    procedure* proc;
    tagged_value temp;
    tagged_value returnvalue;
    std::map<std::string, tagged_value> vars;
    std::map<std::string, dfuncd> funcs;
    std::vector <std::string> strings;
    std::vector < std::map <int, tagged_value> > arrays;
    std::vector <procedure*> procedures;    //pointers to procedures are maintained here for cleanup in destructor - actually called from vars map.
    g_data data;
    bool receivedequals;
    bool receivedinequal;

    void getnextcolor();
    void evaluate(block*);
    void evaluate(statement*);
    tagged_value evaluate(expression*);
    tagged_value evaluate(comparison*);
    tagged_value evaluate(sum*);
    tagged_value evaluate(term*);
    tagged_value evaluate(value*);
    void evaluate(explicitplot*);
    void evaluate(implicitplot*);
    void evaluate(parametricplot*);
    int addstring(std::string);     // finds/adds new string, returns string index.


    interpreter (std::map<std::string, dfuncd>, g_data);
    ~interpreter();
};
