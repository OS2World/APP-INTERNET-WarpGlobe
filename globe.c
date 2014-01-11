#define INCL_GPIBITMAPS
#define INCL_BASE
#define INCL_WIN
#include <os2.h>
#include <math.h>
#include <stdlib.h>
#include <signal.h>

PSZ resultpoint;
unsigned char *mapa;
char coloris24bit;
unsigned int mapancho,mapalto;
unsigned char *imagen;
unsigned char paleta[256][3];
unsigned long int imgancho=1024,imgalto=768;
HOBJECT hObject;
unsigned char *pmapa1,*pmapa2,*pimagen1,*pimagen2,*pimagen3,*pimagen4;
unsigned char elfichero=0;
char name1[255];
char name2[244];
char name3[11];
double angulo;
int lluz;
int strellas;
double luz;
char backgrd[150];
char namefichero[130];
char copiafichero[150];
char extension[4];
unsigned char maxbuffer[65536];
short int posicion_user[10000];
unsigned long int puntero_buffer;
char cadena2[11];
char fin;
char modo1,modo2;
long int gtimezone,diametro,gangulo,cityx,cityy;
unsigned long int periodo;
long int resx;
long int resy;
long int margen;
long int timer;
HTIMER timerhandle;
char stados;
ULONG Scope, PriorityClass, ID;
LONG PriorityDelta;
unsigned char starcolor;
unsigned char totaluz,bluec,redc,greenc;
double totalluz;

/* Esta rutina permite optimizar la lectura de la imagen desde disco */

char lee_buffer(HFILE handle) {
    ULONG basura;
    APIRET rc;

    if(puntero_buffer==65536) {
        rc=DosRead(handle,maxbuffer,65536,&basura);
        puntero_buffer=0;
    }
    return(maxbuffer[puntero_buffer++]);
}


/* Esta rutina permite optimizar la lectura de la imagen desde disco */

short int lee_buffer2(HFILE handle) {
    ULONG basura;
    APIRET rc;

    if(puntero_buffer==10000) {
        rc=DosRead(handle,posicion_user,10000,&basura);
        puntero_buffer=0;
    }
    return(posicion_user[puntero_buffer++]);
}


/* Esta rutina lee el PCX de la imagen de fondo */

void lee_pcx(void) {

    unsigned char buffer[129];
    unsigned char caracter,error;
    unsigned int px,py,contador,bucle,bucle2,ppasos;
    HFILE handle;
    ULONG action,basura;
    APIRET rc,rc2;

    /* Abrimos el fichero */

    action=0;
    puntero_buffer=65536;
    rc2 = DosOpen("mundo.pcx",&handle,&action,0,0,1,32,NULL);

    if(rc2!=0) {
        printf("Error while loading earth map");
        exit(1);
    }

    if(rc2==0) {
        rc=DosRead(handle,&buffer,128,&basura);
        if((buffer[0]==10)&&(buffer[2]==1)&&(buffer[3]==8)&&(buffer[64]==0)) {
            mapancho=buffer[8]+256*buffer[9];
            mapalto=buffer[10]+256*buffer[11];
            mapancho++;
            mapalto++;
            if(buffer[65]==1) {
                mapa=(char *) malloc(mapancho*mapalto);
                coloris24bit=0;

                for(py=0;py<mapalto;py++)
                    for(px=0;px<mapancho;) {
                        caracter=lee_buffer(handle);
                        if(caracter<192)
                            mapa[py*mapancho+(px++)]=caracter;
                        else {
                            contador=caracter-192;
                            caracter=lee_buffer(handle);
                            while((contador>0)&&(px<mapancho)) {
                                mapa[py*mapancho+(px++)]=caracter;
                                contador--;
                            }
                        }
                    }
                caracter=lee_buffer(handle);
                for(bucle=0;bucle<256;bucle++)
                    for(bucle2=0;bucle2<3;bucle2++) {
                        caracter=lee_buffer(handle);
                        paleta[bucle][bucle2]=caracter;
                    }
            } else {
                mapa=(char *) malloc(3*mapancho*mapalto);
                coloris24bit=1;

                for(py=0;py<mapalto;py++)
                    for(ppasos=0;ppasos<3;ppasos++)
                        for(px=0;px<mapancho;) {
                            caracter=lee_buffer(handle);
                            if(caracter<192)
                                mapa[py*mapancho*3+3*(px++)+ppasos]=caracter;
                            else {
                                contador=caracter-192;
                                caracter=lee_buffer(handle);
                                while((contador>0)&&(px<mapancho)) {
                                    mapa[py*mapancho*3+3*(px++)+ppasos]=caracter;
                                    contador--;
                                }
                            }
                        }

            }
            rc=DosClose(handle);
            }
    }
}


/* Esta rutina graba el bitmap ya creado, listo para ser usado como background */

void graba_bmp(unsigned long int ancho, unsigned long int alto) {
    BITMAPFILEHEADER cabecera;
    HFILE handle,bytes;
    ULONG action=0;
    APIRET rc;

    cabecera.usType=0x4d42;
    cabecera.cbSize=26;
    cabecera.offBits=26;
    cabecera.bmp.cbFix=12;
    cabecera.bmp.cx=ancho;
    cabecera.bmp.cy=alto;
    cabecera.bmp.cPlanes=1;
    cabecera.bmp.cBitCount=24;

    rc = DosOpen(namefichero,&handle,&action,0,0,18,17,NULL);
    if(rc==0) {
        rc = DosWrite(handle,&cabecera.usType,26,&bytes);
        rc = DosWrite(handle,imagen,3*ancho*alto,&bytes);
        rc = DosClose(handle);
    } else {
        printf("Error opening output file");
        exit(1);
    }
}


