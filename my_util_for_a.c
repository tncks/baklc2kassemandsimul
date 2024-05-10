//Util functions (현재 스켈레톤에, 추가해도 일단 문제없는 코드들 써도그만 안써도 그만 함수 코드 segments들 ! 모음)
// git direct upload this file or / 혹은 gist public하게 하나 만들어도됨

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// define labels struc
struct labels {
    char letters[100];
    int addre;
};

struct Labels{
    char label[7];
    int address;
};

// define enum op
enum Operator {
    ADD,
    NOR,
    LW,
    SW,
    BEQ,
    JALR,
    HALT,
    NOOP,
    FILL,
    UNKNOWN
};

// declare functions
void findDuplicates(struct Labels list[MAXLINELENGTH], int num_labels);
enum Operator c2o(const char* c);
static inline int8_t checkReg(char* s);
int32_t everything2dec(const char* op, const char* arg0, const char* arg1, const char* arg2);

int add(char *arg0, char *arg1, char *arg2);
int nor(char *arg0, char *arg1, char *arg2);
int lw(int type, struct Labels list[MAXLINELENGTH], int address, int num_labels, char *arg0, char *arg1, char *arg2);
int sw(int type, struct Labels list[MAXLINELENGTH], int address, int num_labels, char *arg0, char *arg1, char *arg2);
int beq(int type, struct Labels list[MAXLINELENGTH], int address, int num_labels, char *arg0, char *arg1, char *arg2);
int jalr(char *arg0, char *arg1, char *arg2);
int halt();
int noop();
int fill(int type, struct Labels list[MAXLINELENGTH], int address, int num_labels, char *arg0, char *arg1, char *arg2);
//end

// case handling, dealing with some cases
int judge_valid_reg(char* arg) {
    int reg = -1;
    reg = atoi(arg);
    if (reg < 0 || reg > 7) {
        return 0;
    }
    while (*arg != '\0') {
        if ((*arg) < '0' || (*arg) > '7') {
            return 0;
        }
        arg++;
    }
    return 1;
}

int judge_valid(int num) {
    if (num < -32768 || num > 32767) {
        return 0;
    }
    return 1;
}

int find(char* arg2, struct labels label_list[], int lab_num, int cur_address) {
    int offset = 0;
    int record_address = -1;
    for (int i = 0; i < lab_num; i++) {
        if (!strcmp(label_list[i].letters, arg2)) {
            record_address = label_list[i].addre;
            offset = record_address - cur_address - 1;
            break;
        }
    }
    if (record_address == -1) {
        printf("Error:address not found");
        exit(1);
    }
    return offset;
}

int find_lsw(char* arg2, struct labels label_list[], int lab_num) {
    int record_address = -1;
    for (int i = 0; i < lab_num; i++) {
        if (!strcmp(label_list[i].letters, arg2)) {
            record_address = label_list[i].addre;
            break;
        }
    }
    if (record_address == -1) {
        printf("Error:address not found");
        exit(1);
    }
    return record_address;
}

int fill(char* arg0, char* arg1, char* arg2, struct labels label_list[], int lab_num) {
    int result = 0;
    if (isNumber(arg0)) {
        result = atoi(arg0);
        if (judge_valid(result) == 0) {
            printf("Error: Invalid number");
            exit(0);
        }
    } else {
        result = find_lsw(arg0, label_list, lab_num);
    }
    return result;
}

void findduplen(struct labels label_list[], int lab_num) {
    size_t length;
    for (int i = 0; i < lab_num; i++) {
        length = strlen(label_list[i].letters);
        if (length > 7) {
            printf("Error: Too long");
            exit(1);
        }
        for (int j = i + 1; j < lab_num; j++) {
            if (strcmp(label_list[i].letters, label_list[j].letters) == 0) {
                printf("Error:Duplicate labels");
                exit(1);
            }
        }
    }
}

