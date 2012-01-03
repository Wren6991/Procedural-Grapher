#include "interpreter.h"


interpreter::interpreter(block* program_, std::map<std::string, dfuncd> funcs_, g_data data_)
{
    program = program_;
    funcs = funcs_;
    data = data_;
}

interpreter::~interpreter()
{
    delete program;
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
        default:
            throw(stat->type);
    }
}

double interpreter::evaluate(expression *exp)
{
    double total = evaluate(exp->a->a);
    switch (exp->a->oper)
    {
        case t_eof:
            break;
        case t_lessthan:
            total = evaluate(exp->a->b) - total;
            receivedinequal = true;
            break;
        case t_greaterthan:
            total = total - evaluate(exp->a->b);
            receivedinequal = true;
            break;
        case t_lteq:
            total = evaluate(exp->a->b) - total;
            if (total == 0)
                total = 0.00001;
            receivedinequal = true;
            receivedequals = true;
            break;
        case t_gteq:
            total = total - evaluate(exp->a->b);
            if (total == 0)
                total = 0.00001;
            receivedinequal = true;
            receivedequals = true;
            break;
        case t_equals:
            total = evaluate(exp->a->b) - total - 100000;
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
        default:
            n = 0;
            break;
    }
    if (v->expd)
        n = pow(n, evaluate(v->exponent));
    if (v->negative)
        n = -n;
    return n;
}

void interpreter::evaluate(explicitplot* relation)
{
    setcolor(data.currentcolor);
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
    getnextcolor();
}

void interpreter::evaluate(implicitplot* relation)
{
    bool equalsonly = relation->haseq && !relation->hasineq;
    int ncells = data.detail / 2 + 1;
    double** grid = new double*[ncells + 1];
    for (int i = 0; i <= ncells; i++)
        grid[i] = new double[ncells + 1];
    double stepx = (data.right - data.left) / ncells;
    double stepy = (data.top - data.bottom) / ncells;
    double x, y;
    x = data.left;
    for (int i = 0; i <= ncells; i++)
    {
        vars["x"] = x;
        y = data.bottom;
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
    double lastx = data.left;
    x = lastx + stepx;
    int lasti = 0;
    double lasty;
    int lastj, mscase;

    if (!(relation->haseq||relation->hasineq))
    {
        for (int i = 1; i <= ncells; i++)
        {
            lastj = 0;
            lasty = data.bottom;
            y = lasty + stepy;
            for(int j = 1; j <= ncells; j++)
            {
                {
                    setcolor(data.currentcolor, grid[i][j]);
                    quad2(lastx, lasty, lastx, y, x, y, x, lasty);
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
            lasty = data.bottom;
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
            lasty = data.bottom;
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
}
