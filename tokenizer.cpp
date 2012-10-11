#include <iostream>
#include <vector>
#include <map>

#include "tokenizer.h"

enum state_enum
{
    s_start = 0,
    s_number,
    s_number_f,
    s_id,
    s_lessthan,
    s_greaterthan,
    s_comment,
    s_string
};



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

    std::map<char, token> symbols;
    symbols['+'] = token(t_plus, "+");
    symbols['-'] = token(t_minus, "-");
    symbols['*'] = token(t_times, "*");
    symbols['/'] = token(t_divide, "/");
    symbols['('] = token(t_lparen, "(");
    symbols[')'] = token(t_rparen, ")");
    symbols['['] = token(t_lsquareb, "[");
    symbols[']'] = token(t_rsquareb, "]");
    symbols['{'] = token(t_lbrace, "{");
    symbols['}'] = token(t_rbrace, "}");
    symbols['^'] = token(t_exp, "^");
    symbols[','] = token(t_comma, ",");
    symbols['.'] = token(t_dot, ".");
    symbols[':'] = token(t_colon, ":");
    symbols['='] = token(t_equals, "=");

    std::vector <token> tokens;

    std::string v;
    int index = -1;
    const char *buffer = str.c_str();
    char c;
    int startindex;
    state_enum state = s_start;
    do  // while(c);  - breaks at end of loop so we can process ids etc. that rest up against the end of the string.
    {
        c = buffer[++index];
        switch(state)
        {
            case s_start:
                startindex = index;
                if (c >= '0' && c <= '9')
                    state = s_number;
                else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
                    state = s_id;
                else if (c == ' ' || c == '\t' || c == '\n' || c == '\r')   // swallow all the whitespace
                {
                    while ((c = buffer[++index]))
                        if (!(c == ' ' || c == '\t' || c == '\n' || c == '\r'))
                            break;
                    index--;        // we've now encountered a character that isn't whitespace; unget it so the next loop can pick it up.
                }
                else if (symbols.find(c) != symbols.end())  //if we find a matching symbol, push a matching token onto the list.
                    tokens.push_back(symbols.find(c)->second);
                else if (c == '<')
                    state = s_lessthan;
                else if (c == '>')
                    state = s_greaterthan;
                else if (c == '#')
                    state = s_comment;
                else if (c == '"')
                    state = s_string;
                else if (c)
                    throw(1337);
                break;
            case s_number:
                if (c == '.')
                    state = s_number_f;
                else if (c < '0' || c > '9')
                {
                    tokens.push_back(token(t_number, str.substr(startindex, index - startindex)));
                    index--;    // unget this character.
                    state = s_start;
                }
                break;
            case s_number_f:
                if (c < '0' || c > '9')
                {
                    tokens.push_back(token(t_number, str.substr(startindex, index - startindex)));
                    index--;    // unget this character.
                    state = s_start;
                }
                break;
            case s_lessthan:
                if (c == '=')
                {
                    tokens.push_back(token(t_lteq, "<="));
                }
                else
                {
                    tokens.push_back(token(t_lessthan, "<"));
                    index--;
                }
                state = s_start;
                break;
            case s_greaterthan:
                if (c == '=')
                {
                    tokens.push_back(token(t_gteq, ">="));
                }
                else
                {
                    tokens.push_back(token(t_greaterthan, ">"));
                    index--;
                }
                state = s_start;
                break;
            case s_string:
                if (c == '"')
                {
                    tokens.push_back(token(t_string, str.substr(startindex + 1, index - startindex - 2)));
                    state = s_start;
                }
                break;
            case s_comment:
                if (c == '\n' || c == '\r')
                    state = s_start;
                break;
            case s_id:
                if (index == startindex + 1 && buffer[startindex] == 'd' && buffer[index] == '/' && buffer[index + 1] == 'd')   //check if this is a differential operator. If the char following the slash is not a d, then this if falls through and the slash is handled as an operator from the start state.
                {
                    tokens.push_back(token(t_dif, "d/d"));
                    index++;
                    state = s_start;
                }
                else if ((c < 'a' || c > 'z') && (c < 'A' || c > 'Z') && (c < '0' || c > '9') && c != '_')
                {
                    token t(t_id, str.substr(startindex, index - startindex));
                    if (keywords.find(t.value) != keywords.end())
                        t.type = keywords.find(t.value)->second;
                    tokens.push_back(t);
                    state = s_start;
                    index--;
                }
                break;
        }
    } while (c);
    for (unsigned int i = 0; i < tokens.size(); i++)
        std::cout << tokens[i].type << ":\t\"" << tokens[i].value << "\"\n";
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