int calculate_mc(int i, char* arg0, char* arg1, char* arg2, int addres, struct labels label_list[], int lab_num) {
    int result = 0;
    if (i == 0 || i == 1) {
        if (judge_valid_reg(arg0) == 0 || judge_valid_reg(arg1) == 0 || judge_valid_reg(arg2) == 0) {
            printf("Error:Register not integer");
            exit(1);
        }
        result = ((((((i << 3) + atoi(arg0)) << 3) + atoi(arg1)) << 16) + atoi(arg2));
    } else if (i == 2 || i == 3 || i == 4) {
        if (judge_valid_reg(arg0) == 0 || judge_valid_reg(arg1) == 0) {
            printf("Error:Register not integer");
            exit(1);
        }
        result = ((((((i << 3) + atoi(arg0)) << 3) + atoi(arg1)) << 16));
        if (isNumber(arg2)) {
            if (judge_valid(atoi(arg2)) == 0) {
                printf("Error:Invalid offset");
                exit(1);
            }
            result |= (atoi(arg2) & 0xFFFF);
        } else {
            if (i == 4) {
                result += ((find(arg2, label_list, lab_num, addres)) & 0xFFFF);
            } else {
                result += ((find_lsw(arg2, label_list, lab_num)) & 0xFFFF);
            }
        }
    } else if (i == 5) {
        if (judge_valid_reg(arg0) == 0 || judge_valid_reg(arg1) == 0) {
            printf("Error:Register not integer");
            exit(1);
        }
        result = ((((((i << 3) + atoi(arg0)) << 3) + atoi(arg1)) << 16));

    } else if (i == 6 || i == 7) {
        result = ((((((i << 3) + atoi(arg0)) << 3) + atoi(arg1)) << 16));
    }
    return result;
}

// more
enum Operator c2o(const char* c) {
    char s[9][9] = {"add", "nor", "lw", "sw", "beq", "jalr", "halt", "noop", ".fill"};
    enum Operator ops[9] = {ADD, NOR, LW, SW, BEQ, JALR, HALT, NOOP, FILL};
    for (uint8_t i = 0; i < 9; i++) {
        if (!strcmp(s[i], c))
            return ops[i];
    }
    return UNKNOWN;
}

static inline int8_t checkReg(char* s) {
    if (!isNumber(s))
        exit(1);
    int32_t num = atoi(s);
    return num >= 0 && num <= 7;
}

int32_t everything2dec(const char* op, const char* arg0, const char* arg1, const char* arg2) {
    // const uint8_t SHIFT[5] = {0, 22, 19, 16, 0};
    // opcode, arg0, arg1, arg2
    enum Operator opp = c2o(op);
    if (opp == FILL) {
        return atoi(arg0);
    }
    return ((opp << 22) & ((1 << 25) - (1 << 22))) + ((atoi(arg0) << 19) & ((1 << 22) - (1 << 19))) + ((atoi(arg1) << 16) & ((1 << 19) - (1 << 16))) + ((atoi(arg2)) & ((1 << 16) - 1));
}

int findAddress (int type, char* arg0, struct Labels list[], int address, int num_labels){
    int tempAddress = -1;
    for (int i = 0; i < num_labels; ++i){
        if (!strcmp(list[i].label, arg0)){
            tempAddress = list[i].address;
            break;
        }
    }
    if (tempAddress == -1){
        printf("error in find address\n");
        exit(1);
    }
    if (type == 2 || type == 3) {
        tempAddress &= 65535;
    }
    else if (type == 4) {
        tempAddress = (65535 & (tempAddress - address - 1));
    }
    return tempAddress;
}


int add(char *arg0, char *arg1, char *arg2){
    unsigned int arg0_val;
    unsigned int arg1_val;
    unsigned int arg2_val;
    int val = 0;
    
    val += (0 << 22);
    arg0_val = atoi(arg0);
    val = val + (arg0_val << 19);
    
    arg1_val = atoi(arg1);
    val = val + (arg1_val << 16);
    
    arg2_val = atoi(arg2);
    val = val + (arg2_val);
    return val;
}


