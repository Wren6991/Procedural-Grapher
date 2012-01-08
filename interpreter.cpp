#include "interpreter.h"
#include <wx/glcanvas.h>



interpreter::interpreter(std::map<std::string, dfuncd> funcs_, g_data data_)
{
    funcs = funcs_;
    data = data_;
    vars["mousex"] = data.mousex;
    vars["mousey"] = data.mousey;
    vars["time"] = data.time / 1000.0;
}

interpreter::~interpreter()
{
    //delete program;
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
    switch(stat->type)
    {
        case t_while:
            while (evaluate(stat->stat.whilestat->cond) > 0)
                evaluate(stat->stat.whilestat->whileblock);
            break;
        case t_for:
            int a, b;
            a = evaluate(stat->stat.forstat->a);
            b = evaluate(stat->stat.forstat->b);
            for (int i = a; i <= b; i++)
            {
                vars[stat->stat.forstat->id] = i;
                evaluate(stat->stat.forstat->forblock);
            }
            break;
        case t_if:
            if (evaluate(stat->stat.ifstat->cond) > 0)  //if first condition is true
            {
                evaluate(stat->stat.ifstat->ifblock);
            }
            else
            {
                bool blocked = false;
                for (unsigned int i = 0; i < stat->stat.ifstat->elseifs.size(); i++)     //for each elseif statement
                {
                    if (evaluate(stat->stat.ifstat->elseifs[i]->cond) > 0)  //if elseif condition is true
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
                int n_ass = stat->stat.assignstat->extra_ids.size();
                double singleval = evaluate(stat->stat.assignstat->rvalue);
                double *vals = new double[n_ass];
                for (int i = 0; i < n_ass; i++)
                {
                    vals[i] = evaluate(stat->stat.assignstat->extra_rvalues[i]);
                }
                vars[stat->stat.assignstat->id] = singleval;
                for (int i = 0; i < n_ass; i++)
                {
                    vars[stat->stat.assignstat->extra_ids[i]] = vals[i];
                }
                delete vals;
            }
            else
                vars[stat->stat.assignstat->id] = evaluate(stat->stat.assignstat->rvalue);
            break;
        case t_func:
            funcs[stat->stat.funcstat->name](evaluate(stat->stat.funcstat->arg));
            break;
        case s_plot_exp:
            evaluate(stat->stat.expplot);
            break;
        case s_plot_imp:
            evaluate(stat->stat.impplot);
            break;
        case t_def:
            std::cout << "Defining procedure " << stat->stat.defstat->name << "\n";
            procedures[stat->stat.defstat->name] =  new procedure(stat->stat.defstat->args, stat->stat.defstat->entrypoint);
            break;
        case n_procedure:
            if (procedures.find(stat->stat.procstat->name) == procedures.end()) //not defined
                throw(n_procedure);
            proc =  procedures[stat->stat.procstat->name];

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
            }
            else
            {
                if (stat->stat.procstat->args.size() > proc->args.size())
                    throw(error(std::string("Error: too many arguments to procedure ") + stat->stat.procstat->name));
                else
                    throw(error(std::string("Error: too few arguments to procedure ") + stat->stat.procstat->name));
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

double interpreter::evaluate(expression *expr)
{
    double total = evaluate(expr->comparisons[0]);
    for(unsigned int i = 0; i < expr->operators.size(); i++)
    {
        if (expr->operators[i] == t_and)
            total = min(total, evaluate(expr->comparisons[i + 1]));
        else
            total = max(total, evaluate(expr->comparisons[i + 1]));
    }
    return total;
}

double interpreter::evaluate(comparison *comp)
{
    double total = evaluate(comp->a);
    switch (comp->oper)
    {
        case t_eof:
            break;
        case t_lessthan:
            total = evaluate(comp->b) - total;
            receivedinequal = true;
            break;
        case t_greaterthan:
            total = total - evaluate(comp->b);
            receivedinequal = true;
            break;
        case t_lteq:
            total = evaluate(comp->b) - total;
            if (total == 0)
                total = 0.00001;
            receivedinequal = true;
            receivedequals = true;
            break;
        case t_gteq:
            total = total - evaluate(comp->b);
            if (total == 0)
                total = 0.00001;
            receivedinequal = true;
            receivedequals = true;
            break;
        case t_equals:
            total = evaluate(comp->b) - total - 100000;
            if(total == -100000)
                total = 0.00001;
            receivedequals = true;
            break;
        default:
            break;
    }
    return total;
}

double interpreter::evaluate(sum* s)
{
    double total = evaluate(s->terms[0]);
    for(unsigned int i = 0; i < s->operators.size(); i++)
    {
        if (s->operators[i] == t_plus)
            total += evaluate(s->terms[i + 1]);
        else
            total -= evaluate(s->terms[i + 1]);
    }
    return total;
}

double interpreter::evaluate(term* t)
{
    double total = evaluate(t->values[0]);
    for(unsigned int i = 0; i < t->operators.size(); i++)
    {
        if (t->operators[i] == t_times)
            total *= evaluate(t->values[i + 1]);
        else
            total /= evaluate(t->values[i + 1]);
    }
    return total;
}

double interpreter::evaluate(value *v)
{
    double n;
    switch (v->type)
    {
        case t_number:
            n = v->n;
            break;
        case t_id:
            n = vars[v->var];
            break;
        case t_func:
            n = funcs[v->funccall->name](evaluate(v->funccall->arg));
            break;
        case n_expression:
            n = evaluate(v->expr);
            break;
        case n_procedure:
            if (procedures.find(v->proccall->name) == procedures.end()) //not defined
                throw(n_procedure);
            proc =  procedures[v->proccall->name];

            if(v->proccall->args.size() == proc->args.size())
            {
                for (unsigned int i = 0; i < proc->args.size(); i++)
                {
                    vars[proc->args[i]] = evaluate(v->proccall->args[i]);
                }
                try
                {
                    evaluate(proc->entrypoint);
                }
                catch (token_type_enum t)
                {
                    if (t != t_return)
                        throw(t);
                    n = returnvalue;
                }
            }
            else
            {
                if (v->proccall->args.size() > proc->args.size())
                    throw(error(std::string("Error: too many arguments to procedure ") + v->proccall->name));
                else
                    throw(error(std::string("Error: too few arguments to procedure ") + v->proccall->name));
            }
            break;
        case t_dif:
            n = evaluate(v->b);
            temp = vars[v->var];
            vars[v->var] = temp + 0.00001;
            n = (evaluate(v->b) - n) / 0.00001;
            vars[v->var] = temp;
            break;
        default:
            n = 0;
            break;
    }
    if (v->expd)
        n = pow(n, evaluate(v->b));
    if (v->negative)
        n = -n;
    return n;
}

void interpreter::evaluate(explicitplot* relation)
{
    setcolor(data.currentcolor);
    if (!data.is3d)
    {
        if (relation->rangevar == "y")
        {
            double x, y, lastx, lasty, step;
            x = data.left;
            step = (data.right - data.left)/data.detail;
            vars["x"] = x;
            y = evaluate(relation->expr);
            while(x < data.right + step)
            {
                lastx = x;
                lasty = y;
                x += step;
                vars["x"] = x;
                y = evaluate(relation->expr);
                line2(lastx, lasty, x, y);
            }
        }
        else if (relation->rangevar == "x")
        {
            double x, y, lastx, lasty, step;
            y = data.bottom;
            step = (data.top - data.bottom)/data.detail;
            vars["y"] = y;
            x = evaluate(relation->expr);
            while(y < data.top + step)
            {
                lastx = x;
                lasty = y;
                y += step;
                vars["y"] = y;
                x = evaluate(relation->expr);
                line2(lastx, lasty, x, y);
            }
        }
        else if (relation->rangevar == "r")
        {
            double r, theta, lastr, lasttheta, step;
            theta = -10;
            step = 10.0/data.detail;
            vars["theta"] = theta;
            r = evaluate(relation->expr);
            while(theta < 10 + step)
            {
                lastr = r;
                lasttheta = theta;
                theta += step;
                vars["theta"] = theta;
                r = evaluate(relation->expr);
                line2(lastr * cos(lasttheta), lastr * sin(lasttheta), r * cos(theta), r * sin(theta));
            }
        }
        else
        {
            throw (e_ordinate);
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
            x = data.left - stepx;
            for (int i = 0; i < ncells + 3; i++)
            {
                vars["x"] = x;
                z = data.back - stepz;
                for(int j = 0; j < ncells + 3; j++)
                {
                    vars["z"] = z;
                    grid[i][j] = evaluate(relation->expr);
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
            y = data.bottom - stepy;
            for (int i = 0; i < ncells + 3; i++)
            {
                vars["y"] = y;
                z = data.back - stepz;
                for(int j = 0; j < ncells + 3; j++)
                {
                    vars["z"] = z;
                    grid[i][j] = evaluate(relation->expr);
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
                                           -(stepy * stepz * 4),
                                           (grid[i+2][j+1] - grid[i][j+1]) * stepz * 2,
                                           (grid[i+1][j+2] - grid[i+1][j]) * stepy * 2
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
            x = data.left - stepx;
            for (int i = 0; i < ncells + 3; i++)
            {
                vars["x"] = x;
                y = data.bottom - stepy;
                for(int j = 0; j < ncells + 3; j++)
                {
                    vars["y"] = y;
                    grid[i][j] = evaluate(relation->expr);
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
                                           (grid[i+2][j+1] - grid[i][j+1]) * stepy * 2,
                                           (grid[i+1][j+2] - grid[i+1][j]) * stepx * 2,
                                            -(stepx * stepy * 4)
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
        else
        {
            throw (e_ordinate);
        }

    }
    getnextcolor();
}

void interpreter::evaluate(implicitplot* relation)
{
    bool equalsonly = relation->haseq && !relation->hasineq;
    int ncells = data.detail / 2 + 1;
    double** grid = new double*[ncells + 1];
    for (int i = 0; i <= ncells; i++)
        grid[i] = new double[ncells + 1];
    double stepx = (data.right - data.left) / (ncells - 1);
    double stepy = (data.top - data.bottom) / (ncells - 1);
    double x, y;
    x = floor(data.left/stepx) * stepx;
    for (int i = 0; i <= ncells; i++)
    {
        vars["x"] = x;
        y = floor(data.bottom/stepy) * stepy;
        for(int j = 0; j <= ncells; j++)
        {
            vars["y"] = y;
            grid[i][j] = evaluate(relation->expr);
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
                        std::cout << lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]) << "\n";
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
                        std::cout << lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]) << "\n";
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
                        std::cout << lasty + stepy * grid[lasti][lastj] / (grid[lasti][lastj] - grid[lasti][j]) << "\n";
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
    getnextcolor();
    for (int i = 0; i <= ncells; i++)
        delete grid[i];
    delete grid;
}

void interpreter::evaluate(parametricplot* parp)
{
    setcolor(data.currentcolor);
    double from, to, step;
    if (parp->givenfrom)
        from = evaluate(parp->from);
    else
        from = -1;

    if (parp->givento)
        to = evaluate(parp->to);
    else
        to = 1;

    if (parp->givenstep)
        step = max(evaluate(parp->step), (to - from) / (data.detail * 500));
    else
        step = (to - from) / data.detail;

    int nassignments = parp->assignments.size();
    double t = from;
    vars[parp->parname] = t;
    for(int i = 0; i < nassignments; i++)
        vars[parp->assignments[i]->id] = evaluate(parp->assignments[i]->rvalue);
    double lastx = vars["x"];
    double lasty = vars["y"];
    double x, y;
    t = t + step;
    for(; t < to + step; t += step)
    {
        vars[parp->parname] = t;
        for(unsigned int i = 0; i < nassignments; i++)
            vars[parp->assignments[i]->id] = evaluate(parp->assignments[i]->rvalue);
        x = vars["x"];
        y = vars["y"];
        line2(lastx, lasty, x, y);
        lastx = x;
        lasty = y;
    }
    getnextcolor();
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

error::error(std::string errstring_)
{
    errstring = errstring_;
}
