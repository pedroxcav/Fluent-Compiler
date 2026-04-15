# Fluent Language

**Teoria da Computação e Compiladores** 
Entrega: 23 de Abril de 2026

Linguagem com sintaxe inspirada no inglês natural, estaticamente tipada e compilada.

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
> 

---

## Tipos Primitivos

| Tipo | Token | Literal / Regex |
| --- | --- | --- |
| `integer` | `TYPE_INTEGER` | `[0-9]+` |
| `float` | `TYPE_FLOAT` | `[0-9]+\.[0-9]+` |
| `string` | `TYPE_STRING` | `"[^"]*"` |
| `boolean` | `TYPE_BOOLEAN` | `true` | `false` |

Declaração: `TYPE IDENTIFIER receives EXPR` → ex: `float pi receives 3.14`

---

## Operadores

| Token | Lexema | Operação |
| --- | --- | --- |
| `OP_PLUS` | `plus` | adição |
| `OP_MINUS` | `minus` | subtração |
| `OP_TIMES` | `times` | multiplicação |
| `OP_DIV` | `divided by` | divisão |
| `OP_POW` | `to the power of` | exponenciação |
| `OP_EQ` | `equals` | igual |
| `OP_NEQ` | `differs from` | diferente |
| `OP_LT` | `is less than` | menor que |
| `OP_GT` | `is greater than` | maior que |
| `OP_LTE` | `is less than or equal to` | menor ou igual |
| `OP_GTE` | `is greater than or equal to` | maior ou igual |
| `KW_RECEIVES` | `receives` | atribuição |

---

## Tokens

| Token | Lexema / Regex |
| --- | --- |
| `KW_IF` | `if` |
| `KW_ELSE` | `else` |
| `KW_WHILE` | `while` |
| `KW_FUNCTION` | `function` |
| `KW_RETURN` | `return` |
| `KW_THEN` | `then` |
| `KW_END` | `end` |
| `KW_SAY` | `say` |
| `KW_TRUE` | `true` |
| `KW_FALSE` | `false` |
| `IDENTIFIER` | `[a-zA-Z_][a-zA-Z0-9_]*` |
| `LPAREN` | `(` |
| `RPAREN` | `)` |
| `COMMA` | `,` |

**Ignorados:** espaços/tabs `[ \t]+`, quebras de linha `\n|\r\n`, comentários `#[^\n]*`

> ⚠️ Prioridade no lexer: (1) operadores multi-palavra, do mais longo ao mais curto; (2) palavras-chave e tipos; (3) identificadores.
> 

---

## Funções — Sequência do Parser

```
TYPE KW_FUNCTION IDENTIFIER LPAREN (TYPE IDENTIFIER (COMMA TYPE IDENTIFIER)*)? RPAREN KW_THEN
```