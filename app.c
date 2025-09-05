#include <stdio.h>
#include <stdlib.h>

#define IMAGE_WIDTH 740
#define IMAGE_HEIGHT 1109

unsigned char* readPGM(const char *filename, int *width, int *height) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Erro ao abrir arquivo");
        return NULL;
    }

    char header[64];
    // lê o tipo (P5)
    if (!fgets(header, sizeof(header), file)) {
        perror("Erro lendo cabeçalho");
        fclose(file);
        return NULL;
    }

    // ignora comentários
    do {
        if (!fgets(header, sizeof(header), file)) {
            perror("Erro lendo cabeçalho");
            fclose(file);
            return NULL;
        }
    } while (header[0] == '#');

    // lê largura e altura
    sscanf(header, "%d %d", width, height);

    // lê o valor máximo (ex: 255)
    if (!fgets(header, sizeof(header), file)) {
        perror("Erro lendo cabeçalho");
        fclose(file);
        return NULL;
    }

    // aloca memória
    unsigned char *pixels = malloc((*width) * (*height));
    if (!pixels) {
        perror("Erro alocando memória");
        fclose(file);
        return NULL;
    }

    fread(pixels, 1, (*width) * (*height), file);
    fclose(file);
    return pixels;
}

void filterImage(unsigned char *imageData, int width, int height) {
    for (int i = 0; i < width * height; i++) {
        imageData[i] = (imageData[i] < 128) ? 0 : 255;
    }
}

int main() {
    int width, height;
    unsigned char *imageData = readPGM("entrada.pgm", &width, &height);
    if (!imageData) return EXIT_FAILURE;

    filterImage(imageData, width, height);

    FILE *outputFile = fopen("saida.pgm", "wb");
    if (!outputFile) {
        perror("Erro ao abrir arquivo de saída");
        free(imageData);
        return EXIT_FAILURE;
    }

    fprintf(outputFile, "P5\n%d %d\n255\n", width, height);
    fwrite(imageData, 1, width * height, outputFile);

    fclose(outputFile);
    free(imageData);
    return EXIT_SUCCESS;
}
//compile com: gcc app.c -o app