#include "interpreter.h"
#include <wx/glcanvas.h>
#include <algorithm>
#include <sstream>

const int MAX_STACK_LEVEL = 800;
const double PI = 3.14159265358979323846264338327950288419716939937510;  // TODO: make this more precise

std::string val_names[] = {
    "nil",
    "number",
    "string",
    "procedure",
    "array"
    };

#include "msdata.h"

double vertlist[12][3];
vert3f normallist[12];


interpreter::interpreter(std::map<std::string, dfuncd> funcs_, g_data data_)
{
    for(std::map <std::string, dfuncd>::iterator iter = funcs_.begin(); iter != funcs_.end(); iter++)
    {
        funcs.push_back(iter->second);
    }
    data = data_;
    vars["mousex"] = data.mousex;
    vars["mousey"] = data.mousey;
    vars["time"] = data.time / 1000.0;
    vars["dt"] = data.dt / 1000.0;
    vars["pi"] = PI;
    vars["e"] = 2.71828183;
    pass_self = false;
    stacklevel = 0;
}

interpreter::~interpreter()
{
    std::map<std::string, tagged_value>::iterator iter;
    for (iter = vars.begin(); iter != vars.end(); iter++)
        switch (iter->second.type)
        {
            case val_number:
                break;
            case val_string:
                //delete iter->second.val.str;
                break;
            case val_procedure:
                break;
            case val_array:
                break;
            default:
                break;
        }

    /*for (int i = 0; i < arrays.size(); i++)
    {
        std::cout << "Enumerating array " << i << ":\n";
        std::map<int, tagged_value>::iterator iter = arrays[i].begin();
        for(; iter != arrays[i].end(); iter++)
            std::cout << iter->first << ": (" << val_names[iter->second.type] << ") " << iter->second.val.str << "\n";

    }*/

}

void interpreter::getnextcolor()
{
    data.colorindex++;
    if (data.colorindex >= data.colors.size())
        data.colorindex = 0;
    data.currentcolor = data.colors[data.colorindex];
}

void interpreter::evaluate(block* blk)
{
    for(unsigned int i = 0; i < blk->statements.size(); i++)
    {
        evaluate(blk->statements[i]);
    }
}

void interpreter::evaluate(statement* stat)
{
    tagged_value temp;
   switch(stat->type)
    {
        case t_while:
            temp = evaluate(stat->stat.whilestat->cond);
            while (temp.type == val_number && temp.val.n > 0)
            {
                evaluate(stat->stat.whilestat->whileblock);
                temp = evaluate(stat->stat.whilestat->cond);
            }
            break;
        case t_for:
            int a, b;
            temp = evaluate(stat->stat.forstat->a);
            if (temp.type != val_number)
                throw(error("Error: attempt to use non-numeric value as loop bound"));
            a = temp.val.n;
            temp = evaluate(stat->stat.forstat->b);
            if (temp.type != val_number)
                throw(error("Error: attempt to use non-numeric value as loop bound"));
            b = temp.val.n;
            for (int i = a; i <= b; i++)
            {
                vars[stat->stat.forstat->id] = tagged_value((double)i);
                evaluate(stat->stat.forstat->forblock);
            }
            break;
        case t_if:
            temp = evaluate(stat->stat.ifstat->cond);
            if (temp.type == val_number && temp.val.n > 0)  //if first condition is true
            {
                evaluate(stat->stat.ifstat->ifblock);
            }
            else
            {
                bool blocked = false;
                for (unsigned int i = 0; i < stat->stat.ifstat->elseifs.size(); i++)     //for each elseif statement
                {
                temp = evaluate(stat->stat.ifstat->elseifs[i]->cond);
                    if (temp.type == val_number && temp.val.n > 0)  //if elseif condition is true
                    {
                        evaluate(stat->stat.ifstat->elseifs[i]->ifblock);        //evaluate block
                        blocked = true;
                        break;
                    }
                }
                if(!blocked && stat->stat.ifstat->haselse)                      //evaluate else block:
                    evaluate(stat->stat.ifstat->elseblock);
            }
            break;
        case t_let:
            if (stat->stat.assignstat->ismultiple)
            {
                int n_ass = stat->stat.assignstat->extra_lvalues.size();
                tagged_value singleval = evaluate(stat->stat.assignstat->rvalue);
                tagged_value *vals = new tagged_value[n_ass];
                for (int i = 0; i < n_ass; i++)
                {
                    vals[i] = evaluate(stat->stat.assignstat->extra_rvalues[i]);
                }
                if (!stat->stat.assignstat->lvalue.isarray)
                    vars[stat->stat.assignstat->lvalue.str] = singleval;
                else
                {
                    temp = evaluate(stat->stat.assignstat->lvalue.ai->array->arritem->array);
                    if (temp.type != val_array)
                        throw(error("Error: attempt to index non-array (" + val_names[temp.type] + ")"));
                    arrays[temp.val.arr][evaluate(stat->stat.assignstat->lvalue.ai->array->arritem->index)] = singleval;
                }
                for (int i = 0; i < n_ass; i++)
                {
                    if (!stat->stat.assignstat->extra_lvalues[i].isarray)
                        vars[stat->stat.assignstat->extra_lvalues[i].str] = vals[i];
                    else
                    {
                        temp = evaluate(stat->stat.assignstat->extra_lvalues[i].ai->array->arritem->array);     // array item -> value that contains the arrayitem -> arrayitem class (contains actual array and the index) -> actual array
                        if (temp.type != val_array)
                             throw(error("Error: attempt to index non-array (" + val_names[temp.type] + ")"));
                        //std::cout << "Assigning to array " << temp.val.arr << ", index " << evaluate(stat->stat.assignstat->extra_lvalues[i].ai->array->arritem->index).val.n << ", type " << val_names[vals[i].type] << "\n";

                        arrays[temp.val.arr][evaluate(stat->stat.assignstat->extra_lvalues[i].ai->array->arritem->index)] = vals[i];
                        //std::cout << "assigned a " << val_names[vals[i].type] << " to array number " << temp.val.arr << "\n";
                    }
                }
                delete vals;
            }
            else
                if (!stat->stat.assignstat->lvalue.isarray)
                    vars[stat->stat.assignstat->lvalue.str] = evaluate(stat->stat.assignstat->rvalue);
                else
                {
                    temp = evaluate(stat->stat.assignstat->lvalue.ai->array->arritem->array);
                    if (temp.type != val_array)
                         throw(error("Error: attempt to index non-array (" + val_names[temp.type] + ")"));
                        //std::cout << "Assigning to array " << temp.val.arr << ", index " << evaluate(stat->stat.assignstat->lvalue.ai->array->arritem->index).val.n << ", type " << val_names[evaluate(stat->stat.assignstat->rvalue).type] << "\n";
                        tagged_value index = evaluate(stat->stat.assignstat->lvalue.ai->array->arritem->index);
                        tagged_value rvalue = evaluate(stat->stat.assignstat->rvalue);
                        //std::cout << "assigned a " << val_names[rvalue.type] << " to array number " << temp.val.arr << ", index type " << val_names[index.type] << "\n";
                        arrays[temp.val.arr][index] = rvalue;

                }
            break;
        case t_func:
        {
            arglist_member* arglist_top = NULL;
            arglist_member* arglist_current = NULL;
            arglist_member* arglist_next = NULL;
            if (stat->stat.funcstat->args.size() > 0)
            {
                arglist_top = new arglist_member();
                arglist_top->v = evaluate(stat->stat.funcstat->args[0]);
                arglist_current = arglist_top;
            }
            for (unsigned int i = 1; i < stat->stat.funcstat->args.size(); i++)
            {
                arglist_next = new arglist_member;
                arglist_next->v = evaluate(stat->stat.funcstat->args[i]);
                arglist_current->next = arglist_next;
                arglist_current = arglist_next;
            }
            if (arglist_current != NULL)
                arglist_current->next = NULL;
            //std::cout << stat->stat.funcstat->id << "\n";
            funcs[stat->stat.funcstat->id](arglist_top);
            if (arglist_top != NULL)
                delete arglist_top;
            break;
        }
        case s_plot_exp:
            evaluate(stat->stat.expplot);
            break;
        case s_plot_imp:
            evaluate(stat->stat.impplot);
            break;
        case t_def:
            //std::cout << "Defining procedure " << stat->stat.defstat->name << "\n";
            proc =  new procedure(stat->stat.defstat->args, stat->stat.defstat->entrypoint);
            procedures.push_back(proc);
            vars[stat->stat.defstat->name] = tagged_value(proc);
            break;
        case n_procedure:
            stacklevel++;
            if (stacklevel > MAX_STACK_LEVEL)
                throw(error("Error: Maximum stack size exceeded"));
            temp = evaluate(stat->stat.procstat->name);
            if (temp.type != val_procedure) //not defined
            {
                std::stringstream ss;
                ss << "Error: procedure \"" << stat->stat.procstat->name << "\" is not defined.";
                throw(error(ss.str()));
            }
            proc = temp.val.proc;

            if(stat->stat.procstat->args.size() == proc->args.size())
            {
                for (unsigned int i = 0; i < proc->args.size(); i++)
                {
                    vars[proc->args[i]] = evaluate(stat->stat.procstat->args[i]);
                }
                try
                {
                    evaluate(proc->entrypoint);
                }
                catch (token_type_enum t)
                {
                    if (t != t_return)
                        throw(t);
                }
                stacklevel--;
            }
            else
            {
                if (stat->stat.procstat->args.size() > proc->args.size())
                    throw(error("Error: too many arguments to procedure"));
                else
                    throw(error("Error: too few arguments to procedure"));
            }
            break;
        case t_return:
            returnvalue = evaluate(stat->stat.returnstat->expr);
            throw(t_return);
            break;
        case t_par:
            evaluate(stat->stat.parplot);
            break;
        default:
            throw(stat->type);
    }
}

