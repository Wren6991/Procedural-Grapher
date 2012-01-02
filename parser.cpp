#include "tokenizer.h"
#include "parser.h"
#include <vector>
#include <map>
#include <iostream>
#include <stdlib.h>
#include <cmath>
#include "draw.h"


////////////<Must>/////////////
// and, or, not
///////////<Should>////////////
//  arrays (+ type system?)
//  implicit plots
//  time
//  file dialogs
//  settings
//  local variables - either stack of maps, or store original values in a vector at call time and retore after return.
//  multiple assignments
///////////<Could>/////////////
//  parametrics
//  strings/ other values.
//  3d
//  memoize everything! have an array of values from 1...ntokens plus bools for fixed and eval'd, and check list before eval'ing an expression / w/e! (fixed could propagate up one level each evaluation - eprs that are evaluated lots are perfectly optimised, others less so.)
///////////<Won't>/////////////

extern std::string token_type_names[];
extern double left;

parser::parser(std::vector <token> tokens_)
{
    parser::tokens = tokens_;
    parser::tindex = 0;
    if (parser::tokens.size() > 0)
        parser::t = tokens[0];
    parser::gettoken();
    data.left = -1;
    data.right = 1;
    data.top = 1;
    data.bottom = -1;
}

parser::parser(std::vector <token> tokens_, std::map <std::string, dfuncd> funcs_)
{
    parser::tokens = tokens_;
    parser::tindex = 0;
    if (parser::tokens.size() > 0)
        parser::t = tokens[0];
    parser::gettoken();
    parser::funcs = funcs_;
    data.left = -1;
    data.right = 1;
    data.top = 1;
    data.bottom = -1;
}

parser::parser(std::vector <token> tokens_, std::map <std::string, dfuncd> funcs_, g_data data_)
{
    parser::tokens = tokens_;
    parser::tindex = 0;
    if (parser::tokens.size() > 0)
        parser::t = tokens[0];
    parser::gettoken();
    parser::funcs = funcs_;
    data = data_;
}


void parser::gettoken()
{
    last = t;
    if (tindex < tokens.size())
        t = tokens[tindex];
    else
        parser::t = token(t_eof, "<eof>");
    tindex++;
}

bool parser::accept(token_type_enum type)
{
    if (parser::t.type == type)
    {
        //std::cout << "Accepted token type " << token_type_names[t.type] << " \"" << t.value << "\"\n";
        gettoken();
        return true;
    }
    else
        return false;
}

void parser::expect(token_type_enum type)
{
    if (!parser::accept(type))
        throw (type);
}

double parser::expression()
{

    return comp();
}

double parser::comp()
{
    double total = sum();
    if (accept(t_equals))
    {
       total = sum() - total - 100000;
        if (total == -100000)
            total = 0.00001;        //dirty!
    }
    else if (accept(t_lessthan))
    {
        total = sum() - total;
    }
    else if (accept(t_greaterthan))
    {
        total = total - sum();
    }
    else if (accept(t_lteq))
    {
        total = sum() - total;
        if (total == 0)
            total = 0.00001;
    }
    else if (accept(t_gteq))
    {
        total = total - sum();
        if (total == 0)
            total = 0.00001;
    }
    return total;
}

double parser::sum()
{
    double total = parser::term();
    while (parser::accept(t_plus)||parser::accept(t_minus))
    {
        if (parser::last.type == t_plus)
            total += parser::term();
        else
            total -= parser::term();
    }
    return total;
}

double parser::term()
{
    double total = parser::value();
    while (parser::accept(t_times)||parser::accept(t_divide))
    {
        if (parser::last.type == t_times)
            total *= parser::value();
        else
            total /= parser::value();
    }
    return total;
}

