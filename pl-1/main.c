#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

const char* separators[] = {":=","-=", "+=",";","*","\""}; //line seperator operators
const int sep_count = 6;
const char* keywords[]={"number","repeat","times","write","and","newline"}; //keywords names
int blockCount=0; //block control
int blockLines[100];
int blockLineIndex = 0;


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
        blockLines[blockLineIndex++] = lineNumber;
        blockCount++;
    }else if(type[0]=='}'){
        if (blockCount == 0) {
            fprintf(outputFile, "Error on line %d: Closing block without opening block!\n", lineNumber);
        }else {
            blockCount--;
            blockLineIndex--;
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

int main(int argc, char *argv[]) {

    int lineControl = 0;
    if (argc < 2) {
        printf("Kullanım: %s <dosya_adı>\n", argv[0]);
        return 1;
    }

    // Giriş dosyasını aç
    char inputFilename[256];
    snprintf(inputFilename, sizeof(inputFilename), "%s.plus", argv[1]);
    FILE *dosya = fopen(inputFilename, "r");
    if (!dosya) {
        printf("Dosya açılamadı: %s\n", inputFilename);
        return 1;
    }

    // Çıkış dosyasının adını üret (.plus yerine .lx)
    char outputFilename[256];
    strcpy(outputFilename, inputFilename);
    char *dot = strrchr(outputFilename, '.');
    if (dot != NULL) {
        strcpy(dot, ".lx");  // uzantıyı değiştir
    } else {
        strcat(outputFilename, ".lx"); // uzantı yoksa ekle
    }

    FILE *outputFile = fopen(outputFilename, "w");
    if (!outputFile) {
        printf("Çıkış dosyası açılamadı: %s\n", outputFilename);
        return 1;
    }

    // Geri kalan kod (tokenization vs) burada senin yazdığın şekilde devam ediyor...
    char line[1024];
    int skipMode=0;
    
    while (fgets(line, sizeof(line), dosya)) {
        lineControl++;
        replaceSeperator(line);

        char *token = strtok(line, " \t\n");
        while (token != NULL) {

            if (strcmp(token, "*") == 0) { // * gelirse ignore ediyoruz
                skipMode = !skipMode;
            }
            else if (!skipMode) {

                if (strcmp(token, "\"") == 0) {   // normal " " isaretine göre ayırıyor 
                    
                    char strConst[1024] = "";
                    token = strtok(NULL, " \t\n");
                    while (token != NULL && strcmp(token, "\"") != 0) {
                        if (strlen(strConst) > 0) strcat(strConst, " ");
                        strcat(strConst, token);
                        token = strtok(NULL, " \t\n");
                    }
                
                    if (token != NULL && strcmp(token, "\"") == 0) {
                        fprintf(outputFile,"StringConstant ( '%s' )\n", strConst);
                        token = strtok(NULL, " \t\n");
                        continue;
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

    if(blockCount > 0){
        for (int i = 0; i < blockLineIndex; i++) {
            fprintf(outputFile,"Error: Unclosed block opened on line %d\n", blockLines[i]);
    }
}
    fclose(outputFile);
    return 0;
}