double min(double a, double b)
{
    return a < b ? a : b;
}

double max(double a, double b)
{
    return a > b ? a : b;
}

tagged_value interpreter::evaluate(expression *expr)
{
    tagged_value rv = evaluate(expr->comparisons[0]);
    if (rv.type == val_number)
    {
        for(unsigned int i = 0; i < expr->operators.size(); i++)
        {
            temp = evaluate(expr->comparisons[i + 1]);
            if (temp.type != val_number)
                throw(error("Error: attempt to compare number with non-number."));
            if (expr->operators[i] == t_and)

                rv.val.n = min(rv.val.n, temp.val.n);
            else
                rv.val.n = max(rv.val.n, temp.val.n);
        }
    }
    return rv;
}

tagged_value interpreter::evaluate(comparison *comp)
{
    tagged_value rv = evaluate(comp->a);    // rv => return value;
    if (comp->oper == t_eof)
        return rv;
    temp = evaluate(comp->b);
    if (temp.type != val_number)
        throw(error("Error: attempt to compare number with non-number."));
    switch (comp->oper)
    {
        case t_lessthan:
            rv.val.n = temp.val.n - rv.val.n;
            break;
        case t_greaterthan:
            rv.val.n = rv.val.n - temp.val.n;
            break;
        case t_lteq:
            rv.val.n = temp.val.n - rv.val.n;
            if (rv.val.n == 0)
                rv.val.n = 0.00001;
            break;
        case t_gteq:
            rv.val.n = rv.val.n - temp.val.n;
            if (rv.val.n == 0)
                rv.val.n = 0.00001;
            break;
        case t_equals:
            rv.val.n = temp.val.n - rv.val.n - 100000;
            if(rv.val.n == -100000)
                rv.val.n = 0.00001;
            break;
        default:
            break;
    }
    return rv;
}

tagged_value interpreter::evaluate(sum* s)
{
    tagged_value rv = evaluate(s->terms[0]);
    if (rv.type != val_number)
        return rv;
    for(unsigned int i = 0; i < s->operators.size(); i++)
    {
        temp = evaluate(s->terms[i + 1]);
        if (temp.type != val_number)
            throw(error("Error: attempted to perform arithmetic on non-number."));
        if (s->operators[i] == t_plus)
            rv.val.n += temp.val.n;
        else
            rv.val.n -= temp.val.n;
    }
    return rv;
}

tagged_value interpreter::evaluate(term* t)
{
    tagged_value rv = evaluate(t->values[0]);
    if (rv.type != val_number)
        return rv;
    for(unsigned int i = 0; i < t->operators.size(); i++)
    {
        temp = evaluate(t->values[i + 1]);
        if (temp.type != val_number)
            throw(error("Error: attempted to perform arithmetic on non-number."));
        if (t->operators[i] == t_times)
            rv.val.n *= temp.val.n;
        else
            rv.val.n /= temp.val.n;
    }
    return rv;
}

tagged_value interpreter::evaluate(value *v)
{
    tagged_value rv;
    switch (v->type)
    {
        case t_number:
            rv.type = val_number;
            rv.val.n = v->n;
            break;
        case t_id:
            rv = vars[v->var];
            break;
        case t_func:
        {
            arglist_member* arglist_top;
            arglist_member* arglist_current;
            arglist_member* arglist_next;
            arglist_top = NULL;
            if (v->funccall->args.size() > 0)
            {
                arglist_top = new arglist_member;
                arglist_top->v = evaluate(v->funccall->args[0]);
                arglist_current = arglist_top;
            }
            for (unsigned int i = 1; i < v->funccall->args.size(); i++)
            {
                arglist_next = new arglist_member;
                arglist_next->v = evaluate(v->funccall->args[i]);
                arglist_current->next = arglist_next;
                arglist_current = arglist_next;
                arglist_current->next = NULL;
            }

            rv = funcs[v->funccall->id](arglist_top);
            if (arglist_top != NULL)
                delete arglist_top;
            break;
        }
        case n_expression:
            rv = evaluate(v->expr);
            break;
        case n_procedure:
        {
            stacklevel++;
            if (stacklevel > MAX_STACK_LEVEL)
                throw(error("Error: Maximum stack size exceeded"));
            std::map <std::string, tagged_value> temps;
            rv = evaluate(v->proccall->name);
            if (rv.type != val_procedure) //not defined
                throw(n_procedure);
            proc = rv.val.proc;
            unsigned int argoffset;
            if (pass_self && proc->args.size() > 0)      //obj:method()  ->  obj.method(obj)
            {
                argoffset = 1;
                temps[proc->args[0]] = vars[proc->args[0]];
                vars[proc->args[0]] = temp;
                pass_self = false;
            }
            else
            {
                argoffset = 0;
            }
            if(v->proccall->args.size() + argoffset == proc->args.size())
            {
                for (unsigned int i = argoffset; i < proc->args.size(); i++)            //oooo  ->  xooo
                {
                    temps[proc->args[i]] = vars[proc->args[i]];
                    vars[proc->args[i]] = evaluate(v->proccall->args[i - argoffset]);
                }
                try
                {
                    evaluate(proc->entrypoint);
                }
                catch (token_type_enum t)
                {
                    if (t != t_return)
                        throw(t);
                    rv = returnvalue;
                }
                for (std::map <std::string, tagged_value>::iterator iter = temps.begin(); iter != temps.end(); iter++)
                {
                    vars[iter->first] = iter->second;
                }
                stacklevel--;
            }
            else
            {
                std::cout << v->proccall->args.size() + argoffset << " != " << proc->args.size() << "\n";
                if (v->proccall->args.size() + argoffset > proc->args.size())
                    throw(error("Error: too many arguments to procedure "));
                else
                    throw(error("Error: too few arguments to procedure"));
            }
            break;
        }
        case t_lsquareb:
            rv = evaluate(v->arritem->array);
            if (rv.type == val_string)
            {
                rv = tagged_value(static_cast <double> (strings[rv.val.str][evaluate(v->arritem->index).val.n]));
            }
            else if (rv.type != val_array)
                throw(error("Error: attempt to index non-array"));
            else
            {
                //std::cout << "indexing array " << rv.val.arr << ", index " << evaluate(v->arritem->index).val.n << "\n";
                if (v->arritem->pass_self)
                {
                    pass_self = true;
                    temp = rv;
                }
                rv = arrays[rv.val.arr][evaluate(v->arritem->index)];
            }
            break;
        case t_lbrace:
            rv.type = val_array;
            rv.val.arr = arrays.size();
            arrays.push_back(std::map<tagged_value, tagged_value>());
            //std::cout << "Creating array " << rv.val.arr << ":\n";
            for (unsigned int i = 0; i < v->arrinit->explist.size(); i++)
            {
                temp = evaluate(v->arrinit->explist[i]);
                arrays[rv.val.arr][i].type = temp.type;
                arrays[rv.val.arr][i].val = temp.val;
                //std::cout << "Pushed " << val_names[arrays[rv.val.arr][i].type] << " to array " << rv.val.arr << "\n";
            }
            //std::cout << "Created array " << rv.val.arr << ", size " << v->arrinit->explist.size() << "\n";
            break;
        case t_dif:
            {
                rv = evaluate(v->b);
                if (rv.type != val_number)
                    throw(error("Error: attempt to differentiate non-numeric expression"));
                double tempval = vars[v->var].val.n;
                if (temp.type != val_number)
                    throw(error("Error: attempt to differentiate with respect to non-numeric variable"));
                vars[v->var].val.n += 0.00001;
                rv.val.n = (evaluate(v->b).val.n - rv.val.n) / 0.00001;
                vars[v->var].val.n = tempval;
            }
            break;
        case t_string:
            rv.type = val_string;
            rv.val.str = addstring(v->var);
            break;
        default:
            //n = 0;
            break;
    }
    if (v->expd)
    {
        temp = evaluate(v->b);
        if (rv.type != val_number || temp.type != val_number)
             throw(error("Error: attempted to perform arithmetic on non-number."));
        rv.val.n = pow(rv.val.n, temp.val.n);
    }

    if (v->negative)
    {
        if (rv.type != val_number)
             throw(error("Error: attempted to perform arithmetic on non-number."));
        rv.val.n = -rv.val.n;
    }
    //std::cout << "Returning " << val_names[rv.type] << ": " << (rv.type == val_number ? rv.val.n : rv. val.arr) << "";
    //std::cout << "\n";
    return rv;
}

