# Fluent Language

**Teoria da Computação e Compiladores**
Entrega: 23 de Abril de 2026

Linguagem com sintaxe inspirada no inglês natural, tipada e compilada.

---

## Estrutura do Projeto

```
Fluent-Compiler/
├── build/                  # Artefatos de compilação
├── src/
│   ├── main.c              # Ponto de entrada
│   ├── lexer/
│   │   ├── token.h         # Definição dos tipos de tokens
│   │   ├── lexer.h         # Interface do lexer
│   │   └── lexer.c         # Implementação do lexer
│   ├── parser/
│   │   ├── parser.h        # Interface do parser
│   │   └── parser.c        # Implementação do parser
│   └── ast/
│       ├── ast.h           # Definição dos nós da AST
│       └── ast.c           # Alocação, impressão e liberação da AST
├── tests/
│   └── exemplo.fluent
├── Makefile
└── README.md
```

---

## Compilar e Executar

```bash
# Limpar, compilar e executar
make re

# Ou separadamente
make clean
make
./build/fluent tests/exemplo.fluent
```

---

## Sintaxe

```
# Variável
integer x receives 5;

# Número negativo
integer y receives negative 5;

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

# Função com retorno
integer function soma(integer a, integer b) then
    return a plus b;
end

# Função sem retorno
void function greet(string name) then
    say(name);
end

# Chamada de função como statement
greet("world");
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
| `KW_NEGATIVE` | `negative`                    | negação unária |

---

## Tokens

| Token          | Lexema / Regex              |
|----------------|------------------------------|
| `KW_IF`        | `if`                        |
| `KW_ELSE`      | `else`                      |
| `KW_WHILE`     | `while`                     |
| `KW_FUNCTION`  | `function`                  |
| `KW_RETURN`    | `return`                    |
| `KW_THEN`      | `then`                      |
| `KW_END`       | `end`                       |
| `KW_SAY`       | `say`                       |
| `KW_VOID`      | `void`                      |
| `KW_NEGATIVE`  | `negative`                  |
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
statement = variable_declaration | assignment | expression_statement | if_statement | while_statement | function_definition | return_statement | say_statement ;

(* Declaração de variável *)
variable_declaration = TYPE IDENTIFIER KW_RECEIVES expression SEMICOLON ;

(* Atribuição *)
assignment = IDENTIFIER KW_RECEIVES expression SEMICOLON ;

(* Chamada de função como statement *)
expression_statement = function_call SEMICOLON ;

(* Condicional *)
if_statement = KW_IF LPAREN expression RPAREN KW_THEN { statement } [ KW_ELSE { statement } ] KW_END ;

(* Repetição *)
while_statement = KW_WHILE LPAREN expression RPAREN KW_THEN { statement } KW_END ;

(* Definição de função *)
function_definition = RETURN_TYPE KW_FUNCTION IDENTIFIER LPAREN [ parameter { COMMA parameter } ] RPAREN KW_THEN { statement } KW_END ;

RETURN_TYPE = TYPE | KW_VOID ;

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

exponentiation = unary [ OP_POW exponentiation ] ;

unary = KW_NEGATIVE unary | atom ;

atom = LIT_INTEGER | LIT_FLOAT | LIT_STRING | LIT_TRUE | LIT_FALSE | function_call | IDENTIFIER | LPAREN expression RPAREN ;

(* Chamada de função *)
function_call = IDENTIFIER LPAREN [ expression { COMMA expression } ] RPAREN ;

(* Tipos *)
TYPE = TYPE_INTEGER | TYPE_FLOAT | TYPE_STRING | TYPE_BOOLEAN ;
```

> A hierarquia de expressões (`comparison` -> `addition` -> `multiplication` -> `exponentiation` -> `unary` -> `atom`) define a precedência dos operadores: operadores mais abaixo na hierarquia são resolvidos primeiro. Cada nível vira uma função recursiva no parser, formando um algoritmo de árvore com recursividade.

---

## Lexer

O lexer utiliza regex pré-compiladas e uma tabela declarativa de tokens (`TokenValue`), seguindo a ordem de prioridade abaixo em cada chamada de `next_token`:

1. **Operadores multipalavra** (`complex_token`), usando entradas com `complex = true`
2. **Números** (`read_number`) — regex `^[0-9]+\.[0-9]+` e `^[0-9]+`
3. **Strings** (`read_string`) — regex `^"[^"]*"`
4. **Identificadores e reservadas** (`read_identifier`) — regex `^[a-zA-Z_][a-zA-Z0-9_]*`, seguida de lookup na tabela para entradas com `complex = false`
5. **Símbolos de um caractere** (`read_symbol`) — percorre a tabela procurando entradas com `complex = false` e padrões: `(`, `)`, `,`, `;`

Os regexs são compilados uma única vez em `init_lexer` e armazenados em `lexer.regex[REGEX_COUNT]`, sendo liberados em `del_lexer`. Além disso, a função `del_lexer` libera a memória do source internamente. Por isso, o source passado para `init_lexer` deve ser alocado com `malloc` e não deve ser liberado manualmente após `del_lexer`.

---

## Parser

O parser é recursivo descendente, com uma função por regra da gramática EBNF. Constrói a AST diretamente, sem passar por uma parse tree intermediária.

### Janela de dois tokens

