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
 * para continuar. (muestra el mensaje con esa instruccion)
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

void inscribirMateria(Alumno* alumno) {
    limpiar();
    cout << "============================" << endl
         << "     INSCRIBIR MATERIA      " << endl
         << "============================" << endl;

    if (Escuela.grupos == NULL) {
        cout << "No hay grupos disponibles para inscribir." << endl;
        dormir(1000);
        return;
    }

    cout << "Grupos disponibles:" << endl;
    Grupo* gAct = Escuela.grupos;
    while(gAct != NULL) {
        /* calculamos cupo disponible */
        int disponibles = gAct->cupoMax - gAct->inscritos;
        if(disponibles > 0) {
            /* Hay que buscar el nombre de la materia para que se vea bonito */
            char* nomMat = (char*)"Desconocida";
            Materia* m = Escuela.materias;
            while(m != NULL) {
                if(m->uid == gAct->uidMateria) {
                    nomMat = m->nombre;
                    break;
                }
                m = m->sig;
            }
            cout << "\t[ID " << gAct->uid << "] " << gAct->clave
                 << " - " << nomMat
                 << " (Cupo: " << disponibles << ")" << endl;
        }
        gAct = gAct->sig;
    }

    cout << "Ingrese el ID del Grupo a inscribir:" << endl;
    int idGrupoSel = getint();

    /* verificamos que el grupo exista y tenga cupo */
    Grupo* grupoSeleccionado = NULL;
    gAct = Escuela.grupos;
    while(gAct != NULL) {
        if(gAct->uid == (unsigned)idGrupoSel) {
            grupoSeleccionado = gAct;
            break;
        }
        gAct = gAct->sig;
    }

    if(!grupoSeleccionado) {
        cout << "Grupo no encontrado." << endl;
        dormir(800);
        return;
    }

    if(grupoSeleccionado->inscritos >= grupoSeleccionado->cupoMax) {
        cout << "El grupo esta lleno!" << endl;
        dormir(800);
        return;
    }

    /* validamos que no este ya inscrito en ese grupo */
    Inscripcion* iAct = alumno->materiasInscritas;
    while(iAct != NULL) {
        if(iAct->idGrupo == grupoSeleccionado->uid) {
            cout << "El alumno ya esta inscrito en este grupo." << endl;
            dormir(800);
            return;
        }
        iAct = iAct->sig;
    }

    /* crear inscripcion */
    Inscripcion* nuevaInsc = (Inscripcion*)malloc(sizeof(Inscripcion));
    if(!nuevaInsc) {
        cout << "Error de memoria al inscribir" << endl;
        return;
    }

    nuevaInsc->idGrupo = grupoSeleccionado->uid;
    /* inicializar calificaciones en 0 */
    for(int k=0; k < CALFIFACIONTAM; k++) {
        nuevaInsc->calificaciones[k] = 0.0f;
    }
    nuevaInsc->sig = NULL;

    insertarNodo((void**)&alumno->materiasInscritas, nuevaInsc);

    /* actualizar cupo del grupo */
    grupoSeleccionado->inscritos++;

    cout << "Inscripcion exitosa!" << endl;
    dormir(800);
}

