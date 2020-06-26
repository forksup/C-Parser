/*
 * Copyright (C) Rida Bazzi, 2019
 *
 * Do not share this file with anyone
 */
#ifndef __PARSER_H__
#define __PARSER_H__

#include <string>
#include "lexer.h"
#include <vector>

using namespace std;

struct poly_arg;

struct symbol_table_struct {
    char symbol;
    int index;
};

struct variable {
    std::string symbol;
    int index;
};


struct coefficient {
    int x;
    int y = 1;//x is memory location y is exponent
    coefficient *next;
};

struct coefficient_list {
    coefficient* coLL;
    int term;
    TokenType sign;
    coefficient_list *next;
};

struct body {
    coefficient_list *list;
    int value;
    TokenType sign;
    body *next;
};


struct poly_decleration {
    std::string name;
    vector<char> Parameters;
    coefficient_list *body;
    int line_no;
};

std::vector<poly_decleration> poly_declerations;

struct poly {
    int Poly;
    vector<poly_arg> arguments;
};

struct stmt {
    int variable;
    poly* poly_eval;
    stmt* next;
    TokenType stmt_type;
};
struct poly_arg {
    TokenType Type;
    int value;
    int index;
    stmt* poly_eval;
};

class Parser {
public:
    Parser() {
        parse_input();
    }

private:
    LexicalAnalyzer lexer;
    int lineNumber;
    vector<symbol_table_struct> variable_table;
    vector<variable> symbol_table;

    vector<int> memory;
    static void syntax_error();

    Token expect(TokenType expected_type);
    Token peek();

    void parse_input();
    stmt * parse_program();
    void parse_poly_decl_section();
    stmt * parse_start();
    vector<char> parse_id_list();
    int parse_exponent();
    coefficient * parse_monomial_list();
    coefficient_list * parse_polynomial_body();
    coefficient_list * parse_term_list();
    coefficient_list * parse_term();
    void parse_coeffecient();

    stmt * parse_statement_list();
    stmt * parse_statement();
    stmt * parse_poly_evaluation_statement();
    stmt * parse_polynomial_evaluation();
    void parse_polynomial_name();

    poly_arg parse_argument();
    vector<struct poly_arg> parse_argument_list();
    int parse_input_statement();
    poly_decleration * parse_poly_decl();
    poly_decleration * parse_polynomial_header();
    void parse_add_operator();

    vector<int> parse_inputs(int i);

    void parse_monomial(coefficient *col);

    void execute_program(stmt *pStmt, vector<int> inputs);

    int evaluate_polynomial(poly *pPoly, int *memory);

    int findMemory(string input);

    int allocateMemory(string input);

    void symantics_error(int code);

    void print_errors();

    int calculateCoeffecientList(coefficient_list *body, vector<int> argumentIntegers, vector<char> parameters);

    int allocateVariable(char input);

    int findIndex(vector<char> parameters, int index);

    int calculateCoeffecient(coefficient *col, vector<int> argumentIntegers, vector<char> parameters);
};

#endif