double parser::value()
{
    bool negative = parser::accept(t_minus);
    double v;
    if (parser::accept(t_number))
    {
        v = atof(parser::last.value.c_str());
    }
    else if (parser::accept(t_id))
    {
        std::string id = last.value;
        if (accept(t_lparen))
        {
            if (procedures.find(id) != procedures.end())
            {
                procedure& proc = procedures[id];
                int nargs = proc.args.size();
                for(int i = 0; i < nargs - 1; i++)
                {
                    vars[proc.args[i]] = expression();
                    expect(t_comma);
                }
                if (nargs > 0)
                    vars[proc.args[nargs - 1]] = expression();
                expect(t_rparen);
                stack.push_back(tindex - 1);
                jumpto(proc.entrypoint);
                try{
                    block();
                    expect(t_end);
                }
                catch (double returnval)
                {
                    v = returnval;
                }
                jumpto(stack[stack.size() - 1]);
                stack.pop_back();
            }
            else
                throw (n_procedure);
        }
        else
            v = vars[id];
    }
    else if (parser::accept(t_lparen))
    {
        v = parser::expression();
        parser::expect(t_rparen);
    }
    else if (accept(t_func))
    {
        std::string funcname = last.value;
        expect(t_lparen);
        v = funcs[funcname](expression());
        expect(t_rparen);
    }
    else
    {
        throw(n_value);
    }
    if (parser::accept(t_exp))
        v = pow(v, parser::value());
    if (negative)
        v = -v;
    return v;
}

double parser::statement()
{
    if (parser::accept(t_let))
    {
        parser::expect(t_id);
        std::string varname = parser::last.value;
        parser::expect(t_equals);
        parser::vars[varname] = parser::expression();
    }
    else if (parser::accept(t_if))
    {
        double condval = parser::expression();
        parser::expect(t_then);
        if (condval > 0)
        {
            parser::block();
            parser::skiptoend();
        }
        else
        {
            parser::skipblock();
            while (last.type == t_elseif)
            {
                condval = expression();
                expect(t_then);
                if (condval > 0)
                {
                    parser::block();
                    parser::skiptoend();
                    break;
                }
                else
                    parser::skipblock();
            }
            if (last.type == t_else)
            {
                parser::block();
                parser::expect(t_end);
            }
        }
    }
    else if (parser::accept(t_while))
    {
        int entryindex = tindex - 1;
        while (expression() > 0)
        {
            expect(t_do);
            block();
            expect(t_end);
            jumpto(entryindex);
        }
        expect(t_do);
        skiptoend();
    }
    else if (accept(t_for))
    {
        expect(t_id);
        std::string varname = last.value;
        expect(t_equals);
        int a, b;
        a = expression();
        expect(t_to);
        b = expression();
        int entryindex = tindex;
        expect(t_do);
        for(int i = a; i <= b; i++)
        {
            vars[varname] = i;
            block();
            expect(t_end);
            jumpto(entryindex);
        }
        skiptoend();
    }
    else if (accept(t_id))
    {
        std::string id = last.value;

        if (accept(t_lparen))
        {
            if (procedures.find(id) != procedures.end())
            {
                procedure& proc = procedures[id];
                int nargs = proc.args.size();
                for(int i = 0; i < nargs - 1; i++)
                {
                    vars[proc.args[i]] = expression();
                    expect(t_comma);
                }
                if (nargs > 0)
                    vars[proc.args[nargs - 1]] = expression();
                expect(t_rparen);
                stack.push_back(tindex - 1);
                jumpto(proc.entrypoint);
                try{
                    block();
                    expect(t_end);
                }
                catch (double){}
                jumpto(stack[stack.size() - 1]);
                stack.pop_back();
            }
            else
                throw (n_procedure);
        }
        else if (accept(t_equals))
        {
            explicitplot(id);
        }
        else
        {
            implicitplot(tindex - 2); //location of the id (tindex acually points to the token about to be assigned to t, not the one that has been.)
        }

    }
    else if (accept(t_func))
    {
        std::string funcname = last.value;
        expect(t_lparen);
        funcs[funcname](expression());
        expect(t_rparen);
    }
    else if (accept(t_return))
    {
        throw(expression());
    }
    else if (accept(t_plot))
    {
        implicitplot(tindex - 1);
    }
    else if (accept(t_def))
    {
        expect(t_id);
        std::string procname = last.value;
        expect(t_lparen);
        std::vector <std::string> args;
        if (accept(t_id))
        {
            args.push_back(last.value);
             while (accept(t_comma))
            {
                expect(t_id);
                args.push_back(last.value);
            }
        }
        expect(t_rparen);
        procedures[procname] = procedure(tindex - 1, args);
        skiptoend();
    }
    else
        throw (e_nostatement);
    return 0;
}

