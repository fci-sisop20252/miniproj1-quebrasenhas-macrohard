#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include "hash_utils.h"

/**
 * PROCESSO COORDENADOR - Mini-Projeto 1: Quebra de Senhas Paralelo
 *
 * Este programa coordena múltiplos workers para quebrar senhas MD5 em paralelo.
 * O MD5 JÁ ESTÁ IMPLEMENTADO - você deve focar na paralelização (fork/exec/wait).
 *
 * Uso: ./coordinator <hash_md5> <tamanho> <charset> <num_workers>
 *
 * Exemplo: ./coordinator "900150983cd24fb0d6963f7d28e17f72" 3 "abc" 4
 *
 * SEU TRABALHO: Implementar os TODOs marcados abaixo
 */

#define MAX_WORKERS 16
#define RESULT_FILE "password_found.txt"

/**
 * Calcula o tamanho total do espaço de busca
 *
 * @param charset_len Tamanho do conjunto de caracteres
 * @param password_len Comprimento da senha
 * @return Número total de combinações possíveis
 */
long long calculate_search_space(int charset_len, int password_len)
{
    long long total = 1;
    for (int i = 0; i < password_len; i++)
    {
        total *= charset_len;
    }
    return total;
}

/**
 * Converte um índice numérico para uma senha
 * Usado para definir os limites de cada worker
 *
 * @param index Índice numérico da senha
 * @param charset Conjunto de caracteres
 * @param charset_len Tamanho do conjunto
 * @param password_len Comprimento da senha
 * @param output Buffer para armazenar a senha gerada
 */
void index_to_password(long long index, const char *charset, int charset_len,
                       int password_len, char *output)
{
    for (int i = password_len - 1; i >= 0; i--)
    {
        output[i] = charset[index % charset_len];
        index /= charset_len;
    }
    output[password_len] = '\0';
}

/**
 * Função principal do coordenador
 */
int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        fprintf(stderr,
                "Uso: %s <hash_md5> <tamanho> <charset> <num_workers>\n"
                "Exemplo: %s \"900150983cd24fb0d6963f7d28e17f72\" 3 \"abc\" 4\n",
                argv[0], argv[0]);
        return 1;
    }

    // Parsing dos argumentos (após validação)
    const char *target_hash = argv[1];
    int password_len = atoi(argv[2]);
    const char *charset = argv[3];
    int num_workers = atoi(argv[4]);
    int charset_len = strlen(charset);

    if (password_len < 1 || password_len > 10)
    {
        fprintf(stderr, "Erro: <tamanho> deve estar entre 1 e 10.\n");
        return 1;
    }
    if (charset_len <= 0)
    {
        fprintf(stderr, "Erro: <charset> não pode ser vazio.\n");
        return 1;
    }
    if (num_workers < 1 || num_workers > MAX_WORKERS)
    {
        fprintf(stderr, "Erro: <num_workers> deve estar entre 1 e %d.\n", MAX_WORKERS);
        return 1;
    }

    printf("=== Mini-Projeto 1: Quebra de Senhas Paralelo ===\n");
    printf("Hash MD5 alvo: %s\n", target_hash);
    printf("Tamanho da senha: %d\n", password_len);
    printf("Charset: %s (tamanho: %d)\n", charset, charset_len);
    printf("Número de workers: %d\n", num_workers);

    // Calcular espaço de busca total
    long long total_space = calculate_search_space(charset_len, password_len);
    printf("Espaço de busca total: %lld combinações\n\n", total_space);

    // Remover arquivo de resultado anterior se existir
    unlink(RESULT_FILE);

    // Registrar tempo de início
    time_t start_time = time(NULL);

    long long passwords_per_worker = total_space / num_workers;
    long long remaining = total_space % num_workers;

    pid_t workers[MAX_WORKERS];

    printf("Iniciando workers...\n");

    for (int i = 0; i < num_workers; i++)
    {
        char first_passwd[password_len + 1], last_passwd[password_len + 1];
        index_to_password(i * passwords_per_worker, charset, charset_len, password_len, first_passwd);

        long long end_index;
        if (i == num_workers - 1)
        {
            end_index = (i + 1) * passwords_per_worker + remaining - 1;
        }
        else
        {
            end_index = (i + 1) * passwords_per_worker - 1;
        }
        index_to_password(end_index, charset, charset_len, password_len, last_passwd);

        pid_t pid = fork();

        if (pid < 0)
        {
            perror("Erro ao criar worker");
            exit(1);
        }
        else if (pid == 0)
        {
            char password_len_str[12];
            char worker_id_str[12];
            sprintf(password_len_str, "%d", password_len);
            sprintf(worker_id_str, "%d", i);

            execl("./worker", "worker", target_hash, first_passwd, last_passwd,
                  charset, password_len_str, worker_id_str, NULL);

            perror("Erro no execl");
            exit(1);
        }
        else
        {
            workers[i] = pid;
        }
    }

    printf("\nTodos os workers foram iniciados. Aguardando conclusão...\n");

    // TODO 8: Aguardar todos os workers terminarem usando wait()
    // IMPORTANTE: O pai deve aguardar TODOS os filhos para evitar zumbis

    // IMPLEMENTE AQUI:
    // - Loop para aguardar cada worker terminar
    int finished_count = 0;
    for (int i = 0; i < num_workers; i++)
    {
        // - Usar wait() para capturar status de saída
        int status;
        pid_t pid = wait(&status);
        int worker_index = 0;
        for (int i = 0; i < num_workers; i++)
        {
            if (pid == workers[i])
            {
                worker_index = i;
                break;
            }
        }
        if (!WIFEXITED(status))
        {
            printf("ERRO: worker terminou com erro");
            exit(1);
        }

        int exit_code = WEXITSTATUS(status);

        finished_count++;
    }
    // - Identificar qual worker terminou- Verificar se terminou normalmente ou com erro
    // - Contar quantos workers terminaram

    // Registrar tempo de fim
    time_t end_time = time(NULL);
    double elapsed_time = difftime(end_time, start_time);

    printf("\n=== Resultado ===\n");

    // TODO 9: Verificar se algum worker encontrou a senha
    // Ler o arquivo password_found.txt se existir

    // IMPLEMENTE AQUI:
    // - Abrir arquivo RESULT_FILE para leitura
    int fd = open(RESULT_FILE, O_RDONLY);
    if (fd >= 0)
    {
        char buffer[1024];

        read(fd, buffer, 1024);
        char *pass = strchr(buffer, ':');
        pass++;
        pass[password_len] = 0;
        char hash[33];
        md5_string(pass, hash);
        if (!strcmp(hash, target_hash))
            printf("Senha Encontrada com sucesso!: %s\n", pass);
        else
            printf("Senha não encontrada!");
    }
    // - Ler conteúdo do arquivo
    // - Fazer parse do formato "worker_id:password"
    // - Verificar o hash usando md5_string()
    // - Exibir resultado encontrado

    // Estatísticas finais (opcional)
    // TODO: Calcular e exibir estatísticas de performance

    return 0;
}
