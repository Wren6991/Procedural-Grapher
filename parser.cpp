#include "tokenizer.h"
#include "parser.h"
#include <vector>
#include <map>
#include <iostream>
#include <stdlib.h>
#include <cmath>
#include "draw.h"


////////////<Must>/////////////
//
///////////<Should>////////////
//  fix normal calculations - calculate only where they're needed.
//  procedures as values
//  anonymous functions
//  make interpretation iterative - self-maintained stack.
//  3d polar plots
//  1 base for arrays intead of 0 base?
//  settings
//  local variables - either stack of maps, or store original values in a vector at call time and restore after return.
///////////<Could>/////////////
//  make predefined functions first-class values
//  allow predefined functions to take multiple arguments
//  mouse tracing and intersections
//  fixed expressions (evaluate at parse time, then never again)
///////////<Won't>/////////////
//  I should probably put something in here
//  solve p v ¬p
//  how about that
//  shunting yard?
///////////////////////////////

parser::parser(){}


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
    int i = 0;
    for (std::map <std::string, dfuncd>::iterator iter = funcs_.begin(); iter != funcs_.end(); iter++)
    {
        funcnames[iter->first] = i++;
    }

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
    e->comparisons.push_back(comp());
    while(accept(t_and)||accept(t_or))
    {
        e->operators.push_back(last.type);
        e->comparisons.push_back(comp());
    }
    return e;
}

