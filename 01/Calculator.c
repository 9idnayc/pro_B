#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX_EXPRESSION_LENGTH 100

struct Item_of_expression {
    int data;
    bool is_symbol;
};
typedef struct Item_of_expression Item;

struct Stack_of_item {
    Item entity[MAX_EXPRESSION_LENGTH];
    int top;
};
typedef struct Stack_of_item Stack;

bool char_is_in_array(char character,char array[]);
bool is_numeric(char character);
bool is_operator(char character);
short is_brackets(char character);
void pushback(Stack *expression, Item element);
bool pop_up(Stack *expression, Item *result);
void reset_stack(Stack *expression);
char item_to_char(Item element);
int precedence(char op);

int main() {
    char primitive_expression[MAX_EXPRESSION_LENGTH];  
    
    //输入
    printf("Please enter the expression: \n");
    fgets(primitive_expression, sizeof(primitive_expression), stdin);

    //排除空格和回车
    int primitive_len = strlen(primitive_expression); 
    int char_expression_len = 0;

    for (int i = 0; i < primitive_len; i++) {

        if (is_numeric(primitive_expression[i]) || is_operator(primitive_expression[i]) || is_brackets(primitive_expression[i]) != false) {
            char_expression_len++;
        }
    }

    char char_expression[char_expression_len];
    int index_of_char_expression = 0;
    
    for (int i = 0; i < primitive_len; i++) {

        if (is_numeric(primitive_expression[i]) || is_operator(primitive_expression[i]) || is_brackets(primitive_expression[i]) != false) {
            char_expression[index_of_char_expression++] = primitive_expression[i];
        }
    }

    //整合多位数字
    int item_count = 0;
    bool number_is_starting = false;

    for (int i = 0; i < char_expression_len; i++) {
        char current_char = char_expression[i];

        if (is_operator(current_char) || is_brackets(current_char) != false) {

            if (current_char == '-') {

                if (i == 0 || is_numeric(char_expression[i-1]) == false) continue;
                else item_count++;
            }
            else item_count++;

            if (number_is_starting) {
                item_count++;
                number_is_starting = false;
            }            
        }
        else number_is_starting = true;  
        // DEBUG
        // printf("DEBUG: current_char=%c, number_is_starting=%d, item_count=%d\n", current_char, number_is_starting, item_count);          
    }

    if (number_is_starting) {
        item_count++;
    }
    number_is_starting = false;

    // token化
    Item expression[item_count];
    int index_of_item = 0;
    char number_transition[char_expression_len];
    int top = 0;
    bool next_number_is_minus = false;

    for (int i = 0; i < char_expression_len; i++) {
        char current_char = char_expression[i];

        // 收集符号和括号
        if (is_operator(current_char) || is_brackets(current_char) != false) {

            if (current_char == '-') {

                if (i == 0 || is_numeric(char_expression[i-1]) == false) {
                    next_number_is_minus = true;
                    continue;
                }                
            }
            
            if (number_is_starting) {
                number_is_starting = false;
                number_transition[top] = '\0';
                int current_number = atoi(number_transition);

                if (next_number_is_minus) {
                    expression[index_of_item].data = current_number * (-1);
                    next_number_is_minus = false;
                }               
                else {
                    expression[index_of_item].data = current_number;
                }               
                expression[index_of_item].is_symbol = false;
                index_of_item++;
                top = 0;
                memset(number_transition, 0, sizeof(char)*char_expression_len);
            }
            expression[index_of_item].data = (int)current_char;
            expression[index_of_item].is_symbol = true;
            index_of_item++;            
        }
        // 收集数字
        else {
            number_is_starting = true;
            number_transition[top] = current_char;
            top++;
        }             
    }
    
    if (number_is_starting) {
        number_transition[top] = '\0';
        int current_number = atoi(number_transition);
        expression[index_of_item].data = current_number;
        expression[index_of_item].is_symbol = false;
        index_of_item++;
    }

    // DEBUG
    /*
    printf("\nDEBUG: ");
    for (int i = 0; i < item_count; i++) {
        Item current_item = expression[i];

        if (current_item.is_symbol) {
            printf("%c", (char)current_item.data);
        }
        else
        printf("[%d]", current_item.data);        
    }
    printf("\n");
    */
    
    // 中缀转后缀：逆波兰式
    Stack symbol_stack, RPN;
    symbol_stack.top = RPN.top = -1;
    Item temp_item;

    for (int i = 0; i < index_of_item; i++) {
        Item current_item = expression[i];

        // 数字直接压入RPN
        if (current_item.is_symbol == false) {
            pushback(&RPN, expression[i]);
        }
        else {

            // 符号栈为空，直接压栈
            if (symbol_stack.top == -1) {
                pushback(&symbol_stack, current_item);
            }
            else {

                // 左括号直接压进符号栈
                if (is_brackets(item_to_char(current_item)) == 1) {
                    pushback(&symbol_stack, current_item);
                }                
                else if (is_brackets(item_to_char(current_item)) == 2) { 

                    // 遇到右括号则逐渐将符号弹出，并逐一压入RPN，直至左括号；最后弹出左括号
                    while (symbol_stack.top >= 0 && item_to_char(symbol_stack.entity[symbol_stack.top]) != '(') {
                        pop_up(&symbol_stack, &temp_item);
                        pushback(&RPN, temp_item);
                    }
                    if (symbol_stack.top < 0) {
                        printf("Error: Mismatched parentheses (missing '('.)\n");
                        return 1;
                    }
                    symbol_stack.top--;
                }
                // 其他符号需要判断优先级
                else {
                    char current_char = item_to_char(current_item);

                    while (symbol_stack.top != -1 && item_to_char(symbol_stack.entity[symbol_stack.top]) != '(' && precedence(item_to_char(symbol_stack.entity[symbol_stack.top])) >= precedence(current_char)) {
                        pop_up(&symbol_stack, &temp_item);
                        pushback(&RPN, temp_item);
                    }
                    pushback(&symbol_stack, current_item);
                }
            }
        }
    }

    while (pop_up(&symbol_stack, &temp_item)) {

        // 检查是否为左括号
        if (is_brackets(item_to_char(temp_item)) == 1) { 
        printf("Error: Mismatched parentheses (Missing closing parenthesis).\n");
        return 1; 
        }
        pushback(&RPN, temp_item);
    }

    // DEBUG
    /*
    printf("\nRPN Expression: ");
    for (int i = 0; i <= RPN.top; i++)
    {
        if (RPN.entity[i].is_symbol)
        {
            printf("%c ", (char)RPN.entity[i].data);
        }
        else
        {
            printf("%d ", RPN.entity[i].data);
        }
    }
    printf("\n");
    */

    // DEBUG 
    /* 
    printf("\nDetailed RPN Check:\n");
    for (int i = 0; i <= RPN.top; i++)
    {
        if (RPN.entity[i].is_symbol)
        {
            printf("Index %d: Symbol '%c' (is_symbol: true)\n", i, (char)RPN.entity[i].data);
        }
        else
        {
            printf("Index %d: Number %d (is_symbol: false)\n", i, (int)RPN.entity[i].data);
        }
    }
    printf("\n");
    */  

    // RPN求值
    Stack value_stack;
    value_stack.top = -1;   

    for (int i = 0; i <= RPN.top; i++) {
        Item current_item = RPN.entity[i];

        if (!current_item.is_symbol) {
            pushback(&value_stack, RPN.entity[i]);
        }
        else {
            char op = item_to_char(current_item);

            Item left_item, right_item;

            if (!pop_up(&value_stack, &right_item) || !pop_up(&value_stack, &left_item)) {
                printf("Error: Not enough operands for operator '%c'.\n", op);
                return 1;
            }

            int le = left_item.data;
            int ri = right_item.data;
            int result = 0;

            switch (op) {
            case '+': result = le + ri; break;
            case '-': result = le - ri; break;
            case '*': result = le * ri; break;
            case '/': 
                if (ri == 0) {
                    printf("Error: Division by zero.\n");
                    return 1;
                }
                result = le / ri; 
                break;
            default: 
                printf("Error: Unknown operator '%c'.\n", op);
                return 1;
            }
            Item result_item = {result, false};
            pushback(&value_stack, result_item);
        }       
    }
    
    if (value_stack.top == 0) {
        printf("%d\n", value_stack.entity[0].data);
    }
    else if (value_stack.top == -1) {
        printf("Error: Empty expression.\n");
    }
    else {
        printf("Error: Invalid RPN expression (Too many operands).\n");
    }   

    return 0;
}



