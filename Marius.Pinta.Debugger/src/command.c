#include "debugger.h"

#include <stdlib.h>

#include <stdio.h>
#include <string.h>

#if 0
#define DBG printf
#else
#define DBG(...)
#endif

#define MAX_VAR_LENGTH 256

static u8 tcl_heap[64 * 1024];

typedef u8 PintaTclValue;

typedef enum PintaTclToken PintaTclToken;
typedef enum PintaTclFlow PintaTclFlow;
typedef struct PintaTclCore PintaTclCore;

typedef struct PintaTclCommand PintaTclCommand;
typedef struct PintaTclVariable PintaTclVariable;
typedef struct PintaTclEnvironment PintaTclEnvironment;

PintaTclFlow tcl_eval(PintaTclCore *tcl, const u8 *buffer, u32 buffer_length);

typedef PintaTclFlow(*PintaTclCommandFunction)(PintaTclCore *tcl, PintaTclValue *arguments, void *user_data);

/* Token type and control flow constants */
enum PintaTclToken
{
    PINTA_TCL_TOKEN_COMMAND,
    PINTA_TCL_TOKEN_WORD,
    PINTA_TCL_TOKEN_PART,
    PINTA_TCL_TOKEN_ERROR
};

enum PintaTclFlow
{
    PINTA_TCL_FLOW_ERROR,
    PINTA_TCL_FLOW_NORMAL,
    PINTA_TCL_FLOW_RETURN,
    PINTA_TCL_FLOW_BREAK,
    PINTA_TCL_FLOW_AGAIN
};

struct PintaTclCommand
{
    PintaTclValue *name;
    u32 argument_count;
    PintaTclCommandFunction function;
    void *user_data;
    PintaTclCommand *next;
};

struct PintaTclVariable
{
    PintaTclValue *name;
    PintaTclValue *value;
    PintaTclVariable *next;
};

struct PintaTclEnvironment
{
    PintaTclVariable *variables;
    PintaTclEnvironment *parent;
};

struct PintaTclParser
{
    const u8 *from;
    const u8 *to;
    const u8 *start;
    const u8 *end;
    u32 is_quoted;
    PintaTclToken token;
};

struct PintaTclCore
{
    PintaNativeMemory *memory;

    PintaTclEnvironment *environment;
    PintaTclCommand *commands;
    PintaTclValue *result;

    void *user_data;
};

/* A helper parser struct and macro (requires C99) */
#define tcl_each(buffer, buffer_length, skiperr)                                                                \
  for (struct PintaTclParser p = {NULL, NULL, (buffer), (buffer) + (buffer_length), 0, PINTA_TCL_TOKEN_ERROR};  \
       p.start < p.end &&                                                                                       \
       (((p.token = tcl_next(p.start, p.end - p.start, &p.from, &p.to,                                          \
                             &p.is_quoted)) != PINTA_TCL_TOKEN_ERROR) ||                                        \
        (skiperr));                                                                                             \
       p.start = p.to)

static u32 tcl_is_special(u8 c, u32 is_quoted)
{
    return (c == '$'
        || (!is_quoted && (c == '{' || c == '}' || c == ';' || c == '\r' || c == '\n'))
        || c == '[' || c == ']' || c == '"' || c == '\0');
}

static u32 tcl_is_space(u8 c)
{
    return (c == ' ' || c == '\t');
}

static u32 tcl_is_end(u8 c)
{
    return (c == '\n' || c == '\r' || c == ';' || c == '\0');
}

