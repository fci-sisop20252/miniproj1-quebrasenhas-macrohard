# Relatório: Mini-Projeto 1 - Quebra-Senhas Paralelo

Enzo Lopes Campanholo (10190463) | Gian Lucca Campanha (10438361) | Felipe Bonatto Zwaizdis Scaquetti (10438149)
---

## 1. Estratégia de Paralelização


**Como você dividiu o espaço de busca entre os workers?**

O indice percorre todas as possíveis combinações, para então o trecho dividir esse espaço igualmente entre os workers, "passwords_per_worker" é a quantidade base de combinações para cada worker e "remaining" é o que sobra quando a divisão não é exata. worker recebe um intervalo. Em resumo: todos os workers processam aproximadamente o mesmo número de senhas, e o último recebe uma carga ligeiramente maior para incluir o que sobrou do espaço total.

**Código relevante:**
```c
long long passwords_per_worker = total_space / num_workers;
long long remaining = total_space % num_workers;
```

---

## 2. Implementação das System Calls

**Descreva como você usou fork(), execl() e wait() no coordinator:**

Os processos foram criados com chamadas a fork(), onde cada iteração do loop gera um novo processo filho. Dentro do filho, foi usado execl() para substituir sua execução pelo programa worker, passando como argumentos o hash alvo, a senha inicial e final do intervalo, o charset, o tamanho da senha e o identificador do worker, garantindo que cada processo receba apenas a parte do espaço de busca que deve explorar. Já no processo pai, foi armazenado os PIDs dos filhos e utilizamos wait() em um loop para aguardar a conclusão de todos os workers, coletando seus códigos de saída e verificando como terminaram antes de prosseguir para a análise do resultado.

**Código do fork/exec:**
```c
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
```

---

## 3. Comunicação Entre Processos

**Como você garantiu que apenas um worker escrevesse o resultado?**

Usando uma escrita atômica no arquivo de saída, apenas um worker escreve o resultado. Para isso, no worker.c, o arquivo password_found.txt é aberto com as flags O_WRONLY | O_CREAT | O_EXCL. A flag O_EXCL só permite a criação do arquivo se ele não existir. Caso já tiver sido criado por outro worker, a chamada falha imediatamente. Mesmo que vários processos tentem gravar, apenas o primeiro que acertar a senha consegue criar o arquivo, e os demais não sobrescrevem o resultado. Assim a condição de corrida é evitada, que ocorreria se dois workers escrevessem simultaneamente no mesmo arquivo, causando dados corrompidos ou resultados errados. Com a escrita atômica usando open(), o sistema operacional controla que apenas um worker consiga registrar a senha encontrada.

**Como o coordinator consegue ler o resultado?**

O coordinator consegue ler o resultado abrindo o arquivo password_found.txt em modo leitura depois que todos os workers terminam. Em seguida, ele faz a leitura do conteúdo com read() para dentro de um buffer e procura pelo caractere ':', que separa a palavra-chave (FOUND:) da senha encontrada. A partir desse ponto, o programa extrai a senha selecionada, ela passa pelo filtro de tamanho e então calcula novamente o MD5 dessa senha usando md5_string() para comparar com o hash da senha inserida. Assim, o coordinator valida que a senha escrita no arquivo é a correta antes de exibir o resultado.

---

## 4. Análise de Performance
Complete a tabela com tempos reais de execução:
O speedup é o tempo do teste com 1 worker dividido pelo tempo com 4 workers.

| Teste | 1 Worker | 2 Workers | 4 Workers | Speedup (4w) |
|-------|----------|-----------|-----------|--------------|
| Hash: 202cb962ac59075b964b07152d234b70<br>Charset: "0123456789"<br>Tamanho: 3<br>Senha: "123" | 0,010s | 0,011s | 0,009s | 1,11 |
| Hash: 5d41402abc4b2a76b9719d911017c592<br>Charset: "abcdefghijklmnopqrstuvwxyz"<br>Tamanho: 5<br>Senha: "hello" | 4,594s | 4,624s | 0,866s | 5,30 |

**O speedup foi linear? Por quê?**

O speedup não foi linear. Dobrar o número de workers vai reduzir o tempo de execução por dividir o espaço de busca, mas a velocidade não dobra exatamente. O overhead de criar e gerenciar processos com fork() e execl(), consome tempo antes mesmo de cada worker começar a testar as senhas. Pelo ultimo worker receber um bloco maior há um pequeno desequilíbrio na divisão do trabalho entre workers.

---

## 5. Desafios e Aprendizados
**Qual foi o maior desafio técnico que você enfrentou?**

Tivemos dificulade na implmentação dos workers, mas depois de muita tentativa conseguimos implementar corretamente.

---

## Comandos de Teste Utilizados

```bash
# Teste básico
./coordinator "900150983cd24fb0d6963f7d28e17f72" 3 "abc" 2

# Teste de performance
time ./coordinator "202cb962ac59075b964b07152d234b70" 3 "0123456789" 1
time ./coordinator "202cb962ac59075b964b07152d234b70" 3 "0123456789" 4

# Teste com senha maior
time ./coordinator "5d41402abc4b2a76b9719d911017c592" 5 "abcdefghijklmnopqrstuvwxyz" 4
```
---

**Checklist de Entrega:**
- [x] Código compila sem erros
- [x] Todos os TODOs foram implementados
- [x] Testes passam no `./tests/simple_test.sh`
- [x] Relatório preenchido