void interpreter::evaluate(explicitplot* relation)
{
    setcolor(data.currentcolor);
    if (!data.is3d)
    {
        if (relation->rangevar == "y")
        {
            double x, lastx, lasty, step;
            tagged_value y;
            x = data.left;
            step = (data.right - data.left)/data.detail;
            vars["x"].type = val_number;
            vars["x"].val.n = x;
            y = evaluate(relation->expr);
            if (y.type != val_number)
                throw(error("Error: attempt to plot non-numeric expression"));
            while(x < data.right + step)
            {
                lastx = x;
                lasty = y.val.n;
                x += step;
                vars["x"].val.n = x;
                y = evaluate(relation->expr);
                line2(lastx, lasty, x, y.val.n);
            }
        }
        else if (relation->rangevar == "x")
        {
            double y, lastx, lasty, step;
            tagged_value x;
            y = data.bottom;
            step = (data.top - data.bottom)/data.detail;
            vars["y"].type = val_number;
            vars["y"].val.n = y;
            x = evaluate(relation->expr);
            if (x.type != val_number)
                throw(error("Error: attempt to plot non-numeric expression"));
            while(y < data.top + step)
            {
                lastx = x.val.n;
                lasty = y;
                y += step;
                vars["y"] = y;
                x = evaluate(relation->expr);
                line2(lastx, lasty, x.val.n, y);
            }
        }
        else if (relation->rangevar == "r")
        {
            double  theta, lastr, lasttheta, step;
            tagged_value r;
            theta = -10;
            step = 10.0/data.detail;
            vars["theta"].type = val_number;
            vars["theta"].val.n = theta;
            r = evaluate(relation->expr);
            if (r.type != val_number)
                throw(error("Error: attempt to plot non-numeric expression"));
            while(theta < 10 + step)
            {
                lastr = r.val.n;
                lasttheta = theta;
                theta += step;
                vars["theta"].val.n = theta;
                r = evaluate(relation->expr);
                line2(lastr * cos(lasttheta), lastr * sin(lasttheta), r.val.n * cos(theta), r.val.n * sin(theta));
            }
        }
        else
        {
            std::stringstream ss;
            ss << "Error: " << relation->rangevar << " is not a valid subject for a relation";
            throw (ss.str());
        }
    }
    else
    {
        if (relation->rangevar == "y")
        {
            int ncells = data.detail / 2 + 1;
            double** grid = new double*[ncells + 3];
            for (int i = 0; i < ncells + 3; i++)
                grid[i] = new double[ncells + 3];             //    "fencing" vertex (+1) and 1 step padding on each side for normal calculation (+2).
            double stepx = (data.right - data.left) / ncells;
            double stepz = (data.front - data.back) / ncells;
            double x, z;
            vars["x"].type = val_number;
            vars["z"].type = val_number;
            if(evaluate(relation->expr).type != val_number)
                throw(error("Error: attempt to plot non-numeric expression"));
            x = data.left - stepx;
            for (int i = 0; i < ncells + 3; i++)
            {
                vars["x"].val.n = x;
                z = data.back - stepz;
                for(int j = 0; j < ncells + 3; j++)
                {
                    vars["z"].val.n = z;
                    grid[i][j] = evaluate(relation->expr).val.n;
                    z += stepz;
                }
                x += stepx;
            }
            vert3f** normals = new vert3f*[ncells + 1];
            for(int i = 0; i < ncells + 1; i++)
                normals[i] = new vert3f[ncells + 1];

            for(int i = 0; i <= ncells; i++)
                for(int j = 0; j <= ncells; j++)
                {
                    normals[i][j] = vert3f(
                                           (grid[i+2][j+1] - grid[i][j+1]) * stepz * 2,
                                           -(stepx * stepz * 4),
                                           (grid[i+1][j+2] - grid[i+1][j]) * stepx * 2
                                           );                                           //cross product of the two tangent vectors
                }
            double lastx = data.left;

            int lasti = 0;
            double lastz;
            int lastj;
            for (int i = 1; i <= ncells; i++)
            {
                x = lastx + stepx;
                lastj = 0;
                lastz = data.back;
                for(int j = 1; j <= ncells; j++)
                {
                    z = lastz + stepz;
                    glBegin(GL_POLYGON);
                    glNormal3f(normals[lasti][lastj].x, normals[lasti][lastj].y, normals[lasti][lastj].z);
                    glVertex3d(lastx, grid[lasti + 1][lastj + 1], lastz);   // +1 because of padding on left and bottom of grid (for normal calcs)
                    glNormal3f(normals[i][lastj].x, normals[i][lastj].y, normals[i][lastj].z);
                    glVertex3d(x, grid[i + 1][lastj + 1], lastz);
                    glNormal3f(normals[i][j].x, normals[i][j].y, normals[i][j].z);
                    glVertex3d(x, grid[i + 1][j + 1], z);
                    glNormal3f(normals[lasti][j].x, normals[lasti][j].y, normals[lasti][j].z);
                    glVertex3d(lastx, grid[lasti + 1][j + 1], z);
                    glEnd();
                    lastz = z;
                    lastj = j;
                }
                lastx = x;
                lasti = i;
            }
            for (int i = 0; i < ncells + 3; i++)
                delete[] grid[i];
            delete[] grid;
            for (int i = 0; i < ncells + 1; i++)
                delete[] normals[i];
            delete[] normals;
        }
        else if (relation->rangevar == "x")
        {
            int ncells = data.detail / 2 + 1;
            double** grid = new double*[ncells + 3];
            for (int i = 0; i < ncells + 3; i++)
                grid[i] = new double[ncells + 3];             //    "fencing" vertex (+1) and 1 step padding on each side for normal calculation (+2).
            double stepy = (data.top - data.bottom) / ncells;
            double stepz = (data.front - data.back) / ncells;
            double y, z;
            vars["y"].type = val_number;
            vars["z"].type = val_number;
            if(evaluate(relation->expr).type != val_number)
                throw(error("Error: attempt to plot non-numeric expression"));
            y = data.bottom - stepy;
            for (int i = 0; i < ncells + 3; i++)
            {
                vars["y"].val.n = y;
                z = data.back - stepz;
                for(int j = 0; j < ncells + 3; j++)
                {
                    vars["z"].val.n = z;
                    grid[i][j] = evaluate(relation->expr).val.n;
                    z += stepz;
                }
                y += stepy;
            }
            vert3f** normals = new vert3f*[ncells + 1];
            for(int i = 0; i < ncells + 1; i++)
                normals[i] = new vert3f[ncells + 1];

            for(int i = 0; i <= ncells; i++)
                for(int j = 0; j <= ncells; j++)
                {
                    normals[i][j] = vert3f(
                                           (stepy * stepz * 4),
                                           -(grid[i+2][j+1] - grid[i][j+1]) * stepz * 2,
                                           -(grid[i+1][j+2] - grid[i+1][j]) * stepy * 2
                                           );                                           //cross product of the two tangent vectors
                }
            double lasty = data.bottom;

            int lasti = 0;
            double lastz;
            int lastj;
            for (int i = 1; i <= ncells; i++)
            {
                y = lasty + stepy;
                lastj = 0;
                lastz = data.back;
                for(int j = 1; j <= ncells; j++)
                {
                    z = lastz + stepz;
                    glBegin(GL_POLYGON);
                    glNormal3f(normals[lasti][lastj].x, normals[lasti][lastj].y, normals[lasti][lastj].z);
                    glVertex3d(grid[lasti + 1][lastj + 1], lasty, lastz);   // +1 because of padding on left and bottom of grid (for normal calcs)
                    glNormal3f(normals[i][lastj].x, normals[i][lastj].y, normals[i][lastj].z);
                    glVertex3d(grid[i + 1][lastj + 1], y, lastz);
                    glNormal3f(normals[i][j].x, normals[i][j].y, normals[i][j].z);
                    glVertex3d(grid[i + 1][j + 1], y, z);
                    glNormal3f(normals[lasti][j].x, normals[lasti][j].y, normals[lasti][j].z);
                    glVertex3d(grid[lasti + 1][j + 1], lasty, z);
                    glEnd();
                    lastz = z;
                    lastj = j;
                }
                lasty = y;
                lasti = i;
            }
            for (int i = 0; i < ncells + 3; i++)
                delete grid[i];
            delete grid;
            for (int i = 0; i < ncells + 1; i++)
                delete normals[i];
            delete normals;
        }
        else if (relation->rangevar == "z")
        {
            int ncells = data.detail / 2 + 1;
            double** grid = new double*[ncells + 3];
            for (int i = 0; i < ncells + 3; i++)
                grid[i] = new double[ncells + 3];             //    "fencing" vertex (+1) and 1 step padding on each side for normal calculation (+2).
            double stepx = (data.right - data.left) / ncells;
            double stepy = (data.top - data.bottom) / ncells;
            double x, y;
            vars["x"].type = val_number;
            vars["y"].type = val_number;
            if(evaluate(relation->expr).type != val_number)
                throw(error("Error: attempt to plot non-numeric expression"));
            x = data.left - stepx;
            for (int i = 0; i < ncells + 3; i++)
            {
                vars["x"].val.n = x;
                y = data.bottom - stepy;
                for(int j = 0; j < ncells + 3; j++)
                {
                    vars["y"].val.n = y;
                    grid[i][j] = evaluate(relation->expr).val.n;
                    y += stepy;
                }
                x += stepx;
            }
            vert3f** normals = new vert3f*[ncells + 1];
            for(int i = 0; i < ncells + 1; i++)
                normals[i] = new vert3f[ncells + 1];

            for(int i = 0; i <= ncells; i++)
                for(int j = 0; j <= ncells; j++)
                {
                    normals[i][j] = vert3f(
                                           -(grid[i+2][j+1] - grid[i][j+1]) * stepy * 2,
                                           -(grid[i+1][j+2] - grid[i+1][j]) * stepx * 2,
                                            (stepx * stepy * 4)
                                          );                                           //cross product of the two tangent vectors
                }
            double lastx = data.left;

            int lasti = 0;
            double lasty;
            int lastj;
            for (int i = 1; i <= ncells; i++)
            {
                x = lastx + stepx;
                lastj = 0;
                lasty = data.bottom;
                for(int j = 1; j <= ncells; j++)
                {
                    y = lasty + stepy;
                    glBegin(GL_POLYGON);
                    glNormal3f(normals[lasti][lastj].x, normals[lasti][lastj].y, normals[lasti][lastj].z);
                    glVertex3d(lastx, lasty, grid[lasti + 1][lastj + 1]);   // +1 because of padding on left and bottom of grid (for normal calcs)
                    glNormal3f(normals[i][lastj].x, normals[i][lastj].y, normals[i][lastj].z);
                    glVertex3d(x, lasty, grid[i + 1][lastj + 1]);
                    glNormal3f(normals[i][j].x, normals[i][j].y, normals[i][j].z);
                    glVertex3d(x, y, grid[i + 1][j + 1]);
                    glNormal3f(normals[lasti][j].x, normals[lasti][j].y, normals[lasti][j].z);
                    glVertex3d(lastx, y, grid[lasti + 1][j + 1]);
                    glEnd();
                    lasty = y;
                    lastj = j;
                }
                lastx = x;
                lasti = i;
            }
            for (int i = 0; i < ncells + 3; i++)
                delete grid[i];
            delete grid;
            for (int i = 0; i < ncells + 1; i++)
                delete normals[i];
            delete normals;
        }
        else if (relation->rangevar == "r")
        {
            int ncells = data.detail / 2 + 1;
            vert3f** grid = new vert3f*[ncells + 3];        // mapping polar coord (theta, phi) to cartesian (x, y, z) - makes normal calcs + rendering easier.
            for (int i = 0; i < ncells + 3; i++)
                grid[i] = new vert3f[ncells + 3];           //    "fencing" vertex (+1) and 1 step padding on each side for normal calculation (+2).
            double steptheta = PI * 2 / ncells;
            double stepphi = PI * 2 / ncells;
            double theta, phi, r;
            vars["theta"].type = val_number;
            vars["phi"].type = val_number;
            if(evaluate(relation->expr).type != val_number)
                throw(error("Error: attempt to plot non-numeric expression"));
            theta = -PI;
            for (int i = 0; i < ncells + 3; i++)
            {
                vars["theta"].val.n = theta;
                phi = -PI;
                for(int j = 0; j < ncells + 3; j++)
                {
                    vars["phi"].val.n = phi;
                    r = evaluate(relation->expr).val.n;
                    grid[i][j] = vert3f(r * cos(theta) * cos(phi), r * sin(theta), r * cos(theta) * sin(phi));
                    //std::cout << "r: " << r << "v: (" << grid[i][j].x << ", " << grid[i][j].y << ", " << grid[i][j].z << ")\n";
                    phi += stepphi;
                }
                theta += steptheta;
            }
            vert3f** normals = new vert3f*[ncells + 1];
            for(int i = 0; i < ncells + 1; i++)
                normals[i] = new vert3f[ncells + 1];

            vert3f tana, tanb;
            for(int i = 0; i <= ncells; i++)
                for(int j = 0; j <= ncells; j++)
                {
                    tana = grid[i+2][j+1] - grid[i][j+1]; //tangent vector in line with theta polar rotation
                    tanb = grid[i+1][j+2] - grid[i+1][j]; //tangent vector in line with phi polar rotation
                    normals[i][j] = vert3f(
                                           tana.y * tanb.z - tana.z * tanb.y,
                                           tana.z * tanb.x - tana.x * tanb.z,
                                           tana.x * tanb.y - tana.y * tanb.x
                                          );                                            //cross product of the two tangent vectors
                }

            int lasti = 0;
            int lastj;

            vert3f currentvert;
            for (int i = 1; i <= ncells; i++)
            {
                lastj = 0;
                for(int j = 1; j <= ncells; j++)
                {

                    glBegin(GL_POLYGON);
                    glNormal3f(normals[lasti][lastj].x, normals[lasti][lastj].y, normals[lasti][lastj].z);
                    currentvert = grid[lasti + 1][lastj + 1];                          // +1 because of padding on left and bottom of grid (for normal calcs)
                    glVertex3f(currentvert.x, currentvert.y, currentvert.z);
                    glNormal3f(normals[i][lastj].x, normals[i][lastj].y, normals[i][lastj].z);
                    currentvert = grid[i + 1][lastj + 1];
                    glVertex3f(currentvert.x, currentvert.y, currentvert.z);
                    glNormal3f(normals[i][j].x, normals[i][j].y, normals[i][j].z);
                    currentvert = grid[i + 1][j + 1];
                    glVertex3f(currentvert.x, currentvert.y, currentvert.z);
                    glNormal3f(normals[lasti][j].x, normals[lasti][j].y, normals[lasti][j].z);
                    currentvert = grid[lasti + 1][j + 1];
                    glVertex3f(currentvert.x, currentvert.y, currentvert.z);
                    glEnd();
                    lastj = j;
                }
                lasti = i;
            }
            for (int i = 0; i < ncells + 3; i++)
                delete grid[i];
            delete grid;
            for (int i = 0; i < ncells + 1; i++)
                delete normals[i];
            delete normals;
        }
        else            // not x, y, z, r
        {
            std::stringstream ss;
            ss << "Error: " << relation->rangevar << " is not a valid subject for a relation";
            throw (ss.str());
        }

    }
    getnextcolor();
}

