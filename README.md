# Fluent Language

**Teoria da Computação e Compiladores**
Entrega: 23 de Abril de 2026

Linguagem com sintaxe inspirada no inglês natural, tipada e compilada.

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
integer x receives 5;

# Condicional
if (x is greater than 0) then
    say("positivo");
else
    say("negativo");
end

# Repetição
while (x is greater than 0) then
    x receives x minus 1;
end

# Função
integer function soma(integer a, integer b) then
    return a plus b;
end
```

> Blocos sempre delimitados por `then` ... `end`. Comentários com `#` (apenas de linha).

---

## Tipos Primitivos

Cada tipo primitivo possui dois tokens distintos: a **palavra-chave** usada em declarações e a **literal** que representa o valor em expressões.

**Palavras-chave de tipo** (usadas em declarações e parâmetros):

| Tipo      | Token          |
|-----------|----------------|
| `integer` | `TYPE_INTEGER` |
| `float`   | `TYPE_FLOAT`   |
| `string`  | `TYPE_STRING`  |
| `boolean` | `TYPE_BOOLEAN` |

**Literais** (valores concretos em expressões):

| Literal          | Token         | Regex / Lexema   |
|------------------|---------------|------------------|
| `42`, `0`, `100` | `LIT_INTEGER` | `[0-9]+`         |
| `3.14`, `2.5`    | `LIT_FLOAT`   | `[0-9]+\.[0-9]+` |
| `"hello"`        | `LIT_STRING`  | `"[^"]*"`        |
| `true`           | `LIT_TRUE`    | `true`           |
| `false`          | `LIT_FALSE`   | `false`          |

Declaração: `TYPE IDENTIFIER receives EXPR` -> exemplo: `float pi receives 3.14`

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
| `LIT_INTEGER`  | `[0-9]+`                    |
| `LIT_FLOAT`    | `[0-9]+\.[0-9]+`            |
| `LIT_STRING`   | `"[^"]*"`                   |
| `LIT_TRUE`     | `true`                      |
| `LIT_FALSE`    | `false`                     |
| `IDENTIFIER`   | `[a-zA-Z_][a-zA-Z0-9_]*`    |
| `LPAREN`       | `(`                         |
| `RPAREN`       | `)`                         |
| `COMMA`        | `,`                         |
| `SEMICOLON`    | `;`                         |

**Ignorados:** espaços/tabs `[ \t]+`, quebras de linha `\n|\r\n`, comentários `#[^\n]*`

> Prioridade no lexer: (1) operadores multi-palavra, do mais longo ao mais curto. (2) palavras-chave e tipos. (3) identificadores.

---

## Gramática EBNF
 
```ebnf
(* Programa *)
program = { statement } ;
 
(* Statements *)
statement = variable_declaration | assignment | if_statement | while_statement | function_definition | return_statement | say_statement ;
 
(* Declaração de variável *)
variable_declaration = TYPE IDENTIFIER KW_RECEIVES expression SEMICOLON ;
 
(* Atribuição *)
assignment = IDENTIFIER KW_RECEIVES expression SEMICOLON ;
 
(* Condicional *)
if_statement = KW_IF LPAREN expression RPAREN KW_THEN { statement } [ KW_ELSE { statement } ] KW_END ;
 
(* Repetição *)
while_statement = KW_WHILE LPAREN expression RPAREN KW_THEN { statement } KW_END ;
 
(* Definição de função *)
function_definition = TYPE KW_FUNCTION IDENTIFIER LPAREN [ parameter { COMMA parameter } ] RPAREN KW_THEN { statement } KW_END ;
 
parameter = TYPE IDENTIFIER ;
 
(* Return *)
return_statement = KW_RETURN expression SEMICOLON ;
 
(* Say *)
say_statement = KW_SAY LPAREN expression RPAREN SEMICOLON ;
 
(* Expressões — hierarquia define precedência *)
expression = comparison ;
 
comparison = addition [ ( OP_EQ | OP_NEQ | OP_LT | OP_GT | OP_LTE | OP_GTE ) addition ] ;
 
addition = multiplication { ( OP_PLUS | OP_MINUS ) multiplication } ;
 
multiplication = exponentiation { ( OP_TIMES | OP_DIV ) exponentiation } ;
 
exponentiation = atom [ OP_POW exponentiation ] ;
 
atom = LIT_INTEGER | LIT_FLOAT | LIT_STRING | LIT_TRUE | LIT_FALSE | function_call | IDENTIFIER | LPAREN expression RPAREN ;
 
(* Chamada de função *)
function_call = IDENTIFIER LPAREN [ expression { COMMA expression } ] RPAREN ;
 
(* Tipos *)
TYPE = TYPE_INTEGER | TYPE_FLOAT | TYPE_STRING | TYPE_BOOLEAN ;
```

> A hierarquia de expressões (`comparison` → `addition` → `multiplication` → `exponentiation` → `atom`) define a precedência dos operadores: operadores mais abaixo na hierarquia são resolvidos primeiro. Cada nível vira uma função recursiva no parser.

---

## Lexer

O lexer utiliza regex pré-compiladas e uma tabela declarativa de tokens (`TokenValue`), seguindo a ordem de prioridade abaixo em cada chamada de `next_token`:

1. **Operadores multipalavra** (`complex_token`), usando entradas com `complex = true`
2. **Números** (`read_number`) — regex `^[0-9]+\.[0-9]+` e `^[0-9]+`
3. **Strings** (`read_string`) — regex `^"[^"]*"`
4. **Identificadores e reservadas** (`read_identifier`) — regex `^[a-zA-Z_][a-zA-Z0-9_]*`, seguida de lookup na tabela para entradas com `complex = false`

Os regexs são compilados uma única vez em `init_lexer` e armazenadas em `lexer.regex[REGEX_COUNT]`. São liberadas em `del_lexer`.