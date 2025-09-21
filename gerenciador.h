#ifndef GERENCIADOR_H
#define GERENCIADOR_H

#include "utilitarios_pgm.h"

#define MAX_FILENAME 100
#define IMAGES_FILE "images.bin"
#define KEYS_FILE "keys.bin"

typedef struct {
    char nome[MAX_FILENAME];
    long offset;
    int tamanho_total;
    int linhas;
    int colunas;
    int max_gray;
    int ativo;
} KeyRecord;

// Funções de gerenciamento das imagens
int insert_image(const char *pgm_filename, const char *nome_imagem);
void list_images();
KeyRecord* find_key_by_name(const char *nome);
int export_image(const char *nome_imagem, const char *output_filename, int tipo_exportacao, int limiar);
int delete_image(const char *nome_imagem);
void compact_images();

#endif 