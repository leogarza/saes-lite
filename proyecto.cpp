#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
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
#define limpiar() system("clear")
#endif

using namespace std;

/* cabe recalcar que esto no es de lo mas
 * seguro, verdad? XD
 * era curso de algoritmos y estructuras
 * no de ciberseguridad ;) */
#define ADMINUSER "admin"
#define ADMINPASS "password123"
static bool adminMode = false;

/* tambien deberia haber un generador
 * de identificadores unicos pero
 * ya no hubo presupuesto */
 unsigned contador = 0;

struct Materia {
    struct Materia* sig;

    unsigned int uid;
    char *nombre;
    char *codigo;
    unsigned short int periodo;
    float creditos;
};

struct BloqueHorario {
    struct BloqueHorario* sig;

    enum { LUN, MAR, MIE, JUE, VIE, SAB } dia;
    unsigned char hora; /* hora de inicio */
    unsigned char minuto; /* minuto de inicio */
    unsigned char edificio; /* numero de edificio */
    unsigned short int salon; /* numero de salon */
    unsigned char duracionMin; /* duracion de la clase en min */
};

struct Profesor {
    struct Profesor* sig;

    unsigned int uid;
    char *nombre;
};

enum CALIFICACION {
    PRIMERPAR = 0, /* primer parcial */
    SEGUNDOPAR, /* segundo parcial */
    TERCERPAR, /* tercer parcial */
    FINAL, /* calificación final */
    EXTRA, /* extraordinario */
    CALFIFACIONTAM
};

struct Inscripcion {
    struct Inscripcion* sig;

    unsigned int idGrupo;
    float calificaciones[CALFIFACIONTAM];
};

struct Alumno {
    struct Alumno* sig;

    char *nombre;
    unsigned int boleta;
    unsigned short int periodo;
    Inscripcion* materiasInscritas;
};

struct Grupo {
    struct Grupo* sig;

    unsigned int uid;
    unsigned int uidMateria;
    unsigned int uidProfesor;
    char *clave; /* ej 2CV1 */
    struct BloqueHorario* horario;
    unsigned short int cupoMax;
    unsigned short int inscritos;
};

struct {
    struct Alumno* alumnos = NULL;
    struct Profesor* profesores = NULL;
    struct Materia* materias = NULL;
    struct Grupo* grupos = NULL;
} Escuela;

/* estructura de utilidad:
 * como todas las estructuras tienen
 * de primer miembro el puntero al
 * siguiente nodo se pueden castear a esta
 * estructura */
struct NodoGenerico {
    struct NodoGenerico* sig;
};

/* -- funciones de manejo de listas -- */

/*
 * @details funcion para insertar un nodo generico
 * @param[in, out] cabeza el puntero al primer elemento de la lista
 * @param[in] nuevo el puntero al nodo nuevo al cual insertar
 */
void insertarNodo(void** cabeza, void* nuevo) {
    assert(nuevo && "el nodo nuevo es NULL!");
    if(!nuevo) return;
    NodoGenerico** head = (NodoGenerico**)cabeza;
    NodoGenerico* nuevoNodo = (NodoGenerico*)nuevo;

    nuevoNodo->sig = NULL;

    if(head == NULL) {
        /* la lista esta vacia */
        *head = nuevoNodo;
    } else {
        NodoGenerico* actual = *head;
        /* nos vamos hasta el final */
        while(actual->sig != NULL) {
            /* recorriendo uno a uno */
            actual = actual->sig;
        }
        /* al final insertamos nuestro nuevo nodo :) */
        actual->sig = nuevoNodo;
    }
}

/* -- funciones utilidades -- */

/* utilidad para controlar el flujo de caracteres
 * manualmente. (comportandose como getch de windows) */
char gchar();

/*
 * @details mi funcion para agarrar texto de la consola
 * @param[in] password habilita el modo clave (pone asteriscos)
 * @return un puntero al texto recibido (se le tiene que hacer
 * free despues). o null si no se ingreso nada. */
char* gettext(bool password = 0);

/* @details Espera al que el usuario ingrese un entero
 * @return el entero del usuario o 0 si no fue valido */
int getint();

/* @details espera al que el usuario ingrese un flotante
 * @return el flotante ingresado por el usuario
 */