double parser::block()
{
    try
    {
        while (true)
            parser::statement();
    }
    catch (token_type_enum t)
    {
        if (t != e_nostatement)
            throw(t);   //This isn't an exception that should be handled here - pass it on.
        //else
            //std::cout << "Reached end of block.\n";
    }
    return 0;
}

void parser::skiptoend()
{
    if (parser::jumps.find(parser::tindex) != parser::jumps.end())
    {
        parser::jumpto(parser::jumps[parser::tindex]);
    }
    else
    {
        int entryindex = parser::tindex;
        int level = 1;
        while (level > 0)
        {
            if (parser::accept(t_if)||parser::accept(t_for)||parser::accept(t_while))
                level++;
            else if (parser::accept(t_end))
                level--;
            else
            {
                parser::gettoken();
                if (parser::t.type == t_eof)
                    throw (t_end);
            }
        }
        parser::jumps[entryindex] = parser::tindex - 1;
    }
}

void parser::skipblock()
{
    if (jumps.find(tindex) != jumps.end())
    {
        jumpto(jumps[tindex]);
    }
    else
    {
        int entryindex = parser::tindex;
        int level = 1;
        while (level > 0)
        {
            if (accept(t_if)||accept(t_for)||parser::accept(t_while))
                level++;
            else if (level == 1 && (parser::accept(t_else)||parser::accept(t_elseif)))
                level--;
            else if (parser::accept(t_end))
                level--;
            else
            {
                parser::gettoken();
                if (parser::t.type == t_eof)
                    throw (t_end);
            }
        }
        parser::jumps[entryindex] = parser::tindex - 1;
    }
}

void parser::explicitplot(std::string rangevar)
{
        int expressionstart = tindex - 1;
        if (rangevar == std::string("y"))
        {
            double x, y, lastx, lasty, step;
            x = data.left;
            step = (data.right - data.left)/data.detail;
            vars["x"] = x;
            y = expression();
            jumpto(expressionstart);
            while(x < data.right + step)
            {
                lastx = x;
                lasty = y;
                x += step;
                vars["x"] = x;
                y = expression();
                line2(lastx, lasty, x, y);
                jumpto(expressionstart);
            }
            expression();
            //triangle2(1, 1, 0, 0, -1, 1);
        }
        else if (rangevar == std::string("x"))
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
        else if (rangevar == std::string("r"))
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
        }
        else
        {
            throw (e_ordinate);
        }

}

void parser::implicitplot(int exprindex)
{
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
            jumpto(exprindex);
            grid[i][j] = expression();
            /*color4(0, 0, 1, grid[i][j]);
            float polydata[8];
            polydata[0] = x - stepx;
            polydata[1] = y - stepy;
            polydata[2] = x - stepx;
            polydata[3] = y;
            polydata[4] = x;
            polydata[5] = y;
            polydata[6] = x;
            polydata[7] = y - stepy;
            poly2(polydata, 4);*/
            y += stepy;
        }
        x += stepx;
    }
    //expression();

    double lastx = data.left;
    x = lastx + stepx;
    int lasti = 0;
    double lasty;
    int lastj, mscase;

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
    expression();


}

void parser::jumpto(int target)
{
    //std::cout << "Jumped to token " << target << "\n";
    tindex = target;
    gettoken();
}

procedure::procedure()
{

}
procedure::procedure(int entrypoint_, std::vector <std::string> args_)
{
    entrypoint = entrypoint_;
    args = args_;
}