void gestionInscripciones() {
    limpiar();
    cout << "====================================" << endl;
    cout << "      GESTION DE INSCRIPCIONES      " << endl;
    cout << "====================================" << endl;

    if(Escuela.alumnos == NULL) {
        cout << "No hay alumnos registrados." << endl;
        dormir(1000);
        return;
    }

    cout << "Ingrese la boleta del alumno:" << endl;
    unsigned int boleta = getint();

    Alumno* alumno = NULL;
    Alumno* act = Escuela.alumnos;
    while(act != NULL) {
        if(act->boleta == boleta) {
            alumno = act;
            break;
        }
        act = act->sig;
    }

    if(!alumno) {
        cout << "Alumno no encontrado." << endl;
        dormir(800);
        return;
    }

    /* submenu del alumno */
    while(1) {
        limpiar();
        cout << "Alumno: " << alumno->nombre << " (Boleta: " << alumno->boleta << ")" << endl;
        cout << "------------------------------------" << endl;
        cout << "Materias Inscritas:" << endl;

        Inscripcion* ins = alumno->materiasInscritas;
        if(ins == NULL) {
            cout << "\t(Sin inscripciones)" << endl;
        } else {
            while(ins != NULL) {
                /* buscar datos del grupo para mostrar algo util */
                Grupo* g = Escuela.grupos;
                char* claveG = (char*)"???";
                while(g != NULL) {
                    if(g->uid == ins->idGrupo) {
                        claveG = g->clave;
                        break;
                    }
                    g = g->sig;
                }
                cout << "\tGrupo ID " << ins->idGrupo << " [" << claveG << "]" << endl;
                ins = ins->sig;
            }
        }
        cout << "------------------------------------" << endl;
        cout << "Opciones: [1] Inscribir materia; [2] Dar de baja; [3] Regresar" << endl;
        cout << "> ";
        int opcion = getint();

        switch(opcion) {
            case 1:
                inscribirMateria(alumno);
                break;
            case 2: {
                if(alumno->materiasInscritas == NULL) {
                    cout << "No tiene materias para dar de baja." << endl;
                    break;
                }
                cout << "Ingrese ID del Grupo a dar de baja:" << endl;
                int idBaja = getint();

                Inscripcion* prev = NULL;
                Inscripcion* curr = alumno->materiasInscritas;
                bool borrado = false;

                while(curr != NULL) {
                    if(curr->idGrupo == (unsigned)idBaja) {
                        /* tambien hay que bajar el contador del grupo */
                        Grupo* g = Escuela.grupos;
                        while(g != NULL) {
                            if(g->uid == curr->idGrupo) {
                                if(g->inscritos > 0) g->inscritos--;
                                break;
                            }
                            g = g->sig;
                        }

                        borrarSiguienteNodo((void**)&alumno->materiasInscritas, prev);
                        cout << "Materia dada de baja." << endl;
                        borrado = true;
                        break;
                    }
                    prev = curr;
                    curr = curr->sig;
                }
                if(!borrado) cout << "No se encontro inscripcion con ese ID de grupo." << endl;
                dormir(800);
                break;
            }
            case 3:
                return;
            default:
                cout << "Opcion invalida!" << endl;
                dormir(800);
        }
    }
}

void reporteAlumnosDetallado() {
    limpiar();
    cout << "------------------------------------" << endl;
    cout << "      REPORTE DETALLADO ALUMNOS     " << endl;
    cout << "------------------------------------" << endl;

    if(Escuela.alumnos == NULL) {
        cout << "No hay alumnos registrados." << endl;
        return;
    }

    Alumno* aAct = Escuela.alumnos;
    while(aAct != NULL) {
        cout << "Boleta: " << aAct->boleta << " | Nombre: " << aAct->nombre
             << " | Semestre: " << aAct->periodo << endl;

        /* listar materias inscritas */
        Inscripcion* iAct = aAct->materiasInscritas;
        if(iAct == NULL) {
            cout << "\t(Sin materias inscritas)" << endl;
        } else {
            while(iAct != NULL) {
                /* necesitamos buscar info del grupo para saber la materia */
                Grupo* g = Escuela.grupos;
                char* nombreMat = (char*)"Desc.";
                char* claveG = (char*)"---";

                while(g != NULL) {
                    if(g->uid == iAct->idGrupo) {
                        claveG = g->clave;
                        /* y ahora buscamos la materia dentro del grupo xd */
                        Materia* m = Escuela.materias;
                        while(m != NULL) {
                            if(m->uid == g->uidMateria) {
                                nombreMat = m->nombre;
                                break;
                            }
                            m = m->sig;
                        }
                        break;
                    }
                    g = g->sig;
                }

                cout << "\t-> Grupo " << claveG << ": " << nombreMat << endl;

                /* calculamos promedio simple */
                float suma = 0;
                int c = 0;
                for(int k=0; k<FINAL; k++) { /* promedio de parciales */
                     if(iAct->calificaciones[k] > 0) {
                        suma += iAct->calificaciones[k];
                        c++;
                     }
                }
                if(c > 0)
                    cout << "\t   Promedio parciales: " << (suma/c) << endl;

                iAct = iAct->sig;
            }
        }
        cout << "------------------------------------" << endl;
        aAct = aAct->sig;
    }
}