PintaTclToken tcl_next(const u8 *buffer, u32 buffer_length, const u8 **from, const u8 **to, u32 *is_quoted)
{
    u32 i = 0;
    i32 depth = 0;
    u8 open;
    u8 close;

    DBG("tcl_next(%.*s)+%d+%d|%d\n", buffer_length, buffer, *from - buffer, *to - buffer, *is_quoted);

    /* Skip leading spaces if not quoted */
    for (; !*is_quoted && buffer_length > 0 && tcl_is_space(*buffer); buffer++, buffer_length--)
    {
    }
    *from = buffer;
    /* Terminate command if not quoted */
    if (!*is_quoted && buffer_length > 0 && tcl_is_end(*buffer))
    {
        *to = buffer + 1;
        return PINTA_TCL_TOKEN_COMMAND;
    }
    if (*buffer == '$')
    {
        i32 mode, result;
        /* Variable token, must not start with a space or quote */
        if (tcl_is_space(buffer[1]) || buffer[1] == '"')
            return PINTA_TCL_TOKEN_ERROR;

        mode = *is_quoted;
        *is_quoted = 0;
        result = tcl_next(buffer + 1, buffer_length - 1, to, to, is_quoted);
        *is_quoted = mode;
        return ((result == PINTA_TCL_TOKEN_WORD && *is_quoted) ? PINTA_TCL_TOKEN_PART : result);
    }

    if (*buffer == '[' || (!*is_quoted && *buffer == '{'))
    {
        /* Interleaving pairs are not welcome, but it simplifies the code */
        open = *buffer;
        close = (open == '[' ? ']' : '}');
        for (i = 0, depth = 1; i < buffer_length && depth != 0; i++)
        {
            if (i > 0 && buffer[i] == open)
                depth++;
            else if (buffer[i] == close)
                depth--;
        }
    }
    else if (*buffer == '"')
    {
        *is_quoted = !*is_quoted;
        *from = *to = buffer + 1;
        if (*is_quoted)
            return PINTA_TCL_TOKEN_PART;

        if (buffer_length < 2 || (!tcl_is_space(buffer[1]) && !tcl_is_end(buffer[1])))
            return PINTA_TCL_TOKEN_ERROR;

        *from = *to = buffer + 1;
        return PINTA_TCL_TOKEN_WORD;
    }
    else
    {
        while (i < buffer_length && (*is_quoted || !tcl_is_space(buffer[i])) && !tcl_is_special(buffer[i], *is_quoted))
            i++;

    }
    *to = buffer + i;
    if (i == buffer_length)
        return PINTA_TCL_TOKEN_ERROR;

    if (*is_quoted)
        return PINTA_TCL_TOKEN_PART;

    return (tcl_is_space(buffer[i]) || tcl_is_end(buffer[i])) ? PINTA_TCL_TOKEN_WORD : PINTA_TCL_TOKEN_PART;
}

const u8 *tcl_string(PintaTclValue *value)
{
    return value;
}

i32 tcl_int(PintaTclValue *value)
{
    return atoi(value);
}

i32 tcl_length(PintaTclValue *value)
{
    return value == NULL ? 0 : strlen(value);
}

void tcl_free(PintaTclValue *value)
{
    free(value);
}

PintaTclValue *tcl_append_string(PintaTclValue *value, const u8 *buffer, u32 buffer_length)
{
    u32 n = tcl_length(value);
    PintaTclValue* new_value = realloc(value, n + buffer_length + 1);
    if (!new_value) {
        tcl_free(value);
        return NULL;
    }

    value = new_value;
    memset((u8 *)tcl_string(value) + n, 0, buffer_length + 1);
    strncpy((u8 *)tcl_string(value) + n, buffer, buffer_length);
    return value;
}

PintaTclValue *tcl_append(PintaTclValue *value, PintaTclValue *tail)
{
    value = tcl_append_string(value, tcl_string(tail), tcl_length(tail));
    tcl_free(tail);
    return value;
}

PintaTclValue *tcl_alloc(const u8 *buffer, u32 buffer_length)
{
    return tcl_append_string(NULL, buffer, buffer_length);
}

PintaTclValue *tcl_dup(PintaTclValue *value)
{
    return tcl_alloc(tcl_string(value), tcl_length(value));
}

PintaTclValue *tcl_list_alloc()
{
    return tcl_alloc("", 0);
}

i32 tcl_list_length(PintaTclValue *value)
{
    i32 count = 0;
    tcl_each(tcl_string(value), tcl_length(value) + 1, 0)
    {
        if (p.token == PINTA_TCL_TOKEN_WORD)
            count++;
    }
    return count;
}

void tcl_list_free(PintaTclValue *value)
{
    free(value);
}

PintaTclValue *tcl_list_at(PintaTclValue *value, i32 index)
{
    i32 i = 0;
    tcl_each(tcl_string(value), tcl_length(value) + 1, 0)
    {
        if (p.token == PINTA_TCL_TOKEN_WORD)
        {
            if (i == index)
            {
                if (p.from[0] == '{')
                    return tcl_alloc(p.from + 1, p.to - p.from - 2);

                return tcl_alloc(p.from, p.to - p.from);
            }

            i++;
        }
    }
    return NULL;
}

