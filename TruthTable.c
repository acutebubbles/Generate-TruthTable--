#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#define MAX 100
//bool定义
typedef enum {
    FALSE = 0,
    TRUE = 1
} Bool;
//结构体别名声明
typedef struct vari vari;
//结构体定义
struct vari
{
    vari * input_l;
    vari * input_r;
    Bool output_value;
    void (* func)(vari *);
};
typedef union {
    char op;         // 操作符
    vari *varPtr;    // 变量指针
} StackItem;

typedef struct {
    int top;
    StackItem items[MAX];
} Stack;

//成员函数声明
void free_vari(vari *A);
char * Format(const char *string, ...);
Bool sFormat(const char *string, const char *logic_values);
vari * calc_output_value(vari *A);
void speci_init_total();
vari * speci_init_vari(vari* A, Bool output_value);
vari * set_vari(vari* A, vari* input_l, vari* input_r, void (* func)(vari *));
void and(vari *A);
void or(vari *A);
void not(vari *A);
void xor(vari *A);
void nand(vari *A);
void nor(vari *A);
void xnor(vari *A);
void imp(vari * A);
void print_vari(vari * A);
// 栈操作函数声明
void initStack(Stack* stack);
int isFull(Stack* stack);
int isEmpty(Stack* stack);
void pushOp(Stack* stack, char op);
void pushVar(Stack* stack, vari* varPtr);
char popOp(Stack* stack);
vari* popVar(Stack* stack);
char peekOp(Stack* stack);
vari* peekVar(Stack* stack);

int precedence(char op);
int isOperator(char c);
void reverse(char* exp);
void infixToPrefix(const char* infix, char* prefix);
void infixToPostfix(const char* infix, char* postfix);
// 打印 BTable
void printBTable();
//特殊变量声明
vari VTrue;
vari VFalse;
char BTable[MAX];
#define format(...) Format(__VA_ARGS__, NULL)
//爷的主函数

int main(void){
    
    speci_init_total();
    format("%v=%v", "A", "B");
    printBTable();
    return 0;
}

//成员函数定义
char * Format(const char *string, ...) {
    va_list args;
    va_start(args, string);
    
    char *variables[MAX]; // 用于存储变量名
    int count = 0;

    // 获取所有变量名
    while (count < MAX) {
        char *arg = va_arg(args, char *);
        if (arg == NULL) break; // 如果没有更多参数，则退出
        variables[count++] = arg;
    }
    va_end(args);

    // 生成逻辑值的所有组合
    char logic_values[1 << count][count + 1]; // 2^count个逻辑值
    for (int i = 0; i < (1 << count); i++) {
        for (int j = 0; j < count; j++) {
            logic_values[i][j] = (i & (1 << j)) ? 'T' : 'F'; // 设置逻辑值
        }
        logic_values[i][count] = '\0'; // 添加结束符
    }

    // 处理所有组合
    for (int i = 0; i < (1 << count); i++) {
        // 构造格式化字符串
        char format[MAX]; // 假设格式字符串足够大
        snprintf(format, sizeof(format), string, logic_values[i]);
        // 调用 sFormat 计算结果
        Bool result = sFormat(format, logic_values[i]);
        printf("Result for %s: %d\n", logic_values[i], result);
    }
    
    return BTable; // 根据需要返回合适的值
}

Bool sFormat(const char *string, const char *logic_values) {
    char infix[MAX];
    char postfix[MAX];

    strcpy(infix, string);
    infixToPostfix(infix, postfix);

    const char *p = postfix;
    vari *temp_var = NULL;
    vari *var_l = NULL;
    vari *var_r = NULL;
    Stack varStack;
    initStack(&varStack);
    int logic_index = 0; // 逻辑值索引

    while (*p != '\0') {
        if (*p == '%') {
            if (*(++p) == 'v') {
                // 读取逻辑值
                if (logic_values[logic_index] == 'T') {
                    temp_var = &VTrue; // 假设 &True 是全局或静态变量
                } else if (logic_values[logic_index] == 'F') {
                    temp_var = &VFalse; // 假设 &False 是全局或静态变量
                } else {
                    printf("Invalid logic value: %c\n", logic_values[logic_index]);
                    return FALSE; // 错误处理
                }
                pushVar(&varStack, temp_var);
                logic_index++; // 移动到下一个逻辑值
            }
        } else if (isOperator(*p)) {
            var_r = popVar(&varStack);
            if (*p != '~') {
                var_l = popVar(&varStack);
            } else {
                var_l = NULL;
            }
            vari *new_var = (vari *)malloc(sizeof(vari));
            if (!new_var) {
                printf("Memory allocation failed\n");
                return FALSE;
            }
            switch (*p) {
                case '&':
                    set_vari(new_var, var_l, var_r, and);
                    break;
                case '|':
                    set_vari(new_var, var_l, var_r, or);
                    break;
                case '>':
                    set_vari(new_var, var_l, var_r, imp);
                    break;
                case '=':
                    set_vari(new_var, var_l, var_r, xnor);
                    break;
                case '^':
                    set_vari(new_var, var_l, var_r, xor);
                    break;
                case '~':
                    set_vari(new_var, var_r, NULL, not);
                    break;
                default:
                    printf("UNKNOWN OPERATOR\n");
                    free(new_var);
                    return FALSE;
            }
            calc_output_value(new_var);
            pushVar(&varStack, new_var);    
        }
        p++;
    }
    vari *result_var = popVar(&varStack);
    Bool result = result_var->output_value;
    // 确保释放 result_var 的内存
    free(result_var); 
    return result;
}