void reporteGruposDetallado() {
    limpiar();
    cout << "------------------------------------" << endl;
    cout << "       REPORTE DETALLADO GRUPOS     " << endl;
    cout << "------------------------------------" << endl;

    if(Escuela.grupos == NULL) {
        cout << "No hay grupos creados." << endl;
        return;
    }

    Grupo* gAct = Escuela.grupos;
    while(gAct != NULL) {
        /* Buscar nombre materia */
        char* matName = (char*)"Desconocida";
        Materia* m = Escuela.materias;
        while(m != NULL) {
            if(m->uid == gAct->uidMateria) {
                matName = m->nombre;
                break;
            }
            m = m->sig;
        }

        /* Buscar nombre profesor */
        char* profName = (char*)"Sin Asignar";
        Profesor* p = Escuela.profesores;
        while(p != NULL) {
            if(p->uid == gAct->uidProfesor) {
                profName = p->nombre;
                break;
            }
            p = p->sig;
        }

        cout << "Grupo: " << gAct->clave << " [ID " << gAct->uid << "]" << endl;
        cout << "Materia: " << matName << endl;
        cout << "Profesor: " << profName << endl;
        cout << "Cupo: " << gAct->inscritos << "/" << gAct->cupoMax << endl;

        cout << "Horario:" << endl;
        BloqueHorario* h = gAct->horario;
        const char* dias[] = {"LUN","MAR","MIE","JUE","VIE","SAB"};
        while(h != NULL) {
            cout << "\t" << dias[h->dia] << " "
                 << (int)h->hora << ":" << (int)(h->minuto < 10 ? 0 : h->minuto) /* hack para el 0 */
                 << (int)h->minuto
                 << " (Salon " << h->salon << ")" << endl;
            h = h->sig;
        }
        cout << "------------------------------------" << endl;

        gAct = gAct->sig;
    }
}

void mostrarReportes() {
    while(1) {
        limpiar();
        cout << "============================" << endl
             << "     REPORTES GENERALES     " << endl
             << "============================" << endl;
        cout << "[1] Listado de Alumnos y Materias" << endl;
        cout << "[2] Listado de Grupos y Horarios" << endl;
        cout << "[3] Regresar al menu principal" << endl;

        cout << "> ";
        int opcion = getint();

        switch(opcion) {
            case 1:
                reporteAlumnosDetallado();
                pausar();
                break;
            case 2:
                reporteGruposDetallado();
                pausar();
                break;
            case 3:
                return;
            default:
                cout << "Opcion invalida" << endl;
                dormir(500);
        }
    }
}

/* -- SECCION DE MANTENIMIENTO Y ALGORITMOS -- */

/*
 * A continuacion implementamos MergeSort.
 * Elegi este porque para listas ligadas es el rey (O(n log n)).
 * El QuickSort se pone roñoso con listas simples y el Burbuja
 * es para cuando tienes 3 datos locos, no para un sistema "serio" ;)
 */

/* helper para el merge sort: divide la lista en dos mitades
 * usando el truco de la tortuga y la liebre */
void partirListaAlumnos(Alumno* fuente, Alumno** frente, Alumno** atras) {
    Alumno* rapido;
    Alumno* lento;

    if (fuente == NULL || fuente->sig == NULL) {
        *frente = fuente;
        *atras = NULL;
    } else {
        lento = fuente;
        rapido = fuente->sig;

        /* avanzamos 'rapido' dos veces y 'lento' una vez */
        while (rapido != NULL) {
            rapido = rapido->sig;
            if (rapido != NULL) {
                lento = lento->sig;
                rapido = rapido->sig;
            }
        }

        /* 'lento' esta antes del punto medio en la lista, asi que cortamos ahi */
        *frente = fuente;
        *atras = lento->sig;
        lento->sig = NULL; /* cortamos el enlace */
    }
}

/* helper para fusionar dos listas ordenadas de alumnos */
Alumno* fusionarAlumnos(Alumno* a, Alumno* b) {
    Alumno* resultado = NULL;

    /* casos base de la recursion */
    if (a == NULL) return b;
    if (b == NULL) return a;

    /* aqui comparamos por BOLETA para que quede ordenado
     * de menor a mayor */
    if (a->boleta <= b->boleta) {
        resultado = a;
        resultado->sig = fusionarAlumnos(a->sig, b);
    } else {
        resultado = b;
        resultado->sig = fusionarAlumnos(a, b->sig);
    }
    return resultado;
}

/* funcion principal del ordenamiento recursivo */
void mergeSortAlumnos(Alumno** cabezaRef) {
    Alumno* cabeza = *cabezaRef;
    Alumno* a = NULL;
    Alumno* b = NULL;

    /* si la lista esta vacia o solo tiene uno, ya esta ordenada (duh) */
    if ((cabeza == NULL) || (cabeza->sig == NULL)) {
        return;
    }

    /* partimos la lista en a y b */
    partirListaAlumnos(cabeza, &a, &b);

    /* ordenamos las sublistas recursivamente */
    mergeSortAlumnos(&a);
    mergeSortAlumnos(&b);

    /* fusionamos las listas ya ordenadas */
    *cabezaRef = fusionarAlumnos(a, b);
}