/* Esta rutina convierte un string en un valor numerico */

int convierte_string(char *buffer,int longitud) {
    long int temporal,bucle;
    char signo,error;

    signo=0;
    bucle=0;
    error=0;
    temporal=0;
    if(buffer[0]=='-') {
        signo=1;
        bucle++;
    }
    if(longitud>5)
       error=2;
    if(error==0)
        for(;bucle<longitud;bucle++) {
            if((buffer[bucle]<48)||(buffer[bucle]>57))
                error=1;
            if(error==0) {
                temporal=(10*temporal)+(buffer[bucle]-48);
            }
        }
    if(signo==1)
        temporal=-temporal;
    if(error!=0)
        temporal=0;
    return(temporal);
}

/* Esta rutina lee un valor numerico entero por el teclado */

int lee_teclado(char xx,char yy) {
    long int temporal,bucle;
    KBDINFO statdata;
    char buffer[257];
    char signo,error;
    STRINGINBUF longitud;
    APIRET rc;

    do {
        rc=VioSetCurPos((USHORT)yy,(USHORT)xx,0);
        statdata.cb=10;
        statdata.fsMask=9;
        rc=KbdSetStatus(&statdata,0);
        longitud.cb=255;
        rc=KbdStringIn(buffer,&longitud,0,0);
        signo=0;
        bucle=0;
        error=0;
        temporal=0;
        if(buffer[0]=='-') {
            signo=1;
            bucle++;
        }
        if(longitud.cchIn>5)
            error=2;
        if(error==0)
            for(;bucle<longitud.cchIn;bucle++) {
                if((buffer[bucle]<48)||(buffer[bucle]>57))
                    error=1;
                if(error==0) {
                    temporal=(10*temporal)+(buffer[bucle]-48);
                }
            }
        if(signo==1)
            temporal=-temporal;
        if(error!=0) {
            rc = DosBeep(2000,100);
            rc = VioWrtCharStr("                              ",30,(USHORT)yy,xx,0);
        } else
            return(temporal);
        if(error==1)
            rc = VioWrtCharStr("It is not a number ",19,2+(USHORT)yy,0,0);
        if(error==2)
            rc = VioWrtCharStr("Number out of range",19,2+(USHORT)yy,0,0);
    } while (error!=0);
}


/* Esta rutina borra la pantalla */

void borrar(void) {
    char atributo[2]={32,7};
    VioScrollDn(0,0,25,79,25,atributo,0);
}


/* Esta rutina espera a que se pulse una tecla */

void wait_tecla(void) {
    KBDKEYINFO dato;
    APIRET rc;
    printf("\n\n\nPress any key to continue");
    rc = KbdCharIn(&dato,0,0);
}


/* Esta rutina graba la configuracion actual */

void graba_cfg(void) {
    HFILE fhandle;
    ULONG action,basura;
    unsigned char temporal;
    APIRET rc;

    rc = DosDelete("GLOBE.CFG");
    action=0;
    rc = DosOpen("GLOBE.CFG",&fhandle,&action,0,0,18,17,NULL);
    temporal=250;
    rc = DosWrite(fhandle,&temporal,1,&basura);
    rc = DosWrite(fhandle,&gtimezone,4,&basura);
    rc = DosWrite(fhandle,&modo1,1,&basura);
    rc = DosWrite(fhandle,&modo2,1,&basura);
    rc = DosWrite(fhandle,&diametro,4,&basura);
    rc = DosWrite(fhandle,&gangulo,4,&basura);
    rc = DosWrite(fhandle,&periodo,4,&basura);
    rc = DosWrite(fhandle,&lluz,4,&basura);
    rc = DosWrite(fhandle,&strellas,4,&basura);
    rc = DosWrite(fhandle,&margen,4,&basura);
    rc = DosWrite(fhandle,&timer,4,&basura);
    rc = DosWrite(fhandle,&starcolor,1,&basura);
    rc = DosWrite(fhandle,&totaluz,1,&basura);
    rc = DosWrite(fhandle,&bluec,1,&basura);
    rc = DosWrite(fhandle,&redc,1,&basura);
    rc = DosWrite(fhandle,&greenc,1,&basura);
    rc = DosWrite(fhandle,&cityx,4,&basura);
    rc = DosWrite(fhandle,&cityy,4,&basura);
    rc = DosClose(fhandle);
}


/* Esta es la rutina del menu */

