// Autor: Leonardo Rener de Oliveira RA: 201270

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_THREADS 3

#define ANSI_COLOR_RED   "\x1B[31m"
#define ANSI_COLOR_GRN   "\x1B[32m"
#define ANSI_COLOR_YEL   "\x1B[33m"
#define ANSI_COLOR_BLU   "\x1B[34m"
#define ANSI_COLOR_MAG   "\x1B[35m"
#define ANSI_COLOR_CYN   "\x1B[36m"
#define ANSI_COLOR_WHT   "\x1B[37m"
#define ANSI_COLOR_RESET   "\x1b[0m"

typedef struct buscaPalavra_args {
    char **matriz;
    int n;
    char *palavra;
} buscaPalavra_args;

void menuInicial(int *n, char *modo);
char** gerarMatriz(int n, char modo);
void printMatriz(char **matriz, int n);
void recebePalavra(int n, char *palavra);
void* buscaPalavraHorizontal(void *arguments);
void* buscaPalavraVertical(void *arguments);
void* buscaPalavraDiagonal(void *arguments);

int main()
{
    int n;
    char modo;
    char **matriz;
    char *palavra;
    pthread_t thread[MAX_THREADS];

    printf(ANSI_COLOR_BLU "\nBEM VINDO! - ");
    printf(ANSI_COLOR_RED "CAÇA-PALAVRAS - ");
    printf(ANSI_COLOR_BLU "MC504\n\n" ANSI_COLOR_RESET);
    
    // Obtem o valor de n e o modo de geracao do caca-palavras.
    menuInicial(&n, &modo);

    // Preenche a matriz com caracters aleatorios.
    matriz = gerarMatriz(n, modo);
    
    // Mostra a matriz atual.
    printMatriz(matriz, n);

    // Recebe a palavra do usuario.
    recebePalavra(n, palavra);
    printf(ANSI_COLOR_MAG "%s\n", palavra);

    while(strcmp(palavra, "0") != 0){
        // Cria os argumentos para as threads de busca de palavra.
        buscaPalavra_args *args = malloc(sizeof(buscaPalavra_args));
        args->matriz = matriz;
        args->n = n;
        args->palavra = palavra;

        pthread_create(&thread[0], NULL, buscaPalavraHorizontal, (void *)args);
        pthread_create(&thread[1], NULL, buscaPalavraVertical, (void *)args);
        pthread_create(&thread[2], NULL, buscaPalavraDiagonal, (void *)args);

        pthread_join(thread[0], NULL);
        pthread_join(thread[1], NULL); 
        pthread_join(thread[2], NULL); 

        // Mostra a matriz atual.
        printMatriz(matriz, n);

        // Recebe a palavra do usuario.
        recebePalavra(n, palavra);
    }

    printf(ANSI_COLOR_MAG "Programa finalizado!\n\n");

    free(matriz);
    
    return 0;
}

void menuInicial(int *n, char *modo)
{
    int entrada_n;

    while(1){
        printf(ANSI_COLOR_YEL "Digite o tamanho do caça-palavras (n x n): " ANSI_COLOR_WHT);
        scanf("%d", &entrada_n);
        if(entrada_n >= 10 && entrada_n <= 100){
            while(1){
                printf(ANSI_COLOR_GRN "Digite \"A\" para gerar um caca-palavras aleatorio, ou \"M\" para adicionar manualmente: " ANSI_COLOR_WHT);
                scanf("%s", modo);
                if(tolower(*modo) == 'a' || tolower(*modo) == 'm'){
                    *n = entrada_n;
                    return;
                }
                else
                {
                    printf(ANSI_COLOR_MAG "Modo invalido! Digite novamente...\n");
                }
            }
        }
        printf(ANSI_COLOR_MAG "Tamanho invalido! Digite um valor entre 10 e 100\n");
    }
    
}

char** gerarMatriz(int n, char modo)
{
    int i, j;
    char **matriz;

    // aloca um vetor de n ponteiros para linhas.
    matriz = malloc (n * sizeof (char*));
    // aloca cada uma das linhas vetores de n inteiros.
    for(int i = 0; i < n; i++)
        matriz[i] = malloc (n * sizeof (char));

    if(tolower(modo) == 'a'){
        for(i = 0; i < n; i++){
            for(j = 0; j < n; j++){
                matriz[i][j] = 'a' + (char)(rand()%26);
            }
        }
    }
    else{
        printf(ANSI_COLOR_YEL "Digite as letras do caca-palavras (digite uma linha por vez):\n" ANSI_COLOR_WHT);
        for (i = 0; i < n; i++){
            scanf("%s", matriz[i]);
        }
    }

    return matriz;
}