/* Lo mismo pero para Materias (por codigo) para que valga la pena
 * el mantenimiento del sistema completo */
void partirListaMaterias(Materia* fuente, Materia** frente, Materia** atras) {
    Materia* rapido;
    Materia* lento;
    if (fuente == NULL || fuente->sig == NULL) {
        *frente = fuente; *atras = NULL;
    } else {
        lento = fuente; rapido = fuente->sig;
        while (rapido != NULL) {
            rapido = rapido->sig;
            if (rapido != NULL) {
                lento = lento->sig; rapido = rapido->sig;
            }
        }
        *frente = fuente; *atras = lento->sig; lento->sig = NULL;
    }
}

Materia* fusionarMaterias(Materia* a, Materia* b) {
    Materia* resultado = NULL;
    if (a == NULL) return b;
    if (b == NULL) return a;

    /* comparacion de cadenas (alfabetica) */
    if (strcmp(a->codigo, b->codigo) <= 0) {
        resultado = a;
        resultado->sig = fusionarMaterias(a->sig, b);
    } else {
        resultado = b;
        resultado->sig = fusionarMaterias(a, b->sig);
    }
    return resultado;
}

void mergeSortMaterias(Materia** cabezaRef) {
    Materia* cabeza = *cabezaRef;
    Materia* a; Materia* b;
    if ((cabeza == NULL) || (cabeza->sig == NULL)) return;
    partirListaMaterias(cabeza, &a, &b);
    mergeSortMaterias(&a);
    mergeSortMaterias(&b);
    *cabezaRef = fusionarMaterias(a, b);
}

void menuMantenimiento() {
    limpiar();
    cout << "============================================" << endl;
    cout << "      MANTENIMIENTO DEL SISTEMA (ADMIN)     " << endl;
    cout << "============================================" << endl;
    cout << "ATENCION: Esta herramienta reorganiza la memoria." << endl;
    cout << "Se utilizara el algoritmo MergeSort para ordenar" << endl;
    cout << "las listas enlazadas principales." << endl << endl;

    cout << "Condiciones de ordenamiento:" << endl;
    cout << " - Alumnos: Ascendente por numero de BOLETA" << endl;
    cout << " - Materias: Alfabetico por CODIGO" << endl;

    cout << endl << "Desea ejecutar el ordenamiento ahora? [1] Si; [0] No" << endl;
    cout << "> ";
    int op = getint();

    if (op == 1) {
        cout << "Iniciando proceso de optimizacion..." << endl;

        if (Escuela.alumnos != NULL) {
            cout << "Ordenando Alumnos... ";
            mergeSortAlumnos(&Escuela.alumnos);
            cout << "[OK]" << endl;
        } else {
            cout << "Alumnos: Lista vacia, saltando." << endl;
        }

        if (Escuela.materias != NULL) {
            cout << "Ordenando Materias... ";
            mergeSortMaterias(&Escuela.materias);
            cout << "[OK]" << endl;
        } else {
            cout << "Materias: Lista vacia, saltando." << endl;
        }

        /* Podria ordenar profesores y grupos pero
         * ya se entendio el punto XD (totalmente no me estaba quedando sin tiempo) */

        cout << endl << "Mantenimiento completado con exito." << endl;
        cout << "El sistema ahora esta ordenado." << endl;
    } else {
        cout << "Operacion cancelada por el usuario." << endl;
    }
    pausar();
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
        cout << "[8] Mantenimiento" << endl;
        cout << "[8] Salir" << endl;

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
                gestionInscripciones();
                break;
            case 6:
                mostrarReportes();
                break;
            case 7:
                menuMantenimiento();
                break;
            case 8:
                cout << "Saliendo!" << endl;
                dormir(800);
                return;
            default:
                cout << "Opcion invalida!" << endl;
        }
        dormir(800);
    }
}

/*
 * @details Carga datos falsos al sistema para pruebas rapidas
 * Crea materias, profesores, grupos con horarios y un alumno inscrito.
 */
void inicializarDatosPrueba();