void menu(void) {

    KBDKEYINFO CharData;
    APIRET rc;
    HMTX hmtx;
    HEV hev;
    long int tempo;
    VIOCURSORINFO cursordata;

    hev=0;
    rc=DosOpenEventSem("\\SEM32\\RASTERWARPGLOBE",&hev);
    hmtx=0;
    rc=DosOpenMutexSem("\\SEM32\\RASTERWARPGLOBE2",&hmtx);

    if(periodo==0)
        periodo=10;

    rc=DosStartTimer(60000*periodo,(int *)hev,&timerhandle);

    while(fin==0) {

        /* borramos la pantalla */

        borrar();

        cursordata.yStart=-88;
        cursordata.cEnd=-100;
        cursordata.cx=1;
        cursordata.attr=-1;

        rc = VioSetCurType(&cursordata,0);

        /* Imprimimos el menu */

        rc = VioWrtCharStr("WarpGLOBE",9,0,35,0);
        rc = VioWrtCharStr("Version 1.2",11,1,34,0);
        rc = VioWrtCharStr("1: Change solar GMT",19,13,2,0);
        rc = VioWrtCharStr("3: Change sun-following mode",28,14,2,0); /* modo1 */
        rc = VioWrtCharStr("4: Change cities diameter",25,14,40,0);
        rc = VioWrtCharStr("5: Put/quit stars",17,15,2,0);            /* modo2 */
        rc = VioWrtCharStr("6: Change refresh interval",26,15,40,0);
        rc = VioWrtCharStr("8: Change globe diameter",24,16,40,0);
        rc = VioWrtCharStr("9: Change ambient light",23,17,2,0);
        rc = VioWrtCharStr("0: Change Idle mode",19,17,40,0);
        rc = VioWrtCharStr("S: Change stars density",23,18,2,0);
        rc = VioWrtCharStr("F: Change exit mode",19,18,40,0);
        rc = VioWrtCharStr("7: Change startup delay",23,16,2,0);
        rc = VioWrtCharStr("C: Change stars intensity",25,19,2,0);
        rc = VioWrtCharStr("L: Change light intensity",25,19,40,0);
        rc = VioWrtCharStr("B: Change background color",26,20,2,0);
        rc = VioWrtCharStr("I: Change city coordinates",26,20,40,0);
        rc = VioWrtCharStr("R: Refresh background",21,22,2,0);
        rc = VioWrtCharStr("Q: Quit WarpGLOBE",17,22,40,0);
        rc = VioWrtCharStr("(C) 1998 Raster Software Vigo (Spain)",37,24,20,0);

        if(stados==0)
            rc = VioWrtCharStr("          Status: sleeping          ",36,11,22,0);
        if(stados==1)
            rc = VioWrtCharStr("Status: creating image. Please wait.",36,11,22,0);

        rc=VioSetCurPos(3,4,0);
        printf("GMT: %i",gtimezone);
        rc=VioSetCurPos(3,57,0);
        printf("Cities diameter: %i",diametro);
        rc=VioSetCurPos(4,50,0);
        printf("Refresh every: ");
        if(periodo<10)
            printf(" ");
        printf("%i minutes",periodo);
        rc=VioSetCurPos(5,4,0);
        printf("Static sun: ");
        if((modo1&1)==0) {
            printf("no");
            rc=VioSetCurPos(3,26,0);
            printf("Earth angle: %i",gangulo);
            rc = VioWrtCharStr("2: Change earth angle",21,13,40,0);
        } else {
            printf("yes");
            rc=VioSetCurPos(3,26,0);
            printf("Sun angle: %i",gangulo);
            rc = VioWrtCharStr("2: Change sun angle",19,13,40,0);
        }
        rc=VioSetCurPos(5,26,0);
        printf("Paint stars: ");
        if(modo2==0)
            printf("no");
        else
            printf("yes");
        rc=VioSetCurPos(5,58,0);
        printf("Ambient light: ");
        if(lluz<10)
            printf(" ");
        printf("%i",lluz);
        rc=VioSetCurPos(6,4,0);
        printf("Idle mode: ");
        if((modo1&2)==0)
            printf("no");
        else
            printf("yes");
        rc=VioSetCurPos(6,26,0);
        printf("Stars density: %i",strellas);
        rc=VioSetCurPos(6,48,0);
        printf("Exit mode: ");
        if((modo1&4)==0)
            printf("clear background");
        else
            printf(" save background");

        rc=VioSetCurPos(4,4,0);
        printf("Globe diameter: %i",resy-(margen*2));

        rc=VioSetCurPos(4,26,0);
        printf("Startup delay: %i",timer);

        rc=VioSetCurPos(7,4,0);
        printf("Light intensity: %i",totaluz);

        rc=VioSetCurPos(7,26,0);
        printf("Stars intensity: %i",starcolor);

        rc=VioSetCurPos(6,26,0);

        rc = KbdCharIn(&CharData,0,0);

        cursordata.yStart=-88;
        cursordata.cEnd=-100;
        cursordata.cx=1;
        cursordata.attr=0;

        rc = VioSetCurType(&cursordata,0);

        rc=DosRequestMutexSem(hmtx,-1);

        /* FIN */

        if((CharData.chChar=='q')||(CharData.chChar=='Q')) {
            fin=1;
            rc=DosPostEventSem(hev);
        }

        /* Refrescar la imagen */

        if((CharData.chChar=='r')||(CharData.chChar=='R'))
            rc=DosPostEventSem(hev);

        /* Cambiar timezone */

        if(CharData.chChar=='1') {
            borrar();
            rc=VioSetCurPos(0,0,0);
            printf("Old GMT value: %i\n",gtimezone);
            printf("New value? (-12,12):");
            tempo=lee_teclado(21,1);
            if((tempo<-12)||(tempo>12)) {
                DosBeep(1000,100);
                printf("Value out of range");
                wait_tecla();
            } else
                gtimezone=tempo;
        }

        /* Cambiar angulo */

        if(CharData.chChar=='2') {
            borrar();
            rc=VioSetCurPos(0,0,0);
            tempo=gangulo;
            printf("Old angle value: %i\n",tempo);
            printf("New value? (0,359):");
            tempo=lee_teclado(20,1);
            if((tempo<-0)||(tempo>359)) {
                DosBeep(1000,100);
                printf("Angle out of range");
                wait_tecla();
            } else {
                gangulo=tempo;
            }
        }

        /* Modo del sol */

        if(CharData.chChar=='3')
            modo1=modo1^1;

        /* Modo de salida */

        if((CharData.chChar=='f')||(CharData.chChar=='F'))
            modo1=modo1^4;


        /* Modo Idle Time */

        if(CharData.chChar=='0') {
            modo1=modo1^2;
            if((modo1&2)==0) {
                Scope=0;
                PriorityClass=2;
                PriorityDelta=-31;
                ID=0;
                rc=DosSetPriority(Scope,PriorityClass,PriorityDelta,ID);
            } else {
                Scope=0;
                PriorityClass=1;
                PriorityDelta=+31;
                ID=0;
                rc=DosSetPriority(Scope,PriorityClass,PriorityDelta,ID);
            }
        }

        /* Cambiar la intensidad de las estrellas */

        if((CharData.chChar=='c')||(CharData.chChar=='C')) {
            borrar();
            rc=VioSetCurPos(0,0,0);
            printf("Old intensity value: %i\n",starcolor);
            printf("New value? (0,255):");
            tempo=lee_teclado(20,1);
            if((tempo<0)||(tempo>255)) {
                DosBeep(1000,100);
                printf("Value out of range");
                wait_tecla();
            } else
                starcolor=(char)tempo;
        }

        /* Cambiar la ciudad */

        if((CharData.chChar=='i')||(CharData.chChar=='I')) {
            borrar();
            rc=VioSetCurPos(0,0,0);
            printf("Use coordinates 0,0 to disable this option\n");
            printf("Old x coordinate: %i\n",cityx);
            printf("New value? (0,2047):");
            tempo=lee_teclado(21,2);
            if((tempo<0)||(tempo>2047)) {
                DosBeep(1000,100);
                printf("Value out of range");
                wait_tecla();
            } else
                cityx=tempo;
            printf("\nOld y coordinate: %i\n",cityy);
            printf("New value? (0,1023):");
            tempo=lee_teclado(21,4);
            if((tempo<0)||(tempo>1023)) {
                DosBeep(1000,100);
                printf("Value out of range");
                wait_tecla();
            } else
                cityy=tempo;
        }

        /* Cambiar el color de fondo */

        if((CharData.chChar=='b')||(CharData.chChar=='B')) {
            borrar();
            rc=VioSetCurPos(0,0,0);
            printf("Old RED value: %i\n",redc);
            printf("New value? (0,255):");
            tempo=lee_teclado(20,1);
            if((tempo<0)||(tempo>255)) {
                DosBeep(1000,100);
                printf("Value out of range");
                wait_tecla();
            } else
                redc=(char)tempo;
            printf("\nOld GREEN value: %i\n",greenc);
            printf("New value? (0,255):");
            tempo=lee_teclado(20,3);
            if((tempo<0)||(tempo>255)) {
                DosBeep(1000,100);
                printf("Value out of range");
                wait_tecla();
            } else
                greenc=(char)tempo;
            printf("\nOld BLUE value: %i\n",bluec);
            printf("New value? (0,255):");
            tempo=lee_teclado(20,5);
            if((tempo<0)||(tempo>255)) {
                DosBeep(1000,100);
                printf("Value out of range");
                wait_tecla();
            } else
                bluec=(char)tempo;
        }


        /* Cambiar la intensidad de la luz */

        if((CharData.chChar=='l')||(CharData.chChar=='L')) {
            borrar();
            rc=VioSetCurPos(0,0,0);
            printf("Old intensity value: %i\n",totaluz);
            printf("New value? (1,99):");
            tempo=lee_teclado(19,1);
            if((tempo<1)||(tempo>99)) {
                DosBeep(1000,100);
                printf("Value out of range");
                wait_tecla();
            } else
                totaluz=(char)tempo;
        }


        /* Cambiar numero de estrellas */

        if((CharData.chChar=='s')||(CharData.chChar=='S')) {
            borrar();
            rc=VioSetCurPos(0,0,0);
            printf("Old density value: %i\n",strellas);
            printf("New value? (1,10000):");
            tempo=lee_teclado(22,1);
            if((tempo<1)||(tempo>10000)) {
                DosBeep(1000,100);
                printf("Value out of range");
                wait_tecla();
            } else
                strellas=tempo;
        }

        /* Cambiar diametro */

        if(CharData.chChar=='4') {
            borrar();
            rc=VioSetCurPos(0,0,0);
            printf("Old diameter value: %i\n",diametro);
            printf("New value? (0,9):");
            tempo=lee_teclado(18,1);
            if((tempo<0)||(tempo>9)) {
                DosBeep(1000,100);
                printf("Value out of range");
                wait_tecla();
            } else
                diametro=tempo;
        }

        /* Estrellas */

        if(CharData.chChar=='5')
            modo2=1-modo2;

        /* Cambiar refresco */

        if(CharData.chChar=='6') {
            borrar();
            rc=VioSetCurPos(0,0,0);
            printf("Old refresh interval: %i minutes\n",periodo);
            printf("New value? (1,60):");
            tempo=lee_teclado(19,1);
            if((tempo<1)||(tempo>60)) {
                DosBeep(1000,100);
                printf("Value out of range");
                wait_tecla();
            } else {
                periodo=tempo;
                rc = DosStopTimer(timerhandle);
                rc = DosStartTimer(60000*periodo,(int *)hev,&timerhandle);
            }
        }

        /* Cambiar diametro del globo */

        if(CharData.chChar=='8') {
            borrar();
            rc=VioSetCurPos(0,0,0);
            printf("Old value: %i\n",resy-(margen*2));
            printf("New value? (25,%i):",resy-2);
            tempo=lee_teclado(21,1);
            if((tempo<25)||(tempo>(resy-2))) {
                DosBeep(1000,100);
                printf("\nValue out of range");
                wait_tecla();
            } else {
                margen=(resy-tempo)/2;
            }
        }

        /* Cambiar espera inicial */

        if(CharData.chChar=='7') {
            borrar();
            rc=VioSetCurPos(0,0,0);
            printf("Old value: %i\n",timer);
            printf("New value? (1,239):");
            tempo=lee_teclado(20,1);
            if((tempo<1)||(tempo>239)) {
                DosBeep(1000,100);
                printf("\nValue out of range");
                wait_tecla();
            } else {
                timer=tempo;
            }
        }


        /* Cambiar luz ambiental */

        if(CharData.chChar=='9') {
            borrar();
            rc=VioSetCurPos(0,0,0);
            printf("Old value: %i\n",lluz);
            printf("New value? (0,99):");
            tempo=lee_teclado(19,1);
            if((tempo<0)||(tempo>99)) {
                DosBeep(1000,100);
                printf("Value out of range");
                wait_tecla();
            } else
                lluz=tempo;
        }

        graba_cfg();

        rc=DosReleaseMutexSem(hmtx);
    }
    rc=DosCloseEventSem(hev);
    rc=DosCloseMutexSem(hmtx);
}


