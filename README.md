# Fluent Language

**Teoria da Computação e Compiladores**
Entrega: 23 de Abril de 2026

Linguagem com sintaxe inspirada no inglês natural, tipada estaticamente. O compilador implementa as fases de análise léxica, sintática e semântica — produzindo e validando a AST. A geração de código está em desenvolvimento.

---

## Estrutura do Projeto

```
Fluent-Compiler/
├── src/
│   ├── main.c               # Ponto de entrada — encadeia todas as fases
│   ├── lexer/
│   │   ├── token.h          # Definição dos tipos de tokens
│   │   ├── lexer.h          # Interface do lexer (inclui peek_token)
│   │   └── lexer.c          # Implementação do lexer
│   ├── parser/
│   │   ├── parser.h         # Interface do parser
│   │   └── parser.c         # Parser por descida recursiva LL(2)
│   ├── ast/
│   │   ├── ast.h            # Definição dos nós da AST (union discriminada)
│   │   └── ast.c            # Construtores, print_ast e del_tree
│   └── semantic/
│       ├── semantic.h       # Interface da análise semântica
│       └── semantic.c       # Análise semântica com pilha de escopos
├── tests/
│   ├── exemplo.fluent       # Programa de exemplo principal
│   └── battery/             # Bateria de testes semânticos
├── Makefile
└── README.md
```

---

## Compilar e Executar

> **Atenção:** o Makefile usa comandos do `cmd.exe` (`if not exist`, `rd /s /q`) e foi desenvolvido para **Windows com MinGW**. Em Linux/macOS, substitua `mingw32-make` por `make` e ajuste os comandos de `mkdir`/`rm` no Makefile conforme seu shell.

```bash
mingw32-make         # compila → build/fluent.exe
mingw32-make run     # executa com tests/exemplo.fluent
mingw32-make clean   # remove a pasta build/
```

O executável recebe um arquivo `.fluent` como argumento:

```bash
./build/fluent.exe meu_programa.fluent
```

A saída atual exibe a AST anotada com tipos semânticos e reporta erros léxicos, sintáticos e semânticos no stderr.

---

## Sintaxe

```fluent
# Variáveis
integer x receives 10;
float pi receives 3.14;
string msg receives "hello";
boolean flag receives true;

# Número negativo
integer y receives negative x;

# Condicional
if (x is greater than 5) then
    say("maior");
else
    say("menor");
end

# Repetição
while (x is greater than 0) then
    x receives x minus 1;
end

# Função com retorno
integer function max(integer a, integer b) then
    if (a is greater than b) then
        return a;
    else
        return b;
    end
end

# Função void
void function saudar(string nome) then
    say(nome);
end

# Chamada de função como statement
saudar("mundo");
integer resultado receives max(10, 20);
say(resultado);
```

> Blocos sempre delimitados por `then` ... `end`. Comentários com `#` (apenas de linha).

---

## Tipos Primitivos

| Tipo      | Declaração       | Literal              |
|-----------|------------------|----------------------|
| `integer` | `integer x`      | `42`, `0`, `100`     |
| `float`   | `float x`        | `3.14`, `2.5`        |
| `string`  | `string x`       | `"hello"`            |
| `boolean` | `boolean x`      | `true`, `false`      |

---

## Palavras Reservadas

`integer` `float` `string` `boolean` `void` `true` `false` `receives` `if` `else` `while` `function` `return` `then` `end` `say` `negative`

---

## Operadores

| Lexema                        | Operação          | Resultado              |
|-------------------------------|-------------------|------------------------|
| `plus`                        | adição            | integer ou float       |
| `minus`                       | subtração         | integer ou float       |
| `times`                       | multiplicação     | integer ou float       |
| `divided by`                  | divisão           | integer ou float       |
| `to the power of`             | exponenciação     | integer ou float       |
| `equals`                      | igual             | boolean                |
| `differs from`                | diferente         | boolean                |
| `is less than`                | menor que         | boolean                |
| `is greater than`             | maior que         | boolean                |
| `is less than or equal to`    | menor ou igual    | boolean                |
| `is greater than or equal to` | maior ou igual    | boolean                |
| `negative`                    | negação unária    | mesmo tipo do operando |

---

## Gramática EBNF

