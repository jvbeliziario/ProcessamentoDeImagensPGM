#include "utilitarios_pgm.h"
#include <string.h>

// FUNÇÕES
// Função para liberar memória da imagem
void liberar_imagem(Image *img) {
    if (img) {
        if (img->pixels) {
            free(img->pixels);
        }
        free(img);
    }
}
// Função para ler uma imagem PGM do arquivo
Image* ler_pgm(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Erro: Não foi possível abrir o arquivo %s\n", filename);
        return NULL;
    }

    Image *img = (Image*)malloc(sizeof(Image));
    if (!img) {
        printf("Erro: Falha na alocação de memória\n");
        fclose(file);
        return NULL;
    }

    char header_line[256];
    if (!fgets(header_line, sizeof(header_line), file) || strncmp(header_line, "P5", 2) != 0) {
        printf("Erro: Formato PGM inválido (esperado P5)\n");
        free(img);
        fclose(file);
        return NULL;
    }

    do {
        if (!fgets(header_line, sizeof(header_line), file)) {
            printf("Erro: Falha ao ler cabeçalho PGM\n");
            free(img);
            fclose(file);
            return NULL;
        }
    } while (header_line[0] == '#');

    if (sscanf(header_line, "%d %d", &img->header.colunas, &img->header.linhas) != 2) {
        printf("Erro: Falha ao ler dimensões do cabeçalho\n");
        free(img);
        fclose(file);
        return NULL;
    }

    if (!fgets(header_line, sizeof(header_line), file)) {
        printf("Erro: Falha ao ler valor máximo de cinza\n");
        free(img);
        fclose(file);
        return NULL;
    }
    sscanf(header_line, "%d", &img->header.max_gray);

    img->header.tamanho_pixels = img->header.linhas * img->header.colunas;
    img->pixels = (unsigned char*)malloc(img->header.tamanho_pixels);
    if (!img->pixels) {
        printf("Erro: Falha na alocação de memória para pixels\n");
        free(img);
        fclose(file);
        return NULL;
    }

    if (fread(img->pixels, 1, img->header.tamanho_pixels, file) != img->header.tamanho_pixels) {
        printf("Erro: Falha ao ler pixels da imagem\n");
        liberar_imagem(img);
        fclose(file);
        return NULL;
    }

    fclose(file);
    return img;
}

// Função para escrever uma imagem PGM no arquivo
int escrever_pgm(const char *filename, Image *img) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Erro: Não foi possível criar o arquivo %s\n", filename);
        return 0;
    }
    
    fprintf(file, "P5\n%d %d\n%d\n", 
            img->header.colunas, img->header.linhas, img->header.max_gray);
    
    if (fwrite(img->pixels, 1, img->header.tamanho_pixels, file) != img->header.tamanho_pixels) {
        printf("Erro: Falha ao escrever pixels\n");
        fclose(file);
        return 0;
    }
    
    fclose(file);
    return 1;
}

// Função para aplicar negativo na imagem
void aplicar_negativo(Image *img) {
    if (!img || !img->pixels) return;
    int total_pixels = img->header.tamanho_pixels;
    int max_gray = img->header.max_gray;
    for (int i = 0; i < total_pixels; i++) {
        img->pixels[i] = max_gray - img->pixels[i];
    }
}

// Função para aplicar limiarização na imagem
void aplicar_limiarizacao(Image *img, int limiar) {
    if (!img || !img->pixels) return;
    int total_pixels = img->header.tamanho_pixels;
    int max_gray = img->header.max_gray;
    for (int i = 0; i < total_pixels; i++) {
        img->pixels[i] = (img->pixels[i] >= limiar) ? max_gray : 0;
    }
}