#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

#define SHM_NAME "/shm_matriz_c"

// Dimensões das matrizes A (N x M) e B (M x P)
#define N 2
#define M 3
#define P 2

int main() {
    // Matrizes de entrada estáticas para simplificar o exemplo
    int A[N][M] = {
        {1, 2, 3},
        {4, 5, 6}
    };
    int B[M][P] = {
        {7, 8},
        {9, 10},
        {11, 12}
    };

    // O tamanho da memória compartilhada será o tamanho da matriz C (N x P)
    size_t shm_size = N * P * sizeof(int);

    // 1. Criar o objeto de memória compartilhada
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Erro no shm_open");
        exit(1);
    }

    // 2. Configurar o tamanho da região de memória
    if (ftruncate(shm_fd, shm_size) == -1) {
        perror("Erro no ftruncate");
        exit(1);
    }

    // 3. Mapear a memória compartilhada para o espaço de endereçamento do processo
    // Trataremos a matriz 2D como um array 1D contíguo na memória
    int *C = mmap(0, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (C == MAP_FAILED) {
        perror("Erro no mmap");
        exit(1);
    }

    printf("Criando processos filhos para calcular a matriz C (%dx%d)...\n\n", N, P);

    // 4. Criar um processo filho para CADA elemento da matriz resultado
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < P; j++) {
            pid_t pid = fork();

            if (pid == -1) {
                perror("Erro no fork");
                exit(1);
            } 
            else if (pid == 0) {
                // --- CÓDIGO DO PROCESSO FILHO ---
                int soma = 0;
                for (int k = 0; k < M; k++) {
                    soma += A[i][k] * B[k][j];
                }
                
                // Escreve o resultado diretamente na posição correspondente
                // C[i][j] mapeado para 1D: (i * P) + j
                C[i * P + j] = soma; 
                
                // O filho termina após calcular seu único elemento
                exit(0); 
            }
        }
    }

    // 5. Código do processo pai: aguardar todos os filhos terminarem
    int total_filhos = N * P;
    for (int i = 0; i < total_filhos; i++) {
        wait(NULL);
    }

    // 6. Imprimir a matriz resultado completa
    printf("Matriz Resultado C:\n");
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < P; j++) {
            printf("%d\t", C[i * P + j]);
        }
        printf("\n");
    }

    // 7. Limpeza e remoção da memória compartilhada
    munmap(C, shm_size);
    close(shm_fd);
    shm_unlink(SHM_NAME);

    return 0;
}