#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILENAME 100
#define IMAGES_FILE "images.bin"
#define KEYS_FILE "keys.bin"

typedef struct {
    int linhas;
    int colunas;
    int max_gray;
    size_t tamanho_pixels;
} ImageHeader;

typedef struct {
    char nome[MAX_FILENAME];
    long offset;
    int tamanho_total;
    int linhas;
    int colunas;
    int max_gray;
    int ativo;
} KeyRecord;

typedef struct {
    ImageHeader header;
    unsigned char *pixels;
} Image;

void liberar_imagem(Image *img) {
    if (img) {
        if (img->pixels) {
            free(img->pixels);
        }
        free(img);
    }
}

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

void aplicar_negativo(Image *img) {
    int total_pixels = img->header.tamanho_pixels;
    int max_gray = img->header.max_gray;
    for (int i = 0; i < total_pixels; i++) {
        img->pixels[i] = max_gray - img->pixels[i];
    }
}

void aplicar_limiarizacao(Image *img, int limiar) {
    int total_pixels = img->header.tamanho_pixels;
    int max_gray = img->header.max_gray;
    for (int i = 0; i < total_pixels; i++) {
        img->pixels[i] = (img->pixels[i] >= limiar) ? max_gray : 0;
    }
}

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
    
    printf("Imagem '%s' inserida com sucesso!\n", nome_imagem);
    return 1;
}

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

Image* read_image_from_offset(long offset) {
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
        free(img->pixels);
        free(img);
        fclose(images_file);
        return NULL;
    }
    
    fclose(images_file);
    return img;
}

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
            printf("Aplicando transformação: Negativo\n");
            break;
        case 2:
            aplicar_limiarizacao(img, limiar);
            printf("Aplicando transformação: Limiarização (T=%d)\n", limiar);
            break;
        case 0:
        default:
            printf("Exportando imagem original\n");
            break;
    }
    
    int resultado = escrever_pgm(output_filename, img);
    
    if (resultado) {
        printf("Imagem '%s' exportada para '%s' com sucesso!\n", nome_imagem, output_filename);
    }
    
    liberar_imagem(img);
    free(key);
    return resultado;
}

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
    
    if (encontrado) {
        printf("Imagem '%s' removida com sucesso!\n", nome_imagem);
        printf("Use a opção de compactação para liberar espaço em disco.\n");
        return 1;
    } else {
        printf("Erro: Imagem '%s' não encontrada ou já está inativa\n", nome_imagem);
        return 0;
    }
}

void compact_images() {
    printf("Iniciando compactação...\n");
    
    FILE *keys_file = fopen(KEYS_FILE, "rb");
    if (!keys_file) {
        printf("Erro: Arquivo de chaves não encontrado\n");
        return;
    }
    
    fseek(keys_file, 0, SEEK_END);
    int num_keys = ftell(keys_file) / sizeof(KeyRecord);
    rewind(keys_file);

    if (num_keys == 0) {
        printf("Nenhuma imagem para compactar.\n");
        fclose(keys_file);
        return;
    }

    KeyRecord* all_keys = malloc(num_keys * sizeof(KeyRecord));
    if (!all_keys || fread(all_keys, sizeof(KeyRecord), num_keys, keys_file) != num_keys) {
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

                memmove(&all_keys[active_keys_count], &all_keys[i], sizeof(KeyRecord));
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

void mostrar_menu() {
    printf("\n==================== GERENCIADOR DE IMAGENS PGM ====================\n");
    printf("1. Inserir nova imagem\n");
    printf("2. Listar todas as imagens\n");
    printf("3. Buscar imagem por nome\n");
    printf("4. Exportar imagem\n");
    printf("5. Remover imagem\n");
    printf("6. Compactar arquivo de imagens\n");
    printf("0. Sair\n");
    printf("=====================================================================\n");
    printf("Escolha uma opção: ");
}

int main() {
    int opcao;
    char pgm_filename[256], nome_imagem[MAX_FILENAME], output_filename[256];
    int tipo_exportacao, limiar;
    KeyRecord *key;
    
    do {
        mostrar_menu();
        
        char input_buffer[16];
        if (scanf("%15s", input_buffer) != 1) {
             while (getchar() != '\n'); 
             opcao = -1; 
        } else {
             opcao = atoi(input_buffer);
        }
        
        while (getchar() != '\n'); 

        switch (opcao) {
            case 1:
                printf("\nDigite o caminho do arquivo PGM: ");
                scanf("%255s", pgm_filename);
                while (getchar() != '\n');
                printf("Digite um nome para a imagem: ");
                scanf("%99s", nome_imagem);
                while (getchar() != '\n');
                insert_image(pgm_filename, nome_imagem);
                break;
                
            case 2:
                list_images();
                break;
                
            case 3:
                printf("\nDigite o nome da imagem: ");
                scanf("%99s", nome_imagem);
                while (getchar() != '\n');
                
                key = find_key_by_name(nome_imagem);
                if (key && key->ativo) {
                    printf("\n=== INFORMAÇÕES DA IMAGEM ===\n");
                    printf("Nome: %s\n", key->nome);
                    printf("Dimensões: %d x %d pixels\n", key->colunas, key->linhas);
                    printf("Max Gray: %d\n", key->max_gray);
                    printf("Tamanho: %d bytes\n", key->tamanho_total);
                    printf("Offset no arquivo: %ld\n", key->offset);
                    printf("Status: %s\n", key->ativo ? "Ativo" : "Inativo");
                } else {
                    printf("Imagem '%s' não encontrada ou está inativa.\n", nome_imagem);
                }
                
                if (key) free(key);
                break;
                
            case 4:
                printf("\nDigite o nome da imagem: ");
                scanf("%99s", nome_imagem);
                 while (getchar() != '\n');
                printf("Digite o nome do arquivo de saída: ");
                scanf("%255s", output_filename);
                 while (getchar() != '\n');
                
                printf("\nTipo de exportação:\n0 - Original\n1 - Negativo\n2 - Limiarização\nEscolha: ");
                scanf("%d", &tipo_exportacao);
                 while (getchar() != '\n');
                
                if (tipo_exportacao == 2) {
                    printf("Digite o valor do limiar (0-255): ");
                    scanf("%d", &limiar);
                     while (getchar() != '\n');
                } else {
                    limiar = 0;
                }
                
                export_image(nome_imagem, output_filename, tipo_exportacao, limiar);
                break;
                
            case 5:
                printf("\nDigite o nome da imagem a remover: ");
                scanf("%99s", nome_imagem);
                while (getchar() != '\n');
                
                printf("Tem certeza que deseja remover '%s'? (1=Sim, 0=Não): ", nome_imagem);
                int confirma;
                scanf("%d", &confirma);
                 while (getchar() != '\n');
                
                if (confirma == 1) {
                    delete_image(nome_imagem);
                } else {
                    printf("Operação cancelada.\n");
                }
                break;
                
            case 6:
                printf("\nEsta operação irá reorganizar o arquivo de imagens.\n");
                printf("Tem certeza? (1=Sim, 0=Não): ");
                int confirma_compact;
                scanf("%d", &confirma_compact);
                while (getchar() != '\n');
                
                if (confirma_compact == 1) {
                    compact_images();
                } else {
                    printf("Operação cancelada.\n");
                }
                break;
                
            case 0:
                printf("\nSaindo do programa...\n");
                break;
                
            default:
                printf("\nOpção inválida! Tente novamente.\n");
                break;
        }
        
        if (opcao != 0) {
            printf("\nPressione Enter para continuar...");
            getchar();
        }
        
    } while (opcao != 0);
    
    return 0;
}