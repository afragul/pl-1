#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

const char* separators[] = {":=", ",-=", "+=",";","*","\""}; //line seperator operators
const int sep_count = 6;
const char* keywords[]={"number","repeat","times","write","and","newline"}; //keywords names


void replaceSeperator(char *line) {           //seperate the line   BURAYI BI BASITLESTIR KRAL
    for (int i = 0; i < sep_count; i++) {
        char *pos = line;
        size_t sep_len = strlen(separators[i]);
        while ((pos = strstr(pos, separators[i])) != NULL) {
            memmove(pos + sep_len + 2, pos + sep_len, strlen(pos + sep_len) + 1);
            pos[0] = ' ';
            memcpy(pos + 1, separators[i], sep_len);
            pos[sep_len + 1] = ' ';
            pos += sep_len + 2;
        }
    }
}

int isNumber(const char *str) {     //control the variable for is it integer
    char *endptr;
    if (str == NULL || *str == '\0')
        return 0;
    strtod(str, &endptr);
    return (*endptr == '\0');
}

void keywordType(char *type){
    for (int i = 0; i < 6; i++) {
        if (strcmp(type, keywords[i]) == 0) {
            printf("Keyword ( %s )\n", type);
            return;
        }
    }
    if(type[0]=='{'){
        printf("OpenBlock\n");
    }else if(type[0]=='}'){
        printf("CloseBlock\n");
    }else{
        printf("Identifier ( '%s' )\n", type);
    }
}

int main() {
    FILE *dosya = fopen("deneme1.plus", "r");
    if (!dosya) {
        printf("Dosya açılamadı\n");
        return 1;
    }

    char line[1024];
    int skipMode=0;
    while (fgets(line, sizeof(line), dosya)) {
        // Satırdaki çok karakterli ayırıcıları boşlukla ayır
        replaceSeperator(line);

        char *token = strtok(line, " \t\n");
        while (token != NULL) {

            if (strcmp(token, "*") == 0) { // * gelirse ignore ediyoruz
                skipMode = !skipMode;
            }
            else if (!skipMode) {
                if (strcmp(token, ";") == 0) { //burasi sayesinde ; dan sonra * i okuyabiliyoruz
                    printf("EndOfLine\n");
                }
                else {
                    int isOperator = 0;
                    for (int i = 0; i < sep_count; i++) {
                        if (strcmp(token, separators[i]) == 0 && strcmp(token, "*") != 0 && strcmp(token, "\"") != 0) { 
                            printf("Operator ( '%s' )\n", token);
                            isOperator = 1;
                            break;
                        }
                    }
        
                    if (!isOperator) {
                        if (isNumber(token)) {
                            printf("IntConstant ( '%s' )\n", token);
                        } else {
                            keywordType(token);
                        }
                    }
                }
            }
        
            // BU SATIR her durumda en sonda çağrılır
            token = strtok(NULL, " \t\n");
        }
        
    }

    fclose(dosya);
    return 0;
}
