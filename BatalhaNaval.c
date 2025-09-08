#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* ------------------ Configurações ------------------ */
#define N 10          // tamanho do tabuleiro NxN
#define AGUA   0
#define NAVIO  1
#define ACERTO 2
#define ERRO   3

typedef struct { int linhas, colunas; } Tab;

/* ------------------ Utilitários ------------------ */
int dentro(Tab t, int x, int y) {
    return (x >= 0 && x < t.linhas && y >= 0 && y < t.colunas);
}

void zera(int M[N][N], Tab t) {
    for (int i = 0; i < t.linhas; i++)
        for (int j = 0; j < t.colunas; j++)
            M[i][j] = AGUA;
}

void mostra(int M[N][N], Tab t, int revelar) {
    printf("\n    ");
    for (int j = 0; j < t.colunas; j++) printf("%2d ", j);
    printf("\n");
    for (int i = 0; i < t.linhas; i++) {
        printf("%2d |", i);
        for (int j = 0; j < t.colunas; j++) {
            int v = M[i][j];
            char c = '~';                 // água oculta
            if (v == ERRO)   c = 'o';     // tiro na água
            else if (v == ACERTO) c = 'X';// acerto de navio
            else if (v == NAVIO && revelar) c = 'N';
            printf(" %c ", c);
        }
        printf("|\n");
    }
}

/* ------------------ Frota ------------------ */
/* direções: 0=horizontal, 1=vertical, 2=diag principal, 3=diag secundaria */
int cabe_navio(int M[N][N], Tab t, int x, int y, int tam, int dir, int permitir_diag) {
    int dx = 0, dy = 0;
    if      (dir == 0) dy = 1;
    else if (dir == 1) dx = 1;
    else if (permitir_diag && dir == 2) { dx = 1; dy = 1; }
    else if (permitir_diag && dir == 3) { dx = 1; dy = -1; }
    else return 0;

    for (int k = 0; k < tam; k++) {
        int xi = x + k*dx, yi = y + k*dy;
        if (!dentro(t, xi, yi) || M[xi][yi] != AGUA) return 0;
    }
    return 1;
}

void coloca_navio(int M[N][N], Tab t, int x, int y, int tam, int dir) {
    int dx = 0, dy = 0;
    if      (dir == 0) dy = 1;
    else if (dir == 1) dx = 1;
    else if (dir == 2) { dx = 1; dy = 1; }
    else if (dir == 3) { dx = 1; dy = -1; }

    for (int k = 0; k < tam; k++) {
        int xi = x + k*dx, yi = y + k*dy;
        M[xi][yi] = NAVIO;
    }
}

/* Retorna total de células com navio (para condição de vitória) */
int posiciona_frota(int M[N][N], Tab t, int permitir_diag) {
    // Frota: 1x3, 2x2, 2x1 (7 células)
    int tamanhos[] = {3,2,2,1,1};
    int n = sizeof(tamanhos)/sizeof(int);
    int total = 0; for (int i=0;i<n;i++) total += tamanhos[i];

    int colocados = 0, tentativas = 0;
    while (colocados < n && tentativas < 8000) {
        int tam = tamanhos[colocados];
        int dir = rand() % (permitir_diag ? 4 : 2);
        int x = rand() % N, y = rand() % N;
        if (cabe_navio(M, t, x, y, tam, dir, permitir_diag)) {
            coloca_navio(M, t, x, y, tam, dir);
            colocados++;
        }
        tentativas++;
    }
    return total;
}

/* ------------------ Ataques ------------------ */
void marca_erro(int M[N][N], int x, int y) {
    if (M[x][y] == AGUA) M[x][y] = ERRO;
}

int atacar(int M[N][N], Tab t, int x, int y) {
    if (!dentro(t, x, y)) return 0;
    if (M[x][y] == NAVIO) { M[x][y] = ACERTO; return 1; }
    marca_erro(M, x, y);
    return 0;
}

/* --- Habilidade Cruz (raio R): mesmo x ou y e |dx|+|dy| <= R --- */
void habilidade_cruz(int M[N][N], Tab t, int x, int y, int R) {
    for (int i = x - R; i <= x + R; i++) {
        for (int j = y - R; j <= y + R; j++) {
            if (!dentro(t, i, j)) continue;
            if (i == x || j == y) atacar(M, t, i, j);   // condicional define a CRUZ
        }
    }
}

/* --- Habilidade Octaedro/Losango (raio R): |dx| + |dy| <= R --- */
void habilidade_octaedro(int M[N][N], Tab t, int x, int y, int R) {
    for (int i = x - R; i <= x + R; i++) {
        for (int j = y - R; j <= y + R; j++) {
            if (!dentro(t, i, j)) continue;
            int dx = (i > x) ? (i - x) : (x - i);
            int dy = (j > y) ? (j - y) : (y - j);
            if (dx + dy <= R) atacar(M, t, i, j);       // condicional define o LOSANGO
        }
    }
}