PintaTclValue *tcl_list_append(PintaTclValue *value, PintaTclValue *tail)
{
    if (tcl_length(value) > 0)
        value = tcl_append(value, tcl_alloc(" ", 2));

    if (tcl_length(tail) > 0)
    {
        u32 is_quoted = 0;
        const u8 *part;

        for (part = tcl_string(tail); *part; part++)
        {
            if (tcl_is_space(*part) || tcl_is_special(*part, 0))
            {
                is_quoted = 1;
                break;
            }
        }

        if (is_quoted)
            value = tcl_append(value, tcl_alloc("{", 1));

        value = tcl_append(value, tcl_dup(tail));
        if (is_quoted)
            value = tcl_append(value, tcl_alloc("}", 1));
    }
    else
    {
        value = tcl_append(value, tcl_alloc("{}", 2));
    }
    return value;
}

static PintaTclEnvironment *tcl_env_alloc(PintaNativeMemory *memory, PintaTclEnvironment *parent)
{
    PintaTclEnvironment *environment = pinta_memory_alloc(memory, sizeof(*environment));
    environment->variables = NULL;
    environment->parent = parent;
    return environment;
}

static PintaTclVariable *tcl_env_var(PintaNativeMemory *memory, PintaTclEnvironment *environment, PintaTclValue *name)
{
    PintaTclVariable *var = pinta_memory_alloc(memory, sizeof(PintaTclVariable));
    var->name = tcl_dup(name);
    var->next = environment->variables;
    var->value = tcl_alloc("", 0);
    environment->variables = var;
    return var;
}

static PintaTclEnvironment *tcl_env_free(PintaNativeMemory *memory, PintaTclEnvironment *environment)
{
    PintaTclEnvironment *parent = environment->parent;
    while (environment->variables)
    {
        PintaTclVariable *variables = environment->variables;
        environment->variables = environment->variables->next;
        tcl_free(variables->name);
        tcl_free(variables->value);

        pinta_memory_free(memory, variables);
    }
    pinta_memory_free(memory, environment);
    return parent;
}

PintaTclValue *tcl_variable(PintaTclCore *tcl, PintaTclValue *name, PintaTclValue *value)
{
    PintaTclVariable *variable;

    DBG("var(%s := %.*s)\n", tcl_string(name), tcl_length(value), tcl_string(value));

    for (variable = tcl->environment->variables; variable != NULL; variable = variable->next)
    {
        if (strcmp(variable->name, tcl_string(name)) == 0)
            break;
    }

    if (variable == NULL)
        variable = tcl_env_var(tcl->memory, tcl->environment, name);

    if (value != NULL)
    {
        tcl_free(variable->value);
        variable->value = tcl_dup(value);
        tcl_free(value);
    }

    return variable->value;
}

PintaTclFlow tcl_result(PintaTclCore *tcl, PintaTclFlow flow, PintaTclValue *result)
{
    DBG("tcl_result %.*s, flow=%d\n", tcl_length(result), tcl_string(result), flow);
    tcl_free(tcl->result);
    tcl->result = result;
    return flow;
}

PintaTclFlow tcl_subst(PintaTclCore *tcl, const u8 *buffer, u32 buffer_length)
{
    DBG("subst(%.*s)\n", (int)buffer_length, buffer);
    if (buffer_length == 0)
        return tcl_result(tcl, PINTA_TCL_FLOW_NORMAL, tcl_alloc("", 0));

    switch (buffer[0])
    {
    case '{':
        if (buffer_length <= 1)
            return tcl_result(tcl, PINTA_TCL_FLOW_ERROR, tcl_alloc("", 0));

        return tcl_result(tcl, PINTA_TCL_FLOW_NORMAL, tcl_alloc(buffer + 1, buffer_length - 2));
    case '$':
    {
        if (buffer_length >= MAX_VAR_LENGTH)
            return tcl_result(tcl, PINTA_TCL_FLOW_ERROR, tcl_alloc("", 0));

        u8 buf[5 + MAX_VAR_LENGTH] = "set ";
        strncat(buf, buffer + 1, buffer_length - 1);
        return tcl_eval(tcl, buf, strlen(buf) + 1);
    }
    case '[':
    {
        PintaTclValue *expr = tcl_alloc(buffer + 1, buffer_length - 2);
        PintaTclFlow result = tcl_eval(tcl, tcl_string(expr), tcl_length(expr) + 1);
        tcl_free(expr);
        return result;
    }
    default:
        return tcl_result(tcl, PINTA_TCL_FLOW_NORMAL, tcl_alloc(buffer, buffer_length));
    }
}