float getfloat();

/*
 * @details hace que el usuario tenga que presionar enter
 * para continuar.
 */
void pausar();

/**
 * @details Esta es la funcion que hace el login,
 * de momento es un simple login sin hashing ni nada.
 * Esta mas de adorno que otra cosa.
 * @return
 */
bool login() {
    char* user;
    char* password;

    limpiar();
    cout << "===========================================" << endl;
    cout << "   Sistema Administrativo Escolar (LITE)" << endl;
    cout << "===========================================" << endl;
    cout << "| Datos de admin | Usuario: " << ADMINUSER << " --- Clave: " << ADMINPASS << endl;
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

void agregarMateria() {
    limpiar();
    cout << "============================" << endl
         << "      AGREGAR MATERIA       " << endl
         << "============================" << endl;

    cout << "Nombre de la materia:" << endl;
    char* nombre = gettext();
    cout << "Codigo de la materia: " << endl;
    char* codigo = gettext();
    cout << "Periodo (Semestre): " << endl;
    int periodo = getint();
    cout << "Creditos: " << endl;
    float creditos = getfloat();
    unsigned uid = contador++;
    Materia* materia  = (Materia*)malloc(sizeof(Materia));
    materia->nombre = nombre;
    materia->codigo = codigo;
    materia->periodo = periodo;
    materia->creditos = creditos;
    insertarNodo((void**)&Escuela.materias, materia);
    cout << "Agregada materia!" << endl;
    dormir(800);
}

void gestionMaterias() {
    limpiar();
    cout << "====================================" << endl;
    cout << "        GESTOR DE MATERIAS          " << endl;
    cout << "====================================" << endl;
    if(Escuela.materias == NULL) {
        /* se tienen que agregar materias */
        cout << "No hay materias." << endl;
        cout << "Se tienen que agregar materias" << endl;
        dormir(1000);
        agregarMateria();
    }

}

void adminMenu() {
    while(1) {
        limpiar();
        cout << "Bienvenido " << ADMINUSER << "!" << endl;
        cout << "[1] Gestion de Materias" << endl;
        cout << "[2] Gestion de Grupos" << endl;
        cout << "[3] Gestion de Alumnos" << endl;
        cout << "[4] Gestion de Profesores" << endl;
        cout << "[5] Inscripciones" << endl;
        cout << "[6] Reportes Generales" << endl;
        cout << "[7] Salir" << endl;

        cout << "Introduce el numero de la opcion: " << endl;
        cout << "> ";
        int opcion = getint();

        switch(opcion) {
            case 1:
                cout << "Opcion 1" << endl;
                break;
            case 2:
                cout << "Opcion 2" << endl;
                break;
            case 3:
                cout << "Opcion 3" << endl;
                break;
            case 4:
                cout << "Opcion 4" << endl;
                break;
            case 5:
                cout << "Opcion 5" << endl;
                break;
            case 6:
                cout << "Opcion 6" << endl;
                break;
            case 7:
                cout << "Saliendo!" << endl;
                dormir(800);
                return;
            default:
                cout << "Opcion invalida!" << endl;
        }
        dormir(800);
    }
}

int main() {
    while(1) {
    login();
    if(adminMode) {
        adminMenu();
        continue;
    }

    /* menu del usuario */
    /* ... */
    }
    return 0;
}

char gchar() {
    #ifdef WIN32
    return _getch();
    #else
    /* para los que usan linux o mac o lo que sea */
    struct termios attr, old;
    tcgetattr(0, &old);
    attr = old;
    attr.c_lflag &= ~ECHO;
    attr.c_lflag &= ~ICANON;
    tcsetattr(0, TCSANOW, &attr);
    char c = getchar();
    tcsetattr(0, TCSANOW, &old);
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

int getint() {
    char buffer[64];
    fgets(buffer, 64, stdin);
    return strtol(buffer, NULL, 10);
}

float getfloat() {
    char buffer[64];
    fgets(buffer, 64, stdin);
    return strtof(buffer, NULL);
}

void pausar() {
    cout << "Presiona enter para continuar...";
    fflush(stdout);
    while(gchar() != '\n') {
        dormir(10);
    }
}
