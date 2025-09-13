#include <stdio.h>
#include <stdlib.h>

#define IMAGE_WIDTH 740
#define IMAGE_HEIGHT 1109

unsigned char* readPGM(const char *filename, int *width, int *height) {

    // Abre o arquivo
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Erro ao abrir arquivo");
        return NULL;
    }

    char header[64];
    // Lê o tipo (P5)
    if (!fgets(header, sizeof(header), file)) {
        perror("Erro lendo cabeçalho");
        fclose(file);
        return NULL;
    }

    // Ignora comentários
    do {
        if (!fgets(header, sizeof(header), file)) {
            perror("Erro lendo cabeçalho");
            fclose(file);
            return NULL;
        }
    } while (header[0] == '#');

    // Leitura de largura e altura
    sscanf(header, "%d %d", width, height);

    // Lê o valor máximo (ex: 255)
    if (!fgets(header, sizeof(header), file)) {
        perror("Erro lendo cabeçalho");
        fclose(file);
        return NULL;
    }

    // Alocação de memória
    unsigned char *pixels = malloc((*width) * (*height));
    if (!pixels) {
        perror("Erro alocando memória");
        fclose(file);
        return NULL;
    }

    // Leitura dos dados da imagem
    fread(pixels, 1, (*width) * (*height), file);
    fclose(file);
    return pixels;
}

void filterImage(unsigned char *imageData, int width, int height, int L) {
    for (int i = 0; i < width * height; i++) {
        imageData[i] = (imageData[i] < 128) ? 0 : L;
    }
}

int main() {
    int width, height, L;
    unsigned char *imageData = readPGM("images/entrada.pgm", &width, &height);
    if (!imageData) { return EXIT_FAILURE;
    } else {
        printf("Imagem carregada com sucesso!\n");
    }

    printf("\nDigite um valor inteiro para o tom de cinza (0-255): ");
    if (scanf("%d", &L) != 1) {

        // Limpa o buffer de entrada e evita loop infinito
        // Verifica se o valor inserido é um número inteiro e válido
        fprintf(stderr, "Entrada inválida. Deve ser um número inteiro.\n");
        free(imageData);
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        return EXIT_FAILURE;
    }

        // Verifica se o valor está dentro do intervalo válido
    if (L < 0 || L > 255) {
        fprintf(stderr, "Valor fora do intervalo. Deve ser entre 0 e 255.\n");
        free(imageData);
        return EXIT_FAILURE;
    }

    filterImage(imageData, width, height, L);

    FILE *outputFile = fopen("saida.pgm", "wb");
    if (!outputFile) {
        perror("Erro ao abrir arquivo de saída");
        free(imageData);
        return EXIT_FAILURE;
    } else {
        printf("Imagem processada com sucesso!\n");
    }

    fprintf(outputFile, "P5\n%d %d\n255\n", width, height);
    fwrite(imageData, 1, width * height, outputFile);

    fclose(outputFile);
    free(imageData);
    return EXIT_SUCCESS;
}
//compile com: gcc app.c -o app