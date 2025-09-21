#include "gerenciador.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Função para ler uma imagem do arquivo de imagens a partir do offset
static Image* read_image_from_offset(long offset) {
    FILE *images_file = fopen(IMAGES_FILE, "rb");
    if (!images_file) {
        printf("Erro: Não foi possível abrir %s\n", IMAGES_FILE);
        return NULL;
    }
    
    if (fseek(images_file, offset, SEEK_SET) != 0) {
        printf("Erro: Falha ao posicionar no arquivo\n");
        fclose(images_file);
        return NULL;
    }
    
    Image *img = (Image*)malloc(sizeof(Image));
    if (!img) {
        printf("Erro: Falha na alocação de memória\n");
        fclose(images_file);
        return NULL;
    }
    
    if (fread(&img->header, sizeof(ImageHeader), 1, images_file) != 1) {
        printf("Erro: Falha ao ler cabeçalho da imagem\n");
        free(img);
        fclose(images_file);
        return NULL;
    }
    
    img->pixels = (unsigned char*)malloc(img->header.tamanho_pixels);
    if (!img->pixels) {
        printf("Erro: Falha na alocação de memória para pixels\n");
        free(img);
        fclose(images_file);
        return NULL;
    }
    
    if (fread(img->pixels, 1, img->header.tamanho_pixels, images_file) != img->header.tamanho_pixels) {
        printf("Erro: Falha ao ler pixels da imagem\n");
        liberar_imagem(img);
        fclose(images_file);
        return NULL;
    }
    
    fclose(images_file);
    return img;
}

// Função para encontrar um registro de chave pelo nome
KeyRecord* find_key_by_name(const char *nome) {
    FILE *keys_file = fopen(KEYS_FILE, "rb");
    if (!keys_file) {
        return NULL;
    }
    
    KeyRecord *key = (KeyRecord*)malloc(sizeof(KeyRecord));
    if (!key) {
        fclose(keys_file);
        return NULL;
    }
    
    while (fread(key, sizeof(KeyRecord), 1, keys_file) == 1) {
        if (strcmp(key->nome, nome) == 0) {
            fclose(keys_file);
            return key;
        }
    }
    
    fclose(keys_file);
    free(key);
    return NULL;
}

// Função para inserir uma nova imagem
int insert_image(const char *pgm_filename, const char *nome_imagem) {
    KeyRecord *existing = find_key_by_name(nome_imagem);
    if (existing && existing->ativo) {
        printf("Erro: Já existe uma imagem ativa com o nome '%s'\n", nome_imagem);
        free(existing);
        return 0;
    }
    if (existing) free(existing);
    
    Image *img = ler_pgm(pgm_filename);
    if (!img) {
        return 0;
    }
    
    FILE *images_file = fopen(IMAGES_FILE, "ab");
    if (!images_file) {
        printf("Erro: Não foi possível abrir %s\n", IMAGES_FILE);
        liberar_imagem(img);
        return 0;
    }
    
    fseek(images_file, 0, SEEK_END);
    long offset = ftell(images_file);
    
    if (fwrite(&img->header, sizeof(ImageHeader), 1, images_file) != 1) {
        printf("Erro: Falha ao escrever cabeçalho da imagem\n");
        fclose(images_file);
        liberar_imagem(img);
        return 0;
    }
    
    if (fwrite(img->pixels, 1, img->header.tamanho_pixels, images_file) != img->header.tamanho_pixels) {
        printf("Erro: Falha ao escrever pixels da imagem\n");
        fclose(images_file);
        liberar_imagem(img);
        return 0;
    }
    
    fclose(images_file);
    
    KeyRecord key;
    strncpy(key.nome, nome_imagem, MAX_FILENAME - 1);
    key.nome[MAX_FILENAME - 1] = '\0';
    key.offset = offset;
    key.tamanho_total = sizeof(ImageHeader) + img->header.tamanho_pixels;
    key.linhas = img->header.linhas;
    key.colunas = img->header.colunas;
    key.max_gray = img->header.max_gray;
    key.ativo = 1;
    
    FILE *keys_file = fopen(KEYS_FILE, "ab");
    if (!keys_file) {
        printf("Erro: Não foi possível abrir %s\n", KEYS_FILE);
        liberar_imagem(img);
        return 0;
    }
    
    if (fwrite(&key, sizeof(KeyRecord), 1, keys_file) != 1) {
        printf("Erro: Falha ao escrever registro de chave\n");
        fclose(keys_file);
        liberar_imagem(img);
        return 0;
    }
    
    fclose(keys_file);
    liberar_imagem(img);

    return 1;
}

