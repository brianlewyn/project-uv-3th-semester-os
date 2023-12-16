#include <iostream>
#include <csignal>
#include <string>
#include <cstdlib>
using namespace std;

int getrid();
void msgClient(int idClient, string cadena);
void client(int idClient, int idRouter);

int main() {
    int idClient = getpid();
    int idRouter = getrid();
    
    cout << "Client[idClient]: "<<idClient <<endl;
    client(idClient, idRouter);
    
    return 0;
}

int getrid(){
    int rid;

    FILE *archivo = fopen("rid", "r");
    if (archivo != NULL){
        fread(&rid, sizeof(int), 1, archivo);
        fclose(archivo);
    } else {
        cout << "No se logró la conexión con el router" << endl;
        exit(1);
    }

    return rid;
}

void msgClient(int idClient, string cadena){
    string nombreArchivo = to_string(idClient);

    FILE *archivo = fopen(nombreArchivo.c_str(), "w");
    if (archivo != NULL){
        fseek(archivo, 0, SEEK_SET);
        fputs(cadena.c_str(), archivo);
        fclose(archivo);
    } else {
        cout << "No se logró la conexión con el router" << endl;
        exit(1);
    }
}

 void routerResponse(int signal, siginfo_t *info, void *context) {
     //abrir el archivo
     string nombreArchivo = to_string(getpid());
     char cadena[100];

     FILE *archivo = fopen(nombreArchivo.c_str(), "r");
    if (archivo != NULL){
        fgets(cadena, 100, archivo);
        fclose(archivo);
    } else {
        cout << "No se logró la conexión con el router" << endl;
        exit(1);
    }

    cout << "Mensaje recibido del router: " << cadena << endl;
    remove(nombreArchivo.c_str());
}

void client(int idClient, int idRouter) {
    string cadena;

    cout << "Ingresa una cadena: " << endl;
    getline(cin, cadena);

    msgClient(idClient, cadena);
    kill(idRouter, SIGTERM);

    cout << "Mensaje enviado a router con id: " << idRouter << endl;

    struct sigaction sa;
    sa.sa_sigaction = routerResponse;
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGTERM, &sa, NULL);

    cout << "Esperando la respuesta del servidor..." << endl;

    pause();
}
