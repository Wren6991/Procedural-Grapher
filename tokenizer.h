#ifndef TOKENIZER_H_INCLUDED
#define TOKENIZER_H_INCLUDED

#include <iostream>
#include <iostream>
#include <vector>
#include <map>

typedef enum {
    t_eof = 0,  //end of file
    t_number,
    t_id,
    t_equals,
    t_plus,
    t_minus,
    t_times,
    t_divide,
    t_lparen,
    t_rparen,
    t_exp,
    t_lessthan,
    t_greaterthan,
    t_lteq,
    t_gteq,
    t_comment,
    t_string,
    t_question,
    t_colon,
    t_comma,
    t_let,
    t_if,
    t_then,
    t_elseif,
    t_else,
    t_end,
    t_while,
    t_do,
    t_for,
    t_to,
    t_and,
    t_or,
    t_not,
    t_def,
    t_func,
    t_return,
    t_plot,
    t_dif,
    t_par,
    t_from,
    t_step,
    n_tokens,   //ought to end here really
    e_value,
    e_nostatement,
    e_ordinate,
    n_expression,
    n_term,
    n_value,
    n_procedure,
    s_plot_exp,
    s_plot_imp
    } token_type_enum;

class tagged_value;

typedef tagged_value (*dfuncd)(tagged_value);

class token
{
    public:
    token_type_enum type;
    std::string value;
    token();
    token(token_type_enum, std::string);
};

class streambuffer
{
    public:
    std::string str;
    int index;
    int size;
    streambuffer (std::string);
    char next();
};

std::vector <token> tokenize(std::string);
std::vector <token> tokenize(std::string, std::map <std::string, dfuncd>);

#endif // TOKENIZER_H_INCLUDED
