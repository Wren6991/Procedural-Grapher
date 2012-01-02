#include "parser.h"


class interpreter
{
    public:
    node root;
    double expression();
    double term();
    double value();
    interpreter(node);
}
