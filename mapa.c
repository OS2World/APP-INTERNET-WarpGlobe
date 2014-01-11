#define INCL_GPIBITMAPS
#define INCL_BASE
#include <os2.h>
#include <math.h>
#include <stdlib.h>

unsigned char *mapa;
unsigned char *imagen;
unsigned int mapancho,mapalto;
unsigned char paleta[256][3];
unsigned char maxbuffer[65536];
short int posicion_user[10000];
unsigned long int puntero_buffer;

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
    unsigned int py,contador,bucle,bucle2,ppasos;
    int px;
    HFILE handle;
    ULONG action,basura;
    APIRET rc,rc2;
    
    /* Abrimos el fichero */
    
    action=0;
    puntero_buffer=65536;
    rc2 = DosOpen("mapa.pcx",&handle,&action,0,0,1,32,NULL);
    
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
            printf("\n%d\n%d\n",mapancho,mapalto);
            mapa=(char *) malloc(mapancho*mapalto);
            for(py=0;py<mapalto;py++)
                for(px=mapancho-1;px>=0;) {
                    caracter=lee_buffer(handle);
                    if(caracter<192)
                        mapa[py*mapancho+(px--)]=caracter;
                    else {
                        contador=caracter-192;
                        caracter=lee_buffer(handle);
                        while((contador>0)&&(px>=0)) {
                            mapa[py*mapancho+(px--)]=caracter;
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
    
    rc = DosOpen("SGLOBE.BMP",&handle,&action,0,0,18,17,NULL);
    if(rc==0) {
        rc = DosWrite(handle,&cabecera.usType,26,&bytes);
        rc = DosWrite(handle,imagen,mapancho*mapalto*3,&bytes);
        rc = DosClose(handle);
    } else {
        printf("Error opening output file");
        exit(1);
    }
}

/* Esta es la rutina principal */

void main(int argc, char **argv) {
    
    long int bucle;
    unsigned long int x,y,untempx,untempy;
    int diametro;
    ULONG action;
    long int tempo,tempo2;
    HFILE fhandle;
    APIRET rc;

    /* Inicializamos algunas variables */
    
    if(argc==1)
        diametro=1;
    else
        diametro=argv[1][0]-48;
    
    if((diametro<1)||(diametro>5))
        diametro=1;
    
    rc = DosDelete("SGLOBE.BMP");

    lee_pcx();
    
    imagen=(char *) malloc(mapancho*mapalto*3);
    
    for(bucle=0;bucle<(mapancho*mapalto);bucle++) {
        imagen[bucle*3]=paleta[mapa[bucle]][2];
        imagen[1+bucle*3]=paleta[mapa[bucle]][1];
        imagen[2+bucle*3]=paleta[mapa[bucle]][0];
    }
    
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
            x=tempo*mapancho/2048;
            y=tempo2*mapalto/1024;
            for(untempx=0;untempx<diametro;untempx++)
                for(untempy=0;untempy<diametro;untempy++) {
                    imagen[(y+untempy)*mapancho*3+(x+untempx)*3]=0;
                    imagen[(y+untempy)*mapancho*3+(x+untempx)*3+1]=0;
                    imagen[(y+untempy)*mapancho*3+(x+untempx)*3+2]=255;
                }
        }
    } while (tempo!=30000);
    DosClose(fhandle);
    graba_bmp(mapancho,mapalto);
}