//
// subfuctions
//
bool char_is_in_array(char character,char valid_characters[])
{

    bool is_valid = 0;
    int len = strlen(valid_characters);

    for (int i = 0; i < len; i++)
    {
        if (character == valid_characters[i]) {
            is_valid = true;
            break;
        }
    }
    return is_valid;
    
}

bool is_numeric(char character)
{
    return character >= '0' && character <= '9';
}

bool is_operator(char character)
{
    char operators[] = "+-/*";
    return char_is_in_array(character, operators);
}

short is_brackets(char character)
{
    short bracket_mark = false;
    if (character == '(') bracket_mark = 1;
    else if (character == ')') bracket_mark = 2;
    return bracket_mark;   
}

void pushback(Stack *expression, Item element)
{
    if ((*expression).top >= MAX_EXPRESSION_LENGTH) return;
    else 
    {
        expression->top++;
        expression->entity[(*expression).top] = element;
    }
}

bool pop_up(Stack *expression, Item *result) 
{
    if (expression->top < 0) {
        return false;
    }
    *result = expression->entity[expression->top];
    expression->top--;
    return true;
}

void reset_stack(Stack *expression) 
{
    (*expression).top = -1;
}

char item_to_char(Item element)
{
    return (char)element.data;
}

int precedence(char op)
{
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    if (op == '~') return 3;
    return 0;  
}