void speci_init_total(){
    speci_init_vari(&VTrue, TRUE);
    speci_init_vari(&VFalse, FALSE);
}
void free_vari(vari *A) {
    if (A != NULL) {
        if (A->input_l != NULL) {
            free_vari(A->input_l);
            A->input_l = NULL;  // 释放后将指针置空
        }
        if (A->input_r != NULL) {
            free_vari(A->input_r);
            A->input_r = NULL;  // 释放后将指针置空
        }
        free(A);
        A = NULL;  // 释放后将指针置空
    }
}

vari * calc_output_value(vari *A){
    if (A->func == NULL){
        //printf("func==NULL\n");
        return A;
    }
    else {
        if (A->input_l == NULL){
            //printf("l==NULL\n");
            //return A;
        }
        else calc_output_value(A->input_l);

        if (A->input_r == NULL){
            //printf("r==NULL\n");
            //return A;
        }
        else calc_output_value(A->input_r);
        A->func(A);
    }
    return A;
}
vari * speci_init_vari(vari* A, Bool output_value){
    A->input_l=NULL;
    A->input_r=NULL;
    A->func=NULL;
    A->output_value = output_value;
    return A;
}
vari * set_vari(vari* A, vari* input_l, vari* input_r, void (* func)(vari *)){
    A->input_l = input_l;
    A->input_r = input_r;
    A->func = func;
    return A;
}
void and(vari * A){
    Bool b_input_l = A->input_l->output_value;
    Bool b_input_r = A->input_r->output_value;
    A->output_value = (b_input_l&&b_input_r);
}
void or(vari * A){
    Bool b_input_l = A->input_l->output_value;
    Bool b_input_r = A->input_r->output_value;
    A->output_value = (b_input_l||b_input_r);
}
void not(vari * A){
    Bool b_input_l = A->input_l->output_value;
    A->output_value = !(b_input_l);
}
void xor(vari * A){
    Bool b_input_l = A->input_l->output_value;
    Bool b_input_r = A->input_r->output_value;
    A->output_value = (b_input_l^b_input_r);
}
void nand(vari * A){
    Bool b_input_l = A->input_l->output_value;
    Bool b_input_r = A->input_r->output_value;
    A->output_value = !(b_input_l&&b_input_r);
}
void nor(vari * A){
    Bool b_input_l = A->input_l->output_value;
    Bool b_input_r = A->input_r->output_value;
    A->output_value = !(b_input_l||b_input_r);
}
void xnor(vari * A){
    Bool b_input_l = A->input_l->output_value;
    Bool b_input_r = A->input_r->output_value;
    A->output_value = (b_input_l==b_input_r);
}
void imp(vari * A){
    Bool b_input_l = A->input_l->output_value;
    Bool b_input_r = A->input_r->output_value;
    A->output_value = (!b_input_l||b_input_r);
}
void print_vari(vari * A){
    printf("The output is %d.\n", A->output_value);
}

// 栈操作函数
void initStack(Stack* stack) {
    stack->top = -1;
}

int isFull(Stack* stack) {
    return stack->top == MAX - 1;
}

int isEmpty(Stack* stack) {
    return stack->top == -1;
}

void pushOp(Stack* stack, char op) {
    if (!isFull(stack)) {
        stack->items[++stack->top].op = op;
    }
}