```ebnf
program = { top_level_statement } ;

top_level_statement  = function_definition | statement ;

statement = variable_declaration | assignment | expression_statement | if_statement | while_statement | return_statement | say_statement ;

variable_declaration = TYPE IDENTIFIER KW_RECEIVES expression SEMICOLON ;

assignment = IDENTIFIER KW_RECEIVES expression SEMICOLON ;

expression_statement = IDENTIFIER LPAREN [ expression { COMMA expression } ] RPAREN SEMICOLON ;

if_statement = KW_IF LPAREN expression RPAREN KW_THEN { statement } [ KW_ELSE { statement } ] KW_END ;

while_statement = KW_WHILE LPAREN expression RPAREN KW_THEN { statement } KW_END ;

function_definition = RETURN_TYPE KW_FUNCTION IDENTIFIER LPAREN [ parameter { COMMA parameter } ] RPAREN KW_THEN { statement } KW_END ;

RETURN_TYPE = TYPE | KW_VOID ;

parameter = TYPE IDENTIFIER ;

return_statement = KW_RETURN expression SEMICOLON ;

say_statement = KW_SAY LPAREN expression RPAREN SEMICOLON ;

expression = comparison ;

comparison = addition [ ( OP_EQ | OP_NEQ | OP_LT | OP_GT | OP_LTE | OP_GTE ) addition ] ;

addition = multiplication { ( OP_PLUS | OP_MINUS ) multiplication } ;

multiplication = exponentiation { ( OP_TIMES | OP_DIV ) exponentiation } ;

exponentiation = unary [ OP_POW exponentiation ] ;

unary = KW_NEGATIVE unary | atom ;

atom = LIT_INTEGER | LIT_FLOAT | LIT_STRING | LIT_TRUE | LIT_FALSE | IDENTIFIER [ LPAREN [ expression { COMMA expression } ] RPAREN ] | LPAREN expression RPAREN ;

TYPE = TYPE_INTEGER | TYPE_FLOAT | TYPE_STRING | TYPE_BOOLEAN ;
```

---

## Análise Semântica

Realizada em passagem única sobre a AST. Usa uma pilha de tabelas de símbolos para gerenciar escopos — um novo escopo é aberto a cada função, `if` e `while`. Funções são registradas em uma primeira varredura do nível global, permitindo que sejam chamadas antes de sua definição no código.

**Verificações realizadas:**

| Situação                               | Erro emitido                                                                        |
|----------------------------------------|------------------------------------------------------------------------------------ |
| Variável usada sem declaração          | `semantic error on line N: undefined variable 'x'`                                 |
| Variável declarada duas vezes          | `semantic error on line N: variable 'x' already declared`                          |
| Tipo errado na declaração              | `semantic error on line N: type mismatch in declaration of 'x': expected X, got Y` |
| Tipo errado na atribuição              | `semantic error on line N: type mismatch in assignment to 'x': expected X, got Y`  |
| Função chamada sem declaração          | `semantic error on line N: undefined function 'f'`                                 |
| Número de argumentos incorreto         | `semantic error on line N: function 'f' expects M arguments, got K`                |
| Tipo de argumento incompatível         | `semantic error on line N: argument 1 of 'f': expected X, got Y`                  |
| `return` fora de função                | `semantic error on line N: 'return' outside function`                              |
| `return` com tipo errado               | `semantic error on line N: return type mismatch in 'f': expected X, got Y`        |
| Função não-void sem `return`           | `semantic error: missing return in function 'f'`                                   |
| Parâmetro duplicado na definição       | `semantic error on line N: duplicate parameter 'p' in function 'f'`               |
| Operação entre tipos incompatíveis     | `semantic error on line N: type mismatch: cannot apply 'op' to X and Y`           |
| `negative` aplicado a tipo não-numérico| `semantic error on line N: type mismatch: cannot apply 'negative' to X`           |

**Regras de compatibilidade de tipos:**

| Operandos                  | Resultado  |
|----------------------------|------------|
| `integer` op `integer`     | `integer`  |
| `float` op `float`         | `float`    |
| `integer` op `float`       | `float`    |
| qualquer outro par         | erro       |

Comparações seguem as mesmas regras de compatibilidade, mas sempre retornam `boolean`. Os operadores `equals` e `differs from` aceitam quaisquer dois operandos do mesmo tipo; os de ordem (`is less than`, etc.) aceitam apenas tipos numéricos.

**Conversão implícita:**

Quando há mistura de `integer` e `float` em uma expressão, declaração ou atribuição compatível, a análise semântica insere automaticamente um nó `NODE_CAST` na AST para registrar a conversão necessária. A execução da conversão em si é responsabilidade do gerador de código.

**Propagação de retorno:**

Um `if/else` em que ambos os ramos contêm `return` é considerado como retorno garantido — a função é validada sem exigir um `return` adicional após o bloco.

---

## Lookahead LL(2)

O parser é predominantemente LL(1), mas requer 2 tokens em dois pontos:

| Contexto              | Token 1      | Token 2      | Decisão                    |
|-----------------------|--------------|--------------|----------------------------|
| `top_level_statement` | `TYPE`       | `function`   | → `function_definition`    |
| `top_level_statement` | `TYPE`       | outro        | → `variable_declaration`   |
| `statement`           | `IDENTIFIER` | `(`          | → `expression_statement`   |
| `statement`           | `IDENTIFIER` | `receives`   | → `assignment`             |

O lexer expõe `peek_token()` para suporte a esse lookahead. O token espiado é armazenado internamente e consumido na próxima chamada a `next_token`.