int main() {
    inicializarDatosPrueba(); /* iniciar datos para demo */
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

/* -- funcion auxiliar para duplicar strings --
 * necesaria porque el sistema hace free() de los nombres
 * y no podemos asignar literales directamente */
char* clonarCadena(const char* origen) {
    char* destino = (char*)malloc(strlen(origen) + 1);
    if(destino) strcpy(destino, origen);
    return destino;
}

void inicializarDatosPrueba() {
    cout << "Cargando datos de prueba..." << endl;

    /* --- 1. Crear Materias --- */

    /* Materia 1: Algoritmos */
    unsigned int uidMat1 = contador++;
    Materia* m1 = (Materia*)malloc(sizeof(Materia));
    m1->uid = uidMat1;
    m1->nombre = clonarCadena("Algoritmos y Estructuras");
    m1->codigo = clonarCadena("COM101");
    m1->periodo = 3;
    m1->creditos = 8.5;
    m1->sig = NULL;
    insertarNodo((void**)&Escuela.materias, m1);

    /* Materia 2: Calculo */
    unsigned int uidMat2 = contador++;
    Materia* m2 = (Materia*)malloc(sizeof(Materia));
    m2->uid = uidMat2;
    m2->nombre = clonarCadena("Calculo");
    m2->codigo = clonarCadena("MAT202");
    m2->periodo = 1;
    m2->creditos = 9.0;
    m2->sig = NULL;
    insertarNodo((void**)&Escuela.materias, m2);

    /* --- 2. Crear Profesores --- */

    /* Profe 1 */
    unsigned int uidProf1 = contador++;
    Profesor* p1 = (Profesor*)malloc(sizeof(Profesor));
    p1->uid = uidProf1;
    p1->nombre = clonarCadena("Alan Turing");
    p1->sig = NULL;
    insertarNodo((void**)&Escuela.profesores, p1);

    /* Profe 2 */
    unsigned int uidProf2 = contador++;
    Profesor* p2 = (Profesor*)malloc(sizeof(Profesor));
    p2->uid = uidProf2;
    p2->nombre = clonarCadena("Ada Lovelace");
    p2->sig = NULL;
    insertarNodo((void**)&Escuela.profesores, p2);

    /* --- 3. Crear Grupos y Horarios --- */

    /* Grupo para Algoritmos (m1) con Turing (p1) */
    unsigned int uidGrupo1 = contador++;
    Grupo* g1 = (Grupo*)malloc(sizeof(Grupo));
    g1->uid = uidGrupo1;
    g1->uidMateria = uidMat1;
    g1->uidProfesor = uidProf1;
    g1->clave = clonarCadena("3CV1");
    g1->cupoMax = 30;
    g1->inscritos = 0;
    g1->horario = NULL;
    g1->sig = NULL;

    /* Horario para g1: LUN 7:00 - 8:30 */
    BloqueHorario* h1 = (BloqueHorario*)malloc(sizeof(BloqueHorario));
    h1->dia = LUN;
    h1->hora = 7;
    h1->minuto = 0;
    h1->edificio = 1;
    h1->salon = 101;
    h1->duracionMin = 90;
    h1->sig = NULL;
    insertarNodo((void**)&g1->horario, h1);

    /* Agregamos el grupo a la escuela */
    insertarNodo((void**)&Escuela.grupos, g1);

    /* Grupo para Calculo (m2) con Lovelace (p2) */
    unsigned int uidGrupo2 = contador++;
    Grupo* g2 = (Grupo*)malloc(sizeof(Grupo));
    g2->uid = uidGrupo2;
    g2->uidMateria = uidMat2;
    g2->uidProfesor = uidProf2;
    g2->clave = clonarCadena("1CM5");
    g2->cupoMax = 40;
    g2->inscritos = 0;
    g2->horario = NULL; /* sin horario definido por flojera */
    g2->sig = NULL;
    insertarNodo((void**)&Escuela.grupos, g2);

    /* --- 4. Crear Alumnos --- */

    /* Alumno de prueba */
    Alumno* a1 = (Alumno*)malloc(sizeof(Alumno));
    a1->nombre = clonarCadena("Pepe el Toro");
    a1->password = clonarCadena("1234");
    a1->boleta = 20230001;
    a1->periodo = 3;
    a1->materiasInscritas = NULL;
    a1->sig = NULL;
    insertarNodo((void**)&Escuela.alumnos, a1);

    /* --- 5. Inscripcion Automatica --- */
    /* Inscribimos a Pepe en Algoritmos (g1) */
    Inscripcion* ins = (Inscripcion*)malloc(sizeof(Inscripcion));
    ins->idGrupo = uidGrupo1;
    for(int k=0; k < CALFIFACIONTAM; k++) ins->calificaciones[k] = 0.0f;
    /* simulamos unas calificaciones */
    ins->calificaciones[PRIMERPAR] = 8.5f;
    ins->sig = NULL;

    insertarNodo((void**)&a1->materiasInscritas, ins);
    g1->inscritos++; /* IMPORTANTE: actualizar contador del grupo */

    cout << "Datos cargados exitosamente!" << endl;
    dormir(1000);
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
