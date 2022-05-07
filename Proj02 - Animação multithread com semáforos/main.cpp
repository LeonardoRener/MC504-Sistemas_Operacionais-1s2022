/*
 * Autor: Leonardo Rener de Oliveira
 * RA: 201270
*/

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>

using namespace std;

// Constantes usadas no programa
#define N_TRABALHADORES 10
#define N_CASAS 4
#define N_VAGAS 5
#define N_TIPOS 2
#define N_CORES 4
#define TEMPO 2

// Cores usadas no programa
#define COR_MAGENTA   0
#define COR_VERMELHO  1
#define COR_AMARELO   2
#define COR_CIANO     3

#define COR_PADRAO    7
#define COR_GRAMA     8
#define COR_CEU       9

/*
 * Tipos de trabalhadores existente.
 * Pe - Pedreiro
 * Pi - Pintor
 */
typedef enum { Pe, Pi } tipo_trabalhador;

/*
 * Tipos de estado de uma obra.
 * Alv - Obra na fase de alvenaria, necessita de pedreiro.
 * Pin - Obra na fase de pintura, necessita de pintor.
 * Con - Obra concluida, nao necessita de trabalhadores.
 */
typedef enum { Alv, Pin, Con } estado_casa;

/*
 * Tipos de estado de um trabalhador
 * N - Nao possui estado
 * A - Aguardando vaga na fila.
 * C - Chegou na entrada.
 * T - Trabalhando.
 * S - Sair, trabalho concluido.
 * D - Deixar a obra.
 */
typedef enum{ N, A, C, T, S, D } estado_trab;

estado_trab estadoTrabalhador[N_TRABALHADORES];
estado_casa estadoCasa[N_CASAS];

/*
 * Estado da fila de trabalhadores
 * Oculpado - possui um trabalhador
 * Livre    - nao possui um trabalhador
 */
typedef enum {Oculpado, Livre} estado_fila;
estado_fila estadoFilaTrabalhadores[N_VAGAS];

/*
 * Structs dos Trabalhadores e Casas
*/
struct Trabalhador{
    int id;
    int imagem;
    tipo_trabalhador tipo;
};

struct Casa{
    int id;
    char casa[13][37];
    int cor[13][37];
    estado_casa estado;
};

/*
 * Semáforos
*/
sem_t sem_print;
sem_t sem_vagas;
sem_t sem_estados;
sem_t sem_iniciar_obra[N_CASAS];
sem_t sem_obra_finalizada[N_CASAS];
sem_t sem_trabalhador_obra[N_CASAS];
sem_t sem_escreve_painel[N_TIPOS], sem_le_painel[N_TIPOS];

/*
 * Variaveis globais
*/
int painel[N_TIPOS];
int trabalhadoresVaga[N_VAGAS];
int trabalhadoresObra[N_CASAS];
struct Casa id_casa[N_CASAS];
struct Trabalhador id_trabalhador[N_TRABALHADORES];
const char *TipoTrabalhador[] = { "Pedreiro", "Pintor" };

void iniciaCores();
void printCasas();
void printBackground();
void printAnimacao();
void construirCasa(char casa[13][37], int cor[13][37]);
void pintar(int parede[13][37], int cor, int linha, int inicio, int fim);
void pintarCasa(int cor[13][37]);
void* f_casa(void *v);
void* f_trabalhador(void* v);

