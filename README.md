# Fluent Language

**Teoria da Computação e Compiladores**
Entrega: 23 de Abril de 2026

Linguagem com sintaxe inspirada no inglês natural, estaticamente tipada e compilada.

---

## Estrutura do Projeto

```
Fluent-Compiler/
├── src/
│   ├── token.h       # Definição dos tipos de tokens
│   ├── lexer.h       # Interface do lexer
│   ├── lexer.c       # Implementação do lexer
│   └── main.c        # Ponto de entrada
├── build/            # Artefatos de compilação (.o, .exe)
├── tests/
│   └── exemplo.fluent
├── Makefile
└── README.md
```

---

## Compilar e Executar

```bash
# Limpar, compilar e executar
mingw32-make re

# Ou separadamente
mingw32-make clean
mingw32-make
.\build\exemplo.exe tests/exemplo.fluent
```

---

## Sintaxe

```
# Variável
integer x receives 5

# Condicional
if (x is greater than 0) then
    say("positivo")
else
    say("negativo")
end

# Repetição
while (x is greater than 0) then
    x receives x minus 1
end

# Função
integer function soma(integer a, integer b) then
    return a plus b
end
```

> Blocos sempre delimitados por `then` ... `end`. Comentários com `#` (apenas de linha).

---

## Tipos Primitivos

| Tipo      | Token          | Literal / Regex        |
|-----------|----------------|------------------------|
| `integer` | `TYPE_INTEGER` | `[0-9]+`               |
| `float`   | `TYPE_FLOAT`   | `[0-9]+\.[0-9]+`       |
| `string`  | `TYPE_STRING`  | `"[^"]*"`              |
| `boolean` | `TYPE_BOOLEAN` | `true` \| `false`      |

Declaração: `TYPE IDENTIFIER receives EXPR` → ex: `float pi receives 3.14`

---

## Operadores

| Token         | Lexema                        | Operação       |
|---------------|-------------------------------|----------------|
| `OP_PLUS`     | `plus`                        | adição         |
| `OP_MINUS`    | `minus`                       | subtração      |
| `OP_TIMES`    | `times`                       | multiplicação  |
| `OP_DIV`      | `divided by`                  | divisão        |
| `OP_POW`      | `to the power of`             | exponenciação  |
| `OP_EQ`       | `equals`                      | igual          |
| `OP_NEQ`      | `differs from`                | diferente      |
| `OP_LT`       | `is less than`                | menor que      |
| `OP_GT`       | `is greater than`             | maior que      |
| `OP_LTE`      | `is less than or equal to`    | menor ou igual |
| `OP_GTE`      | `is greater than or equal to` | maior ou igual |
| `KW_RECEIVES` | `receives`                    | atribuição     |

---

## Tokens

| Token          | Lexema / Regex              |
|----------------|-----------------------------|
| `KW_IF`        | `if`                        |
| `KW_ELSE`      | `else`                      |
| `KW_WHILE`     | `while`                     |
| `KW_FUNCTION`  | `function`                  |
| `KW_RETURN`    | `return`                    |
| `KW_THEN`      | `then`                      |
| `KW_END`       | `end`                       |
| `KW_SAY`       | `say`                       |
| `KW_TRUE`      | `true`                      |
| `KW_FALSE`     | `false`                     |
| `IDENTIFIER`   | `[a-zA-Z_][a-zA-Z0-9_]*`   |
| `LPAREN`       | `(`                         |
| `RPAREN`       | `)`                         |
| `COMMA`        | `,`                         |

**Ignorados:** espaços/tabs `[ \t]+`, quebras de linha `\n|\r\n`, comentários `#[^\n]*`

> ⚠️ Prioridade no lexer: (1) operadores multi-palavra, do mais longo ao mais curto; (2) palavras-chave e tipos; (3) identificadores.

---

## Arquitetura do Lexer

O lexer utiliza regex POSIX pré-compiladas e uma tabela declarativa de tokens (`TokenValue`), seguindo a ordem de prioridade abaixo em cada chamada de `next_token`:

1. **Frases multi-palavra** (`complex_token`) — casadas por `strncmp` com word boundary check, usando entradas com `complex = true`
2. **Números** (`read_number`) — regex `^[0-9]+\.[0-9]+` e `^[0-9]+`
3. **Strings** (`read_string`) — regex `^"[^"]*"`
4. **Identificadores e palavras-chave** (`read_identifier`) — regex `^[a-zA-Z_][a-zA-Z0-9_]*`, seguida de lookup na tabela para entradas com `complex = false`

As regexes são compiladas uma única vez em `init_lexer` e armazenadas em `lexer.regex[REGEX_COUNT]`. São liberadas em `del_lexer`.

---

## Funções — Sequência do Parser

```
TYPE KW_FUNCTION IDENTIFIER LPAREN (TYPE IDENTIFIER (COMMA TYPE IDENTIFIER)*)? RPAREN KW_THEN
```
