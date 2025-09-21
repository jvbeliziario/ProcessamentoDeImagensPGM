#include <stdio.h>
#include <stdlib.h>
#include "gerenciador.h"

void limpar_buffer_entrada() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int pedir_confirmacao(const char *mensagem) {
    printf("%s (1=Sim, 0=Não): ", mensagem);
    int confirma = 0;
    if (scanf("%d", &confirma) != 1) {
        confirma = 0; 
    }
    limpar_buffer_entrada();
    return confirma == 1;
}

void handle_inserir_imagem() {
    char pgm_filename[256];
    char nome_imagem[MAX_FILENAME];

    printf("\nDigite o caminho do arquivo PGM de entrada: ");
    if (scanf("%255s", pgm_filename) != 1) return;
    limpar_buffer_entrada();

    printf("Digite um nome para armazenar a imagem: ");
    if (scanf("%99s", nome_imagem) != 1) return;
    limpar_buffer_entrada();
    
    if (insert_image(pgm_filename, nome_imagem)) {
        printf("Imagem '%s' inserida com sucesso!\n", nome_imagem);
    }
}

void handle_buscar_imagem() {
    char nome_imagem[MAX_FILENAME];
    printf("\nDigite o nome da imagem a buscar: ");
    if (scanf("%99s", nome_imagem) != 1) return;
    limpar_buffer_entrada();

    KeyRecord *key = find_key_by_name(nome_imagem);
    if (key && key->ativo) {
        printf("\n=== INFORMAÇÕES DA IMAGEM ===\n");
        printf("Nome: %s\n", key->nome);
        printf("Dimensões: %d x %d pixels\n", key->colunas, key->linhas);
        printf("Max Gray: %d\n", key->max_gray);
        printf("Tamanho: %d bytes\n", key->tamanho_total);
        printf("Offset no arquivo: %ld\n", key->offset);
        printf("Status: Ativo\n");
    } else {
        printf("Imagem '%s' não encontrada ou está inativa.\n", nome_imagem);
    }
    
    if (key) free(key);
}

void handle_exportar_imagem() {
    char nome_imagem[MAX_FILENAME];
    char output_filename[256];
    int tipo_exportacao = 0;
    int limiar = 128;

    printf("\nDigite o nome da imagem a exportar: ");
    if (scanf("%99s", nome_imagem) != 1) return;
    limpar_buffer_entrada();

    printf("Digite o nome do arquivo PGM de saída: ");
    if (scanf("%255s", output_filename) != 1) return;
    limpar_buffer_entrada();
    
    printf("\nTipo de exportação:\n");
    printf("0 - Original\n1 - Negativo\n2 - Limiarização\nEscolha: ");
    if (scanf("%d", &tipo_exportacao) != 1) return;
    limpar_buffer_entrada();
    
    if (tipo_exportacao == 2) {
        printf("Digite o valor do limiar (0-255): ");
        if (scanf("%d", &limiar) != 1) return;
        limpar_buffer_entrada();
    }
    
    if (export_image(nome_imagem, output_filename, tipo_exportacao, limiar)) {
        printf("Imagem '%s' exportada para '%s' com sucesso!\n", nome_imagem, output_filename);
    }
}

void handle_remover_imagem() {
    char nome_imagem[MAX_FILENAME];
    printf("\nDigite o nome da imagem a remover: ");
    if (scanf("%99s", nome_imagem) != 1) return;
    limpar_buffer_entrada();

    if (pedir_confirmacao("Tem certeza que deseja remover esta imagem?")) {
        if (delete_image(nome_imagem)) {
            printf("Imagem '%s' removida com sucesso.\n", nome_imagem);
            printf("Use a opção de compactação para liberar o espaço em disco.\n");
        } else {
            printf("Erro: Imagem '%s' não foi encontrada ou já está inativa.\n", nome_imagem);
        }
    } else {
        printf("Operação cancelada.\n");
    }
}

void handle_compactar() {
    printf("\nA compactação reorganiza o arquivo de imagens para remover espaços vazios.\n");
    if (pedir_confirmacao("Deseja continuar?")) {
        compact_images();
    } else {
        printf("Operação cancelada.\n");
    }
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
    int opcao = -1;
    
    do {
        mostrar_menu();
        if (scanf("%d", &opcao) != 1) {
            opcao = -1;
        }
        limpar_buffer_entrada();

        switch (opcao) {
            case 1:
                handle_inserir_imagem();
                break;
            case 2:
                list_images();
                break;
            case 3:
                handle_buscar_imagem();
                break;
            case 4:
                handle_exportar_imagem();
                break;
            case 5:
                handle_remover_imagem();
                break;
            case 6:
                handle_compactar();
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

// Compile com: gcc -g main.c gerenciador.c utilitarios_pgm.c -o gerenciador_de_imagens 