O parser mantém dois tokens em memória simultaneamente — `current_token` e `lookahead_token` — para resolver pontos de ambiguidade da gramática:

- Em `parse_atom`: um `IDENTIFIER` seguido de `LPAREN` é uma chamada de função; caso contrário, é uma referência a variável.
- Em `parse_statement`: um `TYPE` seguido de `KW_FUNCTION` é uma definição de função; caso contrário, é uma declaração de variável.
- Em `parse_statement`: um `IDENTIFIER` seguido de `LPAREN` é uma chamada de função usada como statement; caso contrário, é uma atribuição.

### Funções primitivas

| Função    | Comportamento |
|-----------|---------------|
| `advance` | Desloca a janela: `current ← lookahead`, `lookahead ← next_token()` |
| `expect`  | Consome o token atual verificando o tipo; retorna o lexeme (ownership transferido ao chamador) |
| `consume` | Consome o token atual sem verificar o tipo; libera o lexeme |

`expect` transfere o ponteiro do lexeme diretamente para o nó da AST, sem cópia. O nó é responsável por liberar a memória via `del_ast`. Tokens consumidos com `consume` têm o lexeme liberado imediatamente.

### Precedência de operadores

A precedência é codificada na hierarquia de chamadas: cada nível chama o nível abaixo antes de processar seu próprio operador. Operadores mais abaixo na hierarquia têm maior precedência.

```
parse_expression
  └── parse_comparison        (==, !=, <, >, <=, >=)   — não-associativo
        └── parse_addition    (+, -)                   — associativo à esquerda
              └── parse_multiplication  (*, /)         — associativo à esquerda
                    └── parse_exponentiation (**)      — associativo à direita
                          └── parse_unary  (negative)  — prefixo, recursivo à direita
                                └── parse_atom
```

---

## AST

A AST usa uma struct única (`ASTNode`) com campos reutilizados por tipo de nó, evitando union de structs.

### Campos

| Campo         | Tipo        | Uso                                                                                           |
|---------------|-------------|-----------------------------------------------------------------------------------------------|
| `type`        | `NodeType`  | Tipo do nó                                                                                    |
| `left`        | `ASTNode *` | Filho esquerdo: condição, operando esquerdo, expressão atribuída, corpo de função, lista de argumentos, operando do unário |
| `right`       | `ASTNode *` | Filho direito: operando direito, corpo do `then`, lista de parâmetros                         |
| `else_branch` | `ASTNode *` | Corpo do `else` (apenas `NODE_IF`; `NULL` quando ausente)                                     |
| `next`        | `ASTNode *` | Próximo nó em lista encadeada (statements, parâmetros, argumentos)                            |
| `value`       | `char *`    | Nome do identificador ou texto do literal (heap-allocated)                                    |
| `operator`    | `TokenType` | Operador de `NODE_BINARY_OP` e `NODE_UNARY_OP`                                                |
| `data_type`   | `TokenType` | Tipo declarado em declarações, parâmetros e definições de função                              |
| `line`        | `int`       | Linha do fonte onde o nó se origina                                                           |

### Tipos de nó

| NodeType                  | `left`              | `right`           | `else_branch`   | `value`        | `data_type`  |
|---------------------------|---------------------|-------------------|-----------------|----------------|--------------|
| `NODE_PROGRAM`            | head dos statements | —                 | —               | —              | —            |
| `NODE_VAR_DECL`           | expressão inicial   | —                 | —               | nome da var    | tipo         |
| `NODE_ASSIGNMENT`         | expressão           | —                 | —               | nome da var    | —            |
| `NODE_IF`                 | condição            | corpo do `then`   | corpo do `else` | —              | —            |
| `NODE_WHILE`              | condição            | corpo             | —               | —              | —            |
| `NODE_FUNCTION_DEF`       | corpo               | lista de params   | —               | nome da função | tipo retorno |
| `NODE_PARAMETER`          | —                   | —                 | —               | nome do param  | tipo         |
| `NODE_RETURN`             | expressão           | —                 | —               | —              | —            |
| `NODE_SAY`                | expressão           | —                 | —               | —              | —            |
| `NODE_EXPRESSION_STATEMENT` | chamada de função | —                 | —               | —              | —            |
| `NODE_BINARY_OP`          | operando esquerdo   | operando direito  | —               | —              | —            |
| `NODE_UNARY_OP`           | operando            | —                 | —               | —              | —            |
| `NODE_FUNCTION_CALL`      | lista de args       | —                 | —               | nome da função | —            |
| `NODE_IDENTIFIER`         | —                   | —                 | —               | nome           | —            |
| `NODE_LIT_INTEGER`        | —                   | —                 | —               | texto literal  | —            |
| `NODE_LIT_FLOAT`          | —                   | —                 | —               | texto literal  | —            |
| `NODE_LIT_STRING`         | —                   | —                 | —               | texto literal  | —            |
| `NODE_LIT_TRUE`           | —                   | —                 | —               | —              | —            |
| `NODE_LIT_FALSE`          | —                   | —                 | —               | —              | —            |

> Todos os nós participam de listas encadeadas via `-> next`, exceto `NODE_PROGRAM`, `NODE_BINARY_OP`, `NODE_UNARY_OP`, e os literais, cujo `-> next` é sempre `NULL`.