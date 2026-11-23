#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_FERRAMENTAS 200
#define TAM_TEXTO 80
#define ARQUIVO_ESTOQUE "ferralog_estoque.dat"
#define ARQUIVO_HISTORICO "ferralog_historico.txt"

typedef struct {
    int codigo;
    char nome[TAM_TEXTO];
    char setor[TAM_TEXTO];
    int quantidade;
    int emprestadas;
    float preco;
    int garantia;
} Ferramenta;

Ferramenta estoque[MAX_FERRAMENTAS];
int total = 0;

void lerTexto(const char *mensagem, char *destino, int tamanho) {
    int c;
    printf("%s", mensagem);
    if (fgets(destino, tamanho, stdin) == NULL) {
        puts("Entrada encerrada.");
        exit(0);
    }
    if (strchr(destino, '\n') != NULL) {
        destino[strcspn(destino, "\n")] = '\0';
    } else {
        while ((c = getchar()) != '\n' && c != EOF) { }
    }
}

int lerInteiro(const char *mensagem, int minimo, int maximo) {
    char entrada[100], extra;
    int valor;
    while (1) {
        lerTexto(mensagem, entrada, sizeof(entrada));
        if (sscanf(entrada, " %d %c", &valor, &extra) == 1 && valor >= minimo && valor <= maximo)
            return valor;
        printf("Entrada invalida. Informe um numero entre %d e %d.\n", minimo, maximo);
    }
}

float lerPreco(const char *mensagem) {
    char entrada[100], extra;
    float valor;
    while (1) {
        lerTexto(mensagem, entrada, sizeof(entrada));
        for (int i = 0; entrada[i] != '\0'; i++)
            if (entrada[i] == ',') entrada[i] = '.';
        if (sscanf(entrada, " %f %c", &valor, &extra) == 1 && valor >= 0 && valor <= 100000000)
            return valor;
        puts("Valor invalido. Informe um preco positivo ou zero.");
    }
}

void lerTextoObrigatorio(const char *mensagem, char *destino, int tamanho) {
    do {
        lerTexto(mensagem, destino, tamanho);
        if (destino[0] == '\0') puts("Este campo nao pode ficar vazio.");
    } while (destino[0] == '\0');
}

int buscarIndice(int codigo) {
    for (int i = 0; i < total; i++) {
        if (estoque[i].codigo == codigo) return i;
    }
    return -1;
}

void salvarEstoque(void) {
    FILE *arquivo = fopen(ARQUIVO_ESTOQUE, "wb");
    if (arquivo == NULL) {
        puts("ERRO: nao foi possivel salvar o estoque.");
        return;
    }
    if (fwrite(&total, sizeof(total), 1, arquivo) != 1 ||
        fwrite(estoque, sizeof(Ferramenta), (size_t)total, arquivo) != (size_t)total)
        puts("ERRO: falha na gravacao do estoque.");
    fclose(arquivo);
}

void carregarEstoque(void) {
    FILE *arquivo = fopen(ARQUIVO_ESTOQUE, "rb");
    if (arquivo == NULL) return;
    int quantidade = 0;
    if (fread(&quantidade, sizeof(quantidade), 1, arquivo) != 1 ||
        quantidade < 0 || quantidade > MAX_FERRAMENTAS ||
        fread(estoque, sizeof(Ferramenta), (size_t)quantidade, arquivo) != (size_t)quantidade) {
        puts("Aviso: arquivo de estoque invalido. Nenhum registro foi carregado.");
        total = 0;
    } else {
        total = quantidade;
    }
    fclose(arquivo);
}

void registrarMovimento(const char *tipo, Ferramenta ferramenta, int quantidade, const char *responsavel) {
    FILE *arquivo = fopen(ARQUIVO_HISTORICO, "a");
    if (arquivo == NULL) {
        puts("Aviso: nao foi possivel gravar o historico.");
        return;
    }
    time_t agora = time(NULL);
    struct tm *data = localtime(&agora);
    char horario[40] = "data indisponivel";
    if (data != NULL) strftime(horario, sizeof(horario), "%d/%m/%Y %H:%M", data);
    fprintf(arquivo, "%s | %s | Codigo: %d | %s | Qtd: %d | Responsavel: %s\n",
            horario, tipo, ferramenta.codigo, ferramenta.nome, quantidade, responsavel);
    fclose(arquivo);
}

void mostrarFerramenta(Ferramenta f) {
    printf("\nCodigo: %d\nNome: %s\nSetor: %s\nDisponiveis: %d\nEmprestadas: %d\n",
           f.codigo, f.nome, f.setor, f.quantidade, f.emprestadas);
    printf("Preco unitario: R$ %.2f\nGarantia: %s\n", f.preco, f.garantia ? "Sim" : "Nao");
}

void cadastrar(void) {
    if (total >= MAX_FERRAMENTAS) {
        puts("Limite de ferramentas atingido.");
        return;
    }
    Ferramenta f;
    puts("\n--- CADASTRAR FERRAMENTA ---");
    f.codigo = lerInteiro("Codigo numerico (1 a 999999): ", 1, 999999);
    if (buscarIndice(f.codigo) != -1) {
        puts("Ja existe uma ferramenta com esse codigo.");
        return;
    }
    lerTextoObrigatorio("Nome: ", f.nome, TAM_TEXTO);
    lerTextoObrigatorio("Setor: ", f.setor, TAM_TEXTO);
    f.quantidade = lerInteiro("Quantidade inicial: ", 0, 1000000);
    f.emprestadas = 0;
    f.preco = lerPreco("Preco unitario (R$): ");
    f.garantia = lerInteiro("Possui garantia? (1=Sim, 0=Nao): ", 0, 1);
    estoque[total++] = f;
    salvarEstoque();
    registrarMovimento("CADASTRO", f, f.quantidade, "Sistema");
    puts("Ferramenta cadastrada com sucesso!");
}