comparison* parser::comp()
{
    comparison* c = new comparison;
    c->a = sm();
    if (accept(t_equals))
    {
        accepted_equals = true;
        c->oper = last.type;
        c->b = sm();
    }
    else if (accept(t_lessthan)||accept(t_greaterthan))
    {
        accepted_ineq = true;
        c->oper = last.type;
        c->b = sm();
    }
    else if (accept(t_lteq)||accept(t_gteq))
    {
        accepted_equals = true;
        accepted_ineq = true;
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
        v->n = atof(last.value.c_str());
    }
    else if (accept(t_func))
    {
        v->type = t_func;
        functioncall *f = new functioncall;
        f->id = funcnames[last.value];
        expect(t_lparen);
        while (!accept(t_rparen))
        {
            f->args.push_back(expr());
            if (!accept(t_comma))
            {
                expect(t_rparen);
                break;
            }
        }
        v->funccall = f;
    }
    else if (accept(t_lparen))
    {
        v->type = n_expression;
        v->expr = expr();
        expect(t_rparen);
    }
    else if (accept(t_string))
    {
        v->type = t_string;
        v->var = last.value;
    }
    else if (accept(t_dif))
    {
        v->type = t_dif;
        expect(t_id);
        v->var = last.value;
        v->b = val();
    }
    else if (accept(t_lbrace))
    {
        arrayinitializer* ai = new arrayinitializer;
        v->type = t_lbrace;
        v->arrinit = ai;
        if (!accept(t_rbrace))
            while (true)
            {
                ai->explist.push_back(expr());
                if (!accept(t_comma))
                {
                    expect(t_rbrace);
                    break;
                }
            }
    }
    else
        throw(n_value);
    while (t.type == t_lsquareb || t.type == t_dot || t.type == t_colon || t.type == t_lparen)
    {
        if (accept(t_lsquareb))
        {
            value* a = new value;
            a->type = t_lsquareb;
            a->negative = false;
            a->arritem = new arrayitem;
            a->arritem->array = v;
            a->arritem->index = expr();

            v->expd = false;    //because we're about to hide v, so these won't be set below.
            v = a;      //stuff v into a, make a the new v - previous array-index pair becomes new array. at the end,  we just return v; this is the top-level array-index pair. At runtime, top-level array is evaluated recursively.
            expect(t_rsquareb);
        }
        else if (accept(t_dot) || accept(t_colon))
        {
            value* a = new value;
            a->type = t_lsquareb;
            a->negative = false;
            a->arritem = new arrayitem;
            a->arritem->array = v;
            if (last.type == t_colon)
                a->arritem->pass_self = true;
            expect(t_id);
            a->arritem->index = new expression();
            a->arritem->index->comparisons.push_back(new comparison());
            a->arritem->index->comparisons[0]->a = new sum();
            a->arritem->index->comparisons[0]->a->terms.push_back(new term());
            a->arritem->index->comparisons[0]->a->terms[0]->values.push_back(new value);
            a->arritem->index->comparisons[0]->a->terms[0]->values[0]->type = t_string;
            a->arritem->index->comparisons[0]->a->terms[0]->values[0]->var = last.value;    //build a whole tree - thank god this only happens at parse-time.

            v->expd = false;    //because we're about to hide v, so these won't be set below.
            v = a;      //stuff v into a, make a the new v - previous array-index pair becomes new array. at the end,  we just return v; this is the top-level array-index pair. At runtime, top-level array is evaluated recursively.
        }
        else if (accept(t_lparen))
        {
            procedurecall *p = new procedurecall;
            p->name = v;
            while (!accept(t_rparen))
            {
                p->args.push_back(expr());
                if(!accept(t_comma))
                {
                    expect(t_rparen);
                    break;
                }
            }
            v = new value();
            v->type = n_procedure;
            v->proccall = p;
            v->negative = p->name->negative;
            p->name->negative = false;
            p->name->expd = false;
        }
    }

    if(accept(t_exp))
    {
        v->expd = true;
        v->b = val();
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

        expect(t_id);
        std::string id = last.value;
        if (accept(t_lparen))   // let f(x) = x^2
        {
            s->type = t_def;
            defstatement *d = new defstatement;
            d->name = id;
            while (!accept(t_rparen))
            {
                expect(t_id);
                d->args.push_back(last.value);
                if(!accept(t_comma))
                {
                    expect(t_rparen);
                    break;
                }
            }
            expect(t_equals);
            block* b = new block;   //build a new block, with a return statement that returns the expression:
            statement* rs = new statement;
            returnstatement* r = new returnstatement;
            r->expr = expr();
            rs->type = t_return;
            rs->stat.returnstat = r;
            b->statements.push_back(rs);
            d->entrypoint = b;
            s->stat.defstat = d;    // (above is equivalent to "def f(x) return x^2 end", but feels a bit more mathsy.)
        }
        else
        {
            s->type = t_let;
            assignment *a = new assignment;
            a->lvalue.isarray = false;
            a->lvalue.str = id;

            value* temp = NULL;

            while (t.type == t_lsquareb || t.type == t_dot || t.type == t_colon || t.type == t_lparen)
            {
                if (accept(t_lsquareb))
                {
                    value* arr = new value;             //create a new array-key pair.
                    arr->negative = false;
                    arr->expd = false;
                    arr->type = t_lsquareb;
                    arr->arritem = new arrayitem;
                    if (a->lvalue.isarray)
                    {
                        arr->arritem->array = a->lvalue.ai->array;
                    }
                    else    //last one was a function, or this is the first loop.
                    {
                        a->lvalue.isarray = true;
                        a->lvalue.ai = new arrayitem;
                        if (temp == NULL)   //this must be the first loop, else the function call would have made this non-null.
                        {
                            arr->arritem->array = new value;
                            arr->arritem->array->type = t_id;
                            arr->arritem->array->var = id;
                        }
                        else    // last loop was a functioncall.
                        {
                            arr->arritem->array = temp;
                            temp = NULL;    //signal to loop end that the last loop was not a function call.
                        }
                    }
                    arr->arritem->index = expr();
                    a->lvalue.ai->array = arr;      //stuff old lvalue array inside arr, make arr the new lvalue array - previous array-key pair becomes new array. at the end,  we just return v; this is the top-level array-index pair. At runtime, top-level array is evaluated recursively.
                    expect(t_rsquareb);             //array is actually a value that contains an arrayitem that contains an array - this is so that id variables can be treated in the same way as arr-key pairs.
                }
                else if (accept(t_dot) || accept(t_colon))
                {
                    value* arr = new value;
                    arr->negative = false;          // arrrr, I'm a pirate
                    arr->expd = false;
                    arr->type = t_lsquareb;
                    arr->arritem = new arrayitem;
                    if (a->lvalue.isarray)
                    {
                        arr->arritem->array = a->lvalue.ai->array;
                    }
                    else    //last one was a function, or this is the first loop.
                    {
                        a->lvalue.isarray = true;
                        a->lvalue.ai = new arrayitem;
                        if (temp == NULL)   //this must be the first loop, else the function call would have made this non-null.
                        {
                            arr->arritem->array = new value;
                            arr->arritem->array->type = t_id;
                            arr->arritem->array->var = id;
                        }
                        else    // last loop was a functioncall.
                        {
                            arr->arritem->array = temp;
                            temp = NULL;    //signal to loop end that the last loop was not a function call.
                        }
                    }
                    if (last.type == t_colon)
                        arr->arritem->pass_self = true;
                    expect(t_id);
                    arr->arritem->index = new expression();
                    arr->arritem->index->comparisons.push_back(new comparison());
                    arr->arritem->index->comparisons[0]->a = new sum();
                    arr->arritem->index->comparisons[0]->a->terms.push_back(new term());
                    arr->arritem->index->comparisons[0]->a->terms[0]->values.push_back(new value);
                    arr->arritem->index->comparisons[0]->a->terms[0]->values[0]->type = t_string;
                    arr->arritem->index->comparisons[0]->a->terms[0]->values[0]->var = last.value;    //build a whole tree - thank god this only happens at parse-time.

                    a->lvalue.ai->array = arr;      //stuff old lvalue array inside arr, make arr the new lvalue array - previous array-key pair becomes new array. at the end,  we just return v; this is the top-level array-index pair. At runtime, top-level array is evaluated recursively.
                }
                else if (accept(t_lparen))
                {
                    procedurecall *p = new procedurecall;
                    if (a->lvalue.isarray)
                    {
                        p->name = a->lvalue.ai->array;
                        a->lvalue.isarray = false;
                    }
                    else if (temp != NULL)
                    {
                        p->name = temp;
                        temp = NULL;
                    }
                    else    // this must be the first loop - array indexes make isarray true, and functioncalls make temp non-null.
                    {
                        p->name = new value;
                        p->name->type = t_id;
                        p->name->negative = false;
                        p->name->expd = false;
                        p->name->var = id;
                    }
                    while (!accept(t_rparen))
                    {
                        p->args.push_back(expr());
                        if(!accept(t_comma))
                        {
                            expect(t_rparen);
                            break;
                        }
                    }
                    temp = new value();
                    temp->type = n_procedure;
                    temp->proccall = p;
                    temp->negative = false;
                    temp->expd = false;
                }
            }
            if (temp != NULL)
                throw(error("Error: cannot assign to expression"));

            a->ismultiple = false;
            while (accept(t_comma))
            {
                a->ismultiple = true;
                assg_lvalue lv;
                lv.isarray = false;
                expect(t_id);
                lv.str = last.value;
                if (t.type == t_lsquareb)
                {
                    lv.isarray = true;
                    lv.ai = new arrayitem;
                    lv.ai->array = new value;
                    lv.ai->array->negative = false;
                    lv.ai->array->expd = false;
                    lv.ai->array->type = t_id;
                    lv.ai->array->var = lv.str;
                    while (accept(t_lsquareb))
                    {
                        value* arr = new value;
                        arr->negative = false;
                        arr->expd = false;
                        arr->type = t_lsquareb;
                        arr->arritem = new arrayitem;
                        arr->arritem->array = lv.ai->array;
                        arr->arritem->index = expr();
                        lv.ai->array = arr;      //stuff old array inside arr, make arr the new array - previous array-index pair becomes new array. at the end,  we just return a; this is the top-level array-index pair. At runtime, top-level array is evaluated recursively.
                        expect(t_rsquareb);
                    }

                }
                a->extra_lvalues.push_back(lv);
            }
            expect(t_equals);
            a->rvalue = expr();
            for (unsigned int i = 0; i < a->extra_lvalues.size(); i++)
            {
                expect(t_comma);
                a->extra_rvalues.push_back(expr());
            }

            s->stat.assignstat = a;
        }
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
            elseif->haselse = false;
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
        f->id = funcnames[last.value];
        expect(t_lparen);
        while (!accept(t_rparen))
        {
            f->args.push_back(expr());
            if (!accept(t_comma))
            {
                expect(t_rparen);
                break;
            }
        }
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
            p->name = new value();
            p->name->type = t_id;
            p->name->var = id;
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
        else
        {
            s->type = s_plot_imp;
            implicitplot *p = new implicitplot;
            accepted_equals = false;
            accepted_ineq = false;
            jumpto(tindex - 1);     //jump back to the id we accepted
            p->expr = expr();       //parse an expression, stuff it into the plot statement
            p->haseq = accepted_equals;
            p->hasineq = accepted_ineq;
            s->stat.impplot = p;
        }
    }
    else if (accept(t_plot))
    {
        s->type = s_plot_imp;
        implicitplot *p = new implicitplot;
        accepted_equals = false;
        accepted_ineq = false;
        p->expr = expr();       //parse an expression, stuff it into the plot statement
        p->haseq = accepted_equals;
        p->hasineq = accepted_ineq;
        s->stat.impplot = p;
    }
    else if (accept(t_def))
    {
        s->type = t_def;
        expect(t_id);
        defstatement *d = new defstatement;

        d->name = last.value;
        expect(t_lparen);
        while (!accept(t_rparen))
        {
            expect(t_id);
            d->args.push_back(last.value);
            if(!accept(t_comma))
            {
                expect(t_rparen);
                break;
            }
        }
        d->entrypoint = blk();
        expect(t_end);
        s->stat.defstat = d;
    }
    else if (accept(t_return))
    {
        s->type = t_return;
        returnstatement *r = new returnstatement;
        r->expr = expr();
        s->stat.returnstat = r;
    }
    else if (accept(t_par))
    {
        expect(t_id);
        s->type = t_par;
        parametricplot *p = new parametricplot;
        p->ismulti = false;
        p->givenfrom = false;
        p->givento = false;
        p->givenstep = false;
        p->parname = last.value;
        if (accept(t_from))
        {
            p->givenfrom = true;
            p->from = expr();
        }

        if (accept(t_to))
        {
            p->to = expr();
            p->givento = true;
        }

        if (accept(t_step))
        {
            p->step = expr();
            p->givenstep = true;
        }

        if (accept(t_comma))
        {
            expect(t_id);
            parametricplot *q = new parametricplot;
            q->ismulti = false;
            q->givenfrom = false;
            q->givento = false;
            q->givenstep = false;
            q->parname = last.value;
            if (accept(t_from))
            {
                q->givenfrom = true;
                q->from = expr();
            }

            if (accept(t_to))
            {
                q->to = expr();
                q->givento = true;
            }

            if (accept(t_step))
            {
                q->step = expr();
                q->givenstep = true;
            }
            p->extraparams = q;
            p->ismulti = true;
        }

        while (accept(t_id))
        {
            assignment *a = new assignment;
            a->lvalue.str = last.value;
            a->lvalue.isarray = false;
            expect(t_equals);
            a->rvalue = expr();
            p->assignments.push_back(a);
        }
        s->stat.parplot = p;
        expect(t_end);
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

void parser::jumpto(int target)
{
    tindex = target - 1;    //tindex points to token _about_to_be_ read.
    gettoken();
}

block::~block()
{
    for (unsigned int i = 0; i < statements.size(); i++)
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
            delete stat.procstat;
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
        case t_return:
            delete stat.returnstat;
            break;
        case t_par:
            delete stat.parplot;
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
    if (lvalue.isarray)
        delete lvalue.ai;
    if (ismultiple)
        for (unsigned int i = 0; i < extra_rvalues.size(); i++)
            delete extra_rvalues[i];
}

functioncall::~functioncall()
{
    for  (unsigned int i = 0; i < args.size(); i++)
        delete args[i];
}

procedurecall::~procedurecall()
{
    for (unsigned int i = 0; i < args.size(); i++)
        delete args[i];
    delete name;
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
    for (unsigned int i = 0; i < elseifs.size(); i++)
        delete elseifs[i];
    if (haselse)
        delete elseblock;
}

returnstatement::~returnstatement()
{
    delete expr;
}

parametricplot::~parametricplot()
{
    if (givenfrom)
        delete from;
    if (givento)
        delete to;
    if (givenstep)
        delete step;

    for (unsigned int i = 0; i < assignments.size(); i++)
    {
        delete assignments[i];
    }

    if (ismulti)
        delete extraparams;
}

expression::~expression()
{
    for (unsigned int i = 0; i < comparisons.size(); i++)
        delete comparisons[i];
}

comparison::~comparison()
{
    delete a;
    if (oper != t_eof)
        delete b;
}

sum::~sum()
{
    for (unsigned int i = 0; i < terms.size(); i++)
        delete terms[i];
}

term::~term()
{
    for (unsigned int i = 0; i < values.size(); i++)
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
        case t_lbrace:
            delete arrinit;
            break;
        default:
            break;
    }
    if (expd)
        delete b;
}

tagged_value::tagged_value()
{
    type = val_number;
    val.n = 0;
}

tagged_value::tagged_value(double n)
{
    type = val_number;
    val.n = n;
}

tagged_value::tagged_value(procedure* p)
{
    type = val_procedure;
    val.proc = p;
}

assg_lvalue::assg_lvalue()
{

}

assg_lvalue::assg_lvalue(std::string str_)
{
    isarray = false;
    str = str_;
}

bool tagged_value::operator<(const tagged_value& other) const
{
    if (type < other.type)
        return true;
    else if (type == other.type)
        if (type == val_number)
            return floor(val.n) < floor(other.val.n);           // compare as integer vals: this lets us do table plots, e.g. y = t[x]. NB this function is only used when looking up from arrays.
        else
            return val.str < other.val.str;
    else
        return false;
}

error::error(std::string errstring_)
{
    errstring = errstring_;
}

value::value()
{
    negative = false;
    expd = false;
}

assg_lvalue parser::getlvalue(std::string str)
{
}

arrayitem::arrayitem()
{
    pass_self = false;
}
