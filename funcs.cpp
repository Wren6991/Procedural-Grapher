#include <cmath>
#include <wx/textctrl.h>
#include "interpreter.h"

extern wxTextCtrl *OutputBox;
extern interpreter *interp_ptr;     // we should parameterize this: make the interpreter stack-based, push arguments onto stack, and pass the interpreter state pointer into the function. Problem solved!
extern std::map <std::string, tagged_value> persistent_vars;
extern std::string type_names[];

double lnbodge(double x)            //identical for positive x; differentiates correctly for -x (where it is usually undefined)
{
    if (x > 0)
        return log(x);
    else
        return log(-x) - 1000000;
}
tagged_value log_tv(arglist_member* arg)
{
    if (arg == NULL)
        throw(error("Error: expected more arguments to function \"ln\""));
    if (arg->v.type == val_number)
        return tagged_value(lnbodge(arg->v.val.n));
    else
        throw(error("Error: expected number as argument to function \"ln\""));
}

tagged_value sin_tv(arglist_member* arg)
{
    if (arg == NULL)
        throw(error("Error: expected more arguments to function \"sin\""));
    if (arg->v.type == val_number)
        return tagged_value(sin(arg->v.val.n));
    else
        throw(error("Error: expected number as argument to function \"sin\""));
}

tagged_value cos_tv(arglist_member* arg)
{
    if (arg == NULL)
        throw(error("Error: expected more arguments to function \"cos\""));
    if (arg->v.type == val_number)
        return tagged_value(cos(arg->v.val.n));
    else
        throw(error("Error: expected number as argument to function \"cos\""));
}

tagged_value tan_tv(arglist_member* arg)
{
    if (arg == NULL)
        throw(error("Error: expected more arguments to function \"tan\""));
    if (arg->v.type == val_number)
        return tagged_value(tan(arg->v.val.n));
    else
        throw(error("Error: expected number as argument to function \"tan\""));
}

tagged_value asin_tv(arglist_member* arg)
{
    if (arg == NULL)
        throw(error("Error: expected more arguments to function \"asin\""));
    if (arg->v.type == val_number)
        return tagged_value(asin(arg->v.val.n));
    else
        throw(error("Error: expected number as argument to function \"asin\""));
}

tagged_value acos_tv(arglist_member* arg)
{
    if (arg == NULL)
        throw(error("Error: expected more arguments to function \"acos\""));
    if (arg->v.type == val_number)
        return tagged_value(acos(arg->v.val.n));
    else
        throw(error("Error: expected number as argument to function \"acos\""));
}

tagged_value atan_tv(arglist_member* arg)
{
    if (arg == NULL)
        throw(error("Error: expected more arguments to function \"atan\""));
    if (arg->v.type == val_number)
        return tagged_value(atan(arg->v.val.n));
    else
        throw(error("Error: expected number as argument to function \"atan\""));
}

tagged_value abs_tv(arglist_member* arg)
{
    if (arg == NULL)
        throw(error("Error: expected more arguments to function \"abs\""));
    if (arg->v.type == val_number)
        return tagged_value(fabs(arg->v.val.n));
    else
        throw(error("Error: expected number as argument to function \"abs\""));
}

tagged_value floor_tv(arglist_member* arg)
{
    if (arg == NULL)
        throw(error("Error: expected more arguments to function \"floor\""));
    if (arg->v.type == val_number)
        return tagged_value(floor(arg->v.val.n));
    else
        throw(error("Error: expected number as argument to function \"floor\""));
}

tagged_value ceil_tv(arglist_member* arg)
{
    if (arg == NULL)
        throw(error("Error: expected more arguments to function \"ceil\""));
    if (arg->v.type == val_number)
        return tagged_value(ceil(arg->v.val.n));
    else
        throw(error("Error: expected number as argument to function \"ceil\""));
}

tagged_value sqrt_tv(arglist_member* arg)
{
    if (arg == NULL)
        throw(error("Error: expected more arguments to function \"sqrt\""));
    if (arg->v.type == val_number)
        return tagged_value(sqrt(arg->v.val.n));
    else
        throw(error("Error: expected number as argument to function \"sqrt\""));
}