PintaTclFlow tcl_eval(PintaTclCore *tcl, const u8 *buffer, u32 buffer_length)
{
    PintaTclValue *list = tcl_list_alloc();
    PintaTclValue *cur = NULL;

    DBG("eval(%.*s)->\n", (int)buffer_length, buffer);

    tcl_each(buffer, buffer_length, 1)
    {
        DBG("tcl_next %d %.*s\n", p.token, (int)(p.to - p.from), p.from);
        switch (p.token)
        {
        case PINTA_TCL_TOKEN_ERROR:
            DBG("eval: PINTA_TCL_FLOW_ERROR, lexer error\n");
            return tcl_result(tcl, PINTA_TCL_FLOW_ERROR, tcl_alloc("", 0));
        case PINTA_TCL_TOKEN_WORD:
            DBG("token %.*s, length=%d, cur=%p (3.1.1)\n", (int)(p.to - p.from), p.from, (int)(p.to - p.from), cur);

            if (cur != NULL)
            {
                PintaTclValue *part;
                tcl_subst(tcl, p.from, p.to - p.from);
                part = tcl_dup(tcl->result);
                cur = tcl_append(cur, part);
            }
            else
            {
                tcl_subst(tcl, p.from, p.to - p.from);
                cur = tcl_dup(tcl->result);
            }

            list = tcl_list_append(list, cur);
            tcl_free(cur);
            cur = NULL;
            break;
        case PINTA_TCL_TOKEN_PART:
        {
            PintaTclValue *part;
            tcl_subst(tcl, p.from, p.to - p.from);
            part = tcl_dup(tcl->result);
            cur = tcl_append(cur, part);
            break;
        }
        case PINTA_TCL_TOKEN_COMMAND:
            if (tcl_list_length(list) == 0)
            {
                tcl_result(tcl, PINTA_TCL_FLOW_NORMAL, tcl_alloc("", 0));
            }
            else
            {
                PintaTclValue *command_name = tcl_list_at(list, 0);
                PintaTclCommand *command = NULL;
                PintaTclFlow result = PINTA_TCL_FLOW_ERROR;
                for (command = tcl->commands; command != NULL; command = command->next)
                {
                    if (strcmp(tcl_string(command_name), tcl_string(command->name)) == 0)
                    {
                        if (command->argument_count == 0 || command->argument_count == tcl_list_length(list))
                        {
                            result = command->function(tcl, list, command->user_data);
                            break;
                        }
                    }
                }
                tcl_free(command_name);
                if (command == NULL || result != PINTA_TCL_FLOW_NORMAL)
                {
                    tcl_list_free(list);
                    return result;
                }
            }
            tcl_list_free(list);
            list = tcl_list_alloc();
        }
    }
    tcl_list_free(list);
    return PINTA_TCL_FLOW_NORMAL;
}

void tcl_register(PintaTclCore *tcl, const u8 *name, PintaTclCommandFunction function, u32 argument_count, void *user_data)
{
    PintaTclCommand *cmd = pinta_memory_alloc(tcl->memory, sizeof(PintaTclCommand));
    cmd->name = tcl_alloc(name, strlen(name));
    cmd->function = function;
    cmd->user_data = user_data;
    cmd->argument_count = argument_count;
    cmd->next = tcl->commands;
    tcl->commands = cmd;
}

static PintaTclFlow tcl_cmd_set(PintaTclCore *tcl, PintaTclValue *arguments, void *user_data)
{
    PintaTclValue *variable = tcl_list_at(arguments, 1);
    PintaTclValue *value = tcl_list_at(arguments, 2);
    PintaTclFlow result = tcl_result(tcl, PINTA_TCL_FLOW_NORMAL, tcl_dup(tcl_variable(tcl, variable, value)));

    (void)user_data;

    tcl_free(variable);
    return result;
}