void pushVar(Stack* stack, vari* varPtr) {
    if (!isFull(stack)) {
        stack->items[++stack->top].varPtr = varPtr;
    }
}
char popOp(Stack* stack) {
    if (!isEmpty(stack)) {
        return stack->items[stack->top--].op;
    }
    return '\0'; // 返回空字符
}

vari* popVar(Stack* stack) {
    if (!isEmpty(stack)) {
        return stack->items[stack->top--].varPtr;
    }
    return NULL; // 返回空指针
}
char peekOp(Stack* stack) {
    if (!isEmpty(stack)) {
        return stack->items[stack->top].op;
    }
    return '\0'; // 返回空字符
}

vari* peekVar(Stack* stack) {
    if (!isEmpty(stack)) {
        return stack->items[stack->top].varPtr;
    }
    return NULL; // 返回空指针
}

// 操作符优先级
int precedence(char op) {
    switch (op) {
        case '~': // 非
            return 4;
        case '&': // 与
            return 3;
        case '^': // 异或
            return 2;
        case '|': // 或
            return 1;
        case '>': // 蕴含
            return 0;
        case '=': // 等价
            return 0;
    }
    return -1; // 不是有效操作符
}

// 检查是否为操作符
int isOperator(char c) {
    return c == '&' || c == '|' || c == '>' || c == '~' || c == '=' || c == '^';
}


// 反转字符串
void reverse(char* exp) {
    int n = strlen(exp);
    for (int i = 0; i < n / 2; i++) {
        char temp = exp[i];
        exp[i] = exp[n - i - 1];
        exp[n - i - 1] = temp;
    }
}

// 中缀表达式转前缀表达式
void infixToPrefix(const char* infix, char* prefix) {
    Stack stack;
    initStack(&stack);
    char temp[MAX];
    int j = 0;

    // 反转中缀表达式
    reverse((char*)infix);
    
    // 遍历反转后的中缀表达式
    for (int i = 0; infix[i]; i++) {
        char c = infix[i];

        // 如果是右括号，压入栈
        if (c == ')') {
            pushOp(&stack, c);
        }
        // 如果是左括号，弹出栈直到右括号
        else if (c == '(') {
            while (!isEmpty(&stack) && peekOp(&stack) != ')') {
                temp[j++] = popOp(&stack);
            }
            popOp(&stack); // 弹出右括号
        }
        // 如果是操作符
        else if (isOperator(c)) {
            while (!isEmpty(&stack) && precedence(peekOp(&stack)) >= precedence(c)) {
                temp[j++] = popOp(&stack);
            }
            pushOp(&stack, c);
        }
        // 否则，将其视为操作数
        else {
            temp[j++] = c;
        }
    }

    // 弹出栈中剩余的操作符
    while (!isEmpty(&stack)) {
        temp[j++] = popOp(&stack);
    }

    temp[j] = '\0'; // 结束字符串

    // 反转输出以得到前缀表达式
    reverse(temp);
    strcpy(prefix, temp);
}

// 中缀表达式转后缀表达式
void infixToPostfix(const char* infix, char* postfix) {
    Stack stack;
    initStack(&stack);
    int j = 0;

    // 遍历中缀表达式
    for (int i = 0; infix[i]; i++) {
        char c = infix[i];

        // 如果是左括号，压入栈
        if (c == '(') {
            pushOp(&stack, c);
        }
        // 如果是右括号，弹出栈直到左括号
        else if (c == ')') {
            while (!isEmpty(&stack) && peekOp(&stack) != '(') {
                postfix[j++] = popOp(&stack);
            }
            popOp(&stack); // 弹出左括号
        }
        // 如果是操作符
        else if (isOperator(c)) {
            while (!isEmpty(&stack) && precedence(peekOp(&stack)) >= precedence(c)) {
                postfix[j++] = popOp(&stack);
            }
            pushOp(&stack, c);
        }
        // 否则，将其视为操作数
        else {
            postfix[j++] = c;
        }
    }

    // 弹出栈中剩余的操作符
    while (!isEmpty(&stack)) {
        postfix[j++] = popOp(&stack);
    }

    postfix[j] = '\0'; // 结束字符串
}

// 打印 BTable 的函数
void printBTable() {
    printf("BTable contents: ");
    for (int i = 0; i < MAX; i++) {
        if (BTable[i] == '\0') { // 如果遇到字符串结束符，则停止打印
            break;
        }
        printf("%c", BTable[i]);
    }
    printf("\n");
}