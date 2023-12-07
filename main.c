#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>

int largura = 50, altura = 30;
int x, y, frutaX, frutaY, pontuacao;
enum eDirecao { PARADO = 0, ESQUERDA, DIREITA, CIMA, BAIXO };
enum eDirecao dir;
int fimDeJogo;
int velocidade = 150000; 
int modoParedeInfinita = 0; 

struct NoCauda {
    int x;
    int y;
    struct NoCauda *proximo;
};

struct NoCauda *inicioCauda = NULL;
struct NoCauda *fimCauda = NULL;

void adicionarNoCauda(int x, int y) {
    struct NoCauda *novoNo = (struct NoCauda*)malloc(sizeof(struct NoCauda));
    novoNo->x = x;
    novoNo->y = y;
    novoNo->proximo = NULL;

    if (inicioCauda == NULL) {
        inicioCauda = novoNo;
        fimCauda = novoNo;
    } else {
        fimCauda->proximo = novoNo;
        fimCauda = novoNo;
    }
}

void removerPrimeiroNoCauda() {
    if (inicioCauda != NULL) {
        struct NoCauda *noARemover = inicioCauda;
        inicioCauda = inicioCauda->proximo;
        free(noARemover);
    }
}

void imprimirCor(char* codigoCor, char caractere) {
    printf("%s%c\033[0m", codigoCor, caractere);
}

int teclaPressionada() {
    struct termios velho, novo;
    int ch;
    int velhof;

    tcgetattr(STDIN_FILENO, &velho);
    novo = velho;
    novo.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &novo);
    velhof = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, velhof | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &velho);
    fcntl(STDIN_FILENO, F_SETFL, velhof);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

int posicaoFrutaValida(int fx, int fy) {
    struct NoCauda *atual = inicioCauda;
    while (atual != NULL) {
        if (atual->x == fx && atual->y == fy) {
            return 0;
        }
        atual = atual->proximo;
    }
    return 1;
}

void novaPosicaoFruta() {
    do {
        frutaX = rand() % largura;
        frutaY = rand() % altura;
    } while (!posicaoFrutaValida(frutaX, frutaY));
}

void configurar() {
    fimDeJogo = 0;
    dir = PARADO;
    x = largura / 2;
    y = altura / 2;
    pontuacao = 0;
    while (inicioCauda != NULL) {
        removerPrimeiroNoCauda();
    }
    srand(time(NULL));
    novaPosicaoFruta(); 
}

void desenhar() {
    printf("\033[H"); 
    for (int i = 0; i < altura; i++) {
        for (int j = 0; j < largura; j++) {
            if (j == 0 || j == largura - 1 || i == 0 || i == altura - 1)
                imprimirCor("\033[1;34m", '#'); 
            else if (i == y && j == x)
                imprimirCor("\033[1;32m", 'O'); 
            else if (i == frutaY && j == frutaX)
                imprimirCor("\033[1;31m", '*'); 
            else {
                int isCauda = 0;
                struct NoCauda *atual = inicioCauda;
                while (atual != NULL) {
                    if (atual->x == j && atual->y == i) {
                        imprimirCor("\033[0;32m", 'o');
                        isCauda = 1;
                        break;
                    }
                    atual = atual->proximo;
                }
                if (!isCauda) printf(" ");
            }
        }
        printf("\n");
    }
    printf("Pontuação: %d\n", pontuacao);
}

void entrada() {
    if (teclaPressionada()) {
        char tecla = getchar();
        switch (tecla) {
            case 'a':
                if (dir != DIREITA) dir = ESQUERDA;
                break;
            case 'd':
                if (dir != ESQUERDA) dir = DIREITA;
                break;
            case 'w':
                if (dir != BAIXO) dir = CIMA;
                break;
            case 's':
                if (dir != CIMA) dir = BAIXO;
                break;
            case 'x':
                fimDeJogo = 1;
                break;
        }
    }
}

void logica() {
    int prevX = x;
    int prevY = y;

    switch (dir) {
        case ESQUERDA:
            x--;
            break;
        case DIREITA:
            x++;
            break;
        case CIMA:
            y--;
            break;
        case BAIXO:
            y++;
            break;
        default:
            break;
    }

    if (modoParedeInfinita) {
        if (x >= largura) x = 0; else if (x < 0) x = largura - 1;
        if (y >= altura) y = 0; else if (y < 0) y = altura - 1;
    } else {
        if (x >= largura || x < 0 || y >= altura || y < 0) fimDeJogo = 1;
    }

    struct NoCauda *atual = inicioCauda;
    while (atual != NULL) {
        if (atual->x == x && atual->y == y) fimDeJogo = 1;
        atual = atual->proximo;
    }

    if (x == frutaX && y == frutaY) {
        pontuacao += 10;
        adicionarNoCauda(prevX, prevY);
        if (velocidade > 100000) {
            velocidade -= 20000;
        }
        novaPosicaoFruta();
    } else if (dir != PARADO) {
        adicionarNoCauda(prevX, prevY);
        removerPrimeiroNoCauda();
    }
}

void escolherModoJogo() {
    printf("Bem-vindo ao Jogo da Cobra!\n");
    printf("Escolha o Modo de Jogo:\n");
    printf("1. Parede Infinita\n");
    printf("2. Parede Limitada\n");
    printf("Digite sua escolha e pressione Enter: ");

    char escolha;
    scanf(" %c", &escolha);

    if (escolha == '1') {
        modoParedeInfinita = 1;
    } else {
        modoParedeInfinita = 0;
    }
}

int main() {
    escolherModoJogo();
    configurar();
    while (!fimDeJogo) {
        desenhar();
        entrada();
        logica();
        usleep(velocidade);
    }

    printf("\033[%d;%dH", altura + 2, 0); 
    printf("Fim de jogo! Sua pontuação foi: %d\n", pontuacao);
    return 0;
}