static PintaTclFlow tcl_cmd_subst(PintaTclCore *tcl, PintaTclValue *arguments, void *user_data)
{
    PintaTclValue *s = tcl_list_at(arguments, 1);
    PintaTclFlow result = tcl_subst(tcl, tcl_string(s), tcl_length(s));

    (void)user_data;

    tcl_free(s);
    return result;
}

static PintaTclFlow tcl_user_proc(PintaTclCore *tcl, PintaTclValue *args, void *user_data)
{
    i32 i;
    PintaTclValue *code = (PintaTclValue *)user_data;
    PintaTclValue *params = tcl_list_at(code, 2);
    PintaTclValue *body = tcl_list_at(code, 3);

    tcl->environment = tcl_env_alloc(tcl->memory, tcl->environment);
    for (i = 0; i < tcl_list_length(params); i++)
    {
        PintaTclValue *param = tcl_list_at(params, i);
        PintaTclValue *v = tcl_list_at(args, i + 1);
        tcl_variable(tcl, param, v);
        tcl_free(param);
    }

    tcl_eval(tcl, tcl_string(body), tcl_length(body) + 1);
    tcl->environment = tcl_env_free(tcl->memory, tcl->environment);
    tcl_free(params);
    tcl_free(body);
    return PINTA_TCL_FLOW_NORMAL;
}

static PintaTclFlow tcl_cmd_proc(PintaTclCore *tcl, PintaTclValue *arguments, void *user_data)
{
    PintaTclValue *name = tcl_list_at(arguments, 1);

    (void)user_data;

    tcl_register(tcl, tcl_string(name), tcl_user_proc, 0, tcl_dup(arguments));
    tcl_free(name);
    return tcl_result(tcl, PINTA_TCL_FLOW_NORMAL, tcl_alloc("", 0));
}

static PintaTclFlow tcl_cmd_if(PintaTclCore *tcl, PintaTclValue *arguments, void *user_data)
{
    i32 i = 1;
    i32 length = tcl_list_length(arguments);
    PintaTclFlow result = PINTA_TCL_FLOW_NORMAL;

    (void)user_data;

    while (i < length)
    {
        PintaTclValue *condition = tcl_list_at(arguments, i);
        PintaTclValue *branch = NULL;

        if (i + 1 < length)
            branch = tcl_list_at(arguments, i + 1);

        result = tcl_eval(tcl, tcl_string(condition), tcl_length(condition) + 1);
        tcl_free(condition);

        if (result != PINTA_TCL_FLOW_NORMAL)
        {
            if (branch != NULL)
                tcl_free(branch);
            break;
        }

        if (branch != NULL && tcl_int(tcl->result))
        {
            result = tcl_eval(tcl, tcl_string(branch), tcl_length(branch) + 1);
            tcl_free(branch);
            break;
        }

        i = i + 2;

        if (branch != NULL)
            tcl_free(branch);
    }

    return result;
}

static PintaTclFlow tcl_cmd_flow(PintaTclCore *tcl, PintaTclValue *arguments, void *user_data)
{
    PintaTclFlow result = PINTA_TCL_FLOW_ERROR;
    PintaTclValue *flow_value = tcl_list_at(arguments, 0);
    const u8 *flow = tcl_string(flow_value);

    (void)user_data;

    if (strcmp(flow, "break") == 0)
        result = PINTA_TCL_FLOW_BREAK;
    else if (strcmp(flow, "continue") == 0)
        result = PINTA_TCL_FLOW_AGAIN;
    else if (strcmp(flow, "return") == 0)
        result = tcl_result(tcl, PINTA_TCL_FLOW_RETURN, tcl_list_at(arguments, 1));

    tcl_free(flow_value);
    return result;
}