void interpreter::evaluate(implicitplot* relation)
{
    if (!data.is3d)
    {
        if(evaluate(relation->expr).type != val_number)
            return; //throw(error("Error: attempt to plot non-numeric expression"));    Disabled to allow for more complex function calls without breaking the parser
        bool equalsonly = relation->haseq && !relation->hasineq;
        int ncells = data.detail / 2 + 1;
        double** grid = new double*[ncells + 1];
        for (int i = 0; i <= ncells; i++)
            grid[i] = new double[ncells + 1];
        double stepx = (data.right - data.left) / (ncells - 1);
        double stepy = (data.top - data.bottom) / (ncells - 1);
        double x, y;
        tagged_value xtemp, ytemp, rtemp, thetatemp;
        xtemp = vars["x"];
        ytemp = vars["y"];
        rtemp = vars["r"];
        thetatemp = vars["theta"];
        vars["x"].type = val_number;
        vars["y"].type = val_number;
        x = floor(data.left/stepx) * stepx;
        for (int i = 0; i <= ncells; i++)
        {
            vars["x"].val.n = x;
            y = floor(data.bottom/stepy) * stepy;
            for(int j = 0; j <= ncells; j++)
            {
                vars["y"].val.n = y;
                vars["theta"] = atan2(y, x);
                vars["r"] = sqrt(x*x + y*y);
                grid[i][j] = evaluate(relation->expr).val.n;
                if (equalsonly)
                    grid[i][j] = (grid[i][j] == 0.00001 ? -100000 : grid[i][j]) + 100000;   //undo logical value munging.
                y += stepy;
            }
            x += stepx;
        }
        double lastx = floor(data.left/stepx) * stepx;
        x = lastx + stepx;
        int lasti = 0;
        double lasty;
        int lastj, mscase;

        if (!(relation->haseq||relation->hasineq))
        {
            for (int i = 1; i <= ncells; i++)
            {
                lastj = 0;
                lasty = floor(data.bottom/stepy) * stepy;
                y = lasty + stepy;
                for(int j = 1; j <= ncells; j++)
                {
                    {
                        rect2_4a(lastx, lasty, x, y, data.currentcolor, grid[lasti][lastj], grid[i][lastj], grid[lasti][j], grid[i][j]);
                    }
                    lasty = y;
                    y += stepy;
                    lastj = j;
                }
                lastx = x;
                x += stepx;
                lasti = i;
            }
        }
        else if (relation->haseq && !relation->hasineq)
        {
            setcolor(data.currentcolor);
            for (int i = 1; i <= ncells; i++)
            {
                lastj = 0;
                lasty = floor(data.bottom/stepy) * stepy;
                y = lasty + stepy;
                for(int j = 1; j <= ncells; j++)
                {
                    mscase = 0;
                    if (grid[lasti][lastj] > 0)
                        mscase |= 1;
                    if (grid[i][lastj] > 0)
                        mscase |= 2;
                    if (grid[i][j] > 0)
                        mscase |= 4;
                    if(grid[lasti][j] > 0)
                        mscase |= 8;
                    switch (mscase)
                    {
                        case 0:
                            break;
                        case 1:
                            line2(lastx, lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]), lastx + stepx * grid[lasti][lastj] / (grid[lasti][lastj] - grid[i][lastj]), lasty);
                            break;
                        case 2:
                            line2(x, lasty + stepy * grid[i][lastj] / (grid[i][lastj] - grid[i][j]), lastx + stepx * grid[lasti][lastj] / (grid[lasti][lastj] - grid[i][lastj]), lasty);
                            break;
                        case 3:
                            line2(lastx, lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]), x, lasty + stepy * grid[i][lastj] / (grid[i][lastj] - grid[i][j]));
                            break;
                        case 4:
                            line2(lastx + stepx * grid[lasti][j] / (grid[lasti][j] - grid[i][j]), y, x, lasty + stepy * grid[i][lastj] / (grid[i][lastj] - grid[i][j]));
                            break;
                        case 5:
                            line2(lastx, lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]), lastx + stepx * grid[lasti][lastj] / (grid[lasti][lastj] - grid[i][lastj]), lasty);
                            line2(lastx + stepx * grid[lasti][j] / (grid[lasti][j] - grid[i][j]), y, x, lasty + stepy * grid[i][lastj] / (grid[i][lastj] - grid[i][j]));
                            break;
                        case 6:
                            line2(lastx + stepx * grid[lasti][lastj] / (grid[lasti][lastj] - grid[i][lastj]), lasty, lastx + stepx * grid[lasti][j] / (grid[lasti][j] - grid[i][j]), y);
                            break;
                        case 7:
                            line2(lastx + stepx * grid[lasti][j] / (grid[lasti][j] - grid[i][j]), y, lastx, lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]));
                            break;
                        case 15:
                            break;
                        case 14:
                            line2(lastx, lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]), lastx + stepx * grid[lasti][lastj] / (grid[lasti][lastj] - grid[i][lastj]), lasty);
                            break;
                        case 13:
                            line2(x, lasty + stepy * grid[i][lastj] / (grid[i][lastj] - grid[i][j]), lastx + stepx * grid[lasti][lastj] / (grid[lasti][lastj] - grid[i][lastj]), lasty);
                            break;
                        case 12:
                            line2(lastx, lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]), x, lasty + stepy * grid[i][lastj] / (grid[i][lastj] - grid[i][j]));
                            break;
                        case 11:
                            line2(lastx + stepx * grid[lasti][j] / (grid[lasti][j] - grid[i][j]), y, x, lasty + stepy * grid[i][lastj] / (grid[i][lastj] - grid[i][j]));
                            break;
                        case 10:
                            line2(lastx, lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]), lastx + stepx * grid[lasti][lastj] / (grid[lasti][lastj] - grid[i][lastj]), lasty);
                            line2(lastx + stepx * grid[lasti][j] / (grid[lasti][j] - grid[i][j]), y, x, lasty + stepy * grid[i][lastj] / (grid[i][lastj] - grid[i][j]));
                            break;
                        case 9:
                            line2(lastx + stepx * grid[lasti][lastj] / (grid[lasti][lastj] - grid[i][lastj]), lasty, lastx + stepx * grid[lasti][j] / (grid[lasti][j] - grid[i][j]), y);
                            break;
                        case 8:
                            line2(lastx + stepx * grid[lasti][j] / (grid[lasti][j] - grid[i][j]), y, lastx, lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]));
                            break;
                        default:
                            break;
                    }
                    lasty = y;
                    y += stepy;
                    lastj = j;
                }
                lastx = x;
                x += stepx;
                lasti = i;
            }
        }
        else
        {
            setcolor(data.currentcolor, 0.5);
            for (int i = 1; i <= ncells; i++)
            {
                lastj = 0;
                lasty = floor(data.bottom/stepy) * stepy;
                y = lasty + stepy;
                for(int j = 1; j <= ncells; j++)
                {
                    mscase = 0;
                    if (grid[lasti][lastj] > 0)
                        mscase |= 1;
                    if (grid[i][lastj] > 0)
                        mscase |= 2;
                    if (grid[i][j] > 0)
                        mscase |= 4;
                    if(grid[lasti][j] > 0)
                        mscase |= 8;
                    switch (mscase)
                    {
                        case 0:
                            break;
                        case 1:
                            triangle2(lastx, lasty, lastx, lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]), lastx + stepx * grid[lasti][lastj] / (grid[lasti][lastj] - grid[i][lastj]), lasty);
                            break;
                        case 2:
                            triangle2(x, lasty + stepy * grid[i][lastj] / (grid[i][lastj] - grid[i][j]), x, lasty, lastx + stepx * grid[lasti][lastj] / (grid[lasti][lastj] - grid[i][lastj]), lasty);
                            break;
                        case 3:
                            quad2(lastx, lasty, lastx, lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]), x, lasty + stepy * grid[i][lastj] / (grid[i][lastj] - grid[i][j]), x, lasty);
                            break;
                        case 4:
                            triangle2(lastx + stepx * grid[lasti][j] / (grid[lasti][j] - grid[i][j]), y, x, lasty + stepy * grid[i][lastj] / (grid[i][lastj] - grid[i][j]), x, y);
                            break;
                        case 5:
                            triangle2(lastx, lasty, lastx, lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]), lastx + stepx * grid[lasti][lastj] / (grid[lasti][lastj] - grid[i][lastj]), lasty);
                            triangle2(lastx + stepx * grid[lasti][j] / (grid[lasti][j] - grid[i][j]), y, x, y, x, lasty + stepy * grid[i][lastj] / (grid[i][lastj] - grid[i][j]));
                            break;
                        case 6:
                            quad2(x, lasty, lastx + stepx * grid[lasti][lastj] / (grid[lasti][lastj] - grid[i][lastj]), lasty, lastx + stepx * grid[lasti][j] / (grid[lasti][j] - grid[i][j]), y, x, y);
                            break;
                        case 7:
                            pentagon2(lastx, lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]), lastx + stepx * grid[lasti][j] / (grid[lasti][j] - grid[i][j]), y, x, y, x, lasty, lastx, lasty);
                            break;
                        case 15:
                            quad2(lastx, lasty, lastx, y, x, y, x, lasty);
                            break;
                        case 14:
                            pentagon2(lastx + stepx * grid[lasti][lastj] / (grid[lasti][lastj] - grid[i][lastj]), lasty, lastx, lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]), lastx, y, x, y, x, lasty);
                            break;
                        case 13:
                            pentagon2(x, lasty + stepy * grid[i][lastj] / (grid[i][lastj] - grid[i][j]), lastx + stepx * grid[lasti][lastj] / (grid[lasti][lastj] - grid[i][lastj]), lasty, lastx, lasty, lastx, y, x, y);
                            break;
                        case 12:
                            quad2(lastx, lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]), lastx, y, x, y, x, lasty + stepy * grid[i][lastj] / (grid[i][lastj] - grid[i][j]));
                            break;
                        case 11:
                            pentagon2(x, lasty, lastx, lasty, lastx, y, lastx + stepx * grid[lasti][j] / (grid[lasti][j] - grid[i][j]), y, x, lasty + stepy * grid[i][lastj] / (grid[i][lastj] - grid[i][j]));
                            break;
                        case 10:
                            triangle2(lastx, lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]), lastx, y, lastx + stepx * grid[lasti][j] / (grid[lasti][j] - grid[i][j]), y);
                            triangle2(lastx + stepx * grid[lasti][lastj] / (grid[lasti][lastj] - grid[i][lastj]), lasty, x, lasty + stepy * grid[i][lastj] / (grid[i][lastj] - grid[i][j]), x, lasty);
                            break;
                        case 9:
                            quad2(lastx, lasty, lastx, y, lastx + stepx * grid[lasti][j] / (grid[lasti][j] - grid[i][j]), y, lastx + stepx * grid[lasti][lastj] / (grid[lasti][lastj] - grid[i][lastj]), lasty);
                            break;
                        case 8:
                            triangle2(lastx, lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]), lastx, y, lastx + stepx * grid[lasti][j] / (grid[lasti][j] - grid[i][j]), y);
                            break;
                        default:
                            break;
                    }
                    lasty = y;
                    y += stepy;
                    lastj = j;
                }
                lastx = x;
                x += stepx;
                lasti = i;
            }
        }
        for (int i = 0; i <= ncells; i++)
            delete grid[i];
        delete grid;
        vars["x"] = xtemp;
        vars["y"] = ytemp;
        vars["r"] = rtemp;
        vars["theta"] = thetatemp;
    }
    else
    {
        bool equalsonly = relation->haseq && !relation->hasineq;
        int ncells = data.detail / 6;
        double*** grid = data.grid3;

        double stepx = (data.right - data.left) / (ncells - 1);
        double stepy = (data.top - data.bottom) / (ncells - 1);
        double stepz = (data.front - data.back) / (ncells - 1);
        double x, y, z;
        if(evaluate(relation->expr).type != val_number)
            return; //throw(error("Error: attempt to plot non-numeric expression"));
        vars["x"].type = val_number;
        vars["y"].type = val_number;
        vars["z"].type = val_number;
        x = floor(data.left/stepx) * stepx - stepx;
        for (int i = 0; i < ncells + 3; i++)
        {
            vars["x"].val.n = x;
            y = floor(data.bottom/stepy) * stepy - stepy;
            for(int j = 0; j < ncells + 3; j++)
            {
                vars["y"].val.n = y;
                z = floor(data.back/stepz) * stepz - stepz;
                for (int k = 0; k < ncells + 3; k++)
                {
                    vars["z"].val.n = z;
                    grid[i][j][k] = evaluate(relation->expr).val.n;
                    if (equalsonly)
                        grid[i][j][k] = (grid[i][j][k] == 0.00001 ? -100000 : grid[i][j][k]) + 100000;   //undo logical value munging.
                   z += stepz;
                }
                y += stepy;
            }
            x += stepx;
        }

        vert3f*** normals = data.normals;
        /*for (int i = 0; i <= ncells; i++)
        {
            normals[i] = new vert3f*[ncells + 1];
            for(int j = 0; j <= ncells; j++)
                normals[i][j] = new vert3f[ncells + 1];
        }*/


        x = floor(data.left/stepx) * stepx;
        setcolor(data.currentcolor);
        if (!relation->haseq)
        {
            glDisable(GL_LIGHTING);
            glBegin(GL_POINTS);
            for (int i = 1; i <= ncells; i++)
            {
                y = floor(data.bottom/stepy) * stepy;
                for(int j = 1; j <= ncells; j++)
                {
                    z = floor(data.back/stepz) * stepz;
                    for (int k = 1; k <= ncells; k++)
                    {
                        if (grid[i][j][k] >= 0)
                        {

                            glVertex3f(x, y, z);
                        }
                       z += stepz;
                    }
                    y += stepy;
                }
                x += stepx;
            }
            glEnd();
            glEnable(GL_LIGHTING);
        }
        else
        {
            int mcase, edges;
            double lastx, lasty, lastz;
            int lasti, lastj, lastk;
            for (int i = 1; i <= ncells + 1; i++)
            {
                for(int j = 1; j <= ncells + 1; j++)
                {
                    for (int k = 1; k <= ncells + 1; k++)
                    {
                        normals[i - 1][j - 1][k - 1] = vert3f(
                                                              grid[i+1][j  ][k  ] - grid[i-1][j  ][k  ],
                                                              grid[i  ][j+1][k  ] - grid[i  ][j-1][k  ],
                                                              grid[i  ][j  ][k+1] - grid[i  ][j  ][k-1]
                                                              );
                    }
                }
            }
            glBegin(GL_TRIANGLES);
            lasti = 1;
            lastx = floor(data.left/stepx) * stepx;
            for (int i = 2; i < ncells + 1; i++)       // starts at 2 because we use the right vertex (so we can use "last" values), and there is a padding cell to the left used in normal calculations.
            {                                          // first sampled value is at 1; left edge, plus one padding (because lasti is initialized to 1)
                lastj = 1;
                x = lastx + stepx;
                lasty = floor(data.bottom/stepy) * stepy;
                for(int j = 2; j < ncells + 1; j++)
                {
                    lastk = 1;
                    y = lasty + stepy;
                    lastz = floor(data.back/stepz) * stepz;
                    for (int k = 2; k < ncells + 1; k++)
                    {
                        z = lastz + stepz;
                        mcase = 0;
                        if (grid[lasti][lastj][lastk] >= 0)
                            mcase |= 1;
                        if (grid[i][lastj][lastk] >= 0)
                            mcase |= 2;
                        if (grid[i][j][lastk] >= 0)
                            mcase |= 4;
                        if (grid[lasti][j][lastk] >= 0)
                            mcase |= 8;
                        if (grid[lasti][lastj][k] >= 0)
                            mcase |= 16;
                        if (grid[i][lastj][k] >= 0)
                            mcase |= 32;
                        if (grid[i][j][k] >= 0)
                            mcase |= 64;
                        if (grid[lasti][j][k] >= 0)
                            mcase |= 128;

                        edges = edge_table[mcase];
                        double t;

                        if (edges & 1)
                        {
                            t = grid[i][lastj][lastk] / (grid[i][lastj][lastk] - grid[lasti][lastj][lastk]);
                            vertlist[0][0] = x - t * stepx;
                            vertlist[0][1] = lasty;
                            vertlist[0][2] = lastz;
                            normallist[0] = normals[lasti][lastj][lastk] * t + normals[i][lastj][lastk] * (1 - t);
                        }
                        if (edges & 2)
                        {
                            t = grid[i][j][lastk] / (grid[i][j][lastk] - grid[i][lastj][lastk]);
                            vertlist[1][0] = x;
                            vertlist[1][1] = y - t * stepy;
                            vertlist[1][2] = lastz;
                            normallist[1] = normals[i][lastj][lastk] * t + normals[i][j][lastk] * (1 - t);
                       }
                        if (edges & 4)
                        {
                            t = grid[i][j][lastk] / (grid[i][j][lastk] - grid[lasti][j][lastk]);
                            vertlist[2][0] = x - t * stepx;
                            vertlist[2][1] = y;
                            vertlist[2][2] = lastz;
                            normallist[2] = normals[lasti][j][lastk] * t + normals[i][j][lastk] * (1 - t);
                        }
                        if (edges & 8)
                        {
                            t = grid[lasti][j][lastk] / (grid[lasti][j][lastk] - grid[lasti][lastj][lastk]);
                            vertlist[3][0] = lastx;
                            vertlist[3][1] = y - t * stepy;
                            vertlist[3][2] = lastz;
                            normallist[3] = normals[lasti][lastj][lastk] * t + normals[lasti][j][lastk] * (1 - t);
                        }
                        if (edges & 16)
                        {
                            t = grid[i][lastj][k] / (grid[i][lastj][k] - grid[lasti][lastj][k]);
                            vertlist[4][0] = x - t * stepx;
                            vertlist[4][1] = lasty;
                            vertlist[4][2] = z;
                            normallist[4] = normals[lasti][lastj][k] * t + normals[i][lastj][k] * (1 - t);
                        }
                        if (edges & 32)
                        {
                            t = grid[i][j][k] / (grid[i][j][k] - grid[i][lastj][k]);
                            vertlist[5][0] = x;
                            vertlist[5][1] = y - t * stepy;
                            vertlist[5][2] = z;
                            normallist[5] = normals[i][lastj][k] * t + normals[i][j][k] * (1 - t);
                        }
                        if (edges & 64)
                        {
                            t = grid[i][j][k] / (grid[i][j][k] - grid[lasti][j][k]);
                            vertlist[6][0] = x - t * stepx;
                            vertlist[6][1] = y;
                            vertlist[6][2] = z;
                            normallist[6] = normals[lasti][j][k] * t + normals[i][j][k] * (1 - t);
                        }
                        if (edges & 128)
                        {
                            t = grid[lasti][j][k] / (grid[lasti][j][k] - grid[lasti][lastj][k]);
                            vertlist[7][0] = lastx;
                            vertlist[7][1] = y - t * stepy;
                            vertlist[7][2] = z;
                            normallist[7] = normals[lasti][lastj][k] * t + normals[lasti][j][k] * (1 - t);
                        }
                        if (edges & 256)
                        {
                            t = grid[lasti][lastj][k] / (grid[lasti][lastj][k] - grid[lasti][lastj][lastk]);
                            vertlist[8][0] = lastx;
                            vertlist[8][1] = lasty;
                            vertlist[8][2] = z - t * stepz;
                            normallist[8] = normals[lasti][lastj][lastk] * t + normals[lasti][lastj][k] * (1 - t);
                        }
                        if (edges & 512)
                        {
                            t = grid[i][lastj][k] / (grid[i][lastj][k] - grid[i][lastj][lastk]);
                            vertlist[9][0] = x;
                            vertlist[9][1] = lasty;
                            vertlist[9][2] = z - t * stepz;
                            normallist[9] = normals[i][lastj][lastk] * t + normals[i][lastj][k] * (1 - t);
                        }
                        if (edges & 1024)
                        {
                            t = grid[i][j][k] / (grid[i][j][k] - grid[i][j][lastk]);
                            vertlist[10][0] = x;
                            vertlist[10][1] = y;
                            vertlist[10][2] = z - t * stepz;
                            normallist[10] = normals[i][j][lastk] * t + normals[i][j][k] * (1 - t);
                        }
                        if (edges & 2048)
                        {
                            t = grid[lasti][j][k] / (grid[lasti][j][k] - grid[lasti][j][lastk]);
                            vertlist[11][0] = lastx;
                            vertlist[11][1] = y;
                            vertlist[11][2] = z - grid[lasti][j][k] / (grid[lasti][j][k] - grid[lasti][j][lastk]) * stepz;
                            normallist[11] = normals[lasti][j][lastk] * t + normals[lasti][j][k] * (1 - t);
                        }

                        short* trilist = tri_table[mcase];
                        for (int i = 0; trilist[i] != -1; i += 3)
                        {
                            glNormal3f(normallist[trilist[i  ]].x,normallist[trilist[i  ]].y,normallist[trilist[i  ]].z);
                            glVertex3f(  vertlist[trilist[i  ]][0], vertlist[trilist[i  ]][1], vertlist[trilist[i  ]][2]);
                            glNormal3f(normallist[trilist[i+1]].x,normallist[trilist[i+1]].y,normallist[trilist[i+1]].z);
                            glVertex3f(  vertlist[trilist[i+1]][0], vertlist[trilist[i+1]][1], vertlist[trilist[i+1]][2]);
                            glNormal3f(normallist[trilist[i+2]].x,normallist[trilist[i+2]].y,normallist[trilist[i+2]].z);
                            glVertex3f(  vertlist[trilist[i+2]][0], vertlist[trilist[i+2]][1], vertlist[trilist[i+2]][2]);
                        }
                        lastz = z;
                        lastk = k;
                    }
                    lasty = y;
                    lastj = j;
                }
                lastx = x;
                lasti = i;
            }
        }
        glEnd();
    }
    getnextcolor();

}

