#include "interpreter.h"


interpreter::interpreter(block* program_, std::map<std::string, dfuncd> funcs_, g_data data_)
{
    program = program_;
    funcs = funcs_;
    data = data_;
}

void interpreter::evaluate(block* blk)
{
    for(int i = 0; i < blk->statements.size(); i++)
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
                for (int i = 0; i < stat->stat.ifstat->elseifs.size(); i++)     //for each elseif statement
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
            break;
        case t_greaterthan:
            total = total - evaluate(exp->a->b);
            break;
        case t_lteq:
            total = evaluate(exp->a->b) - total;
            if (total == 0)
                total = 0.00001;
            break;
        case t_gteq:
            total = total - evaluate(exp->a->b);
            if (total == 0)
                total = 0.00001;
            break;
        case t_equals:
            total = evaluate(exp->a->b) - total - 100000;
            if(total == -100000)
                total = 0.00001;
            break;
        default:
            break;
    }
    return total;
}

double interpreter::evaluate(sum* s)
{
    double total = evaluate(s->terms[0]);
    for(int i = 0; i < s->operators.size(); i++)
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
    for(int i = 0; i < t->operators.size(); i++)
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
        default:
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
    /*else if (relation->rangevar == "x")
    {
        double x, y, lastx, lasty, step;
        y = data.bottom;
        step = (data.top - data.bottom)/data.detail;
        vars["y"] = y;
        x = expression();
        jumpto(expressionstart);
        while(y < data.top + step)
        {
            lastx = x;
            lasty = y;
            y += step;
            vars["y"] = y;
            x = expression();
            line2(lastx, lasty, x, y);
            jumpto(expressionstart);
        }
        expression();
    }
    else if (relation->rangevar == "r")
    {
        double r, theta, lastr, lasttheta, step;
        theta = -10;
        step = 10.0/data.detail;
        vars["theta"] = theta;
        r = expression();
        jumpto(expressionstart);
        while(theta < 10 + step)
        {
            lastr = r;
            lasttheta = theta;
            theta += step;
            vars["theta"] = theta;
            r = expression();
            line2(lastr * cos(lasttheta), lastr * sin(lasttheta), r * cos(theta), r * sin(theta));
            jumpto(expressionstart);
        }
        expression();
    }*/
    else
    {
        throw (e_ordinate);
    }
}