int main() {
  srand(time(0));
  initscr();

  if (has_colors() == FALSE) {
    endwin();
    printf("O seu terminal nao suporta cores :(\n");
    exit(1);
  }

  iniciaCores();

  pthread_t thr_trabalhadores[N_TRABALHADORES], thr_casas[N_CASAS];
  int i; 

  // ========== Inicia os semafores ==========
  sem_init(&sem_print, 0, 1);
  sem_init(&sem_vagas, 0, N_VAGAS);
  sem_init(&sem_estados, 0, 1);
  for (i = 0; i < N_CASAS; i++) {
    sem_init(&sem_iniciar_obra[i], 0, 1);
    sem_init(&sem_trabalhador_obra[i], 0, 0);
    sem_init(&sem_obra_finalizada[i], 0, 0);
    sem_init(&sem_escreve_painel[i], 0, 1);
    sem_init(&sem_le_painel[i], 0, 0);
    trabalhadoresObra[i] = -1;
  }
  for (i = 0; i < N_TIPOS; i++) {
    sem_init(&sem_escreve_painel[i], 0, 1);
    sem_init(&sem_le_painel[i], 0, 0);
  }

  // ========== Inicia o estado da fila ==========
  for (i = 0; i < N_VAGAS; i++) {
    estadoFilaTrabalhadores[i] = Livre;
  }

  // ========== Inicia as pthreads ==========
  for (i = 0; i < N_CASAS; i++) {
    id_casa[i].id = i;
    id_casa[i].estado = Alv;
    for(int j = 0; j < 13; j++) {
      if(j <= 11) {
        strcpy(id_casa[i].casa[j], "                                    ");
      }
      else {
        strcpy(id_casa[i].casa[j], "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
      }
      for(int k = 0; k < 37; k++) {
        if(j <= 10) {
          id_casa[i].cor[j][k] = COR_CEU;
        }
        else {
          id_casa[i].cor[j][k] = COR_GRAMA;
        }
      }
    }
    estadoCasa[i] = Alv;
    pthread_create(&thr_casas[i], NULL, f_casa, (void*) &id_casa[i]);
  }

  for (i = 0; i < N_TRABALHADORES; i++) {
    id_trabalhador[i].id = i;
    id_trabalhador[i].tipo = tipo_trabalhador(rand()%(N_TIPOS));
    estadoTrabalhador[i] = N;
    pthread_create(&thr_trabalhadores[i], NULL, f_trabalhador, (void*) &id_trabalhador[i]);
  }
  
  for (i = 0; i < N_TRABALHADORES; i++) {
    pthread_join(thr_trabalhadores[i], NULL);
  }

  // ========== Finaliza o programa ==========
  endwin();
  
  return 0;
}

void printCasas() {
  int myY, myX;

  for(int casa = 0; casa < N_CASAS; casa++) {
    myX = 41*casa;

    attron(COLOR_PAIR(COR_PADRAO));
    
    mvprintw(1, 18 + myX, "OBRA %d", id_casa[casa].id);

    if(trabalhadoresObra[casa] != -1){
      mvprintw(2, 2 + myX, "Trabalhador: %d - Profissao: %s", trabalhadoresObra[casa], TipoTrabalhador[id_trabalhador[trabalhadoresObra[casa]].tipo]);
    }
    else{
      if(id_casa[casa].estado == Con) {
        mvprintw(2, 2 + myX, "            OBRA CONCLUIDA            ");
      }
      else {
        mvprintw(2, 2 + myX, "     Nao ha trabalhadores na obra     ");
      }
      
    }
    
    for(int i = 0; i < 15; i ++) {
      mvprintw(1 + i, myX, "|");
      mvprintw(1 + i, myX + 39, "|");
    }

    attroff(COLOR_PAIR(COR_PADRAO));
    
    for(int i = 0; i < 13; i ++) {
      for(int j = 0; j < 37; j++) {
        attron(COLOR_PAIR(id_casa[casa].cor[i][j]));

        mvprintw(3 + i, myX + 2 + j, "%c", id_casa[casa].casa[i][j]);

        attroff(COLOR_PAIR(id_casa[casa].cor[i][j]));
      }
    }
  }
  return;
}

void printBackground() {
  attron(COLOR_PAIR(COR_PADRAO));
  for (int y = 0; y < 40; y++) {
    mvhline(y, 0, ' ', COLS);
  }
  attroff(COLOR_PAIR(COR_PADRAO));
}

void printAnimacao() {
  sem_wait(&sem_print);
  refresh();
  int myY, myX, count;

  printBackground();

  printCasas();

  attron(COLOR_PAIR(COR_PADRAO));
  
  myY = 18;
  myX = 0;
  for(int i=0; i<10; i++){
    myY+=1;
    mvprintw(myY, myX, "|");
  }
  mvprintw(19, 6, "Entrada");
  myY = 20;
  myX = 3;
  count = 0;
  
  for(int i=0; i<N_TRABALHADORES; i++){
    if(estadoTrabalhador[i] == C){
      myY += 1;
      mvprintw(myY, myX, "%d(%s)", i, TipoTrabalhador[id_trabalhador[i].tipo]);
      count++;
    }
  }
  for(int i=count; i<N_TRABALHADORES; i++){
    myY += 1;
    mvprintw(myY, myX, "                ");
  }


  myY = 18;
  myX = 17;
  for(int i=0; i<10; i++){
    myY+=1;
    mvprintw(myY, myX, "|");
  }

  mvprintw(19, 19, "Fila de trabalhadores");
  myY = 20;
  myX = 19;
  for(int i=0; i<N_VAGAS; i++){
    myY+=1;
    mvprintw(myY, myX, "F%d -            ", i);
  }
  myY=5 + 15;
  myX=19 + 5;
  for(int i=0; i<N_VAGAS; i++){
    myY+=1;
    if(estadoFilaTrabalhadores[i] == Oculpado){
      mvprintw(myY, myX, "%d(%s)", trabalhadoresVaga[i], TipoTrabalhador[id_trabalhador[trabalhadoresVaga[i]].tipo]);
    }
  }

  myY = 18;
  myX=36 + 5;
  for(int i=0; i<10; i++){
    myY+=1;
    mvprintw(myY, myX, "|");
  }
  
  mvprintw(19, 43, "Trabalhadores saindo");
  myY = 20;
  myX=38 + 5;
  int cs=0;
  for(int i=0; i<N_TRABALHADORES; i++){
    if(estadoTrabalhador[i] == S){
      myY+=1;
      mvprintw(myY, myX, "%d(%s)", i, TipoTrabalhador[id_trabalhador[i].tipo]);
      cs++;
    }
  }
  for(int i=cs; i<N_TRABALHADORES; i++){
    myY+=1;
    mvprintw(myY, myX, "     ");
  }

  myY = 18;
  myX = 64;
  for(int i=0; i<10; i++){
    myY+=1;
    mvprintw(myY, myX, "|");
  }
  
  attroff(COLOR_PAIR(COR_PADRAO));

  refresh();
  sleep(1);
  sem_post(&sem_print);
  return;
}

void construirCasa(char casa[13][37], int cor[13][37]) {
  
  strcpy(casa[12], "~~~~~~\"   \"~~~~~~~~~~~~~~~~~~~~~~~~~");

  sleep((random()%TEMPO));
  printAnimacao();

  strcpy(casa[8],  "  /_I_II  I__I_\\__________________\\ ");
  pintar(cor, COR_PADRAO, 8, 2, 35);
  strcpy(casa[9],  "    I_I|  I__I_____[]_|_[]_____I    ");
  pintar(cor, COR_PADRAO, 9, 4, 32);
  strcpy(casa[10], "    I_II  I__I_____[]_|_[]_____I    ");
  pintar(cor, COR_PADRAO, 10, 4, 32);
  strcpy(casa[11], "    I II__I  I     XXXXXXX     I    ");
  pintar(cor, COR_PADRAO, 11, 4, 32);

  sleep((random()%TEMPO));
  printAnimacao();

  strcpy(casa[0], "                 (_)                ");
  pintar(cor, COR_PADRAO, 0, 17, 20);
  strcpy(casa[1], "         ________[_]________        ");
  pintar(cor, COR_PADRAO, 1, 17, 20);
  strcpy(casa[2], "        /\\        ______    \\       ");
  pintar(cor, COR_PADRAO, 2, 8, 29);
  strcpy(casa[3], "       //_\\       \\    /\\    \\      ");
  pintar(cor, COR_PADRAO, 3, 7, 30);
  strcpy(casa[4], "      //___\\       \\__/  \\    \\     ");
  pintar(cor, COR_PADRAO, 4, 6, 31);
  strcpy(casa[5], "     //_____\\       \\ |[]|     \\    ");
  pintar(cor, COR_PADRAO, 5, 5, 32);
  strcpy(casa[6], "    //_______\\       \\|__|      \\   ");
  pintar(cor, COR_PADRAO, 6, 4, 33);
  strcpy(casa[7], "   //_________\\                  \\  ");
  pintar(cor, COR_PADRAO, 7, 3, 34);

  sleep((random()%TEMPO));
  printAnimacao();

  return;
}

void pintar(int parede[13][37], int cor, int linha, int inicio, int fim) {
  for(int i = inicio; i < fim; i++) {
    parede[linha][i] = cor;
  }
  return;
}

void pintarCasa(int cor[13][37]) {
  int cor_escolhida = (random()%N_CORES);

  sleep((random()%TEMPO));
  pintar(cor, cor_escolhida, 11, 4, 32);
  pintar(cor, cor_escolhida, 10, 4, 32);
  pintar(cor, cor_escolhida, 9, 4, 32);
  printAnimacao();

  sleep((random()%TEMPO));
  pintar(cor, cor_escolhida, 8, 2, 35);
  pintar(cor, cor_escolhida, 7, 3, 34);
  pintar(cor, cor_escolhida, 6, 4, 33);
  printAnimacao();

  sleep((random()%TEMPO));
  pintar(cor, cor_escolhida, 5, 5, 32);
  pintar(cor, cor_escolhida, 4, 6, 31);
  pintar(cor, cor_escolhida, 3, 7, 30);
  pintar(cor, cor_escolhida, 2, 8, 29);
  pintar(cor, cor_escolhida, 1, 17, 20);
  pintar(cor, cor_escolhida, 0, 17, 20);
  printAnimacao();

  return;
}

void* f_casa(void *v) {
  struct Casa casa = *(struct Casa*) v;

  while(casa.estado != Con) {
    sem_wait(&sem_escreve_painel[casa.estado]);
    painel[casa.estado] = casa.id;
    sem_post(&sem_le_painel[casa.estado]);
    sem_wait(&sem_trabalhador_obra[casa.id]);

    sem_wait(&sem_estados);
    estadoCasa[casa.id] = casa.estado;
    sem_post(&sem_estados);

    // Aguardando finalizar a obra atual
    sem_wait(&sem_obra_finalizada[casa.id]);
    for(int i = 0; i < 13; i++) {
      strcpy(casa.casa[i], id_casa[casa.id].casa[i]);
    }
    casa.estado = id_casa[casa.id].estado;
  }

  return NULL;
}

void* f_trabalhador(void* v) {
  sleep(1);
  struct Trabalhador cliente = *(struct Trabalhador*) v;
  int id_obra;
  int minhaCadeiraCliente;

  sleep((random()%TEMPO)*4);

  // Trabalhador chegou
  sem_wait(&sem_estados);
  estadoTrabalhador[cliente.id] = C;
  printAnimacao();
  sem_post(&sem_estados);

  if (sem_trywait(&sem_vagas) == 0) {
    // Trabalhador está esperando para entrar na fila de vagas de emprego
    sem_wait(&sem_estados);
    estadoTrabalhador[cliente.id] = A;
    for (int i = 0; i < N_VAGAS; i++) {
      if (estadoFilaTrabalhadores[i] == Livre) {
        estadoFilaTrabalhadores[i] = Oculpado;
        trabalhadoresVaga[i] = cliente.id;
        minhaCadeiraCliente = i;
        break;
      }
    }

    printAnimacao();
    sem_post(&sem_estados);

    // Trabalhador está esperando pra ler o painel do tipo que ele trabalha
    sem_wait(&sem_le_painel[cliente.tipo]);
    
    // Trabalhador encontrou uma vaga de emprego
    id_obra = painel[cliente.tipo];

    // Trabalhador libera escrita no painel para outra vaga
    sem_post(&sem_escreve_painel[cliente.tipo]);

    // Trabalhador espera a obra liberar para iniciar o trabalho
    sem_wait(&sem_iniciar_obra[id_obra]);

    // Trabalhador inicia seu trabalho
    sem_post(&sem_trabalhador_obra[id_obra]);

    sem_wait(&sem_estados);

    // Trabalhador libera a fila de vagas
    estadoFilaTrabalhadores[minhaCadeiraCliente] = Livre;
    sem_post(&sem_vagas);

    // Trabalhando
    trabalhadoresObra[id_obra] = cliente.id;
    estadoTrabalhador[cliente.id] = T;
    printAnimacao();
    sem_post(&sem_estados);

    if(cliente.tipo == Pe) {
      // ===== CONSTRUCAO =====
      construirCasa(id_casa[id_obra].casa, id_casa[id_obra].cor);
      id_casa[id_obra].estado = Pin; // mudar para nao trocar o estado antes do trabalhador sair
    }
    else if(cliente.tipo == Pi) {
      // ===== PINTURA =====
      pintarCasa(id_casa[id_obra].cor);
      id_casa[id_obra].estado = Con;
    }

    sem_wait(&sem_estados);
    estadoTrabalhador[cliente.id] = S;
    trabalhadoresObra[id_obra] = -1;
    printAnimacao();
    estadoTrabalhador[cliente.id] = D;
    sem_post(&sem_estados);
    sem_post(&sem_iniciar_obra[id_obra]);

    // Finaliza a obra
    sem_post(&sem_obra_finalizada[id_obra]);
  }
  else {
    // Sem vaga na fila de trabalhadores.
    sem_wait(&sem_estados);
    estadoTrabalhador[cliente.id] = D;
    sem_post(&sem_estados);
  }
  return NULL;
}

void iniciaCores() {
  start_color();
  init_pair(COR_PADRAO, COLOR_BLACK, COLOR_WHITE);
  init_pair(COR_GRAMA, COLOR_YELLOW, COLOR_GREEN);
  init_pair(COR_CEU, COLOR_BLACK, COLOR_BLUE);
  init_pair(COR_MAGENTA, COLOR_BLACK, COLOR_MAGENTA);
  init_pair(COR_VERMELHO, COLOR_BLACK, COLOR_RED);
  init_pair(COR_AMARELO, COLOR_BLACK, COLOR_YELLOW);
  init_pair(COR_CIANO, COLOR_BLACK, COLOR_CYAN);
}