//Listar imagens
void list_images() {
    FILE *keys_file = fopen(KEYS_FILE, "rb");
    if (!keys_file) {
        printf("Nenhuma imagem encontrada. Arquivo de chaves não existe.\n");
        return;
    }
    
    KeyRecord key;
    int count = 0;
    
    printf("\n==================== LISTA DE IMAGENS ====================\n");
    printf("%-20s %-10s %-10s %-8s %-10s\n", "Nome", "Linhas", "Colunas", "Max Gray", "Tamanho (bytes)");
    printf("----------------------------------------------------------\n");
    
    while (fread(&key, sizeof(KeyRecord), 1, keys_file) == 1) {
        if (key.ativo) {
            printf("%-20s %-10d %-10d %-8d %-10d\n", 
                   key.nome, key.linhas, key.colunas, key.max_gray, key.tamanho_total);
            count++;
        }
    }
    
    if (count == 0) {
        printf("Nenhuma imagem ativa encontrada.\n");
    } else {
        printf("----------------------------------------------------------\n");
        printf("Total: %d imagens ativas\n", count);
    }
    
    fclose(keys_file);
}

//Exportar imagens com transformações
int export_image(const char *nome_imagem, const char *output_filename, int tipo_exportacao, int limiar) {
    KeyRecord *key = find_key_by_name(nome_imagem);
    if (!key || !key->ativo) {
        printf("Erro: Imagem '%s' não encontrada ou está inativa\n", nome_imagem);
        if (key) free(key);
        return 0;
    }
    
    Image *img = read_image_from_offset(key->offset);
    if (!img) {
        free(key);
        return 0;
    }
    
    switch (tipo_exportacao) {
        case 1:
            aplicar_negativo(img);
            break;
        case 2:
            aplicar_limiarizacao(img, limiar);
            break;
    }
    
    int resultado = escrever_pgm(output_filename, img);
    
    liberar_imagem(img);
    free(key);
    return resultado;
}

//Compactar imagens
void compact_images() {
    FILE *keys_file = fopen(KEYS_FILE, "rb");
    if (!keys_file) {
        printf("Erro: Arquivo de chaves não encontrado\n");
        return;
    }
    
    fseek(keys_file, 0, SEEK_END);
    long num_keys_long = ftell(keys_file);
    if (num_keys_long <= 0) {
        fclose(keys_file);
        printf("Nenhuma imagem para compactar.\n");
        return;
    }
    int num_keys = num_keys_long / sizeof(KeyRecord);
    rewind(keys_file);

    KeyRecord* all_keys = malloc(num_keys * sizeof(KeyRecord));
    if(!all_keys || fread(all_keys, sizeof(KeyRecord), num_keys, keys_file) != num_keys) {
        printf("Erro ao ler registros de chaves.\n");
        fclose(keys_file);
        if(all_keys) free(all_keys);
        return;
    }
    fclose(keys_file);

    FILE *new_images = fopen("images_temp.bin", "wb");
    if (!new_images) {
        printf("Erro: Não foi possível criar arquivo temporário\n");
        free(all_keys);
        return;
    }
    
    long new_offset = 0;
    int active_keys_count = 0;
    for (int i = 0; i < num_keys; i++) {
        if (all_keys[i].ativo) {
            Image *img = read_image_from_offset(all_keys[i].offset);
            if (img) {
                all_keys[i].offset = new_offset;
                
                fwrite(&img->header, sizeof(ImageHeader), 1, new_images);
                fwrite(img->pixels, 1, img->header.tamanho_pixels, new_images);
                
                new_offset += all_keys[i].tamanho_total;
                liberar_imagem(img);

                if (i != active_keys_count) {
                    all_keys[active_keys_count] = all_keys[i];
                }
                active_keys_count++;
            }
        }
    }
    
    fclose(new_images);
    
    remove(IMAGES_FILE);
    rename("images_temp.bin", IMAGES_FILE);
    
    keys_file = fopen(KEYS_FILE, "wb");
    if (keys_file) {
        fwrite(all_keys, sizeof(KeyRecord), active_keys_count, keys_file);
        fclose(keys_file);
    }
    
    free(all_keys);
    printf("Compactação concluída! %d imagens mantidas.\n", active_keys_count);
}

//Remover imagens
int delete_image(const char *nome_imagem) {
    FILE *keys_file = fopen(KEYS_FILE, "r+b");
    if (!keys_file) {
        printf("Erro: Não foi possível abrir %s\n", KEYS_FILE);
        return 0;
    }
    
    KeyRecord key;
    long pos = 0;
    int encontrado = 0;
    
    while (fread(&key, sizeof(KeyRecord), 1, keys_file) == 1) {
        if (strcmp(key.nome, nome_imagem) == 0 && key.ativo) {
            key.ativo = 0;
            fseek(keys_file, pos, SEEK_SET);
            fwrite(&key, sizeof(KeyRecord), 1, keys_file);
            encontrado = 1;
            break;
        }
        pos = ftell(keys_file);
    }
    
    fclose(keys_file);
    return encontrado;
    compact_images();
}

