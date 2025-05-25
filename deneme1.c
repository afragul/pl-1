#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

const char* separators[] = {":=","-=", "+=",";","*","“","”"}; //line seperator operators
const int sep_count = 7;
const char* keywords[]={"number","repeat","times","write","and","newline"}; //keywords names
int blockCount=0; //block control

#define MAX_VARS 100 //kelime dizisi icin max variable sayisi
char declaredVariables[MAX_VARS][64];
int varCount = 0;

void addVariable(const char* var) {
    if (varCount < MAX_VARS) {
        strcpy(declaredVariables[varCount++], var);
    }
}

int isDeclared(const char* var) {
    for (int i = 0; i < varCount; i++) {
        if (strcmp(var, declaredVariables[i]) == 0) return 1;
    }
    return 0;
}

void replaceSeperator(char *line) {
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

void keywordType(char *type, FILE *outputFile, int lineNumber) {

    if (strcmp(type, "number") == 0) {
        fprintf(outputFile,"Keyword ( %s )\n", type); 

        char* next = strtok(NULL, " \t\n");
        if (next && isalpha(next[0])) {
            fprintf(outputFile,"Identifier ( %s )\n", next);
            addVariable(next);
        } else {
            fprintf(outputFile,"Error on line %d: Invalid variable declaration after 'number'\n", lineNumber);
        }
        return;
    }

    for (int i = 1; i < 6; i++) {
        if (strcmp(type, keywords[i]) == 0) {
            fprintf(outputFile,"Keyword ( %s )\n", type);
            return;
        }
    }

    if(type[0]=='{'){
        fprintf(outputFile,"OpenBlock\n");
        blockCount++;
    }else if(type[0]=='}'){
        if (blockCount == 0) {
            fprintf(outputFile, "Error on line %d: Closing block without opening block!\n", lineNumber);
        } else {
            blockCount--;
            fprintf(outputFile,"CloseBlock\n");
        }
    }else{
        if (!isDeclared(type)) {
            fprintf(outputFile, "Error on line %d: Undeclared variable '%s'\n", lineNumber, type);
        } else {
            fprintf(outputFile,"Identifier ( %s )\n", type);
        }
    }
}

int main() {
    int lineControl=0;
    FILE *dosya = fopen("deneme1.plus", "r");
    if (!dosya) {
        printf("Dosya açılamadı\n");
        return 1;
    }
    FILE *outputFile = fopen("output1.lx", "w");
    if (!outputFile) {
        printf("Dosyası açılamadı\n");
        return 1;
    }

    char line[1024];
    int skipMode=0;
    int strSkip=0;
    while (fgets(line, sizeof(line), dosya)) {
        lineControl++;
        replaceSeperator(line);

        char *token = strtok(line, " \t\n");
        while (token != NULL) {

            if (strcmp(token, "*") == 0) { // * gelirse ignore ediyoruz
                skipMode = !skipMode;
            }
            else if (!skipMode) {

                if (strcmp(token, "“") == 0) {
                    
                    char strConst[1024] = ""; 
                    token = strtok(NULL, " \t\n"); // İlk tırnaktan sonra gelen ilk kelimeyi tokena ceviriyorsun.
                    while (token != NULL && strcmp(token, "”") != 0) {
                        if (strlen(strConst) > 0) strcat(strConst, " "); // Eğer daha önce string'e bir şey eklendiyse, yeni kelimeden önce bir boşluk ekle ki stringler bitişik olmasın.
                        strcat(strConst, token); // O anki kelimeyi strConst değişkenine ekle.
                        token = strtok(NULL, " \t\n");
                    }
                    if (token != NULL && strcmp(token, "”") == 0) { 
                        
                        fprintf(outputFile, "StringConstant ( '%s' )\n", strConst); // yazdirma kismi
                        token = strtok(NULL, " \t\n"); 
                        continue; // bir sonraki tokene geç
                    }

                }else if (strcmp(token, ";") == 0) { //burasi sayesinde ; dan sonra * i okuyabiliyoruz
                    fprintf(outputFile,"EndOfLine\n");
                }
                else {
                    int isOperator = 0;
                    for (int i = 0; i < sep_count; i++) {
                        if (strcmp(token, separators[i]) == 0 && strcmp(token, "*") != 0 && strcmp(token, "“") != 0 && strcmp(token, "”") != 0) { 
                            fprintf(outputFile,"Operator ( '%s' )\n", token);
                            isOperator = 1;
                            break;
                        }
                    }
        
                    if (!isOperator) {
                        if (isNumber(token)) {
                            fprintf(outputFile,"IntConstant ( '%s' )\n", token);
                        } else {
                            keywordType(token,outputFile,lineControl);
                        }
                    }
                }
            }
            token = strtok(NULL, " \t\n");
        }
        
    }
    fclose(dosya);

    if(blockCount>0){ //kapanmamis {} hatasi HANGI SATIRDA BU HATA BUNU YAPAMADIM
        fprintf(outputFile,"Error: Unclosed block detected!\n");
    }
    fclose(outputFile);
    return 0;
}
