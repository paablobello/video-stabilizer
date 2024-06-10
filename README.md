
# Video Stabilization Project

Proyecto para estabilizar videos mediante la comparación de bloques de píxeles en fotogramas consecutivos, utilizando técnicas de procesamiento de imágenes y optimizaciones.

## Descripción

El programa estabiliza un video siguiendo estos pasos:

1. Lee el archivo de video proporcionado como argumento de línea de comandos.
2. Clona el primer fotograma del video como referencia para la estabilización.
3. Establece las coordenadas del bloque de referencia en el centro de la imagen.
4. Procesa cada fotograma del video:
    - Calcula la diferencia mínima entre el bloque de referencia y los bloques en el fotograma actual utilizando la función `compararBloques`.
    - Desplaza el fotograma actual según los valores de desplazamiento calculados utilizando la función `desplazarImagen`.
    - Muestra el fotograma estabilizado en una ventana llamada "Frame Centrado".
    - Calcula y muestra una barra de progreso basada en la cantidad de fotogramas procesados.
    - Mide el tiempo transcurrido desde el inicio del procesamiento del video.
5. Espera la pulsación de una tecla para finalizar.
6. Libera los recursos utilizados y cierra las ventanas.

## Funcionalidades

- **compararBloques**: Compara dos bloques de píxeles en dos imágenes diferentes utilizando SIMD para realizar operaciones de comparación de manera eficiente. Devuelve la diferencia entre los bloques.
- **desplazarImagen**: Desplaza la imagen según los valores de desplazamiento calculados. Rellena las áreas desplazadas con píxeles negros.
- **main**: La función principal que realiza todos los pasos mencionados anteriormente.
- **Barra de progreso**: Muestra visualmente el progreso del procesamiento de los fotogramas.

## Requisitos

- OpenCV
- Biblioteca SIMD
- Bibliotecas estándar de C

## Instalación

1. Clona este repositorio:
   ```bash
   git clone https://github.com/paablobello/video-stabilizer
   ```
2. Navega al directorio del proyecto:
   ```bash
   cd video-stabilizer
   ```
3. Compila el programa:
   ```bash
   gcc -o video_stabilization main.c -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc -lopencv_videoio
   ```

## Uso

Para ejecutar el programa, utiliza la siguiente línea de comandos:
```bash
./video_stabilization ruta_al_archivo_de_video
```

Ejemplo:
```bash
./video_stabilization video.mp4
```

