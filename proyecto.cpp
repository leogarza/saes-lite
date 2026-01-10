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
#define ADMINUSER 1234
#define ADMINPASS "password123"
unsigned usuarioActual = 0;

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

enum DiaSemana { LUN, MAR, MIE, JUE, VIE, SAB };

struct BloqueHorario {
    struct BloqueHorario* sig;

    DiaSemana dia;
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
    char *password;
    unsigned int boleta;
    unsigned short int periodo;
    Inscripcion* materiasInscritas;
};

Alumno* alumnoLogeado = NULL;

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
 * @details funcion para insertar un nodo generico (al final)
 * @param[in, out] cabeza el puntero al primer elemento de la lista
 * @param[in] nuevo el puntero al nodo nuevo al cual insertar
 */
void insertarNodo(void** cabeza, void* nuevo) {
    assert(nuevo && "el nodo nuevo es NULL!");
    if(!nuevo) return;
    NodoGenerico** head = (NodoGenerico**)cabeza;
    NodoGenerico* nuevoNodo = (NodoGenerico*)nuevo;

    nuevoNodo->sig = NULL;

    if(*head == NULL) {
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

/*
 * @details funcion para borrar un nodo dado el puntero a su anterior
 * @param[in, out] cabeza el puntero al primer elemento de la lista
 * @param[in] anterior el puntero al nodo previo al que se quiere eliminar (NULL para eliminar la cabeza)
 */
void borrarSiguienteNodo(void** cabeza, void* anterior) {
    NodoGenerico** head = (NodoGenerico**)cabeza;
    NodoGenerico* prev = (NodoGenerico*)anterior;

    /* si 'anterior' es NULL, borramos el primer nodo (cabeza) */
    if(prev == NULL) {
        assert(*head && "la lista esta vacia, no se puede borrar la cabeza!");
        if(*head == NULL) return;

        NodoGenerico* temp = *head;
        /* movemos la cabeza al siguiente nodo */
        *head = temp->sig;

        /* liberamos la memoria del nodo eliminado */
        free(temp);
    }
    /* caso 2: Borrado de un nodo interno o final */
    else {
        /* verificamos que exista algo que borrar despues de 'anterior' */
        assert(prev->sig && "No existe un nodo siguiente para borrar!");
        if(prev->sig == NULL) return;

        NodoGenerico* aBorrar = prev->sig;

        /* saltamos el nodo a borrar: anterior apunta al siguiente del borrado */
        prev->sig = aBorrar->sig;

        /* liberamos la memoria del nodo eliminado :) */
        free(aBorrar);
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
long getint();

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
    long boleta;
    char* password;

    limpiar();
    cout << "===========================================" << endl;
    cout << "   Sistema Administrativo Escolar (LITE)" << endl;
    cout << "===========================================" << endl;
    cout << "| Datos de admin | Boleta: " << ADMINUSER << " --- Clave: " << ADMINPASS << endl;
    cout << "Ingrese boleta: " << endl;
    boleta = getint();
    cout << "Ingrese clave: " << endl;
    password = gettext(true);

    if(boleta == ADMINUSER) {
        if (strcmp(password, ADMINPASS) == 0) {
            cout << "CHEAT CODE: MODO ADMIN ACTIVADO" << endl;
            usuarioActual = ADMINUSER;
        }
    } else {
    Alumno* actual = Escuela.alumnos;
    usuarioActual = 0;
    alumnoLogeado = NULL;
    while(actual != NULL) {
        if(actual->boleta == boleta) {
            if(strcmp(actual->password, password) == 0) {
                usuarioActual = actual->boleta;
                alumnoLogeado = actual;
            }
        }
        actual = actual->sig;
    }
    }

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
    if(!materia) {
        cout << "No se pudo agregar materia" << endl;
        return;
    }
    materia->nombre = nombre;
    materia->codigo = codigo;
    materia->periodo = periodo;
    materia->creditos = creditos;
    materia->uid = uid;
    materia->sig = NULL;
    insertarNodo((void**)&Escuela.materias, materia);
    cout << "Agregada materia!" << endl;
    dormir(800);
}

void gestionMaterias() {
    while(1) {
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
        continue;
    }
    cout << "Materias disponibles: " << endl;
    Materia *act = Escuela.materias;
    while(act != NULL) {
        cout << "\t" << act->codigo << ": " << act->nombre << endl;
        act = act->sig;
    };
    cout << "Opciones: [1] Agregar materia; [2] Borrar materia; [3] Salir" << endl;
    cout << "> ";
    int opcion = getint();
    switch (opcion) {
        case 1:
            agregarMateria();
            break;
        case 2: {
            cout << "Cual materia borrar? (ingrese su codigo):" << endl;
            char* opcionmat = gettext();
            Materia* anterior = NULL;
            Materia* actual = Escuela.materias;
            bool borrado = false;
            while(actual != NULL) {
                    if(strcmp(actual->codigo, opcionmat) == 0) {
                        cout << "Borrando " << opcionmat << "!" << endl;
                        free(actual->codigo);
                        free(actual->nombre);
                        borrarSiguienteNodo((void**)&Escuela.materias, anterior);
                        cout << "Borrado!" << endl;
                        borrado = true;
                        break;
                    }
                anterior = actual;
                actual = actual->sig;
            }
            if(!borrado) {
                cout << "No se encontra tal materia con ese codigo" << endl;
            }
            free(opcionmat);
            break;
        }
        case 3:
            cout << "Saliendo" << endl;
            return;
        default:
            cout << "Opcion invalida!" << endl;
    }
    dormir(800);
    }
}

void agregarProfesor() {
    limpiar();
    cout << "============================" << endl
         << "      AGREGAR PROFESOR      " << endl
         << "============================" << endl;

    cout << "Nombre del profesor:" << endl;
    char* nombre = gettext();

    /* el contador es global btw */
    unsigned uid = contador++;

    Profesor* profesor = (Profesor*)malloc(sizeof(Profesor));
    if(!profesor) {
        cout << "No se pudo agregar profesor" << endl;
        return;
    }
    profesor->nombre = nombre;
    profesor->uid = uid;
    profesor->sig = NULL;

    insertarNodo((void**)&Escuela.profesores, profesor);
    cout << "Agregado profesor!" << endl;
    dormir(800);
}

void gestionProfesores() {
    while(1) {
    limpiar();
    cout << "====================================" << endl;
    cout << "        GESTOR DE PROFESORES        " << endl;
    cout << "====================================" << endl;

    if(Escuela.profesores == NULL) {
        /* se tienen que agregar profesores */
        cout << "No hay profesores." << endl;
        cout << "Se tienen que agregar profesores" << endl;
        dormir(1000);
        agregarProfesor();
        continue;
    }

    cout << "Profesores disponibles: " << endl;
    Profesor *act = Escuela.profesores;
    while(act != NULL) {
        /* se usa UID ya que un profesor no tiene codigo xd */
        cout << "\tID " << act->uid << ": " << act->nombre << endl;
        act = act->sig;
    };

    cout << "Opciones: [1] Agregar profesor; [2] Borrar profesor; [3] Salir" << endl;
    cout << "> ";
    int opcion = getint();
    switch (opcion) {
        case 1:
            agregarProfesor();
            break;
        case 2: {
            cout << "Cual profesor borrar? (ingrese su ID):" << endl;
            int opcionId = getint();
            Profesor* anterior = NULL;
            Profesor* actual = Escuela.profesores;
            bool borrado = false;
            while(actual != NULL) {
                    if(actual->uid == opcionId) {
                        cout << "Borrando profesor ID " << opcionId << "!" << endl;
                        free(actual->nombre);
                        borrarSiguienteNodo((void**)&Escuela.profesores, anterior);
                        cout << "Borrado!" << endl;
                        borrado = true;
                        break;
                    }
                anterior = actual;
                actual = actual->sig;
            }
            if(!borrado) {
                cout << "No se encontro tal profesor con ese ID" << endl;
            }
            break;
        }
        case 3:
            cout << "Saliendo" << endl;
            return;
        default:
            cout << "Opcion invalida!" << endl;
    }
    dormir(800);
    }
}

void agregarGrupo() {
    limpiar();
    cout << "============================" << endl
         << "       AGREGAR GRUPO        " << endl
         << "============================" << endl;

    if (Escuela.materias == NULL || Escuela.profesores == NULL) {
        cout << "Error: Se requieren Materias y Profesores registrados." << endl;
        dormir(1000);
        return;
    }

    /* seleccionar Materia */
    cout << "Ingrese el codigo de la Materia para el grupo:" << endl;
    char* codigoMat = gettext();
    Materia* matActual = Escuela.materias;
    unsigned int uidMateria = 0;
    bool encontrada = false;

    while(matActual != NULL) {
        if(strcmp(matActual->codigo, codigoMat) == 0) {
            uidMateria = matActual->uid;
            encontrada = true;
            cout << "Materia seleccionada: " << matActual->nombre << endl;
            break;
        }
        matActual = matActual->sig;
    }
    free(codigoMat);

    if(!encontrada) {
        cout << "Materia no encontrada." << endl;
        dormir(800);
        return;
    }

    /* seleccionar profesor */
    cout << "Ingrese el ID del Profesor:" << endl;
    int idProf = getint();
    Profesor* profActual = Escuela.profesores;
    unsigned int uidProfesor = 0;
    encontrada = false;

    while(profActual != NULL) {
        if(profActual->uid == (unsigned)idProf) {
            uidProfesor = profActual->uid;
            encontrada = true;
            cout << "Profesor seleccionado: " << profActual->nombre << endl;
            break;
        }
        profActual = profActual->sig;
    }

    if(!encontrada) {
        cout << "Profesor no encontrado." << endl;
        dormir(800);
        return;
    }

    /* datos generales del grupo */
    cout << "Clave del Grupo (ej. 2CV1):" << endl;
    char* clave = gettext();
    cout << "Cupo Maximo:" << endl;
    int cupo = getint();

    BloqueHorario* listaHorario = NULL;

    while(1) {
        cout << "--------------------------------" << endl;
        cout << "Agregar bloque de horario? [1] Si; [0] No" << endl;
        cout << "> ";
        int op = getint();
        if(op != 1) break;

        BloqueHorario* bloque = (BloqueHorario*)malloc(sizeof(BloqueHorario));
        if(!bloque) continue;

        cout << "Dia (0=LUN, 1=MAR, 2=MIE, 3=JUE, 4=VIE, 5=SAB): ";
        int d = getint();
        bloque->dia = (DiaSemana)d;
        cout << "Hora inicio (0-23): ";
        bloque->hora = (unsigned char)getint();
        cout << "Minuto inicio (0-59): ";
        bloque->minuto = (unsigned char)getint();
        cout << "Edificio (numero): ";
        bloque->edificio = (unsigned char)getint();
        cout << "Salon (numero): ";
        bloque->salon = (unsigned short)getint();
        cout << "Duracion (minutos): ";
        bloque->duracionMin = (unsigned char)getint();

        bloque->sig = NULL;

        insertarNodo((void**)&listaHorario, bloque);
    }

    unsigned uid = contador++;
    Grupo* grupo = (Grupo*)malloc(sizeof(Grupo));
    if(!grupo) {
        cout << "Error al crear grupo" << endl;
        return;
    }

    grupo->uid = uid;
    grupo->uidMateria = uidMateria;
    grupo->uidProfesor = uidProfesor;
    grupo->clave = clave;
    grupo->cupoMax = cupo;
    grupo->inscritos = 0;
    grupo->horario = listaHorario;
    grupo->sig = NULL;

    insertarNodo((void**)&Escuela.grupos, grupo);
    cout << "Agregado grupo " << clave << "!" << endl;
    dormir(800);
}

void gestionGrupos() {
    while(1) {
    limpiar();
    cout << "====================================" << endl;
    cout << "          GESTOR DE GRUPOS          " << endl;
    cout << "====================================" << endl;

    if(Escuela.grupos == NULL) {
        cout << "No hay grupos registrados." << endl;
        cout << "Opciones: [1] Agregar grupo; [3] Salir" << endl;
    } else {
        cout << "Grupos disponibles: " << endl;
        Grupo *act = Escuela.grupos;
        while(act != NULL) {
            /* se muestra el UID del grupo y su clave */
            cout << "\t[ID " << act->uid << "] " << act->clave
                 << " (Mat ID: " << act->uidMateria
                 << ", Prof ID: " << act->uidProfesor << ")" << endl;

            /* opcional: mostrar un resumen del horario */
            BloqueHorario* h = act->horario;
            while(h != NULL) {
                cout << "\t   -> Dia: " << h->dia << " Hora: "
                     << (int)h->hora << ":" << (int)h->minuto << endl;
                h = h->sig;
            }
            act = act->sig;
        };
        cout << "Opciones: [1] Agregar grupo; [2] Borrar grupo; [3] Salir" << endl;
    }

    cout << "> ";
    int opcion = getint();

    switch (opcion) {
        case 1:
            agregarGrupo();
            break;
        case 2: {
            if(Escuela.grupos == NULL) break;
            cout << "Cual grupo borrar? (ingrese su UID de grupo):" << endl;
            int opcionUid = getint();
            Grupo* anterior = NULL;
            Grupo* actual = Escuela.grupos;
            bool borrado = false;
            while(actual != NULL) {
                    if(actual->uid == (unsigned)opcionUid) {
                        cout << "Borrando grupo " << actual->clave << "..." << endl;
                        free(actual->clave);

                        /* liberar memoria de la lista de horarios */
                        BloqueHorario* hAct = actual->horario;
                        BloqueHorario* hSig;
                        while(hAct != NULL) {
                            hSig = hAct->sig;
                            free(hAct);
                            hAct = hSig;
                        }

                        borrarSiguienteNodo((void**)&Escuela.grupos, anterior);
                        cout << "Borrado!" << endl;
                        borrado = true;
                        break;
                    }
                anterior = actual;
                actual = actual->sig;
            }
            if(!borrado) {
                cout << "No se encontro grupo con ese UID" << endl;
            }
            break;
        }
        case 3:
            cout << "Saliendo" << endl;
            return;
        default:
            cout << "Opcion invalida!" << endl;
    }
    dormir(800);
    }
}

void agregarAlumno() {
    limpiar();
    cout << "============================" << endl
         << "       AGREGAR ALUMNO       " << endl
         << "============================" << endl;

    cout << "Nombre del alumno:" << endl;
    char* nombre = gettext();
    cout << "Password:" << endl;
    char* password = gettext();
    cout << "Numero de Boleta:" << endl;
    unsigned int boleta = getint();
    cout << "Periodo (Semestre): " << endl;
    int periodo = getint();

    Alumno* alumno = (Alumno*)malloc(sizeof(Alumno));
    if(!alumno) {
        cout << "No se pudo agregar alumno" << endl;
        return;
    }
    alumno->nombre = nombre;
    alumno->password = password;
    alumno->boleta = boleta;
    alumno->periodo = periodo;
    alumno->materiasInscritas = NULL; /* inicia sin materias */
    alumno->sig = NULL;

    insertarNodo((void**)&Escuela.alumnos, alumno);
    cout << "Agregado alumno!" << endl;
    dormir(800);
}

void gestionAlumnos() {
    while(1) {
    limpiar();
    cout << "====================================" << endl;
    cout << "         GESTOR DE ALUMNOS          " << endl;
    cout << "====================================" << endl;

    if(Escuela.alumnos == NULL) {
        /* se tienen que agregar alumnos */
        cout << "No hay alumnos." << endl;
        cout << "Se tienen que agregar alumnos" << endl;
        dormir(1000);
        agregarAlumno();
        continue;
    }

    cout << "Alumnos registrados: " << endl;
    Alumno *act = Escuela.alumnos;
    while(act != NULL) {
        cout << "\tBoleta " << act->boleta << ": " << act->nombre << endl;
        act = act->sig;
    };

    cout << "Opciones: [1] Agregar alumno; [2] Borrar alumno; [3] Salir" << endl;
    cout << "> ";
    int opcion = getint();
    switch (opcion) {
        case 1:
            agregarAlumno();
            break;
        case 2: {
            cout << "Cual alumno borrar? (ingrese su boleta):" << endl;
            unsigned int opcionBoleta = getint();
            Alumno* anterior = NULL;
            Alumno* actual = Escuela.alumnos;
            bool borrado = false;
            while(actual != NULL) {
                    if(actual->boleta == opcionBoleta) {
                        cout << "Borrando alumno con boleta " << opcionBoleta << "!" << endl;
                        free(actual->nombre);
                        free(actual->password);
                        /* TODO: aqui tambien deberiamos liberar la memoria de
                         * materiasInscritas si tuviera, pero se deja asi
                         * por que todavia no lo he hecho */
                        borrarSiguienteNodo((void**)&Escuela.alumnos, anterior);
                        cout << "Borrado!" << endl;
                        borrado = true;
                        break;
                    }
                anterior = actual;
                actual = actual->sig;
            }
            if(!borrado) {
                cout << "No se encontro tal alumno con esa boleta" << endl;
            }
            break;
        }
        case 3:
            cout << "Saliendo" << endl;
            return;
        default:
            cout << "Opcion invalida!" << endl;
    }
    dormir(800);
    }
}

void adminMenu() {
    while(1) {
        limpiar();
        cout << "Bienvenido " << ADMINUSER << "!" << endl;
        cout << "[1] Gestion de Materias" << endl;
        cout << "[2] Gestion de Profesores" << endl;
        cout << "[3] Gestion de Alumnos" << endl;
        cout << "[4] Gestion de Grupos" << endl;
        cout << "[5] Inscripciones" << endl;
        cout << "[6] Reportes Generales" << endl;
        cout << "[7] Salir" << endl;

        cout << "Introduce el numero de la opcion: " << endl;
        cout << "> ";
        int opcion = getint();

        switch(opcion) {
            case 1: /* gestion de materias */
                gestionMaterias();
                break;
            case 2: /* gestion de profesores */
                gestionProfesores();
                break;
            case 3:
                gestionAlumnos();
                break;
            case 4:
                gestionGrupos();
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
    if(usuarioActual == ADMINUSER) {
        adminMenu();
        continue;
    } else if(usuarioActual == 0) {
        cout << "Boleta o password incorrecto" << endl;
        dormir(800);
        continue;
    }

    limpiar();
    /* menu del usuario */
    if(alumnoLogeado) {
        cout << "Bienvenido " << alumnoLogeado->nombre << "!" << endl;
        pausar();
    }
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

long getint() {
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
