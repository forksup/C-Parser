#include <iostream>
#include <cstdlib>
#include "parser.h"
#include <algorithm>
#include <math.h>

using namespace std;

vector< vector<int> > errors(5, vector<int>(0));

//used for error 2 checking
vector<char> param;

Token token;
int inputSize = 0;

void Parser::syntax_error()
{
    cout << "SYNTAX ERROR !!!\n";
    exit(1);
}

void Parser::symantics_error(int code)
{
    errors[code].push_back(token.line_no);
}

Token Parser::expect(TokenType expected_type)
{
    token = lexer.GetToken();
    if (token.token_type != expected_type)
        syntax_error();
    return token;
}

// this function simply checks the next token without
// consuming the input
// Written by Mohsen Zohrevandi
Token Parser::peek()
{
    Token t = lexer.GetToken();
    lexer.UngetToken(t);
    return t;
}

int Parser::allocateVariable(char input) {

    for(int i =0;i<variable_table.size();i++) {
        if (variable_table[i].symbol == input) {
            return i;
        }
    }

    struct symbol_table_struct newEntry;
    newEntry.symbol = input;
    newEntry.index = 0;
    variable_table.push_back(newEntry);
    return variable_table.size()-1;
}

int Parser::findMemory(string input) {
    for(int i =0;i<symbol_table.size();i++) {
        if (symbol_table[i].symbol == input) {
            return i;
        }
    }

    symantics_error(4);
}

int Parser::allocateMemory(string input) {
    for(int i =0;i<symbol_table.size();i++) {
        if(symbol_table[i].symbol ==  input) {
            return symbol_table[i].index;
        }
    }

    struct variable newEntry;
    newEntry.symbol = input;
    newEntry.index = symbol_table.size();
    symbol_table.push_back(newEntry);
    inputSize++;
    return newEntry.index;
}

void Parser::print_errors() {
    for(int i =0;i<5;i++) {
        sort(errors[i].begin(), errors[i].end());
        if(errors[i].size() > 0) {
            if(i != 0) {
                cout << "\n";
            }
            cout << "Error Code "  << (i+1)  << ": ";
            for (int j = 0; j < errors[i].size(); j++) {
                cout << errors[i][j] << " ";
            }
        }
    }

}

void Parser::parse_input() {
    stmt* program = Parser::parse_program();
    vector<int> inputs = Parser::parse_inputs(0);
    expect(END_OF_FILE);

    bool found = false;
    for(int i = 4;i>-1;i--) {
        if(errors[i].size() > 0) {
            print_errors();
            found = true;
        }
    }

    if(found == false) {
        execute_program(program, inputs);
    }

}

stmt* Parser::parse_program() {
    Parser::parse_poly_decl_section();
    return Parser::parse_start();
}

void Parser::parse_poly_decl_section() {
   poly_declerations.push_back(*parse_poly_decl());

    string polyName = poly_declerations.back().name;

    for(int i = poly_declerations.size()-2;i > -1;i--) {
        if(polyName.compare(poly_declerations[i].name) == 0) {

            if (!std::count(errors[0].begin(), errors[0].end(), poly_declerations[i].line_no)) {
                errors[0].push_back(poly_declerations[i].line_no);
            }
            errors[0].push_back(lineNumber);
            break;
        }
    }

    token = peek();
    if (token.token_type != START) {
        parse_poly_decl_section();
    }
}

poly_decleration* Parser::parse_poly_decl() {
    expect(POLY);
    poly_decleration* newDecleration = parse_polynomial_header();
    expect(EQUAL);

    param = newDecleration->Parameters;
    newDecleration->body = parse_polynomial_body();

    //Invalid monomial name
    if(newDecleration->Parameters.size() == 0) {
        coefficient_list* bptr = newDecleration->body;
    }

    expect(SEMICOLON);
    return newDecleration;
}

coefficient_list* Parser::parse_polynomial_body() {
    return parse_term_list();
}

coefficient_list * Parser::parse_term_list() {
    coefficient_list* newList = parse_term();

    token = lexer.GetToken();
    if(token.token_type == PLUS || token.token_type == MINUS) {
        parse_add_operator();
        newList->sign = token.token_type;
        newList->next = parse_term_list();
        return newList;
    } else if(token.token_type == SEMICOLON) {
        lexer.UngetToken(token);
        return newList;
    } else {
        lexer.UngetToken(token);
    }
    return nullptr;
}

void Parser::parse_add_operator() {
    if(token.token_type != PLUS && token.token_type != MINUS) {
        syntax_error();
    }
}

