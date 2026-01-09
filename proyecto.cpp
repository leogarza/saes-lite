#include <cstdio>
#include <cstring>
#include <iostream>

/* para cositas de la terminal */
#ifdef WIN32
#include <windows.h>
#include <conio.h>
#define dormir(ms) Sleep(ms)
#define limpiar() system("cls")
#else /* UNIX */
#include <termios.h>
#include <unistd.h>
#define dormir(ms) usleep(ms*1000)
#define limpiar system("clear")
#endif

using namespace std;

/* cabe recalcar que esto no es de lo mas
 * seguro, verdad? XD
 * era curso de algoritmos y estructuras
 * no de ciberseguridad ;) */
#define ADMINUSER "admin"
#define ADMINPASS "password123"
static bool adminMode = false;

struct Materia {
    unsigned int uid;
    char nombre[24];
    char codigo[6];
    unsigned short int periodo;
    float creditos;
    struct Materia* sig;
};

struct BloqueHorario {
    enum { LUN, MAR, MIE, JUE, VIE, SAB } dia;
    unsigned short int hora;
    unsigned short int minuto;
    unsigned short int salon;
    struct BloqueHorario* sig;
};

struct HorarioMateria {
    unsigned int materiaUid;
    struct BloqueHorario* bloques; /* lista */
};

struct Profesor {
    char nombre[64];
    struct Profesor* sig;
};

struct Alumno {
    char nombre[64];
    unsigned int boleta;
    unsigned short int periodo;

    struct Alumno* sig;
};

struct Escuela {
    struct Alumno* alumnos = NULL;
    struct Profesor* profesores = NULL;
    struct Materia* materias = NULL;
};

/* utilidad para controlar el flujo de caracteres
 * manualmente. (comportandose como getch de windows) */
char gchar();

/*
 * @details mi funcion para agarrar texto de la consola
 * @param[in] password habilita el modo clave (pone asteriscos)
 * @return un puntero al texto recibido (se le tiene que hacer
 * free despues). o null si no se ingreso nada. */
char* gettext(bool password = 0);

/**
 * @details Esta es la funcion que hace el login,
 * de momento es un simple login sin hashing ni nada.
 * Esta mas de adorno que otra cosa.
 * @return
 */
bool login() {
    char* user;
    char* password;

    cout << "===========================================" << endl;
    cout << "   Sistema Administrativo Escolar (LITE)" << endl;
    cout << "===========================================" << endl;
    cout << "| Datos de admin | Usuario: " << ADMINUSER << " Clave: " << ADMINPASS << endl;
    cout << "Ingrese usuario: " << endl;
    user = gettext();
    cout << "Ingrese clave: " << endl;
    password = gettext(true);

    if(strcmp(user, ADMINUSER) == 0) {
        if (strcmp(password, ADMINPASS) == 0) {
            cout << "CHEAT CODE: MODO ADMIN ACTIVADO" << endl;
            adminMode = true;
        }
    }

    free(user);
    free(password);

    dormir(1000);

    return true;
}

void adminMenu() {
    cout << "Bienvenido " << ADMINUSER << "!" << endl;
    cout << "[1] Gestion de Materias" << endl;
    cout << "[2] Gestion de Grupos" << endl;
    cout << "[3] Gestion de Alumnos" << endl;
    cout << "[4] Gestion de Profesores" << endl;
    cout << "[5] Inscripciones" << endl;
    cout << "[6] Reportes Generales" << endl;
    cout << "[7] Salir" << endl;
}

int main() {
    login();

    return 0;
}

char gchar() {
    #ifdef WIN32
    return _getch();
    #else
    /* para los que usan linux o mac o lo que sea */
    struct termios attr;
    tcgetattr(0, &attr);
    attr.c_lflag &= ~ECHO;
    attr.c_lflag &= ~ICANON;
    tcsetattr(0, TCSANOW, &attr);
    char c = getchar();
    attr.c_lflag |= ECHO;
    attr.c_lflag |= ICANON;
    tcsetattr(0, TCSANOW, &attr);
    return c;
    #endif
}

char* gettext(bool password) {
    char buffer[256];
    memset(buffer, 0 , 256);

    char c = 0;
    unsigned int caracteresIngresados = 0;
    while(1) {
        c = gchar();
        if(c == '\n' || c == '\r') break;
        /* alcazamos nuestro limite de caracteres! */
        /* (restando el caracater nulo y el indice 0) */
        if(caracteresIngresados == 254) break;
        /* al poner nuestros asteriscos estos se van a stdin
         * en algunos sistemas, asi que mejor los ignoramos
         * (solo aplica cuando se esta en modo password) */
        if(c == '*' && password) continue;

        /* tecla retroceso o borrar
         * o como se llame */
        if(c == '\b' || c == 127) {
            if(caracteresIngresados > 0) {
                fputs("\b \b", stdout);
                caracteresIngresados--;
                continue;
            }
        } else if(c >= 32 && c <= 126) {
            /* se agrega simplemente el caracter */
            buffer[caracteresIngresados] = c;
            caracteresIngresados++;
        } else {
            continue;
        }
        /* reemplaza los caracteres
         * con asteriscos */
        putc('\r',stdout);
        for(unsigned int i = 0; i < caracteresIngresados; i++) {
            if(password)
                putc('*', stdout);
            else
                putc(buffer[i], stdout);
        }

        fflush(stdout);
    }
    puts("\r\n");
    if(caracteresIngresados == 0) return NULL;
    buffer[caracteresIngresados] = '\0';
    char* nuevostr = (char*)malloc(caracteresIngresados+1);
    if(!nuevostr) return NULL;
    memcpy(nuevostr, buffer, caracteresIngresados+1);

    return nuevostr;
}
