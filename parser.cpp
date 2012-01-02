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
    while(accept(t_times)||accept(t_minus))
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
    else
        throw(n_value);
    if(accept(t_exp))
    {
        v->expd = true;
        v->exponent = val();
    }
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
    }
    else if (accept(t_if))
    {
        s->type = t_if;
        ifstatement *i = new ifstatement;
        i->cond = expr();
        i->ifblock = blk();
        while(accept(t_elseif))
        {
            ifstatement *elseif = new ifstatement;
            elseif->cond = expr();
            elseif->ifblock = blk();
            i->elseifs.push_back(elseif);
        }
        if (accept(t_else))
        {
            i->haselse = true;
            i->elseblock = blk();
        }
        expect(t_end);
        s->stat.ifstat = i;
    }
    else if (accept(t_for))
    {
        s->type = t_for;
        forstatement *f = new forstatement;
        f->id = last.value;
        expect(t_equals);
        f->a = expr();
        expect(t_to);
        f->b = expr();
        f->forblock = blk();
        expect(t_end);
        s->stat.forstat = f;
    }
    else
    {
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

/*void parser::explicitplot(std::string rangevar)
{
}

void parser::implicitplot(int exprindex)
{
}
void parser::jumpto(int target)
{

}

procedure::procedure()
{

}
procedure::procedure(int entrypoint_, std::vector <std::string> args_)
{
    entrypoint = entrypoint_;
    args = args_;
}
*/