/* Esta rutina crea un GLOBE.CFG por defecto */

void defecto(void) {

    KBDKEYINFO dato;
    APIRET rc;
    char tzstring[100],tzcopia[100];
    int posicion;

    borrar();

    rc = VioWrtCharStr("GLOBE.CFG old, not found or incorrect. Creating one.",52,8,14,0);

    rc = DosScanEnv("TZ",&resultpoint);

    if(rc!=0) {
        rc = VioWrtCharStr("Your system has not defined the TZ variable.",44,10,18,0);
        DosBeep(1000,1000);
        rc = KbdCharIn(&dato,0,0);
        gtimezone=0;
    } else {
        strcpy(tzstring,resultpoint);
        tzcopia[0]=tzstring[3];
        posicion=4;
        while((tzstring[posicion]>47)&&(tzstring[posicion]<58))
            tzcopia[posicion-3]=tzstring[posicion++];
        tzcopia[posicion]=0;
        gtimezone=convierte_string(tzcopia,posicion-3);
        gtimezone-=2;
        while(gtimezone>12)
            gtimezone-=24;
        while(gtimezone<(-12))
            gtimezone+=24;
    }

    modo1=2;
    modo2=1;
    diametro=1;
    gangulo=0;
    periodo=30;
    lluz=26;
    strellas=600;
    margen=25;
    timer=8;
    starcolor=255;
    totaluz=99;
    bluec=0;
    redc=0;
    greenc=0;
    cityx=0;
    cityy=0;
    graba_cfg();
    borrar();
}


