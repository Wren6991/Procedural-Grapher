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
//  shunting yard?
///////////<Won't>/////////////

extern std::string token_type_names[];

parser::parser(std::vector <token> tokens_)
{
    parser::tokens = tokens_;
    parser::tindex = 0;
    if (parser::tokens.size() > 0)
        parser::t = tokens[0];
    parser::gettoken();
}

parser::parser(std::vector <token> tokens_, std::map <std::string, dfuncd> funcs_)
{
    parser::tokens = tokens_;
    parser::tindex = 0;
    if (parser::tokens.size() > 0)
        parser::t = tokens[0];
    parser::gettoken();
    parser::funcs = funcs_;
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

expression* parser::expr()
{
    expression* e = new expression;
    e->a = comp();
    return e;
}

comparison* parser::comp()
{
    comparison* c = new comparison;
    c->a = sm();
    if (accept(t_equals)||accept(t_lessthan)||accept(t_greaterthan)||accept(t_lteq)||accept(t_gteq))
    {
        c->oper = last.type;
        c->b = sm();
    }
    else
        c->oper = t_eof;
    return c;
}

sum* parser::sm()
{
    sum* s = new sum;
    s->terms.push_back(trm());
    while (accept(t_plus)||accept(t_minus))
    {
        s->operators.push_back(last.type);
        s->terms.push_back(trm());
    }
    return s;
}

term* parser::trm()
{
    term* t = new term;
    t->values.push_back(val());
    while(accept(t_times)||accept(t_divide))
    {
        t->operators.push_back(last.type);
        t->values.push_back(val());
    }
    return t;
}

value* parser::val()
{
    value* v  = new value;
    v->negative = accept(t_minus);
    if (accept(t_id))
    {
        v->type = t_id;
        v->var = last.value;
    }
    else if (accept(t_number))
    {
        v->type = t_number;
        v->n = atoi(last.value.c_str());
    }
    else if (accept(t_func))
    {
        v->type = t_func;
        functioncall *f = new functioncall;
        f->name = last.value;
        expect(t_lparen);
        f->arg = expr();
        expect(t_rparen);
        v->funccall = f;
    }
    else if (accept(t_lparen))
    {
        v->type = n_expression;
        v->expr = expr();
        expect(t_rparen);
    }
    else
        throw(n_value);
    if(accept(t_exp))
    {
        v->expd = true;
        v->exponent = val();
    }
    else
        v->expd = false;
    return v;
}

statement* parser::stat()
{
    statement *s = new statement;
    if(accept(t_let))
    {
        s->type = t_let;
        assignment *a = new assignment;
        expect(t_id);
        a->id = last.value;
        expect(t_equals);
        a->rvalue = expr();
        s->stat.assignstat = a;
    }
    else if (accept(t_while))
    {
        s->type = t_while;
        whilestatement *w = new whilestatement;
        w->cond = expr();
        expect(t_do);
        w->whileblock = blk();
        s->stat.whilestat = w;
        expect(t_end);
    }
    else if (accept(t_if))
    {
        s->type = t_if;
        ifstatement *i = new ifstatement;
        i->cond = expr();
        expect(t_then);
        i->ifblock = blk();
        while(accept(t_elseif))
        {
            ifstatement *elseif = new ifstatement;
            elseif->cond = expr();
            expect(t_then);
            elseif->ifblock = blk();
            i->elseifs.push_back(elseif);
        }
        if (accept(t_else))
        {
            i->haselse = true;
            i->elseblock = blk();
        }
        else
            i->haselse = false;
        expect(t_end);
        s->stat.ifstat = i;
    }
    else if (accept(t_for))
    {
        s->type = t_for;
        forstatement *f = new forstatement;
        expect(t_id);
        f->id = last.value;
        expect(t_equals);
        f->a = expr();
        expect(t_to);
        f->b = expr();
        expect(t_do);
        f->forblock = blk();
        expect(t_end);
        s->stat.forstat = f;
    }
    else if (accept(t_func))
    {
        s->type = t_func;
        functioncall *f = new functioncall;
        f->name = last.value;
        expect(t_lparen);
        f->arg = expr();
        expect(t_rparen);
        s->stat.funcstat = f;
    }
    else if (accept(t_id))
    {
        std::string id = last.value;
        if (accept(t_equals))
        {
            s->type = s_plot_exp;
            explicitplot *p = new explicitplot;
            p->rangevar = id;
            p->expr = expr();
            s->stat.expplot = p;
        }
        else if (accept(t_lparen))
        {
            s->type = n_procedure;
            procedurecall *p = new procedurecall;
            p->name = id;
            while (!accept(t_rparen))
            {
                p->args.push_back(expr());
                if(!accept(t_comma))
                {
                    expect(t_rparen);
                    break;
                }
            }
            s->stat.procstat = p;
        }
        else throw(t_equals);
    }
    else
    {
        s->type = t_eof;
        delete s;
        throw(e_nostatement);
    }
    return s;
}

block* parser::blk()
{
    block *b = new block;
    try {
        while (true)
            b->statements.push_back(stat());
    }
    catch (token_type_enum t)
    {
        if (t != e_nostatement)
        {
            throw (t);   //this exception should not be handled here - pass it on.
        }
    }
    return b;
}
/*
procedure::procedure(int entrypoint_, std::vector <std::string> args_)
{
    entrypoint = entrypoint_;
    args = args_;
}
//*/

block::~block()
{
    for (int i = 0; i < statements.size(); i++)
        delete statements[i];
}

statement::~statement()
{
    switch (type)
    {
        case t_if:
            delete stat.ifstat;
            break;
        case t_while:
            delete stat.whilestat;
            break;
        case t_for:
            delete stat.forstat;
            break;
        case t_def:
            delete stat.defstat;
            break;
        case n_procedure:
            //delete stat.procstat;
            break;
        case t_func:
            delete stat.funcstat;
            break;
        case t_let:
            delete stat.assignstat;
            break;
        case s_plot_exp:
            delete stat.expplot;
            break;
        case s_plot_imp:
            delete stat.impplot;
            break;
        default:
            break;
    }
}

implicitplot::~implicitplot()
{
    delete expr;
}

explicitplot::~explicitplot()
{
    delete expr;
}

assignment::~assignment()
{
    delete rvalue;
}

functioncall::~functioncall()
{

    delete arg;
}

procedurecall::~procedurecall()
{
    for (int i = 0; i < args.size(); i++)
        delete args[i];
}

defstatement::~defstatement()
{
    //for (int i = 0; i < args.size(); i++)
        //delete args[i];
    delete entrypoint;
}

forstatement::~forstatement()
{
    delete a;
    delete b;
    delete forblock;
}

whilestatement::~whilestatement()
{
    delete cond;
    delete whileblock;
}

ifstatement::~ifstatement()
{
    delete cond;
    delete ifblock;
    for (int i = 0; i < elseifs.size(); i++)
        delete elseifs[i];
    delete elseblock;
}

expression::~expression()
{
    delete a;
}

comparison::~comparison()
{
    delete a;
    if (oper != t_eof)
        delete b;
}

sum::~sum()
{
    for (int i = 0; i < terms.size(); i++)
        delete terms[i];
}

term::~term()
{
    for (int i = 0; i < values.size(); i++)
        delete values[i];
}

value::~value()
{
    switch(type)
    {
        case t_number:
            break;
        case t_id:
            break;
        case t_func:
            delete funccall;
            break;
        case n_procedure:
            delete proccall;
            break;
        default:
            break;
    }
    if (expd)
        delete exponent;
}

procedure::~procedure()
{
    delete blk;
}