static PintaTclFlow tcl_cmd_while(PintaTclCore *tcl, PintaTclValue *arguments, void *user_data)
{
    PintaTclValue *condition = tcl_list_at(arguments, 1);
    PintaTclValue *loop = tcl_list_at(arguments, 2);
    PintaTclFlow result;

    (void)user_data;

    for (;;)
    {
        result = tcl_eval(tcl, tcl_string(condition), tcl_length(condition) + 1);
        if (result != PINTA_TCL_FLOW_NORMAL)
        {
            tcl_free(condition);
            tcl_free(loop);
            return result;
        }
        if (!tcl_int(tcl->result))
        {
            tcl_free(condition);
            tcl_free(loop);
            return PINTA_TCL_FLOW_NORMAL;
        }

        PintaTclFlow eval_result = tcl_eval(tcl, tcl_string(loop), tcl_length(loop) + 1);
        switch (eval_result)
        {
        case PINTA_TCL_FLOW_BREAK:
            tcl_free(condition);
            tcl_free(loop);
            return PINTA_TCL_FLOW_NORMAL;
        case PINTA_TCL_FLOW_RETURN:
            tcl_free(condition);
            tcl_free(loop);
            return PINTA_TCL_FLOW_RETURN;
        case PINTA_TCL_FLOW_AGAIN:
            continue;
        case PINTA_TCL_FLOW_ERROR:
            tcl_free(condition);
            tcl_free(loop);
            return PINTA_TCL_FLOW_ERROR;
        }
    }
}

static PintaTclFlow tcl_cmd_math(PintaTclCore *tcl, PintaTclValue *arguments, void *user_data)
{
    u8 buffer[64];
    PintaTclValue *operator_value = tcl_list_at(arguments, 0);
    PintaTclValue *left_value = tcl_list_at(arguments, 1);
    PintaTclValue *right_value = tcl_list_at(arguments, 2);
    const u8 *op = tcl_string(operator_value);
    u8 *part;
    u32 is_negative;
    i32 left = tcl_int(left_value);
    i32 right = tcl_int(right_value);
    i32 value = 0;

    (void)user_data;

    if (op[0] == '+')
    {
        value = left + right;
    }
    else if (op[0] == '-')
    {
        value = left - right;
    }
    else if (op[0] == '*')
    {
        value = left * right;
    }
    else if (op[0] == '/')
    {
        value = left / right;
    }
    else if (op[0] == '>' && op[1] == '\0')
    {
        value = left > right;
    }
    else if (op[0] == '>' && op[1] == '=')
    {
        value = left >= right;
    }
    else if (op[0] == '<' && op[1] == '\0')
    {
        value = left < right;
    }
    else if (op[0] == '<' && op[1] == '=')
    {
        value = left <= right;
    }
    else if (op[0] == '=' && op[1] == '=')
    {
        value = left == right;
    }
    else if (op[0] == '!' && op[1] == '=')
    {
        value = left != right;
    }

    part = buffer + sizeof(buffer) - 1;
    is_negative = (value < 0);
    *part-- = 0;
    if (is_negative)
        value = -value;

    do
    {
        *part-- = '0' + (value % 10);
        value = value / 10;
    } while (value > 0);

    if (is_negative)
        *part-- = '-';

    part++;

    tcl_free(operator_value);
    tcl_free(left_value);
    tcl_free(right_value);
    return tcl_result(tcl, PINTA_TCL_FLOW_NORMAL, tcl_alloc(part, strlen(part)));
}

void tcl_init(PintaTclCore *tcl, u8 *heap, u32 heap_size)
{
    u8 *math[] = { "+", "-", "*", "/", ">", ">=", "<", "<=", "==", "!=" };

    tcl->memory = pinta_memory_init(heap, heap_size);
    tcl->environment = tcl_env_alloc(tcl->memory, NULL);
    tcl->result = tcl_alloc("", 0);
    tcl->commands = NULL;
    tcl->user_data = NULL;

    tcl_register(tcl, "set", tcl_cmd_set, 0, NULL);
    tcl_register(tcl, "subst", tcl_cmd_subst, 2, NULL);
    tcl_register(tcl, "proc", tcl_cmd_proc, 4, NULL);
    tcl_register(tcl, "if", tcl_cmd_if, 0, NULL);
    tcl_register(tcl, "while", tcl_cmd_while, 3, NULL);
    tcl_register(tcl, "return", tcl_cmd_flow, 0, NULL);
    tcl_register(tcl, "break", tcl_cmd_flow, 1, NULL);
    tcl_register(tcl, "continue", tcl_cmd_flow, 1, NULL);

    for (u32 i = 0; i < (sizeof(math) / sizeof(math[0])); i++)
        tcl_register(tcl, math[i], tcl_cmd_math, 3, NULL);
}

