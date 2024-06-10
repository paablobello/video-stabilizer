/* 
 * File:   main.c
 * Author: PABLO
 *
 * Created on 11 de mayo de 2023, 13:26
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <emmintrin.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <time.h>
#include <pthread.h> 
#include <Windows.h>

// Tamaño de los bloques
#define ALTOBLOQUE 32
#define ANCHOBLOQUE 32

//Margen de busqueda
#define MARGEN 100

static uint64_t GetTickCount_us() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t) (ts.tv_nsec / 1000) + ((uint64_t) ts.tv_sec * 1000000ull);
}

int compararBloques(IplImage* img1, int i, int j, IplImage* img2, int k, int l, int alto, int ancho) {

    int diferencia;
    __m128i difReg = _mm_set1_epi32(0);
    __m128i aux, r1, r2;
    int f1, f2, cc;

    for (f1 = i, f2 = k; f1 < i + alto; f1++, f2++) {
        __m128i *pImg1 = (__m128i *) (img1->imageData + f1 * img1->widthStep + j * img1->nChannels);
        __m128i *pImg2 = (__m128i *) (img2->imageData + f2 * img2->widthStep + l * img2->nChannels);
        for (cc = 0; cc < img1->nChannels * ancho; cc += 16) {
            r1 = _mm_loadu_si128(pImg1);
            pImg1++;
            r2 = _mm_loadu_si128(pImg2);
            pImg2++;
            aux = _mm_sad_epu8(r1, r2);
            difReg = _mm_add_epi32(aux, difReg);
        }
    }
    diferencia = _mm_cvtsi128_si32(difReg);
    difReg = _mm_srli_si128(difReg, 8);
    diferencia += _mm_cvtsi128_si32(difReg);

    return diferencia;
}

void desplazarImagen(IplImage *image, int alto, int ancho) {


    __m128i aux;

    if (alto > 0) { //Movimiento hacia abajo
        for (int fila = image->height - 1 - alto; fila >= 0; fila--) {
            __m128i* pOrigen = (__m128i*) (image->imageData + (fila * image->widthStep));
            __m128i* pDestino = (__m128i*) (image->imageData + ((fila + alto) * image->widthStep));
            for (int columna = 0; columna < image->widthStep; columna = columna + 16) {
                aux = _mm_loadu_si128(pOrigen++);
                _mm_storeu_si128(pDestino++, aux);
            }
        }
    }

    if (alto < 0) { //Movimiento hacia arriba
        for (int fila = -alto; fila < image->height; fila++) {
            __m128i* pOrigen = (__m128i*) (image->imageData + fila * image->widthStep);
            __m128i* pDestino = (__m128i*) (image->imageData + (fila + alto) * image->widthStep);
            for (int columna = 0; columna < image->widthStep; columna = columna + 16) {
                aux = _mm_loadu_si128(pOrigen++);
                _mm_storeu_si128(pDestino++, aux);
            }
        }
    }

    if (ancho > 0) { //Movimiento hacia derecha
        for (int fila = 0; fila < image->height; fila++) {
            __m128i* pOrigen = (__m128i*) (image->imageData + fila * image->widthStep + (image->width - ancho - 1) * image->nChannels);
            __m128i* pDestino = (__m128i*) (image->imageData + fila * image->widthStep + (image->width - 1) * image->nChannels);
            for (int columna = ancho * image->nChannels; columna < image->widthStep - 16; columna = columna + 16) {
                aux = _mm_loadu_si128(pOrigen--);
                _mm_storeu_si128(pDestino--, aux);
            }
        }
    }

    if (ancho < 0) { //Movimiento hacia izquierda
        for (int fila = 0; fila < image->height; fila++) {
            __m128i* pOrigen = (__m128i*) (image->imageData + fila * image->widthStep + (-ancho) * image->nChannels);
            __m128i* pDestino = (__m128i*) (image->imageData + fila * image->widthStep);
            for (int columna = abs(ancho) * image->nChannels; columna < image->widthStep - 16; columna = columna + 16) {
                aux = _mm_loadu_si128(pOrigen++);
                _mm_storeu_si128(pDestino++, aux);
            }
        }
    }

    if (alto > 0) {
        for (int fila = 0; fila < alto; fila++) {
            __m128i* pOrigen = (__m128i*) (image->imageData + fila * image->widthStep);
            for (int columna = 0; columna < image->widthStep; columna = columna + 16) {
                _mm_storeu_si128(pOrigen++, _mm_set1_epi32(0));
            }
        }
    }
    if (alto < 0) {
        for (int fila = image->height - abs(alto); fila < image->height; fila++) {
            __m128i* pOrigen = (__m128i*) (image->imageData + fila * image->widthStep);
            for (int columna = 0; columna < image->widthStep; columna = columna + 16) {
                _mm_storeu_si128(pOrigen++, _mm_set1_epi32(0));
            }
        }
    }

    if (ancho > 0) {
        for (int fila = 0; fila < image->height; fila++) {
            __m128i* pOrigen = (__m128i*) (image->imageData + fila * image->widthStep);
            for (int columna = 0; columna < ancho * image->nChannels; columna = columna + 16) {
                _mm_storeu_si128(pOrigen++, _mm_set1_epi32(0));
            }
        }
    }

    if (ancho < 0) {
        for (int fila = 0; fila < image->height; fila++) {
            __m128i* pOrigen = (__m128i*) (image->imageData + fila * image->widthStep + (image->width - abs(ancho)) * image->nChannels);
            for (int columna = 0; columna < abs(ancho) * image->nChannels; columna = columna + 16) {

                _mm_storeu_si128(pOrigen++, _mm_set1_epi32(0));
            }
        }
    }
}

int main(int argc, char** argv) {

    if (argc != 2)
        exit(-1);

    // Creamos las imagenes a mostrar
    CvCapture* capture = cvCaptureFromAVI(argv[1]);

    // Comprobar si se ha podido crear el archivo
    if (!capture) {
        printf("Error: fichero %s no leido\n", argv[1]);
        return EXIT_FAILURE;
    }


    printf("Pulsa una tecla para empezar...\n");
    cvWaitKey(0);

    IplImage *InFrameNew;


    
    CvSize size =
            cvSize(
            (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH),
            (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT)
            );

    printf("\nAncho: %d", size.width);
    printf("\nAlto: %d", size.height);

    //Frames por segundo
    double fps = cvGetCaptureProperty(
            capture,
            CV_CAP_PROP_FPS
            );
    
    double fpsTotal = cvGetCaptureProperty(
            capture,
            CV_CAP_PROP_FRAME_COUNT
            );

    printf("\nFrames por segundo: %f", fps);



    //CLONACIÓN
    IplImage *InFrameFirst = cvCloneImage(cvQueryFrame(capture)),
            *InFrameOld = cvCloneImage(InFrameFirst);



    // Establecemos las coordenadas del bloque de referencia.
    int filaBusq = InFrameFirst->height / 2 - ALTOBLOQUE / 2;
    int colBusq = InFrameFirst->width / 2 - ANCHOBLOQUE / 2;



    int filErrorMin, colErrorMin;
    // cvShowImage("Original", InFrameFirst);

    // Iniciamos el cronómetro para medir el tiempo
    uint64_t time_start = GetTickCount_us();

    int progreso = 0, a, b, ancho = 50;

    while ((InFrameNew = cvQueryFrame(capture)) != NULL) { // mientras el while sea not null significa que el capture tiene frames del .avi


        unsigned int dif = UINT_MAX;

        for (int fila = filaBusq - MARGEN; fila <= filaBusq + MARGEN; fila++) {

            for (int col = colBusq - MARGEN; col <= colBusq + MARGEN; col++) {


                unsigned int diferenciaProv = compararBloques(InFrameFirst, filaBusq, colBusq, InFrameNew, fila, col, ALTOBLOQUE, ANCHOBLOQUE);
                if (diferenciaProv < dif) {
                    dif = diferenciaProv;
                    filErrorMin = fila;
                    colErrorMin = col;
                }

            }

        }

        desplazarImagen(InFrameNew, filaBusq - filErrorMin, colBusq - colErrorMin);
        cvShowImage("Frame Centrado", InFrameNew);
        cvWaitKey(1);
        
        //barra de progreso(Implementacion adicional al proyecto)
        float progres = (float) progreso / (float) fpsTotal;
        progreso++;
        
        int bar_rel = progres * ancho;
        int bar_vac = ancho - bar_rel;
        
        printf("[");
        for (a = 0; a < bar_rel; a++){
            
            printf("*");           
            
        }
        
        for (a = 0; a < bar_vac; a++){
            printf(" ");
        }
        printf("] %d%%\r", (int)(progres * 100));
        
        fflush(stdout);//aqui lo pongo para limpiar la consola y que solo se mostrara como una barra de progreso pero ns pq no me funciona.
        
        
    }



    //mostramos cronómetro
    uint64_t time_run = GetTickCount_us() - time_start; //tiempo q tardo en ejecutarse
    printf("\nTiempo transcurrido %.2f \r\n", (float) time_run / 1000);

    printf("Pulsa una tecla para finalizar...\n");
    cvWaitKey(0);

    cvReleaseCapture(&capture);

    cvDestroyWindow("Frame Video");
    cvDestroyWindow("Frame Centrado");

    return EXIT_SUCCESS;


}