/* Esta rutina es el handle para gestionar la se¤al de fin de programa */

void handler(int valor) {
    APIRET rc;
    HMTX hmtx;
    HEV hev;

    hev=0;
    rc=DosOpenEventSem("\\SEM32\\RASTERWARPGLOBE",&hev);
    fin=1;
    rc=DosPostEventSem(hev);
    rc=DosCloseEventSem(hev);
}


/* Esta es la rutina principal */

void main(void) {

    double alfa,beta,zz,bucle2,mresx,mresy,dmapalto,dmapancho,bucle3;
    long int bucle;
    long int x,y,z,tempo,tempo2,mresx2,incremento,nresy,untempx,untempy;
    double lalfa,lbeta,sx,sy,sz,stempo,px,py,pz;
    unsigned char mes,signo,existe;
    DATETIME datetime;
    unsigned char cadena;
    HFILE fhandle;
    ULONG action,basura;
    char hechos;
    APIRET rc;
    int timezone;
    HMTX hmtx;
    ULONG ulflattr;
    BOOL32 f32fState;
    int thread;
    HEV hev;
    int diam,estrx,estry;
    ULONG actdrive,mapadeunidades;
    char unidad;


    /* Inicializamos algunas variables */

    diametro=1;
    hechos=0;
    stados=0;
    f32fState=0;
    ulflattr=0;
    hev=0;

    rc = DosDelete("ZGLOBE.BMP");

    rc=DosCreateEventSem("\\SEM32\\RASTERWARPGLOBE",&hev,ulflattr,f32fState);
    f32fState=0;
    ulflattr=0;
    hmtx=0;
    rc=DosCreateMutexSem("\\SEM32\\RASTERWARPGLOBE2",&hmtx,ulflattr,f32fState);
    ulflattr=0;
    rc = DosResetEventSem(hev,&ulflattr);

    /* Leemos la configuracion */

    action=0;
    rc = DosOpen("globe.cfg",&fhandle,&action,0,0,1,32,NULL);


    /* Si no hay fichero de configuracion */

    if(rc!=0)
        defecto();
    else {
        rc = DosRead(fhandle,&cadena,1,&basura);

        /* O si el primer byte es distinto de 254 (es un fichero de la beta 1, 2 o version 1.0) */

        if(cadena!=250)


            /* Creamos un fichero con valores por defecto */

            defecto();
        else {


            /* Si no, leemos los valores del fichero */

            rc = DosRead(fhandle,&gtimezone,4,&basura);
            rc = DosRead(fhandle,&modo1,1,&basura);
            rc = DosRead(fhandle,&modo2,1,&basura);
            rc = DosRead(fhandle,&diametro,4,&basura);
            rc = DosRead(fhandle,&gangulo,4,&basura);
            rc = DosRead(fhandle,&periodo,4,&basura);
            rc = DosRead(fhandle,&lluz,4,&basura);
            rc = DosRead(fhandle,&strellas,4,&basura);
            rc = DosRead(fhandle,&margen,4,&basura);
            rc = DosRead(fhandle,&timer,4,&basura);
            rc = DosRead(fhandle,&starcolor,1,&basura);
            rc = DosRead(fhandle,&totaluz,1,&basura);
            rc = DosRead(fhandle,&bluec,1,&basura);
            rc = DosRead(fhandle,&redc,1,&basura);
            rc = DosRead(fhandle,&greenc,1,&basura);
            rc = DosRead(fhandle,&cityx,4,&basura);
            rc = DosRead(fhandle,&cityy,4,&basura);
            if(periodo<1)
                periodo=10;
            if((timer==0)||(timer>240))
                timer=20;
        }
    }

    rc = DosClose(fhandle);

    /* Espero el tiempo especificado */

    borrar();
    rc = VioWrtCharStr("Waiting startup delay. Please, wait.",36,8,22,0);

    rc = DosSleep(1000*timer);
    borrar();

    /* Ajustamos la prioridad del programa (Idle time o Normal) */

    if((modo1&2)==0) {
        Scope=0;
        PriorityClass=2;
        PriorityDelta=-31;
        ID=0;
        rc=DosSetPriority(Scope,PriorityClass,PriorityDelta,ID);
    } else {
        Scope=0;
        PriorityClass=1;
        PriorityDelta=+31;
        ID=0;
        rc=DosSetPriority(Scope,PriorityClass,PriorityDelta,ID);
    }

    /* Si existe la variable de sistema TMP, la usamos */

    rc = DosScanEnv("GLOBETMP",&resultpoint);

    if(rc==0) {
        strcpy(name2,resultpoint); /* copio la variable en name2 */
        bucle=0;
        while(name2[bucle]!=0)
            bucle++;
        bucle--;
        if((name2[bucle]!='\\') && (name2[bucle]!='/')) {
            bucle++;
            name2[bucle++]='\\';
            name2[bucle]=0;
        }
    } else {

        /* Leemos el path y la unidad actual */

        rc = DosQueryCurrentDisk(&actdrive,&mapadeunidades);
        mapadeunidades=244;
        unidad=64+((char)actdrive);
        if(unidad==64)
            unidad=67;
        name2[0]=unidad;
        name2[1]=':';
        name2[2]='\\';
        rc = DosQueryCurrentDir(actdrive,&(name2[3]),&mapadeunidades);
        bucle=0;
        while(name2[bucle]!=0)
            bucle++;
        if(name2[bucle-1]!='\\') {
            name2[bucle]='\\';
            name2[bucle+1]=0;
        }
    }

    strcpy(name1,"BACKGROUND=");
    strcpy(name3,"ZGLOB");
    strcpy(copiafichero,name2);
    strcat(copiafichero,"ZGLOBE.BMP");
    strcat(name2,name3);
    strcat(name1,name2);

    elfichero=0;

    fin=0;

    /* leo el mapamundi */

    lee_pcx();

    /* Registro las se¤ales */

    signal(SIGTERM,(void *)handler);
    signal(SIGINT,(void *)handler);

    borrar();
    resx = WinQuerySysValue(HWND_DESKTOP,SV_CXSCREEN);

    resy = WinQuerySysValue(HWND_DESKTOP,SV_CYSCREEN);

    hObject = WinQueryObject("<WP_DESKTOP>");

    /* inicializo la memoria */

    imagen=(char *) malloc(1+resx*resy*3);

    /* La rutina de menu se lanza en un thread aparte, de modo que trabaja
     de forma paralela */

    thread = _beginthread((void *)menu,NULL,16384,NULL);
    rc = DosSleep(1000);

    /* Inicio el bucle. Mientras FIN sea cero, debe repetir el pintado del
     fondo. FIN se hara 1 cuando se elija la opcion de salir del programa o
     cuando se cierre la ventana */

    while(fin==0) {


        /* Pedimos el semaforo, de modo que bloqueamos el thread del menu */

        rc = DosRequestMutexSem(hmtx,-1);


        /* Calculamos la luminosidad de fondo */

        luz=((double)lluz)/100;
        totalluz=((double)totaluz)/99;

        stados=1;
        rc = VioWrtCharStr("Status: creating image. Please wait.",36,11,22,0);


        /* leo la configuracion */

        nresy=resy-(2*margen);

        pimagen1=imagen;
        for(bucle=0;bucle<(resx*resy);bucle++) {
            *pimagen1++=bluec;
            *pimagen1++=greenc;
            *pimagen1++=redc;
        }

        /* Pinto las estrellas */

        if(modo2==1) {
            srand(1);
            for(bucle=0;bucle<strellas;bucle++) {
                estrx=1+(int)((resx-2)*rand()/(RAND_MAX+1));
                estry=1+(int)((resy-2)*rand()/(RAND_MAX+1));
                diam=1+(int)(10*rand()/(RAND_MAX+1));
                if(diam<8)
                    diam=1;
                else
                    diam=2;
                for(untempx=0;untempx<diam;untempx++)
                    for(untempy=0;untempy<diam;untempy++) {
                        imagen[(estry+untempy)*resx*3+(estrx+untempx)*3]=starcolor;
                        imagen[(estry+untempy)*resx*3+(estrx+untempx)*3+1]=starcolor;
                        imagen[(estry+untempy)*resx*3+(estrx+untempx)*3+2]=starcolor;
                    }
            }
        }

        mresx=(double)(resx/2);
        mresx2=resx/2;
        mresy=(double)(nresy/2);
        dmapalto=(double)mapalto;
        dmapancho=(double)mapancho;
        dmapalto--;
        dmapancho--;


        /* Cojo la fecha y hora */

        rc = DosGetDateTime(&datetime);

        mes=datetime.month;


        /* Si estamos entre Enero y Junio, la inclinacion del sol es positiva */

        if((mes>=1)&&(mes<=6))
            lalfa=-((((double)(datetime.day+datetime.month*31-32))-93)*.41)/93;
        else

            /* Si estamos entre Julio y Diciembre, la inclinacion del sol es negativa */

            lalfa=((((double)(datetime.day+(datetime.month-6)*31-32))-93)*.41)/93;


        /* Sumamos a la hora actual la GMT */

        timezone=((int)(datetime.hours))+gtimezone;


        /* Y ajustamos a 24 horas */

        if(timezone<0)
            timezone+=24;
        if(timezone>24)
            timezone-=24;


        /* Calculamos el angulo horizontal con que incide el sol (el vertical ya
         lo calculamos antes, en LALFA */

        lbeta=(((double)(((int)(datetime.minutes))+60*timezone))*6.2832)/1440;


        /* Si el bit 0 de MODO1 es cero, queremos que el sol rote; luego el
         angulo de rotacion de la tierra sera el que especificamos en gangulo */

        if((modo1&1)==0)
            angulo=1.5708+(((float)gangulo)*6.2832/360);
        else

            /* Si el bit 0 es 1, queremos que el sol este fijo y rote la tierra.
             asi que angulo sera el negativo del angulo del sol */

            angulo=lbeta-1.5708+(((float)gangulo)*6.2832/360);

        /* Ajustamos ANGULO */

        if(angulo<0)
            angulo+=6.2832;
        if(angulo>=6.28320)
            angulo-=6.2832;

        sy=sin(lalfa);
        stempo=cos(lalfa);
        sz=stempo*cos(lbeta);
        sx=stempo*sin(lbeta);
        extension[0]=(elfichero/100)+48;
        extension[1]=((elfichero%100)/10)+48;
        extension[2]=((elfichero%100)%10)+48;
        extension[3]=0;
        strcpy(namefichero,name2); /* copio el fichero a la cadena de trabajo */
        strcat(namefichero,extension); /* a¤ado la extension */
        strcat(namefichero,".BMP");
        strcpy(backgrd,name1);
        strcat(backgrd,extension);
        strcat(backgrd,".BMP");
        elfichero++;

        /* Empiezo a pintar el mapa */

        pimagen1=imagen;
        pimagen1+=margen*resx*3;
        pimagen3=(char *)(resx*resy*3);


        /* para cada linea de la pantalla */

        for(bucle=0;bucle<nresy;bucle++) {

            /* calculo la latitud correspondiente (angulo alfa de la
             esfera) */

            pimagen2=pimagen1;
            pimagen2+=bucle*resx*3;

            alfa=acos((mresy-((double)bucle))/mresy);
            py=cos(alfa);


            /* En Y meto el valor de la latitud ajustado al tama¤o de la
             imagen de la tierra */

            y=(int)(alfa*dmapalto/3.1416);


            /* Z contiene el ancho de la esfera para cada linea de la
             pantalla */

            z=(int)(mresy*sin(alfa));
            zz=(double)z;
            pimagen2+=(3*(mresx2-z));
            if(z!=0)

                /* Para cada pixel del meridiano */

                for(bucle2=-zz;bucle2<=zz;bucle2++) {


                    /* Calculo el angulo de longitud (beta) */

                    beta=(angulo+acos(bucle2/zz));
                    while(beta>=6.2832)
                        beta-=6.2832;
                    stempo=sin(alfa);


                    /* Calculo el vector unitario que une el centro de la
                     esfera con el punto actual, asi como el vector unitario de
                     la luz solar */

                    pz=stempo*cos(beta);
                    px=stempo*sin(beta);

                    /* Y calculo su producto escalar, que me dara un valor proporcional
                     a la cantidad de luz que refleja ese punto (Teorema del coseno de
                     lambert). Le sumo LUZ para que las partes que no reciben luz
                     directa posean una cierta luz difusa */

                    stempo=px*sx+py*sy+pz*sz+luz;
                    if(stempo>1)
                        stempo=1;

                    stempo*=totalluz;

                    if(stempo<luz)
                        stempo=luz;

                    /* Como coordenadas para la imagen de la tierra, uso los propios
                     angulos ALFA y BETA, ajustados a las proporciones de la imagen
                     (van almacenados en X e Y )*/

                    x=(int)(beta*dmapancho/6.2832);
                    if(coloris24bit==0) {
                        *pimagen2++=((char)(stempo*((double)(paleta[mapa[y*mapancho+x+1]][2]))));
                        *pimagen2++=((char)(stempo*((double)(paleta[mapa[y*mapancho+x+1]][1]))));
                        *pimagen2++=((char)(stempo*((double)(paleta[mapa[y*mapancho+x+1]][0]))));
                    } else {
                        *pimagen2++=((char)(stempo*((double)mapa[y*mapancho*3+x*3+3])));
                        *pimagen2++=((char)(stempo*((double)mapa[y*mapancho*3+x*3+2])));
                        *pimagen2++=((char)(stempo*((double)mapa[y*mapancho*3+x*3+1])));
                    }
                }


            /* Si FIN no es cero, hago BUCLE igual a NRESY, de modo que
             salgo del bucle directamente */

            if(fin!=0)
                bucle=nresy;
        }

        /* pinto los usuarios */

        existe=0;

        if(diametro!=0) {
            action=0;
            rc = DosOpen("users.nfo",&fhandle,&action,0,0,1,32,NULL);
            if(rc!=0) {
                printf("Error opening file USERS.NFO");
                exit(1);
            }
            puntero_buffer=10000;
            do {
                tempo=lee_buffer2(fhandle);
                if(tempo!=30000) {
                    tempo2=lee_buffer2(fhandle);
                    if((tempo==cityx)&&(tempo2==cityy))
                        existe=1;
                    beta=6.2832*((2048-(double)(tempo)))/2048;
                    alfa=3.1416*((double)(tempo2))/1023;
                    beta-=angulo-.005;
                    while (beta<0)
                        beta+=6.2832;
                    while (beta>6.2832)
                        beta-=6.2832;
                    if((beta>0)&&(beta<3.1415)){
                        y=2+(int)(mresy-mresy*cos(alfa));
                        x=(int)(mresx+mresy*cos(beta)*sin(alfa));
                        for(untempx=0;untempx<diametro;untempx++)
                            for(untempy=0;untempy<diametro;untempy++) {
                                imagen[(y+margen+untempy)*resx*3+(x+untempx)*3]=0;
                                imagen[(y+margen+untempy)*resx*3+(x+untempx)*3+1]=0;
                                imagen[(y+margen+untempy)*resx*3+(x+untempx)*3+2]=255;
                            }
                    }
                }
            } while (tempo!=30000);
            DosClose(fhandle);
            if(existe==1) {
                beta=6.2832*((2048-(double)(cityx)))/2048;
                alfa=3.1416*((double)(cityy))/1023;
                beta-=angulo-.005;
                while (beta<0)
                    beta+=6.2832;
                while (beta>6.2832)
                    beta-=6.2832;
                if((beta>0)&&(beta<3.1415)){
                    y=2+(int)(mresy-mresy*cos(alfa));
                    x=(int)(mresx+mresy*cos(beta)*sin(alfa));
                    for(untempx=0;untempx<diametro;untempx++)
                        for(untempy=0;untempy<diametro;untempy++) {
                            imagen[(y+margen+untempy)*resx*3+(x+untempx)*3]=255;
                            imagen[(y+margen+untempy)*resx*3+(x+untempx)*3+1]=255;
                            imagen[(y+margen+untempy)*resx*3+(x+untempx)*3+2]=50;
                        }
                }
            }
        }
        graba_bmp(resx,resy);
        WinSetObjectData(hObject,backgrd);
        stados=0;
        rc = VioWrtCharStr("          Status: sleeping          ",36,11,22,0);
        rc = DosReleaseMutexSem(hmtx);
        rc = DosWaitEventSem(hev,-1);
        rc = DosResetEventSem(hev,&ulflattr);
        if((fin==0)||((modo1&4)==0))
            rc = DosDelete(namefichero);
        else
            rc = DosMove(namefichero,"ZGLOBE.BMP");
    }
    borrar();
    if((modo1&4)==0)
        WinSetObjectData(hObject,"BACKGROUND=none");
    else {
        strcpy(backgrd,name1);
        strcat(backgrd,"E.BMP");
        WinSetObjectData(hObject,backgrd);
    }
    free (imagen);
    free (mapa);
}