void tcl_destroy(PintaTclCore *tcl)
{
    while (tcl->environment)
        tcl->environment = tcl_env_free(tcl->memory, tcl->environment);

    while (tcl->commands)
    {
        PintaTclCommand *command = tcl->commands;

        tcl->commands = tcl->commands->next;
        tcl_free(command->name);

        pinta_memory_free(tcl->memory, command);
    }

    tcl_free(tcl->result);
}

static PintaTclFlow tcl_cmd_run(PintaTclCore *tcl, PintaTclValue *arguments, void *user_data)
{
    PintaCore *core = (PintaCore*)tcl->user_data;
    PintaWebContext *context = (PintaWebContext*)user_data;
    PintaTclValue *mode = NULL;
    i32 length = tcl_list_length(arguments);

    if (length > 1)
    {
        mode = tcl_list_at(arguments, 1);
        if (_strcmpi("step", mode) == 0)
            pinta_debugger_step(core, context);
        tcl_free(mode);
    }
    else
    {
        pinta_debugger_run(core, context);
    }

    return PINTA_TCL_FLOW_NORMAL;
}

static PintaTclFlow tcl_cmd_domain(PintaTclCore *tcl, PintaTclValue *arguments, void *user_data)
{
    PintaCore *core = (PintaCore*)tcl->user_data;
    PintaWebContext *context = (PintaWebContext*)user_data;
    PintaTclValue *target = tcl_list_at(arguments, 1);

    if (_strcmpi("strings", target) == 0)
        pinta_debugger_send_module_strings(core, context);
    else if (_strcmpi("globals", target) == 0)
        pinta_debugger_send_module_globals(core, context);

    tcl_free(target);
    return PINTA_TCL_FLOW_NORMAL;
}

static PintaTclFlow tcl_cmd_frame(PintaTclCore *tcl, PintaTclValue *arguments, void *user_data)
{
    PintaCore *core = (PintaCore*)tcl->user_data;
    PintaWebContext *context = (PintaWebContext*)user_data;
    i32 length = tcl_list_length(arguments);
    PintaTclValue *index;
    PintaTclValue *target = NULL;
    u32 frame_index = 0;

    if (length > 0)
    {
        index = tcl_list_at(arguments, 1);
        frame_index = (u32)atoi(index);
        tcl_free(index);
    }

    if (length > 1)
        target = tcl_list_at(arguments, 2);

    if (target == NULL)
    {
        pinta_debugger_send_frame_stack(core, context, frame_index);
    }
    else
    {
        if (_strcmpi(target, "this") == 0)
            pinta_debugger_send_frame_this(core, context, frame_index);
        else if (_strcmpi(target, "locals") == 0)
            pinta_debugger_send_frame_locals(core, context, frame_index);
        else if (_strcmpi(target, "arguments") == 0)
            pinta_debugger_send_frame_arguments(core, context, frame_index);
        else if (_strcmpi(target, "closure") == 0)
            pinta_debugger_send_frame_closure(core, context, frame_index);
        else if (_strcmpi(target, "body") == 0)
            pinta_debugger_send_frame_body(core, context, frame_index);
        else
            pinta_debugger_send_frame_stack(core, context, frame_index);
    }

    if (target != NULL)
        tcl_free(target);

    return PINTA_TCL_FLOW_NORMAL;
}

PintaException pinta_debugger_command_execute(PintaCore *core, PintaWebContext *context, u8 *buffer, u32 buffer_length)
{
    PintaException exception = PINTA_OK;
    PintaTclCore tcl;
    PintaTclFlow result;

    tcl_init(&tcl, tcl_heap, sizeof(tcl_heap));
    tcl.user_data = core;

    tcl_register(&tcl, "run", tcl_cmd_run, 0, context);
    tcl_register(&tcl, "domain", tcl_cmd_domain, 2, context);
    tcl_register(&tcl, "frame", tcl_cmd_frame, 0, context);

    result = tcl_eval(&tcl, buffer, buffer_length);
    if (result != PINTA_TCL_FLOW_NORMAL)
        PINTA_THROW(PINTA_EXCEPTION_INVALID_OPERATION);

PINTA_EXIT:
    tcl_destroy(&tcl);
    return exception;
}