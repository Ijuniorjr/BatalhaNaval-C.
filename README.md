# Batalha Naval em C

Trabalho da disciplina de Programação Estruturada (Estácio).  
O objetivo foi praticar *vetores, matrizes, loops aninhados e condicionais* em linguagem C através de uma versão simplificada do jogo *Batalha Naval*.

---

## Funcionalidades
- Tabuleiro representado por uma matriz 10x10.  
- Frota de navios posicionada automaticamente.  
- Modos de jogo:  
  - *Novato* → ataque simples.  
  - *Aventureiro* → permite navios em diagonal e inclui ataque em cruz.  
  - *Mestre* → adiciona habilidades especiais (cruz, cone e octaedro).  
- O jogo termina quando todos os navios são destruídos.  

---

## Como rodar
1. Compilar o arquivo:
   ```bash
   gcc BatalhaNaval.c -o batalha