void interpreter::evaluate(parametricplot* parp)
{
    setcolor(data.currentcolor);
    double from, to, step;
    if (parp->givenfrom)
    {
        if (evaluate(parp->from).type != val_number)
            throw(error("Error: cannot have non-numeric interval bound"));
        from = evaluate(parp->from).val.n;
    }
    else
        from = -1;

    if (parp->givento)
    {
        if (evaluate(parp->to).type != val_number)
            throw(error("Error: cannot have non-numeric interval bound"));
        to = evaluate(parp->to).val.n;
    }
    else
        to = 1;

    if (parp->givenstep)
    {
        if (evaluate(parp->step).type != val_number)
            throw(error("Error: cannot have non-numeric step size"));
        step = max(evaluate(parp->step).val.n, (to - from) / data.detail / 100);
    }
    else
        step = (to - from) / data.detail;

    int nassignments = parp->assignments.size();
    double t = from;
    vars[parp->parname] = t;
    if (!data.is3d)
    {
        for(int i = 0; i < nassignments; i++)
        {
            vars[parp->assignments[i]->lvalue.str] = evaluate(parp->assignments[i]->rvalue);
            if (vars[parp->assignments[i]->lvalue.str].type != val_number)
                throw(error("Error: attempt to plot non-numeric expression"));
        }
        double lastx = vars["x"].val.n;
        double lasty = vars["y"].val.n;
        double x, y;
        t = t + step;
        for(; t < to + step; t += step)
        {
            vars[parp->parname] = t;
            for(int i = 0; i < nassignments; i++)
                vars[parp->assignments[i]->lvalue.str] = evaluate(parp->assignments[i]->rvalue);
            x = vars["x"].val.n;
            y = vars["y"].val.n;
            line2(lastx, lasty, x, y);
            lastx = x;
            lasty = y;
        }
    }
    else if (!parp->ismulti)
    {
        glDisable(GL_LIGHTING);
        glBegin(GL_LINES);
        for(int i = 0; i < nassignments; i++)
        {
            vars[parp->assignments[i]->lvalue.str] = evaluate(parp->assignments[i]->rvalue);
            if (vars[parp->assignments[i]->lvalue.str].type != val_number)
                throw(error("Error: attempt to plot non-numeric expression"));
        }
        double lastx = vars["x"].val.n;
        double lasty = vars["y"].val.n;
        double lastz = vars["z"].val.n;
        double x, y, z;
        t = t + step;
        for(; t < to + step; t += step)
        {
            vars[parp->parname] = t;
            for(int i = 0; i < nassignments; i++)
                vars[parp->assignments[i]->lvalue.str] = evaluate(parp->assignments[i]->rvalue);
            x = vars["x"].val.n;
            y = vars["y"].val.n;
            z = vars["z"].val.n;
            glVertex3f(lastx, lasty, lastz);
            glVertex3f(x, y, z);
            lastx = x;
            lasty = y;
            lastz = z;
        }
        glEnd();
        glEnable(GL_LIGHTING);
    }
    else
    {
        double ufrom, uto, ustep;
        if (parp->extraparams->givenfrom)
        {
            if (evaluate(parp->extraparams->from).type != val_number)
                throw(error("Error: cannot have non-numeric interval bound"));
            ufrom = evaluate(parp->extraparams->from).val.n;
        }
        else
            ufrom = -1;

        if (parp->extraparams->givento)
        {
            if (evaluate(parp->extraparams->to).type != val_number)
                throw(error("Error: cannot have non-numeric interval bound"));
            uto = evaluate(parp->extraparams->to).val.n;
        }
        else
            uto = 1;

        if (parp->extraparams->givenstep)
        {
            if (evaluate(parp->extraparams->step).type != val_number)
                throw(error("Error: cannot have non-numeric step size"));
            ustep = max(evaluate(parp->extraparams->step).val.n, (uto - ufrom) / data.detail / 100);
        }
        else
            ustep = (uto - ufrom) / data.detail * 2;

        double u = ufrom;

        int ncellst = ceil((to - from) / step);
        int ncellsu = ceil((uto - ufrom) / ustep);

        std::cout << "u, v:\n" << ncellst << "\n" << ncellsu << "\n";

        vert3f** grid = new vert3f*[ncellst + 3];
        for (int i = 0; i < ncellst + 3; i++)
            grid[i] = new vert3f[ncellsu + 3];           //    "fencing" vertex (+1) and 1 step padding on each side for normal calculation (+2).

        vars["t"].type = val_number;
        vars["u"].type = val_number;

        for(int i = 0; i < nassignments; i++)
            if (evaluate(parp->assignments[i]->rvalue).type != val_number)
                throw(error("Error: attempt to plot non-numeric espression"));

        for (int i = 0; i < ncellst + 3; i++)
        {
            vars["t"].val.n = t;
            u = ufrom;
            for(int j = 0; j < ncellsu + 3; j++)
            {
                vars["u"].val.n = u;
                for(int k = 0; k < nassignments; k++)
                    vars[parp->assignments[k]->lvalue.str] = evaluate(parp->assignments[k]->rvalue);
                grid[i][j] = vert3f(vars["x"].val.n, vars["y"].val.n, vars["z"].val.n);
                u += ustep;
            }
            t += step;
        }
        vert3f** normals = new vert3f*[ncellst + 1];
        for(int i = 0; i < ncellst + 1; i++)
            normals[i] = new vert3f[ncellsu + 1];

        vert3f tana, tanb;
        for(int i = 0; i <= ncellst; i++)
            for(int j = 0; j <= ncellsu; j++)
            {
                tana = grid[i+2][j+1] - grid[i][j+1]; //tangent vector in line with theta polar rotation
                tanb = grid[i+1][j+2] - grid[i+1][j]; //tangent vector in line with phi polar rotation
                normals[i][j] = vert3f(
                                       tana.y * tanb.z - tana.z * tanb.y,
                                       tana.z * tanb.x - tana.x * tanb.z,
                                       tana.x * tanb.y - tana.y * tanb.x
                                      );                                            //cross product of the two tangent vectors
            }

        int lasti = 0;
        int lastj;

        vert3f currentvert;
        for (int i = 1; i <= ncellst; i++)
        {
            lastj = 0;
            for(int j = 1; j <= ncellsu; j++)
            {

                glBegin(GL_POLYGON);
                glNormal3f(normals[lasti][lastj].x, normals[lasti][lastj].y, normals[lasti][lastj].z);
                currentvert = grid[lasti + 1][lastj + 1];                          // +1 because of padding on left and bottom of grid (for normal calcs)
                glVertex3f(currentvert.x, currentvert.y, currentvert.z);
                glNormal3f(normals[i][lastj].x, normals[i][lastj].y, normals[i][lastj].z);
                currentvert = grid[i + 1][lastj + 1];
                glVertex3f(currentvert.x, currentvert.y, currentvert.z);
                glNormal3f(normals[i][j].x, normals[i][j].y, normals[i][j].z);
                currentvert = grid[i + 1][j + 1];
                glVertex3f(currentvert.x, currentvert.y, currentvert.z);
                glNormal3f(normals[lasti][j].x, normals[lasti][j].y, normals[lasti][j].z);
                currentvert = grid[lasti + 1][j + 1];
                glVertex3f(currentvert.x, currentvert.y, currentvert.z);
                glEnd();
                lastj = j;
            }
            lasti = i;
        }
        for (int i = 0; i < ncellst + 3; i++)
            delete grid[i];
        delete grid;
        for (int i = 0; i < ncellst + 1; i++)
            delete normals[i];
        delete normals;
    }
    getnextcolor();
}

