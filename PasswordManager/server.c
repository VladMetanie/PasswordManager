#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/wait.h>

/* portul folosit */
#define PORT 3102

#define Server "ManagerParole"
/* codul de eroare returnat de anumite apeluri */
extern int errno;

void fnc_treat_zombies(int SignalNr)
{
  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;
}

typedef struct thData
{
  int idThread; // id-ul thread-ului tinut in evidenta de acest program
  int cl;       // descriptorul intors de accept
} thData;

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);

typedef struct categorie
{
  int nrparole;
  char parola[100][1000];
  char nume[1000];
  char notite[1000];
} categorie;

typedef struct userData
{
  int nrCategorii;
  categorie category[100];
} userData;

int NR = 1;
char users[100][1000];
char passwords[100][1000];
userData usersData[100];

int main()
{
  strcpy(users[0], "vlad");
  strcpy(passwords[0], "parola123");
  usersData[0].nrCategorii = 2;
  strcpy(usersData[0].category[0].nume, "munca");
  strcpy(usersData[0].category[1].nume, "socialMedia");
  usersData[0].category[0].nrparole = 1;
  strcpy(usersData[0].category[0].parola[0], "parola1");
  usersData[0].category[1].nrparole = 1;
  strcpy(usersData[0].category[1].parola[0], "parola2");
  strcpy(usersData[0].category[0].notite, "Parola de la Facebook");

  struct sockaddr_in server; // structura folosita de server
  struct sockaddr_in from;
  int nr; // mesajul primit de trimis la client
  int sd; // descriptorul de socket
  int pid;
  pthread_t th[100]; // Identificatorii thread-urilor care se vor crea
  int i = 0;
  char msg[1000];
  char rasp[100];
  int on = 1;
  signal(SIGCHLD, fnc_treat_zombies);

  /* crearea unui socket */

  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("[server]Eroare la socket().\n");
    return errno;
  }
  /* utilizarea optiunii SO_REUSEADDR */

  sd = socket(AF_INET, SOCK_STREAM, 0);
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

  /* pregatirea structurilor de date */
  bzero(&server, sizeof(server));
  bzero(&from, sizeof(from));

  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
  server.sin_family = AF_INET;
  /* acceptam orice adresa */
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  /* utilizam un port utilizator */
  server.sin_port = htons(PORT);

  /* atasam socketul */
  if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[server]Eroare la bind().\n");
    return errno;
  }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen(sd, 2) == -1)
  {
    perror("[server]Eroare la listen().\n");
    return errno;
  }
  /* servim in mod concurent clientii...folosind thread-uri */
  while (1)
  {
    int client;
    // thData * td; //parametru functia executata de thread
    int length = sizeof(from);

    printf("[server]Asteptam la portul %d...\n", PORT);
    fflush(stdout);

    // client= malloc(sizeof(int));
    /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
    if ((client = accept(sd, (struct sockaddr *)&from, &length)) < 0)
    {
      perror("[server]Eroare la accept().\n");
      continue;
    }
    int sockp[2];
    pid = fork();
    if (pid == -1)
    {
      printf("Eroare la fork\n");
      return -1;
    }
    printf("[server]S-a conectat un utilizator\n");
    fflush(stdout);
    if (pid > 0)
    {
      close(sd);
      while (1)
      {
        bzero(msg, 1000);
        fflush(stdout);
        int mesj = read(client, msg, 1000);

        if (mesj < 1)
        {
          perror("[server]Eroare la citirea de la client\n");
          return -1;
          continue;
        }
        printf("[server]Comanda primita de la client\n");
        printf("Comanda: %s", msg);
        fflush(stdout);


        if (strcmp(msg, "logare\n") == 0)
        {
          while (1)
          {
            bzero(msg, 1000);
            strcpy(msg, "Introduceti userul");
            printf("[server]Se trimite mesajul catre client %s\n", rasp);

            if (write(client, msg, 1000) <= 0)
            {
              perror("Eroare la scrierea catre client!");
              close(client);
              exit(0);
            }

            bzero(msg, 1000);
            fflush(stdout);
            int mesj = read(client, msg, 1000);

            if (mesj < 1)
            {
              perror("[server]Eroare la citirea de la client\n");
              return -1;
              continue;
            }

            printf("[server]Comanda primita de la client\n");
            printf("Comanda: %s", msg);

            fflush(stdout);

            if (strcmp(msg, "quit\n") == 0)
            {
              return -1;
            }
            // strcat(msg,"Introduecti o comanda valida \n");
            printf("Mesajul primit de la client este[%s]", msg);

            int nrUser = -1;
            msg[strlen(msg)-1] = 0;
            for(int i = 0 ; i < NR ; i++){
              printf("%s %s\n", users[0], msg);
              if(strcmp(msg,users[i]) == 0){
                nrUser = i;
                printf("s-a gasit userul\n");
                break;
              }
            }


            if (nrUser != -1)
            {
              printf("S-a primit vlad\n");
              while (1)
              {

                strcpy(msg, "Introduceti parola\n");
                fflush(stdout);
                printf("[server]Se trimite mesajul catre client %s\n", rasp);
                if (write(client, msg, 1000) <= 0)
                {
                  perror("Eroare la scrierea catre client!");
                  close(client);
                  exit(0);
                }

                bzero(msg, 1000);
                fflush(stdout);
                int mesj = read(client, msg, 1000);

                if (mesj < 1)
                {
                  perror("[server]Eroare la citirea de la client\n");
                  return -1;
                  continue;
                }
                printf("[server]Comanda primita de la client\n");
                printf("Comanda: %s", msg);
                msg[strlen(msg)-1] = 0;

                if (strcmp(msg, passwords[nrUser]) == 0)
                {
                  while(1){
                    fflush(stdout);
                    bzero(msg, 1000);
                    strcpy(msg, "show, add, newpassword, notite");
                    if (write(client, msg, 1000) <= 0)
                      {
                        perror("Eroare la scrierea catre client!");
                        close(client);
                        exit(0);
                      }
                    bzero(msg, 1000);
                    fflush(stdout);
                    int mesj = read(client, msg, 1000);

                    if(strcmp(msg,"show\n") == 0){
                      bzero(msg, 1000);
                      fflush(stdout);
                      printf("nr categorii %d\n", usersData[nrUser].nrCategorii);
                      for(int i = 0 ; i < usersData[nrUser].nrCategorii; i++){
                          strcat(msg, "Categoria: ");
                          strcat(msg,usersData[nrUser].category[i].nume);
                          strcat(msg, "\n");
                          strcat(msg, "Notite: ");
                          strcat(msg,usersData[nrUser].category[i].notite);
                          strcat(msg, "\n");
                          strcat(msg, "Parole:\n");
                          for(int j = 0 ; j < usersData[nrUser].category[i].nrparole ; j++){
                            strcat(msg, usersData[nrUser].category[i].parola[j]);
                            strcat(msg, "\n");
                          }
                      }
                      strcat(msg, "Apasati pe enter pt a continua");
                      write(client, msg, 1000);
                      read(client, msg, 1000);
                    }
                    if(strcmp(msg,"add\n") == 0){
                      strcpy(msg, "Numele categoriei de adaugat:\n");
                      write(client, msg, 1000);

                      bzero(msg, 1000);
                      fflush(stdout);
                      int mesj = read(client, msg, 1000);

                      msg[strlen(msg) - 1] = 0;
                      strcpy(usersData[nrUser].category[usersData[nrUser].nrCategorii].nume, msg);
                      usersData[nrUser].category[usersData[nrUser].nrCategorii].nrparole = 0;
                      usersData[nrUser].nrCategorii ++;
                    }
                    if(strcmp(msg,"newpassword\n") == 0){
                      strcpy(msg, "Numele categoriei la care adaugam parola:\n");
                      write(client, msg, 1000);

                      bzero(msg, 1000);
                      fflush(stdout);
                      read(client, msg, 1000);
                      msg[strlen(msg) - 1] = 0;
                      int nrcategorie = 0;
                      for(int i = 0; i <  usersData[nrUser].nrCategorii; i++) {
                        if(strcmp(usersData[nrUser].category[i].nume, msg) == 0){
                          nrcategorie = i;
                        }
                      }

                      strcpy(msg, "Parola:\n");
                      write(client, msg, 1000);

                      bzero(msg, 1000);
                      fflush(stdout);
                      read(client, msg, 1000);
                      msg[strlen(msg) - 1] = 0;
                      strcpy(usersData[nrUser].category[nrcategorie].parola[usersData[nrUser].category[nrcategorie].nrparole], msg);
                      usersData[nrUser].category[nrcategorie].nrparole++;

                    }

                    if(strcmp(msg,"notite\n") == 0){
                      strcpy(msg, "Numele categoriei la care adaugam parola:\n");
                      write(client, msg, 1000);

                      bzero(msg, 1000);
                      fflush(stdout);
                      read(client, msg, 1000);
                      msg[strlen(msg) - 1] = 0;
                      int nrcategorie = 0;
                      for(int i = 0; i <  usersData[nrUser].nrCategorii; i++) {
                        if(strcmp(usersData[nrUser].category[i].nume, msg) == 0){
                          nrcategorie = i;
                        }
                      }

                      strcpy(msg, "Notite:\n");
                      write(client, msg, 1000);

                      bzero(msg, 1000);
                      fflush(stdout);
                      read(client, msg, 1000);
                      msg[strlen(msg) - 1] = 0;
                      strcpy(usersData[nrUser].category[nrcategorie].notite, msg);

                    }
                  }
                }
              }
            }
          }
          // write(sockp[0], msg, strlen(msg));

          printf("[server]Mesajul a fost trasmis cu succes.\n");
        }
      }
    }
  }
}
