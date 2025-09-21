#ifndef UTILITARIOS_PGM_H
#define UTILITARIOS_PGM_H

#include <stdio.h>
#include <stdlib.h>

// Structs
typedef struct {
    int linhas;
    int colunas;
    int max_gray;
    size_t tamanho_pixels;
} ImageHeader;

typedef struct {
    ImageHeader header;
    unsigned char *pixels;
} Image;

// Protótipos das funções

Image* ler_pgm(const char *filename);
int escrever_pgm(const char *filename, Image *img);
void aplicar_negativo(Image *img);
void aplicar_limiarizacao(Image *img, int limiar);
void liberar_imagem(Image *img);

#endif 