coefficient_list* Parser::parse_term() {
    token = peek();
    coefficient_list* newList = new coefficient_list();
    newList->term = 1;
    //monomial
    if(token.token_type == ID) {
        newList->coLL = parse_monomial_list();
    } else if(token.token_type == NUM) {
        parse_coeffecient();
        newList->term = stoi(token.lexeme);
        token = peek();
        if(token.token_type==ID) {
            newList->coLL = parse_monomial_list();
        } else {
            newList->coLL = nullptr;
        }
    } else {
        syntax_error();
    }

    return newList;
}

coefficient * Parser::parse_monomial_list() {
    coefficient* col = new coefficient();
    col->next = nullptr;

    parse_monomial(col);

    token = peek();

    //Check to see if there is another monomial
    if(token.token_type == ID) {
        col->next = parse_monomial_list();
    } else {
        col->next = nullptr;
    }

    return col;
}

void Parser::parse_monomial(coefficient* col) {
    expect(ID);

    col->x = allocateVariable(token.lexeme[0]);

    if(param.size() > 0) {
        if (!std::count(param.begin(), param.end(), variable_table[col->x].symbol)) {
            symantics_error(1);
        }
    } else {
        if (variable_table[col->x].symbol != 'x') {
            symantics_error(1);
        }
    }

    token = lexer.GetToken();

    if(token.token_type==POWER) {
        lexer.UngetToken(token);
        col->y = parse_exponent();
        return;
    } else if(token.token_type == PLUS || token.token_type == ID || token.token_type == MINUS || token.token_type == SEMICOLON ) {
        lexer.UngetToken(token);
        col->y = 1;
    } else {
        syntax_error();
    }
}

void Parser::parse_coeffecient() {
    expect(NUM);
}


int Parser::parse_exponent() {
    expect(POWER);
    return stoi(expect(NUM).lexeme);
}


poly_decleration* Parser::parse_polynomial_header() {
    parse_polynomial_name();

    struct poly_decleration* newDecleration = new poly_decleration;
    newDecleration->line_no = token.line_no;
    newDecleration->name = token.lexeme;

    token = lexer.GetToken();

    if(token.token_type == LPAREN) {
        newDecleration->Parameters = parse_id_list();
        token = lexer.GetToken();
        if(token.token_type != RPAREN) {
            syntax_error();
        } else {
            return newDecleration;
        }
    } else {
        lexer.UngetToken(token);
    }
    return newDecleration;
}

vector<char> Parser::parse_id_list() {
    expect(ID);

    vector<char> list;

    list.push_back(token.lexeme[0]);

    token = lexer.GetToken();

    if(token.token_type==COMMA){
        vector<char> list2 = parse_id_list();
        list.insert(list.end(),list2.begin(),list2.end());
        return list;
    } else {
        lexer.UngetToken(token);
        return list;
    }
}


stmt* Parser::parse_start() {
    expect(START);
    return parse_statement_list();
}

stmt* Parser::parse_statement_list() {
    struct stmt* head = parse_statement();

    token = peek();
    if(token.token_type == INPUT || token.token_type == ID) {
        head->next = parse_statement_list();
    }

    return head;
}

stmt* Parser::parse_statement() {
    token = peek();
    stmt* newStatement = new stmt();
    if(token.token_type == INPUT) {
        newStatement->stmt_type = INPUT;
        newStatement->variable = parse_input_statement();
        return newStatement;
    }  if(token.token_type == ID) {
        return parse_poly_evaluation_statement();
    } else {
        syntax_error();
    }
}

int Parser::parse_input_statement() {
    int index = 0;

    expect(INPUT);
    index = allocateMemory(expect(ID).lexeme);

    expect(SEMICOLON);

    return index;
}

stmt* Parser::parse_poly_evaluation_statement() {
    stmt* statement = parse_polynomial_evaluation();
    expect(SEMICOLON);

    return statement;
}

stmt* Parser::parse_polynomial_evaluation() {
    stmt* statement = new stmt();
    poly* argList = new poly();

    parse_polynomial_name();
    int lineNumber = token.line_no;
    argList->Poly = -1;
    for(int i =0; i < poly_declerations.size();i++) {
        if(poly_declerations[i].name == token.lexeme) {
            argList->Poly = i;
            break;
        }
    }

    if(argList->Poly == -1) {
        symantics_error(2);
    }

    token = lexer.GetToken();
    if(token.token_type != LPAREN) {
        syntax_error();
    }


    argList->arguments = parse_argument_list();

    if(argList->Poly != -1) {
        if (poly_declerations[argList->Poly].Parameters.size() != argList->arguments.size() &&
            poly_declerations[argList->Poly].Parameters.size() > 0) {
            errors[3].push_back(lineNumber);
        } else if(poly_declerations[argList->Poly].Parameters.size() == 0 && argList->arguments.size() != 1) {
            errors[3].push_back(lineNumber);
        }
    }


    token = lexer.GetToken();
    if(token.token_type != RPAREN) {
        syntax_error();
    }

    statement->poly_eval = argList;
    statement->stmt_type = POLY;
    statement->variable = 0;

    return statement;
}

