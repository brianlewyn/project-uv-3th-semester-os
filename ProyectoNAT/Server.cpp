#include <iostream>
#include <csignal>
#include <string>
#include <cstdlib>
using namespace std;

void setSignal(int sid);
int getrid();
void showMessage(int port);
bool getPortSignal(int port);
void server(int idRouter);

int main() {
    int idServer = getpid();
    setSignal(idServer);
    
    int idRouter = getrid();
    
    cout << "Server[idServer]: "<<idServer <<endl;
    server(idRouter);
    
    return 0;
}

void setSignal(int sid) {
    FILE *archivo = fopen("sid", "w");
    if (archivo != NULL) {
        fseek(archivo, 0, SEEK_SET);
        fwrite(&sid, sizeof(int), 1, archivo);
        fclose(archivo);
    } else {
        cout << "Se perdió la conexión con el cliente" << endl;
        exit(1);
    }
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

void showMessage(int port){
    string nombreArchivo = "P"+to_string(port);
    char cadena[100];

    FILE *archivo = fopen(nombreArchivo.c_str(), "r");
    if (archivo != NULL){
        fgets(cadena, 100, archivo);
        fclose(archivo);
    } else {
        cout << "No se pudo procesar la solicitud del cliente" << endl;
        exit(1);
    }
}

bool getPortSignal(int port) {
    string nombreArchivo = "P"+to_string(port);
    char cadena[100];

    FILE *archivo = fopen(nombreArchivo.c_str(), "r+");
    if (archivo != NULL){
        fgets(cadena, 100, archivo);

        cout << "Mensaje recibido: " << cadena << endl;

        if (cadena[0] > 64 && cadena[0] < 91) {
            for (char &c: cadena) {
                c = tolower(c);
            }
        } else {
            for (char &c: cadena) {
                c = toupper(c);
            }
        }

        cout << "Mensaje enviado a router " << cadena << endl;
        
        fseek(archivo, 0, SEEK_SET);
        fputs(cadena, archivo);
        fclose(archivo);
    } else {
        cout << "No se pudo procesar la solicitud del router" << endl;
        return false;
    }

    return true;
}

void routerRequest(int signal, siginfo_t *info, void *context) {
    if(signal == SIGTERM){
        cout << endl << "Se recibió una petición:" << endl;
    int idRouter = info->si_pid;
    
    cout << "Server[idRouter]: " << idRouter << endl;
    int port = info->si_value.sival_int;
    // showMessage(idRouter);

    getPortSignal(port);
    sigqueue(idRouter, SIGUSR1, {.sival_int = port});
    }
}

void server(int idRouter) {
    struct sigaction sa;
    sa.sa_sigaction = routerRequest;
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGTERM, &sa, NULL);
    cout << "Esperando solicitud..." << endl;
    while(true) {}
}
