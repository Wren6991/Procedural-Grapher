#include <iostream>
#include <vector>
#include <map>

#include "tokenizer.h"

extern std::string token_type_names[];


token::token()
{
    token::type = t_eof;
    token::value = std::string();
}

token::token(token_type_enum type_, std::string value_)
{
    token::type = type_;
    token::value = value_;
}



streambuffer::streambuffer (std::string str_)
{
    streambuffer::str = str_;
    streambuffer::size = str.length();
    streambuffer::index = 0;
}
char streambuffer::next()
{
    if (streambuffer::index < streambuffer::size)
    {
        std::cout << streambuffer::str[streambuffer::index];
        return streambuffer::str[streambuffer::index++];
    }
    else
    {
        streambuffer::index++;
        return 0;
    }
}

std::vector <token> tokenize(std::string str)
{
    std::map<std::string, token_type_enum> keywords;
    keywords["let"] = t_let;
    keywords["if"] = t_if;
    keywords["elseif"] = t_elseif;
    keywords["else"] = t_else;
    keywords["then"] = t_then;
    keywords["end"] = t_end;
    keywords["while"] = t_while;
    keywords["do"] = t_do;
    keywords["for"] = t_for;
    keywords["to"] = t_to;
    keywords["def"] = t_def;
    keywords["return"] = t_return;
    keywords["plot"] = t_plot;
    keywords["and"] = t_and;
    keywords["or"] = t_or;
    keywords["par"] = t_par;
    keywords["from"] = t_from;
    keywords["step"] = t_step;

    std::vector <token> tokens;
    streambuffer sbuf = streambuffer(str);
    std::string v;
    char c = sbuf.next();
    int startindex;
    start:
        startindex = sbuf.index - 1;
        if (c >= '0' && c <= '9')
            goto number;
        else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
            goto id;
        else if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
            goto whitespace;
        else if (c == '=')
            goto equals;
        else if (c == '+')
            goto plus;
        else if (c == '-')
            goto minus;
        else if (c == '*')
            goto times;
        else if (c == '/')
            goto divide;
        else if (c == '(')
            goto lparen;
        else if (c == ')')
            goto rparen;
        else if (c == '{')
            goto lbrace;
        else if (c == '}')
            goto rbrace;
        else if (c == '[')
            goto lsquareb;
        else if (c == ']')
            goto rsquareb;
        else if (c == '^')
            goto exp;
        else if (c == ',')
            goto comma;
        else if (c == '<')
            goto lessthan;
        else if (c == '>')
            goto greaterthan;
        else if (c == '#')
            goto comment;
        else if (c == '"')
            goto string;
        else
            goto lexed;
    number:
        c = sbuf.next();
        if (c >= '0' && c <= '9')
            goto number;
        else if (c == '.')
            goto number_f;
        tokens.push_back(token(t_number, sbuf.str.substr(startindex, sbuf.index - startindex - 1)));
        goto start;
    number_f:
        c = sbuf.next();
        if (c >= '0' && c <= '9')
            goto number_f;
        tokens.push_back(token(t_number, sbuf.str.substr(startindex, sbuf.index - startindex - 1)));
        goto start;
    id:
        c = sbuf.next();
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_') //letter, digit, underscore
            goto id;
        v = sbuf.str.substr(startindex, sbuf.index - startindex - 1);
        if (v == "d")
        {
             if (c == '/')
            {
                c = sbuf.next();
                if (c == 'd')
                {
                    c = sbuf.next();
                    tokens.push_back(token(t_dif, "d/d"));
                    goto start;
                }
                else
                {
                    tokens.push_back(token(t_id, "d"));
                    tokens.push_back(token(t_divide, "/"));
                    goto start;
                }
            }
            else
            {
                tokens.push_back(token(t_id, "d"));
                goto start;
            }
        }
        else if (keywords.find(v) == keywords.end())
            tokens.push_back(token(t_id, v));
        else
            tokens.push_back(token(keywords[v], v));
        goto start;
    whitespace:
        c = sbuf.next();
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
            goto whitespace;
        goto start;
    equals:
        c = sbuf.next();
        tokens.push_back(token(t_equals, "="));
        goto start;
    plus:
        c = sbuf.next();
        tokens.push_back(token(t_plus, "+"));
        goto start;
    minus:
        c = sbuf.next();
        tokens.push_back(token(t_minus, "-"));
        goto start;
    times:
        c = sbuf.next();
        tokens.push_back(token(t_times, "*"));
        goto start;
    divide:
        c = sbuf.next();
        tokens.push_back(token(t_divide, "/"));
        goto start;
    lparen:
        c = sbuf.next();
        tokens.push_back(token(t_lparen, "("));
        goto start;
    rparen:
        c = sbuf.next();
        tokens.push_back(token(t_rparen, ")"));
        goto start;
    exp:
        c = sbuf.next();
        tokens.push_back(token(t_exp, "^"));
        goto start;
    comma:
        c = sbuf.next();
        tokens.push_back(token(t_comma, ","));
        goto start;
    lbrace:
        c = sbuf.next();
        tokens.push_back(token(t_lbrace, "{"));
        goto start;
    rbrace:
        c = sbuf.next();
        tokens.push_back(token(t_rbrace, "}"));
        goto start;
    lsquareb:
        c = sbuf.next();
        tokens.push_back(token(t_lsquareb, "["));
        goto start;
    rsquareb:
        c = sbuf.next();
        tokens.push_back(token(t_rsquareb, "]"));
        goto start;
    lessthan:
        c = sbuf.next();
        if (c == '=')
        {
            tokens.push_back(token(t_lteq, "<="));
            c = sbuf.next();
            goto start;
        }
        tokens.push_back(token(t_lessthan, "<"));
        goto start;
    greaterthan:
        c = sbuf.next();
        if (c == '=')
        {
            tokens.push_back(token(t_gteq, ">="));
            c = sbuf.next();
            goto start;
        }
        tokens.push_back(token(t_greaterthan, ">"));
        goto start;
    comment:
        c = sbuf.next();
        if (c == '\n' || c == '\r' || c == 0)
            goto start;
        goto comment;
    string:
        c = sbuf.next();
        if (c != '"' && c != 0)
            goto string;
        tokens.push_back(token(t_string, sbuf.str.substr(startindex + 1, sbuf.index - startindex - 2)));
        c = sbuf.next();
        goto start;
    lexed:

    std::cout << "\n";
    for (unsigned int i = 0; i < tokens.size(); i++)
    {
        std::cout << token_type_names[tokens[i].type] << ": " << tokens[i].value << "\n";
    }
    return tokens;

}

std::vector <token> tokenize(std::string str, std::map <std::string, dfuncd> funcs)
{
    std::vector<token> tokens = tokenize(str);
    for (unsigned int i = 0; i < tokens.size(); i++)
    {
        if (tokens[i].type == t_id)
            if (funcs.find(tokens[i].value) != funcs.end())
                tokens[i].type = t_func;
    }
    return tokens;

}