int nor(char *arg0, char *arg1, char *arg2){
    unsigned int arg0_val;
    unsigned int arg1_val;
    unsigned int arg2_val;
    int val = 0;
    
    val += (1 << 22);
    
    arg0_val = atoi(arg0);
    val = val + (arg0_val << 19);
    
    arg1_val = atoi(arg1);
    val = val + (arg1_val << 16);
    
    arg2_val = atoi(arg2);
    val = val + (arg2_val);
    
    return val;
}

int lw(int type, struct Labels list[MAXLINELENGTH], int address, int num_labels, char *arg0, char *arg1, char *arg2){
    int arg0_val;
    int arg1_val;
    int arg2_val;
    int val = 0;
    
    val += (1 << 23);
    
    arg0_val = atoi(arg0);
    val = val + (arg0_val << 19);
    
    arg1_val = atoi(arg1);
    val = val + (arg1_val << 16);
    
    if (isNumber(arg2)){
        arg2_val = atoi(arg2);
        if (arg2_val <= -32769 || arg2_val >= 32768){
            printf("field too big\n");
            exit(1);
        }
        val += arg2_val & 0xFFFF;
    }
    // not number
    else {
        val += findAddress(2, arg2, list, address, num_labels);
    }
    return val;
}

int sw(int type, struct Labels list[MAXLINELENGTH], int address, int num_labels, char *arg0, char *arg1, char *arg2){
    int arg0_val;
    int arg1_val;
    int arg2_val;
    int val = 0;
    
    val += (3 << 22);

    
    arg0_val = atoi(arg0);
    val = val + (arg0_val << 19);
    
    arg1_val = atoi(arg1);
    val = val + (arg1_val << 16);
    
    if (isNumber(arg2)){
        arg2_val = atoi(arg2);
        if (arg2_val <= -32769 || arg2_val >= 32768){
            printf("field too big\n");
            exit(1);
        }
        val += arg2_val & 0xFFFF;
    }
    else {
        val += findAddress(3, arg2, list, address, num_labels);
    }
    return val;
}

int beq(int type, struct Labels list[MAXLINELENGTH], int address, int num_labels, char *arg0, char *arg1, char *arg2){
    signed int arg0_val;
    signed int arg1_val;
    signed int arg2_val;
    int val = 0;
    
    val += (4 << 22);
    
    arg0_val = atoi(arg0);
    val = val + (arg0_val << 19);
    
    arg1_val = atoi(arg1);
    val = val + (arg1_val << 16);
    
    if (isNumber(arg2)){
        arg2_val = atoi(arg2);
        if (arg2_val <= -32769 || arg2_val >= 32768){
            printf("field too big\n");
            exit(1);
        }
        val += arg2_val & 0xFFFF;
    }
    else {
        val += findAddress(4, arg2, list, address, num_labels);
    }
    return val;
}

int jalr(char *arg0, char *arg1, char *arg2){
    int arg0_val;
    int arg1_val;
    int val = 0;
    
    val += (5 << 22);
    
    arg0_val = atoi(arg0);
    val = val + (arg0_val << 19);
    
    arg1_val = atoi(arg1);
    val = val + (arg1_val << 16);
    
    return val;
}

int halt(){
    int val = 0;
    
    val += (6 << 22);
    
    return val;
}

int noop(){
    int val = 0;
    
    val += (7 << 22);
    
    return val;
}

int fill(int type, struct Labels list[MAXLINELENGTH], int address, int num_labels, char *arg0, char *arg1, char *arg2){
    int val = 0;
    int arg0_val = 0;
    if (isNumber(arg0)){
        arg0_val = atoi(arg0);
        if (arg0_val <= -32769 || arg0_val >= 32768){
            printf("field too big\n");
            exit(1);
        }
    }
    else {
        arg0_val = findAddress(-1, arg0, list, address, num_labels);
    }
    val += arg0_val; 
    return val;
}