int interpreter::addstring(std::string str)
{
    std::vector<std::string>::iterator iter = std::find(strings.begin(), strings.end(), str);
    if (iter == strings.end())
    {
        //std::cout << "Pushing string \"" << str << "\"\n";
        strings.push_back(str);
        return strings.size() - 1;

    }
    else
    {
        //std::cout << "Assigning existing string at index " << iter  - strings.begin() << "\n";
        return iter - strings.begin();
    }
}

procedure::procedure(){}

procedure::procedure(std::vector <std::string> args_, block* entrypoint_)
{
    args = args_;
    entrypoint = entrypoint_;
}


procedure::~procedure()
{
    delete entrypoint;
}

arglist_member::arglist_member()
{
    next = NULL;
}

arglist_member::~arglist_member()
{
    if (next != NULL)
        delete next;
}


void g_data::setdetail(int detail_)
{
    int ncells = detail / 6;
    for (int i = 0; i < ncells + 3; i++)
    {
        for(int j = 0; j < ncells + 3; j++)
            delete grid3[i][j];
        delete grid3[i];
    }
    delete grid3;

    for (int i = 0; i < ncells + 1; i++)
    {
        for(int j = 0; j < ncells + 1; j++)
            delete normals[i][j];
        delete normals[i];
    }
    delete normals;

    detail = detail_;
    ncells = detail / 6;

    grid3 = new double**[ncells + 3];
    for (int i = 0; i < ncells + 3; i++)
    {
        grid3[i] = new double*[ncells + 3];
        for(int j = 0; j < ncells + 3; j++)
            grid3[i][j] = new double[ncells + 3];
    }

    normals = new vert3f**[ncells + 1];
    for (int i = 0; i <= ncells; i++)
    {
        normals[i] = new vert3f*[ncells + 1];
        for(int j = 0; j <= ncells; j++)
            normals[i][j] = new vert3f[ncells + 1];
    }
}

void g_data::setdetail(int detail_, bool overloader) //don't delete the data - used at first run, otherwise we try to delete data that doesn't yet exist.
{
    detail = detail_;
    int ncells = detail / 6;

    grid3 = new double**[ncells + 3];
    for (int i = 0; i < ncells + 3; i++)
    {
        grid3[i] = new double*[ncells + 3];
        for(int j = 0; j < ncells + 3; j++)
            grid3[i][j] = new double[ncells + 3];
    }

    normals = new vert3f**[ncells + 1];
    for (int i = 0; i <= ncells; i++)
    {
        normals[i] = new vert3f*[ncells + 1];
        for(int j = 0; j <= ncells; j++)
            normals[i][j] = new vert3f[ncells + 1];
    }
}

