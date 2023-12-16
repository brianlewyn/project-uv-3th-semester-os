#include <iostream>
#include <csignal>
#include <string>
#include <cstdlib>
#include <thread>
#include <semaphore.h>
using namespace std;

sem_t mutex;
int idServer;
int idPort = 1;
bool salir = true;

void setSignal(int rid);
int getsid();
void showMessage(int cid);
//bool setPortSignal();
void clientRequest(int signal, siginfo_t *info, void *context);
void sendReponse(int cid);
void serverResponse(int signal, siginfo_t *info, void *context);
void clientListener(int idRouter);
//void serverListener(int idRouter);

int main() {
    int idRouter = getpid();
    setSignal(idRouter);

    idServer = getsid();
    sem_init(&mutex, 0, 1);

    cout << "Router[idRouter]: "<< idRouter <<endl;
    thread clientThread(clientListener, idRouter);
    //thread serverThread(serverListener, idRouter);

    clientThread.join();
    // //serverThread.join();

    return 0;
}

void setSignal(int rid) {
    FILE *archivo = fopen("rid", "w");
    if (archivo != NULL) {
        fseek(archivo, 0, SEEK_SET);
        fwrite(&rid, sizeof(int), 1, archivo);
        fclose(archivo);
    } else {
        cout << "Se perdió la conexión con el cliente" << endl;
        exit(1);
    }
}

int getsid() {
    int sid;

    FILE *archivo = fopen("sid", "r");
    if (archivo != NULL){
        fread(&sid, sizeof(int), 1, archivo);
        fclose(archivo);
    } else {
        cout << "No se logró la conexión con el servidor" << endl;
        exit(1);
    }

    return sid;
}

void showMessage(int cid) {
    string nombreArchivo = to_string(cid);
    char cadena[100];

    FILE *archivo = fopen(nombreArchivo.c_str(), "r");
    if (archivo != NULL){
        fgets(cadena, 100, archivo);
        cout << "Mensaje recibido: " << cadena << endl;
        fclose(archivo);
    } else {
        cout << "No se pudo procesar la solicitud del cliente" << endl;
        exit(1);
    }
}

bool setPortSignal(int cid, int port) {
    string nombreArchivo = to_string(cid);
    char cadena[100];

    FILE *archivo = fopen(nombreArchivo.c_str(), "r");
    if (archivo != NULL){
        fgets(cadena, 100, archivo);
        fclose(archivo);
    } else {
        cout << "No se pudo procesar la solicitud del cliente" << endl;
        fclose(archivo);
        return false;
    }

    string nombreArchivoP = "P"+to_string(port);
    
    FILE *archivoP = fopen(nombreArchivoP.c_str(), "w");
    if (archivoP != NULL) {
        fseek(archivoP, 0, SEEK_SET);
        fputs(cadena, archivoP);
        fclose(archivoP);
    } else {
        cout << "No se pudo procesar la solicitud del cliente" << endl;
        return false;
    }

    return true;
}

void serverResponse(int signal, siginfo_t *info, void *context) {
    if (signal == SIGUSR1){
        int port = info->si_value.sival_int;
        if (idPort == port) {
            salir = false;
        }
    }
}

void clientRequest(int signal, siginfo_t *info, void *context) {
    if (signal == SIGTERM) {
        int idClient = info->si_pid;
        cout << endl << "Se recibió una solicitud" << endl;
        cout << "Router[idClient]: "<< idClient <<endl;
         showMessage(idClient);

        if (setPortSignal(idClient, idPort)) {
            
            
            sigqueue(idServer, SIGTERM, {.sival_int = idPort});
            cout << "Se envió el mensaje a el servidor" << endl;
            cout << "Router[idServer]: "<< idServer <<endl;
            cout << "Router[idPuerto]: "<< idPort <<endl;
            
            
            

            // Wait a response
            struct sigaction sa;
            sa.sa_sigaction = serverResponse;
            sa.sa_flags = SA_SIGINFO;
            sigaction(SIGUSR1, &sa, NULL);

            while(salir==true){}
            salir = true;

            sendReponse(idClient);

            idPort++;
        } else {
            cout << "No se pudo crear el archivo port" << endl;
        }
    }
}

void sendReponse(int cid){
    string nombreArchivoP = "P"+to_string(idPort);
    char cadena[100];
    
    FILE *archivoP = fopen(nombreArchivoP.c_str(), "r");
    if (archivoP != NULL) {
        fgets(cadena, 100, archivoP);
        fclose(archivoP);  
    } else {
        cout << "No se pudo procesar la solicitud del servidor(r PID)" << endl;
    }

    string nombreArchivo = to_string(cid);

    FILE *archivo = fopen(nombreArchivo.c_str(), "w");
    if (archivo != NULL){
        fseek(archivo, 0, SEEK_SET);
        fputs(cadena, archivo);
        fclose(archivo);
    } else {
        cout << "No se pudo procesar la solicitud del servidor(w cliente)" << endl;
    }

    cout << "Contestación del servidor: " << cadena << endl;
    remove(nombreArchivoP.c_str());
    
    //enviar señal a cliente
    kill(cid, SIGTERM);
    cout << "Mensaje enviado a el cliente" << endl;
}

void clientListener(int idRouter) {
    struct sigaction sa;
    sa.sa_sigaction = clientRequest;
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGTERM, &sa, NULL);
    cout << "Esperando cliente ..." << endl;
    while(true) {}
}