tagged_value print_tv(arglist_member* arg)
{
    while (arg != NULL)
    {
        if (arg->v.type == val_number)
            (*OutputBox) << arg->v.val.n;
        else if (arg->v.type == val_string)
            (*OutputBox) << interp_ptr->strings[arg->v.val.str];
        else if (arg->v.type == val_array)
            (*OutputBox) << "(table: " << arg->v.val.arr << ")";
        else if (arg->v.type == val_nil)
            (*OutputBox) << "(nil)";
        else
            throw(error("Error: expected number or string as argument to function \"print\""));
        arg = arg->next;
    }
    (*OutputBox) << "\n";
    return tagged_value();
}

tagged_value size_tv(arglist_member* arg)
{
    if (arg == NULL)
        throw(error("Error: expected more arguments to function \"ln\""));
    if (arg->v.type == val_array)
        return tagged_value(interp_ptr->arrays[arg->v.val.arr].size());
    else if (arg->v.type == val_string)
        return tagged_value(interp_ptr->strings[arg->v.val.str].size());
    else
        throw(error("Error: expected array or string as argument to function \"size\""));
}

tagged_value char_tv(arglist_member* arg)
{
    if (arg == NULL)
        throw(error("Error: expected more arguments to function \"ln\""));
    if (arg->v.type == val_number)
    {
            tagged_value rv;
            rv.type = val_string;
            char str[] = {0, 0};
            str[0] = static_cast <char> (arg->v.val.n);            //wow i'm tired
            rv.val.str = interp_ptr->addstring(std::string(str));
            return rv;
    }
    else
        throw(error("Error: expected number as argument to function \"char\""));
}

tagged_value rand_tv(arglist_member* arg)
{
    if (arg == NULL)
        return tagged_value(rand() / (float)RAND_MAX);
    else
    {
        if (arg->v.type != val_number)
            throw(error("Error: expected number as argument to function \"char\""));
        return tagged_value(rand() % static_cast <int> (arg->v.val.n));
    }
}

tagged_value prng_tv(arglist_member *arg)
{
    /*srand(123456789);
    while (arg != NULL)
    {
        srand(arg->v.val.n * 10267 * (arg->v.val.n + 3571) + rand() * (arg->v.val.n - 7057));
        rand();
        arg = arg->next;

    }
    return tagged_value(rand() / (float)RAND_MAX);*/
    double x = arg->v.val.n;
    double y = arg->next->v.val.n;
    double seed = 17 + 13 * (pow(5, x) - pow(7, 1 - x) + (pow(11, y) - pow(19, 2 - y)) / 2.0);
    seed = seed - (long)(seed / 5011) * 5011;
    srand(seed);
    rand();
    rand();
    return tagged_value(rand() / static_cast <double> (RAND_MAX));
}

tagged_value getpersistent(arglist_member *arg)
{
    if (arg == NULL || arg->v.type != val_string)
        throw(error("Error: expected string as argument to function \"getpersistent\""));
    return persistent_vars[interp_ptr->strings[arg->v.val.str]];
}

tagged_value setpersistent(arglist_member *arg)
{
    if (arg == NULL || arg->v.type != val_string)
        throw(error("Error: expected string as first argument to function \"setpersistent\""));
    if (arg->next == NULL)
        throw(error("Error: expected more arguments to function \"setpersistent\""));
    persistent_vars[interp_ptr->strings[arg->v.val.str]] = arg->next->v;
    return tagged_value();
}

tagged_value type_tv(arglist_member *arg)
{
    tagged_value rv;
    if (arg == NULL)
    {
        rv.val.str = interp_ptr->addstring("nil");
        rv.type = val_string;
    }
    else
    {
        rv.val.str = interp_ptr->addstring(type_names[arg->v.type]);
        rv.type = val_string;
    }
    return rv;
}

tagged_value mandelbrot_tv(arglist_member *arg)
{
    if (!(arg && arg->next))
        throw(error("Error: mandelbrot expects two arguments."));
    double ca = arg->v.val.n;
    double cb = arg->next->v.val.n;
    double za = ca;
    double zb = cb;
    double zatemp;
    int i;
    for (i = 0; i < 50 && za * za + zb * zb < 4; i++)
    {
        zatemp = za;
        za = za * za - zb * zb + ca;
        zb = 2 * zatemp * zb + cb;
    }
    return tagged_value((double)i / 50.0);
}
