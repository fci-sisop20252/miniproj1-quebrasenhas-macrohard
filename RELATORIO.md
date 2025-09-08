# Relatório: Mini-Projeto 1 - Quebra-Senhas Paralelo

Enzo Lopes Campanholo (10190463), Gian Lucca Campanha (10438361), Felipe Bonatto Zwaizdis Scaquetti (10438149)
---

## 1. Estratégia de Paralelização


**Como você dividiu o espaço de busca entre os workers?**

[Explique seu algoritmo de divisão]

**Código relevante:** Cole aqui a parte do coordinator.c onde você calcula a divisão:
```c
long long passwords_per_worker = total_space / num_workers;
long long remaining = total_space % num_workers;
```

---

## 2. Implementação das System Calls

**Descreva como você usou fork(), execl() e wait() no coordinator:**

[Explique em um parágrafo como você criou os processos, passou argumentos e esperou pela conclusão]

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

[Explique como você implementou uma escrita atômica e como isso evita condições de corrida]
Leia sobre condições de corrida (aqui)[https://pt.stackoverflow.com/questions/159342/o-que-%C3%A9-uma-condi%C3%A7%C3%A3o-de-corrida]

**Como o coordinator consegue ler o resultado?**

[Explique como o coordinator lê o arquivo de resultado e faz o parse da informação]

---

## 4. Análise de Performance
Complete a tabela com tempos reais de execução:
O speedup é o tempo do teste com 1 worker dividido pelo tempo com 4 workers.

| Teste | 1 Worker | 2 Workers | 4 Workers | Speedup (4w) |
|-------|----------|-----------|-----------|--------------|
| Hash: 202cb962ac59075b964b07152d234b70<br>Charset: "0123456789"<br>Tamanho: 3<br>Senha: "123" | ___s | ___s | ___s | ___ |
| Hash: 5d41402abc4b2a76b9719d911017c592<br>Charset: "abcdefghijklmnopqrstuvwxyz"<br>Tamanho: 5<br>Senha: "hello" | ___s | ___s | ___s | ___ |

**O speedup foi linear? Por quê?**
[Analise se dobrar workers realmente dobrou a velocidade e explique o overhead de criar processos]

---

## 5. Desafios e Aprendizados
**Qual foi o maior desafio técnico que você enfrentou?**
[Descreva um problema e como resolveu. Ex: "Tive dificuldade com o incremento de senha, mas resolvi tratando-o como um contador em base variável"]

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
- [ ] Código compila sem erros
- [ ] Todos os TODOs foram implementados
- [ ] Testes passam no `./tests/simple_test.sh`
- [ ] Relatório preenchido