void listar(void) {
    puts("\n--- FERRAMENTAS CADASTRADAS ---");
    if (total == 0) {
        puts("Nenhuma ferramenta cadastrada.");
        return;
    }
    for (int i = 0; i < total; i++) {
        printf("%d) [%d] %s | Setor: %s | Disponivel: %d | Emprestadas: %d\n",
               i + 1, estoque[i].codigo, estoque[i].nome, estoque[i].setor,
               estoque[i].quantidade, estoque[i].emprestadas);
    }
}

void consultar(void) {
    puts("\n--- CONSULTAR FERRAMENTA ---");
    int codigo = lerInteiro("Codigo: ", 1, 999999);
    int indice = buscarIndice(codigo);
    if (indice == -1) puts("Ferramenta nao encontrada.");
    else mostrarFerramenta(estoque[indice]);
}

void retirar(void) {
    puts("\n--- RETIRADA DE FERRAMENTA ---");
    int codigo = lerInteiro("Codigo: ", 1, 999999);
    int indice = buscarIndice(codigo);
    if (indice == -1) {
        puts("Ferramenta nao encontrada.");
        return;
    }
    Ferramenta *f = &estoque[indice];
    mostrarFerramenta(*f);
    if (f->quantidade == 0) {
        puts("Sem unidades disponiveis para retirada.");
        return;
    }
    int quantidade = lerInteiro("Quantidade para retirar: ", 1, f->quantidade);
    char responsavel[TAM_TEXTO];
    lerTextoObrigatorio("Nome de quem retirou: ", responsavel, TAM_TEXTO);
    f->quantidade -= quantidade;
    f->emprestadas += quantidade;
    salvarEstoque();
    registrarMovimento("RETIRADA", *f, quantidade, responsavel);
    printf("Retirada realizada! Restam %d unidade(s) disponiveis.\n", f->quantidade);
}

void devolver(void) {
    puts("\n--- DEVOLUCAO DE FERRAMENTA ---");
    int codigo = lerInteiro("Codigo: ", 1, 999999);
    int indice = buscarIndice(codigo);
    if (indice == -1) {
        puts("Ferramenta nao encontrada.");
        return;
    }
    Ferramenta *f = &estoque[indice];
    mostrarFerramenta(*f);
    if (f->emprestadas == 0) {
        puts("Nao ha unidades emprestadas para devolver.");
        return;
    }
    int quantidade = lerInteiro("Quantidade para devolver: ", 1, f->emprestadas);
    char responsavel[TAM_TEXTO];
    lerTextoObrigatorio("Nome de quem devolveu: ", responsavel, TAM_TEXTO);
    f->quantidade += quantidade;
    f->emprestadas -= quantidade;
    salvarEstoque();
    registrarMovimento("DEVOLUCAO", *f, quantidade, responsavel);
    printf("Devolucao realizada! Disponiveis: %d unidade(s).\n", f->quantidade);
}

void historico(void) {
    puts("\n--- HISTORICO DE MOVIMENTACOES ---");
    FILE *arquivo = fopen(ARQUIVO_HISTORICO, "r");
    if (arquivo == NULL) {
        puts("Nenhuma movimentacao registrada.");
        return;
    }
    char linha[400];
    while (fgets(linha, sizeof(linha), arquivo) != NULL) printf("%s", linha);
    fclose(arquivo);
}

void relatorio(void) {
    puts("\n--- RELATORIO DO ESTOQUE ---");
    int disponiveis = 0, emprestadas = 0, semEstoque = 0;
    float valorDisponivel = 0;
    for (int i = 0; i < total; i++) {
        disponiveis += estoque[i].quantidade;
        emprestadas += estoque[i].emprestadas;
        valorDisponivel += estoque[i].preco * estoque[i].quantidade;
        if (estoque[i].quantidade == 0) semEstoque++;
    }
    printf("Tipos de ferramentas: %d\n", total);
    printf("Unidades disponiveis: %d\n", disponiveis);
    printf("Unidades emprestadas: %d\n", emprestadas);
    printf("Ferramentas sem estoque: %d\n", semEstoque);
    printf("Valor estimado do estoque disponivel: R$ %.2f\n", valorDisponivel);
}

void mostrarMenu(void) {
    puts("\n================================================");
    puts("          FERRALOG - SISTEMA DE GESTAO");
    puts("================================================");
    puts("      Controle e Logistica de Ferramentas\n");
    puts("[1] Cadastrar nova ferramenta");
    puts("[2] Listar ferramentas cadastradas");
    puts("[3] Consultar ferramenta");
    puts("[4] Retirar ferramenta do estoque");
    puts("[5] Devolver ferramenta");
    puts("[6] Consultar movimentacoes");
    puts("[7] Relatorio de estoque");
    puts("[8] Sair do sistema");
    puts("================================================");
}

int main(void) {
    int opcao;
    carregarEstoque();
    puts("Iniciando Ferralog...");
    do {
        mostrarMenu();
        opcao = lerInteiro("Digite a opcao desejada: ", 1, 8);
        switch (opcao) {
            case 1: cadastrar(); break;
            case 2: listar(); break;
            case 3: consultar(); break;
            case 4: retirar(); break;
            case 5: devolver(); break;
            case 6: historico(); break;
            case 7: relatorio(); break;
            case 8: puts("Encerrando Ferralog. Ate logo!"); break;
        }
    } while (opcao != 8);
    return 0;
}
