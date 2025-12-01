#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAX_EXPRESSION_LENGTH 100

char input[MAX_EXPRESSION_LENGTH];
int char_index = 0;
char current_char;

void next();
void syntax_error();
double constant();
double factor();
double term();
double expression();

int main(void) {
    fgets(input, sizeof(input), stdin);

    int len = strlen(input);
    if (input[len - 1] == '\n') 
    {
        input[len - 1] = '\0';
    }

    char_index = 0;
    next();

    double result = expression();

    if (current_char != '\0') {
        syntax_error();
    }

    printf("%.10g\n", result);
    
    return 0;
}

void next() {
    current_char = input[char_index++];
}

void syntax_error() {
    printf("Syntax error occurs at the %d-th character.\n", char_index);
    exit(0);
}

double constant() {

    if (!isdigit(current_char))
    {
        syntax_error();
    }

    double value = 0;

    while (isdigit(current_char))
    {
        value = 10 * value + (current_char - '0');
        next();
    }

    if (current_char == '.')
    {
        next();

        if (!isdigit(current_char))
        {
            syntax_error();
        }

        double decimal_scale = 0.1;

        while (isdigit(current_char))
        {
            value += (current_char - '0') * decimal_scale;
            decimal_scale *= 0.1;
            next();
        }
    }
    
    return value;
}

double factor() {
    double value = 0;

    if (current_char == '-')
    {
        next();
        return -factor();
    }
    
    if (current_char == '(')
    {
        next();    // 跳过'(' 
        value = expression();

        if (current_char != ')')
        {
            syntax_error();
        }
        next();     // 跳过')'
    }

    else if (isdigit(current_char))
    {
        value = constant();
    }

    else {
        syntax_error();
    }
    
    if (current_char == '^')
    {
        next();

        if (!isdigit(current_char))
        {
            syntax_error();
        }

        int exponent = current_char - '0';  // 指数要求只有一位
        next();

        double result = 1;
        for (int i = 0; i < exponent; i++)
        {
            result *= value;
        }

        return result;
    }
    
    return value;
}

double term() {
    double value = factor();

    while (current_char == '*' || current_char == '/')
    {
        char op = current_char;
        next();

        double right_operand = factor();

        if (op == '*')
        {
            value *= right_operand;
        }
        else {

            if (right_operand == 0)
            {
                printf("Zero division occurs.\n");
                exit(0);
            }
            
            value /= right_operand;
        }       
    }
    
    return value;
}

double expression() {
    double value = term();

    while (current_char == '+' || current_char == '-') 
    {
        char op = current_char;
        next();     // 跳过 + 或 -

        double right = term();

        if (op == '+') 
        {
            value += right;
        } 
        else {
            value -= right;
        }
    }

    return value;
}