/* --- Habilidade Cone (raio R + direção):
       Norte  : i<=x e (x-i) >= |j-y| e (x-i) <= R
       Sul    : i>=x e (i-x) >= |j-y| e (i-x) <= R
       Leste  : j>=y e (j-y) >= |i-x| e (j-y) <= R
       Oeste  : j<=y e (y-j) >= |i-x| e (y-j) <= R
--------------------------------------------------- */
void habilidade_cone(int M[N][N], Tab t, int x, int y, int R, char dir) {
    for (int i = x - R; i <= x + R; i++) {
        for (int j = y - R; j <= y + R; j++) {
            if (!dentro(t, i, j)) continue;
            int dx = i - x, dy = j - y;
            int ok = 0;
            if (dir == 'N') ok = (dx <= 0) && (-dx >= (dy<0?-dy:dy)) && (-dx <= R);
            if (dir == 'S') ok = (dx >= 0) && ( dx >= (dy<0?-dy:dy)) && ( dx <= R);
            if (dir == 'L') ok = (dy >= 0) && ( dy >= (dx<0?-dx:dx)) && ( dy <= R);
            if (dir == 'O') ok = (dy <= 0) && (-dy >= (dx<0?-dx:dx)) && (-dy <= R);
            if (ok) atacar(M, t, i, j);                 // condicional dentro dos loops define o CONE
        }
    }
}

/* ------------------ Jogo ------------------ */
int restantes_navio(int M[N][N], Tab t) {
    int q = 0;
    for (int i = 0; i < t.linhas; i++)
        for (int j = 0; j < t.colunas; j++)
            if (M[i][j] == NAVIO) q++;
    return q;
}

void menu_habilidades(int modo) {
    printf("\nAções:\n");
    printf("1 - Ataque simples\n");
    if (modo >= 2) printf("2 - Habilidade Cruz (raio 1)\n");
    if (modo >= 3) {
        printf("3 - Habilidade Cone (raio 2, direcao N/S/L/O)\n");
        printf("4 - Habilidade Octaedro (raio 2)\n");
    }
    printf("0 - Sair\n");
}

int main(void) {
    srand((unsigned)time(NULL));
    Tab t = {N, N};
    int M[N][N]; zera(M, t);

    /* Seleção de modo */
    int modo = 0;
    printf("=== Batalha Naval (C) ===\n");
    printf("Selecione o modo:\n");
    printf("1 - Novato (sem diagonal, ataque simples)\n");
    printf("2 - Aventureiro (navios podem ser diagonais, + Cruz)\n");
    printf("3 - Mestre (tudo: Cruz, Cone, Octaedro)\n");
    printf("Modo: ");
    if (scanf("%d", &modo) != 1 || modo < 1 || modo > 3) {
        printf("Modo inválido. Encerrando.\n");
        return 0;
    }

    int permitir_diag = (modo >= 2);
    int total_navio = posiciona_frota(M, t, permitir_diag);

    int vivos = total_navio;
    int opc, x, y;
    char dir;

    while (1) {
        mostra(M, t, 0);
        printf("\nRestantes (células de navio): %d\n", vivos);
        if (vivos == 0) { printf("\n✅ Vitória! Todos os navios foram afundados.\n"); break; }

        menu_habilidades(modo);
        printf("Opção: ");
        if (scanf("%d", &opc) != 1) { printf("Entrada inválida.\n"); return 0; }
        if (opc == 0) { printf("Saindo...\n"); break; }

        /* Leitura de coordenadas para todas as ações */
        printf("Coordenadas (linha coluna): ");
        if (scanf("%d %d", &x, &y) != 2) { printf("Entrada inválida.\n"); return 0; }

        int antes = restantes_navio(M, t);

        if (opc == 1) {
            atacar(M, t, x, y);
        } else if (opc == 2 && modo >= 2) {
            habilidade_cruz(M, t, x, y, 1);
        } else if (opc == 3 && modo >= 3) {
            printf("Direção (N/S/L/O): ");
            scanf(" %c", &dir);
            if (dir!='N' && dir!='S' && dir!='L' && dir!='O' && dir!='n' && dir!='s' && dir!='l' && dir!='o') {
                printf("Direção inválida.\n");
            } else {
                if (dir>='a' && dir<='z') dir -= 32; // minúscula -> maiúscula
                habilidade_cone(M, t, x, y, 2, dir);
            }
        } else if (opc == 4 && modo >= 3) {
            habilidade_octaedro(M, t, x, y, 2);
        } else {
            printf("Opção indisponível neste modo.\n");
        }

        int depois = restantes_navio(M, t);
        if (depois < antes) printf("🎯 Houve acerto(s)!\n");
        else                printf("💦 Sem acertos desta vez.\n");
    }

    printf("\nTabuleiro final (revelado):\n");
    mostra(M, t, 1);
    return 0;
}