void printMatriz(char **matriz, int n)
{
    printf("\n");
    int i, j;
    for(i = 0; i < n; i++){
        printf("\t");
        for(j = 0; j < n; j++){
            if('A' <= matriz[i][j] && matriz[i][j] <= 'Z'){
                printf(ANSI_COLOR_RED "%c ", matriz[i][j]);
            }
            else {
                printf(ANSI_COLOR_WHT "%c ", matriz[i][j]);
            }
        }
        printf("\n");
    }
    printf(ANSI_COLOR_RESET "\n");
}

void recebePalavra(int n, char *palavra)
{
    while(1){
        printf(ANSI_COLOR_GRN "Digite uma palavra (Ou '0' para encerrar): " ANSI_COLOR_WHT);
        scanf("%s", palavra);
        printf("\n");
        if(strlen(palavra) > n){
            printf(ANSI_COLOR_MAG "palavra invalida!");
        }
        else{
            return;
        }
    }
}

void* buscaPalavraHorizontal(void *arguments)
{
    int i, j, k, w;
    int palavrasEncontradas = 0;
    buscaPalavra_args *args = (buscaPalavra_args*) arguments;
    for(i = 0; i < args->n; i++){
        for(j = 0; j <= args->n - strlen(args->palavra); j++){
            if(tolower(args->matriz[i][j]) == tolower(args->palavra[0])){
                for(k = 0; k < strlen(args->palavra); k++){
                    // Torna as letras da palavra maiusculas.
                    if (tolower(args->palavra[k]) != tolower(args->matriz[i][j + k])){
                        break;
                    }
                    // Se todas as letras forem iguais a da palavra, aumenta a quantidade de palavras encontradas.
                    if(k == strlen(args->palavra)-1){
                        palavrasEncontradas++;
                        for(k = 0; k < strlen(args->palavra); k++){
                            args->matriz[i][j + k] = toupper(args->matriz[i][j + k]);
                        }
                        j = j + k + 1;
                    }
                    
                }
            }
        }
    }
    printf(ANSI_COLOR_CYN "Palavras encontradas na horizontal: %d\n", palavrasEncontradas);
    return NULL;
}

void* buscaPalavraVertical(void *arguments)
{
    int i, j, k, w;
    int palavrasEncontradas = 0;
    buscaPalavra_args *args = (buscaPalavra_args*) arguments;
    for(i = 0; i < args->n; i++){
        for(j = 0; j <= args->n - strlen(args->palavra); j++){
            if(tolower(args->matriz[j][i]) == tolower(args->palavra[0])){
                for(k = 0; k < strlen(args->palavra); k++){
                    if (tolower(args->palavra[k]) != tolower(args->matriz[j + k][i])){
                        break;
                    }
                    // Se todas as letras forem iguais a da palavra, aumenta a quantidade de palavras encontradas.
                    if(k == strlen(args->palavra)-1){
                        palavrasEncontradas++;
                        // Torna as letras da palavra maiusculas.
                        for(k = 0; k < strlen(args->palavra); k++){
                            args->matriz[j + k][i] = toupper(args->matriz[j + k][i]);
                        }
                        j = j + k + 1;
                    }
                    
                }
            }
        }
    }
    printf(ANSI_COLOR_CYN "Palavras encontradas na vertical: %d\n", palavrasEncontradas);
    return NULL;
}

void* buscaPalavraDiagonal(void *arguments)
{
    int i, j, k, w;
    int palavrasEncontradas = 0;
    buscaPalavra_args *args = (buscaPalavra_args*) arguments;
    for(i = 0; i <= args->n - strlen(args->palavra); i++){
        for(j = 0; j <= args->n - strlen(args->palavra); j++){
            if(tolower(args->matriz[i][j]) == tolower(args->palavra[0])){
                for(k = 0; k < strlen(args->palavra); k++){
                    // Torna as letras da palavra maiusculas.
                    if (tolower(args->palavra[k]) != tolower(args->matriz[i + k][j + k])){
                        break;
                    }
                    // Se todas as letras forem iguais a da palavra, aumenta a quantidade de palavras encontradas.
                    if(k == strlen(args->palavra)-1){
                        palavrasEncontradas++;
                        for(k = 0; k < strlen(args->palavra); k++){
                            args->matriz[i + k][j + k] = toupper(args->matriz[i + k][j + k]);
                        }
                        j = j + k + 1;
                    }
                    
                }
            }
        }
    }
    printf(ANSI_COLOR_CYN "Palavras encontradas na diagonal: %d\n", palavrasEncontradas);
    return NULL;
}