vector<struct poly_arg> Parser::parse_argument_list() {
    vector<struct poly_arg> argumentList;
    argumentList.push_back(parse_argument());

    token = lexer.GetToken();
    if(token.token_type == COMMA) {
        vector<struct poly_arg> newList = parse_argument_list();
        argumentList.insert(argumentList.end(),newList.begin(),newList.end());
    } else {
        lexer.UngetToken(token);
        return argumentList;
    }
}

struct poly_arg Parser::parse_argument() {
    token = lexer.GetToken();
    Token token2;
    poly_arg newArg;

    if(token.token_type == ID) {
        token2 = lexer.GetToken();
        if(token2.token_type == LPAREN) {
            lexer.UngetToken(token2);
            lexer.UngetToken(token);
            newArg.Type = POLY;
            newArg.poly_eval = parse_polynomial_evaluation();
            string test = "asdf";
        } else {
            newArg.Type = ID;
            newArg.value = 0;
            newArg.poly_eval = NULL;
            newArg.index = findMemory(token.lexeme);
            lexer.UngetToken(token2);
        }
    } else if (token.token_type != NUM) {
        syntax_error();
    } else {
        newArg.Type = NUM;
        newArg.value = stoi(token.lexeme);
        newArg.poly_eval = NULL;
    }
    return newArg;

}


void Parser::parse_polynomial_name() {
    expect(ID);
    lineNumber = token.line_no;
}

vector<int> Parser::parse_inputs(int i) {
    vector<int> inputs;

    expect(NUM);

    inputs.push_back(stoi(token.lexeme));

    token = lexer.GetToken();

    if (token.token_type == NUM) {
        lexer.UngetToken(token);
        vector<int> newList = parse_inputs(i++);
        inputs.insert(inputs.end(),newList.begin(),newList.end());
    } else if(token.token_type != END_OF_FILE) {
        syntax_error();
    }

    return inputs;

}

void Parser::execute_program(stmt *pStmt, vector<int> inputs) {
    struct stmt * pc;
    int v = -1;
    pc = pStmt;
    int next_input = 0;
    int memory[inputSize];

    bool first = true;
    while(pc != nullptr) {
        switch(pc->stmt_type) {

            case(POLY):
                if(first == false) {
                    cout << endl;
                } else {
                    first = false;
                }

                v = evaluate_polynomial(pc->poly_eval, memory);
                cout << v;
                break;

            case(INPUT):
                memory[pc->variable] = inputs[next_input];
                next_input++;
                break;
        }

        if(pc->next == nullptr) {
            cout << '\n';
        }

        pc = pc->next;
    }
}

int Parser::evaluate_polynomial(poly *pPoly, int memory[]) {

    poly_decleration toEval = poly_declerations[pPoly->Poly];
    vector<int> argumentIntegers;

    for(int i = 0;i<pPoly->arguments.size();i++) {
        switch(pPoly->arguments[i].Type) {
            case (NUM):
                argumentIntegers.push_back(pPoly->arguments[i].value);
                break;
            case (POLY):
                argumentIntegers.push_back(evaluate_polynomial(pPoly->arguments[i].poly_eval->poly_eval, memory));
                break;
            case(ID):
                argumentIntegers.push_back(memory[pPoly->arguments[i].index]);
                string test;
                break;
        }
    }

    return calculateCoeffecientList(toEval.body,argumentIntegers, toEval.Parameters);

}

int Parser::findIndex(vector<char> parameters, int index) {
    for (int i = 0; i < parameters.size(); i++) {
        if (parameters[i] == variable_table[index].symbol) {
            return i;
        }
    }
    return 0;
}

int Parser::calculateCoeffecient(coefficient* col, vector<int> argumentIntegers,vector<char> parameters) {
    if(col == NULL) {
        return 1;
    } else {
        int test = argumentIntegers[findIndex(parameters, col->x)];
        return pow(argumentIntegers[findIndex(parameters, col->x)],col->y) * calculateCoeffecient(col->next,argumentIntegers,parameters);
    }
}

int Parser::calculateCoeffecientList(coefficient_list *body,vector<int> argumentIntegers, vector<char> parameters) {

    coefficient_list* ptr = body->next;

    int total = body->term * calculateCoeffecient(body->coLL,argumentIntegers,parameters);
    TokenType sign = body->sign;

    while(ptr != NULL) {
        int value = ptr->term *calculateCoeffecient(ptr->coLL, argumentIntegers, parameters);
        if (sign == PLUS) {
            total += value;
        } else if (sign == MINUS) {
            total -= value;
        }

        sign = ptr->sign;

        ptr = ptr->next;

    }

    return total;
}

// Parsing
int main()
{
    //freopen("../input.txt", "r",stdin);
    new Parser();
}

