
/**
  * Script definition:
  * SCRIPT ::= {LINE}
  * LINE ::= {LABEL ":" (ASSIGN_EXPR | IF_EXPR | ("GOTO" NUMBER) | ("DELAY" NUMBER) | ("CALL" NUMBER) | "RET" | "END" | ACTION)} "\r\n"
  * LABEL ::= NUMBER
  * EXPR ::= (NUMBER | VAR) {("+" | "-" | "*" | "/") (NUMBER | VAR)}
  * ASSIGN_EXPR := "LET" VAR "=" EXPR
  * IF_EXPR := "IF" "(" EXPR (">" | "<" | ">=" | "<=" | "==" | "!=") EXPR ")" "THEN" LABEL
  * VAR ::= ("A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" | "J" | "K" | "L" | "M" | "N" | "O" | "P" | "Q" | "R" | "S" | "T" | "U" | "V" | "W" | "X" | "Y" | "Z")
  * ACTION ::= ("WAIT", NODE_ID{"," NODE_ID}) | COMMANDS
  * COMMANDS ::= NODE_ID "," COMMAND{";" NODE_ID "," COMMAND}
  * COMMAND ::= ("START" NUMBER) | "STOP" | "HALT" | ("VM" NUMBER) | ("PVM" NUMBER "," NUMBER) | ("AP" NUMBER) | ("PAP" NUMBER "," NUMBER "," NUMBER) | ("RP" NUMBER) | ("PRP" NUMBER "," NUMBER "," NUMBER)
  *
  * Note: ASSIGN_EXPR uses left precedence
  */

/* Includes ------------------------------------------------------------------*/

#include "ScriptProcessor.h"

#include <string.h>
#include <stdio.h>
#include <malloc.h>


/* Private typedef -----------------------------------------------------------*/
typedef struct
{
    int16_t label;
    const char *startOfLine;
}   LINE_ENTRY;


/* Private define ------------------------------------------------------------*/

#define DELAY_MS(ms)        \
    DelayMilliSecondsImpl(ms)


#define SAFE_FREE(p)        \
    do {                    \
        free(p);            \
        p = NULL;           \
    }   while(0)


#define SKIP_SPACE(p)                              \
    do {                                           \
        while(*p == ' ' || *p == '\t' || *p == ';')\
            p++;                                   \
    }   while(0)


#define FORCE_EXEC(func, node_id, local_error_callback, node_error_callback, log_func)       \
    do {                                                                                     \
        uint8_t ret;                                                                         \
        if (log_func)                                                                        \
            log_func(node_id, #func);                                                        \
        while ((ret = func) != MMS_RESP_SUCCESS)                                             \
        {                                                                                    \
            local_error_callback(node_id, ret);                                              \
            if (_stop)                                                                       \
                return 0;                                                                    \
            DELAY_MS(100);                                                                   \
        }                                                                                    \
    }   while(0)


#define FORCE_MOVE(func, node_id, local_error_callback, node_error_callback, log_func)       \
    do {                                                                                     \
        uint8_t ret;                                                                         \
        if (log_func)                                                                        \
            log_func(node_id, #func);                                                        \
        while ((ret = func) != MMS_RESP_SUCCESS)                                             \
        {                                                                                    \
            local_error_callback(node_id, ret);                                              \
            if (_stop)                                                                       \
                return 0;                                                                    \
            if (ret == MMS_RESP_SERVO_ERROR)                                                 \
            {                                                                                \
                uint8_t status, in_position;                                                 \
                log_func(node_id, "Check control status.");                                  \
                while ((ret = MMS_GetControlStatus(node_id, &status, &in_position,           \
                                                   node_error_callback)) != MMS_RESP_SUCCESS)\
                {                                                                            \
                    local_error_callback(node_id, ret);                                      \
                    if (_stop)                                                               \
                        return 0;                                                            \
                    DELAY_MS(100);                                                           \
                }                                                                            \
                if (status == MMS_CTRL_STATUS_NO_CONTROL)                                    \
                {                                                                            \
                    log_func(node_id, "Restart servo.");                                     \
                    while ((ret = MMS_StartServo(node_id, MMS_MODE_KEEP,                     \
                                                 node_error_callback)) != MMS_RESP_SUCCESS)  \
                    {                                                                        \
                        local_error_callback(node_id, ret);                                  \
                        if (_stop)                                                           \
                            return 0;                                                        \
                        DELAY_MS(100);                                                       \
                    }                                                                        \
                }                                                                            \
            }                                                                                \
            DELAY_MS(100);                                                                   \
        }                                                                                    \
    }   while(0)


/* Private macro -------------------------------------------------------------*/

#define MAX_STACK_SIZE 10


/* Private variables ---------------------------------------------------------*/

static uint8_t _stop = 0;
static int16_t vars[26] = {0};  /* 'A' to 'Z' */
static int16_t _nextLabel = 0;  /*      */
static char *_scriptBuf = NULL; /*      */
static LINE_ENTRY *_lineEntries = NULL;
static int _lineCount = -1;


static uint16_t _run_stack[MAX_STACK_SIZE];
static int8_t _stack_pointer = -1;


/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  Execute oneline & return result
  * @note   Called internally by MMS_ExecOneStep()
  * @param  scriptLine: Ecript line to execute
  * @param  nextLabel: Next label to execute, 0 indicates ended, -1 indicates next line, positive value means next label
  * @param  local_error_callback: call back function for local error
  * @param  node_error_callback: call back function for servo error
  * @param  DelayMilliSecondsImpl: ms delay function pointer
  * @retval >=0: succeeded
  *         <0 : something error
  */
static int16_t MMScript_ProcessLine(uint16_t *lineNum, int16_t *nextLabel, MMSCRIPT_LOCAL_ERROR_CALLBACK local_error_callback, MMSCRIPT_NODE_ERROR_CALLBACK node_error_callback, void (*DelayMilliSecondsImpl)(uint32_t ms), MMSCRIPT_LOG log_func);


/**
  * @brief  Evaluate expression
  * @note   This function evaluates expression.
  * @param  expr: address of expression input
  * @param  eval_out: evaluate of expreesion input
  * @retval =0: ended without any error
  *         <0: something error, see exec error codes for detailed info
  */
static int16_t MMScript_Eval(const char *expr, int16_t *eval_out);


/**
  * @brief  Push into the Stack
  * @note   Stack operation
  * @param  val: the linenumber of command input
  * @retval >0: ended without any error
  *         =0: stack full
  */
static int16_t MMScript_PushStack(uint16_t val);


/**
  * @brief  Pop the top element of the Stack
  * @note   Stack operation
  * @param  val: the pointer of variable to store value
  * @retval >0: ended without any error
  *         =0: stack empty
  */
static int16_t MMScript_PopStack(uint16_t *val);


/* Private functions ---------------------------------------------------------*/

static int16_t MMScript_ProcessLine(uint16_t *lineNum, int16_t *nextLabel, MMSCRIPT_LOCAL_ERROR_CALLBACK local_error_callback, MMSCRIPT_NODE_ERROR_CALLBACK node_error_callback, void (*DelayMilliSecondsImpl)(uint32_t ms), MMSCRIPT_LOG log_func)
{
    const char *scriptLine = _lineEntries[*lineNum].startOfLine;
    const char *p = scriptLine;

    *nextLabel = -1;    /* Default to next line */

    if (scriptLine== NULL)
        return MMS_PARSE_ERR_MALLOC;

    if (strncmp(scriptLine, "CALL", 4) == 0)
    {
        //
        // CALL

        int label;

        if (sscanf(scriptLine + 4, "%d", &label) != 1)
            return MMS_ERR_MISSING_CALL_PARAM;

        if (!MMScript_PushStack(*lineNum))
            return MMS_ERR_FULL_STACK;

        *nextLabel = label;
    }
    else if (strncmp(scriptLine, "RET", 3) == 0)
    {
        //
        // RET

        if (!MMScript_PopStack(lineNum))
          return MMS_ERR_EMPTY_STACK;
    }
    else if (strncmp(scriptLine, "LET", 3) == 0)
    {
        //
        // LET

        int16_t ret;
        int16_t result = 0;

        char varname;

        p = strstr(scriptLine, "=");                             /*Find '=' and return the point to p*/
        if (p == NULL)
            return MMS_ERR_MISSING_LET_EQUAL;

        p = scriptLine;
        p += 3;
        SKIP_SPACE(p);

        if (*p < 'A' || *p > 'Z')
            return MMS_ERR_MISSING_LET_VARNAME;

        varname = *p;

        p++;
        while (*p != '=')
        {
            if (*p != ' ')
                return MMS_ERR_INVALID_LET_VARNAME;

            p++;
        }

        p++;
        if ((*p < 'A' || *p > 'Z') && (*p < '0' || *p > '9') && *p != ' ')
            return MMS_ERR_INVALID_LET_EQUAL;

        SKIP_SPACE(p);

        ret = MMScript_Eval(p, &result);

        if(ret < 0)
            return ret;

        vars[varname - 'A'] = result;
    }
    else if (strncmp(scriptLine, "IF", 2) == 0)
    {
        //
        // IF

        int16_t ret;

        int16_t left_val = 0;
        int16_t right_val = 0;
        const char *expr_start;
        char *expr_end;
        char orig_char;
        char *op = NULL;
        char *left_bracket, *right_bracket;

        int label;

        left_bracket = strstr(scriptLine, "(");
        right_bracket = strstr(scriptLine, ")");

        if (left_bracket == NULL || right_bracket == NULL)   /*Find '(' ')' and check*/
            return MMS_ERR_MISSING_IF_BRACKETS;

        if (strstr(left_bracket + 1, "(") != NULL || strstr(right_bracket + 1, ")") != NULL)
            return MMS_ERR_INVALID_IF_BRACKETS;

        p = left_bracket + 1;
        SKIP_SPACE(p);

        expr_start = p;

        /* Locate end of expression */
        while (*p)
        {
            if (*p == '>' || *p == '<' || *p == '=' || *p == '!')
            {
                op = (char *)p;
                orig_char = *op;
                expr_end = (char *)p;
                *expr_end = '\0';

                break;
            }

            p++;
        }

        if (op == NULL)
        {
            return MMS_ERR_INVALID_IF_SYNTAX;
        }

        /* Eval */
        ret = MMScript_Eval(expr_start, &right_val);

        /* Recover */
        *op = orig_char;

        if (ret < 0)
            return ret;

        /* Parse right expr */
        while (*p == '>' || *p == '<' || *p == '=' || *p == '!')
            p++;

        expr_start = p;
        expr_end = strstr(scriptLine, ")");
        *expr_end = '\0';

        ret = MMScript_Eval(expr_start, &left_val);

        /* Recover */
        *expr_end = ')';

        if (ret < 0)
            return ret;

        p = strstr(scriptLine, "THEN");          /* THEN LABEL*/

        if(p == NULL)
            return MMS_ERR_INVALID_IF_SYNTAX;

        p += 4;

        if (sscanf(p, "%d", &label) != 1)
            return MMS_ERR_MISSING_THEN_PARAM;    /**/


        if (strncmp(op, "==", 2) == 0)
        {
            if (right_val == left_val)
            {
                *nextLabel = label;
            }
        }
        else if (strncmp(op, ">=", 2) == 0)
        {
            if (right_val >= left_val)
            {
                *nextLabel = label;
            }
        }
        else if (strncmp(op, "<=", 2) == 0)
        {
            if (right_val <= left_val)
            {
                *nextLabel = label;
            }
        }
        else if (strncmp(op, "!=", 2) == 0)
        {
            if (right_val != left_val)
            {
                *nextLabel = label;
            }
        }
        else if (strncmp(op, ">", 1) == 0)
        {
            if (right_val > left_val)
            {
                *nextLabel = label;
            }
        }
        else if (strncmp(op, "<", 1) == 0)
        {
            if (right_val < left_val)
            {
                *nextLabel = label;
            }
        }
        else
            return MMS_ERR_INVALID_IF_OPERATOR;
    }
    else if (strncmp(scriptLine, "GOTO", 4) == 0)
    {
        //
        // PARAMETERS
        int label;

        if (sscanf(scriptLine + 4, "%d", &label) != 1)
            return MMS_ERR_MISSING_GOTO_PARAM;

        *nextLabel = label;
    }
    else if (strncmp(scriptLine, "END", 3) == 0)
    {
        *nextLabel = 0;
    }
    else if (strncmp(scriptLine, "DELAY", 5) == 0)
    {
        //
        // PARAMETERS
        int delay;

        if (sscanf(scriptLine + 5, "%d", &delay) != 1)
            return MMS_ERR_MISSING_DELAY_PARAM;

        // Delay
        DELAY_MS(delay);
    }
    else if (strncmp(scriptLine, "WAIT", 4) == 0)
    {
        scriptLine += 4;
        p = scriptLine;   //strtok(line, ",");

        while (p)
        {
            int node_id;
            uint8_t status, in_position;

            if (sscanf(p, "%x", &node_id) != 1)
                return MMS_ERR_MISSING_WAIT_PARAM;

            //
            // Wait specified servo to finish command

            status = MMS_CTRL_STATUS_NO_CONTROL;
            in_position = 0;

            while (status != MMS_CTRL_STATUS_POSITION_CONTROL || in_position == 0)
            {
                if (_stop)
                    return 0;

                DELAY_MS(100);
                FORCE_EXEC(MMS_GetControlStatus((uint8_t)node_id, &status, &in_position, node_error_callback), node_id, local_error_callback, node_error_callback, log_func);

                if (status == MMS_CTRL_STATUS_NO_CONTROL)
                {
                    uint8_t ret;

                    log_func(node_id, "Restart servo.");

                    while ((ret = MMS_StartServo(node_id, MMS_MODE_KEEP,
                                                 node_error_callback)) != MMS_RESP_SUCCESS)
                    {
                        local_error_callback(node_id, ret);
                        if (_stop)
                            return 0;
                        DELAY_MS(100);
                    }
                }
            }

            //p = strtok(NULL, ",");
            p = strchr(p, ',');
            if (p)
            {
                p += 1; /* Skip ',' */
            }
        }
    }
    else
    {
        //
        // Get COMMAND

        p = scriptLine;

        while (p != NULL)
        {
            int node_id;
            char *token;

            //
            // NODE_ID

            token = strchr(p, ',');

            if (!token)
                return MMS_ERR_MISSING_NODE_ID;

            if (sscanf(p, "%x", &node_id) != 1)
                return MMS_ERR_MISSING_NODE_ID;

            p = token + 1;
            SKIP_SPACE(p);

            //
            // COMMAND_ID

            if (strncmp(p, "START", 5) == 0)
            {
                //
                // PARAMETERS
                int mode;

                if (sscanf(p + 5, "%d", &mode) != 1)
                    return MMS_ERR_ERROR_START_PARAM;

                if (mode != MMS_MODE_KEEP && mode != MMS_MODE_ZERO && mode != MMS_MODE_RESET)
                    return MMS_ERR_ERROR_START_PARAM;

                FORCE_EXEC(MMS_ResetError(node_id, node_error_callback), node_id, local_error_callback, node_error_callback, log_func);
                FORCE_EXEC(MMS_StartServo(node_id, mode, node_error_callback), node_id, local_error_callback, node_error_callback, log_func);
            }
            else if (strncmp(p, "STOP", 4) == 0)
            {
                FORCE_EXEC(MMS_StopServo(node_id, node_error_callback), node_id, local_error_callback, node_error_callback, log_func);
            }
            else if (strncmp(p, "HALT", 4) == 0)
            {
                FORCE_EXEC(MMS_HaltServo(node_id, node_error_callback), node_id, local_error_callback, node_error_callback, log_func);
            }
            else if (strncmp(p, "VM", 2) == 0)
            {
                //
                // PARAMETERS
                int velocity;

                if (sscanf(p + 2, "%d", &velocity) != 1)
                    return MMS_ERR_MISSING_VM_PARAM;

                FORCE_MOVE(MMS_ProfiledVelocityMove(node_id, velocity, node_error_callback), node_id, local_error_callback, node_error_callback, log_func);
            }
            else if (strncmp(p, "PVM", 3) == 0)
            {
                //
                // PARAMETERS
                int accel, velocity;

                if (sscanf(p + 3, "%d,%d", &accel, &velocity) != 2)
                    return MMS_ERR_MISSING_PVM_PARAM;

                FORCE_EXEC(MMS_SetProfileAcceleration(node_id, accel, node_error_callback), node_id, local_error_callback, node_error_callback, log_func);
                FORCE_MOVE(MMS_ProfiledVelocityMove(node_id, velocity, node_error_callback), node_id, local_error_callback, node_error_callback, log_func);
            }
            else if (strncmp(p, "AP", 2) == 0)
            {
                //
                // PARAMETERS
                long position;

                if (sscanf(p + 2, "%ld", &position) != 1)
                    return MMS_ERR_MISSING_AP_PARAM;

                FORCE_MOVE(MMS_AbsolutePositionMove(node_id, position, node_error_callback), node_id, local_error_callback, node_error_callback, log_func);
            }
            else if (strncmp(p, "PAP", 3) == 0)
            {
                //
                // PARAMETERS
                int accel, velocity;
                long position;

                if (sscanf(p + 3, "%d,%d,%ld", &accel, &velocity, &position) != 3)
                    return MMS_ERR_MISSING_PAP_PARAM;

                FORCE_EXEC(MMS_SetProfileAcceleration(node_id, accel, node_error_callback), node_id, local_error_callback, node_error_callback, log_func);
                FORCE_EXEC(MMS_SetProfileVelocity(node_id, velocity, node_error_callback), node_id, local_error_callback, node_error_callback, log_func);
                FORCE_MOVE(MMS_ProfiledAbsolutePositionMove(node_id, position, node_error_callback), node_id, local_error_callback, node_error_callback, log_func);
            }
            else if (strncmp(p, "RP", 2) == 0)
            {
                //
                // PARAMETERS
                long position;

                if (sscanf(p + 2, "%ld", &position) != 1)
                    return MMS_ERR_MISSING_RP_PARAM;

                FORCE_MOVE(MMS_RelativePositionMove(node_id, position, node_error_callback), node_id, local_error_callback, node_error_callback, log_func);
            }
            else if (strncmp(p, "PRP", 3) == 0)
            {
                //
                // PARAMETERS
                int accel, velocity;
                long position;

                if (sscanf(p + 3, "%d,%d,%ld", &accel, &velocity, &position) != 3)
                    return MMS_ERR_MISSING_PRP_PARAM;

                FORCE_EXEC(MMS_SetProfileAcceleration(node_id, accel, node_error_callback), node_id, local_error_callback, node_error_callback, log_func);
                FORCE_EXEC(MMS_SetProfileVelocity(node_id, velocity, node_error_callback), node_id, local_error_callback, node_error_callback, log_func);
                FORCE_MOVE(MMS_ProfiledRelativePositionMove(node_id, position, node_error_callback), node_id, local_error_callback, node_error_callback, log_func);
            }
            else
            {
                // Unknown command
                return MMS_ERR_UNKNOWN_COMMAND;
            }

            // next COMMAND
            p = strchr(p, ';');

            if (p)
                p++;    /* Skip ';' */
        }
    }

    return 0;
}


static int16_t MMScript_Eval(const char *expr, int16_t *result)
{
    int number = 0;
    int n = 0;

    SKIP_SPACE(expr);

    if (*expr <= '9' && *expr >= '0')
    {
        sscanf(expr,"%d%n", &number, &n);

        if (number > INT16_MAX || number < INT16_MIN)
            return MMS_ERR_INVALID_EXPR_ITEM;

        *result = (int16_t)number;
        expr += n;
    }
    else if (*expr <= 'Z' && *expr >= 'A')
    {
        *result = vars[(*expr) - 'A'];
        expr++;
    }
    else
        return MMS_ERR_INVALID_EXPR_ITEM;

    SKIP_SPACE(expr);

    while (*expr)
    {
        if (*expr == '+')
        {
            expr++;
            SKIP_SPACE(expr);
            if (*expr <= '9' && *expr >= '0')
            {
                sscanf(expr,"%d%n", &number, &n);
                *result += number;
                expr += n;
            }
            else if (*expr <= 'Z' && *expr >= 'A')
            {
                *result += vars[(*expr) - 'A'];
                expr++;
            }
            else
                return MMS_ERR_INVALID_EXPR_ITEM;
        }
        else if (*expr == '-')
        {
            expr++;
            SKIP_SPACE(expr);
            if (*expr <= '9' && *expr >= '0')
            {
                sscanf(expr,"%d%n", &number, &n);
                *result -= number;
                expr += n;
            }
            else if (*expr <= 'Z' && *expr >= 'A')
            {
                *result -= vars[(*expr) - 'A'];
                expr++;
            }
            else
                return MMS_ERR_INVALID_EXPR_ITEM;
        }
        else if (*expr == '*')
        {
            expr++;
            SKIP_SPACE(expr);
            if (*expr <= '9' && *expr >= '0')
            {
                sscanf(expr,"%d%n", &number, &n);
                *result *= number;
                expr += n;
            }
            else if (*expr <= 'Z' && *expr >= 'A')
            {
                *result *= vars[(*expr) - 'A'];
                expr++;
            }
            else
                return MMS_ERR_INVALID_EXPR_ITEM;
        }
        else if (*expr == '/')
        {
            expr++;
            SKIP_SPACE(expr);
            if (*expr <= '9' && *expr >= '0')
            {
                sscanf(expr,"%d%n", &number, &n);
                *result /= number;
                expr += n;
            }
            else if (*expr <= 'Z' && *expr >= 'A')
            {
                *result /= vars[(*expr) - 'A'];
                expr++;
            }
            else
                return MMS_ERR_INVALID_EXPR_ITEM;
        }
        else if (*expr >= 'A' && *expr <= 'Z')
            return MMS_ERR_INVALID_EXPR_ITEM;
        else if (*expr == ' ')
            SKIP_SPACE(expr);
        else
            return MMS_ERR_INVALID_EXPR_OPERATOR;

        SKIP_SPACE(expr);
    }
    return 0;
}


static int16_t MMScript_PushStack(uint16_t val)
{
    if (_stack_pointer >= (MAX_STACK_SIZE - 1))
    {
        return 0;
    }
    else
    {
        _run_stack[++_stack_pointer] = val;
        return 1;
    }
}


static int16_t MMScript_PopStack(uint16_t *val)
{
    if (_stack_pointer == -1)
        return 0;
    else
    {
        *val = _run_stack[_stack_pointer--];
        return 1;
    }
}

/* Public functions ---------------------------------------------------------*/

int16_t MMScript_ParseScript(char *scriptBuf, size_t bufLen)
{
    _scriptBuf = scriptBuf;

    /**
      * Count lines
      */

    _lineCount = 1;

    for (size_t i=0; i<bufLen; i++)
    {
        if (*(_scriptBuf + i) == '\n')
        {
            _lineCount++;
        }
    }

    if (_lineEntries != NULL)
        SAFE_FREE(_lineEntries);

    _lineEntries = (LINE_ENTRY*)malloc(_lineCount * sizeof(LINE_ENTRY));

    if (_lineEntries == NULL)
    {
        SAFE_FREE(_scriptBuf);
        return MMS_PARSE_ERR_MALLOC;
    }


    /**
      * Locate lines
      */

    _lineEntries[0].startOfLine = _scriptBuf;
    int currLine = 1;

    for (size_t i=0; i<bufLen; i++)
    {
        if (*(_scriptBuf + i) == '\n')
        {
            *(_scriptBuf + i) = '\0';
            _lineEntries[currLine].startOfLine = _scriptBuf + i + 1;
            SKIP_SPACE(_lineEntries[currLine].startOfLine);
            currLine++;
        }
    }


    /**
      * Save label for each line
      */

    for (int i=0; i<_lineCount; i++)
    {
        const char *line = _lineEntries[i].startOfLine;

        //
        // Trim

        SKIP_SPACE(line);

        char *p = (char*)line + strlen(line) - 1;

        while (*p == '\r' || *p == ';')
        {
            *p = '\0';
            p--;
        }

        if (*line == '\0')
        {
            _lineEntries[i].label = -1;
            _lineEntries[i].startOfLine = NULL;
            continue;
        }

        //
        // Get LABEL
        p = strchr(line, ':');

        if (p)
        {
            int label;

            *p = '\0';

            if (sscanf(line, "%d", &label) != 1)
            {
                SAFE_FREE(_scriptBuf);
                SAFE_FREE(_lineEntries);
                return MMS_PARSE_ERR_MISSING_LABEL;
            }

            _lineEntries[i].label = label;
            _lineEntries[i].startOfLine = p + 1;
            SKIP_SPACE(_lineEntries[i].startOfLine);

            if (_lineEntries[i].startOfLine == '\0')
            {
                SAFE_FREE(_scriptBuf);
                SAFE_FREE(_lineEntries);
                return MMS_PARSE_ERR_MISSING_COMMAND;
            }
        }
        else
        {
            SAFE_FREE(_scriptBuf);
            SAFE_FREE(_lineEntries);
            return MMS_PARSE_ERR_MISSING_LABEL;
        }
    }
    
    _stack_pointer = -1;

    _stop = 0;
    _nextLabel = _lineEntries[0].label; /* Label of first line */

    return _nextLabel;
}


int16_t MMScript_ExecOneStep(MMSCRIPT_LOCAL_ERROR_CALLBACK local_error_callback, MMSCRIPT_NODE_ERROR_CALLBACK node_error_callback, MMSCRIPT_DELAY_MILLI_SECONDS DelayMilliSecondsImpl, MMSCRIPT_LOG log_func)
{
    uint16_t currLine;

    if (_lineCount == 0 || _lineEntries == NULL)
        return 0;

    /* Find line with label _nextLabel */

    if (_nextLabel == -1)
    {
        currLine = 0;
        _nextLabel = _lineEntries[0].label;
    }
    else
    {
        for (currLine=0; currLine<_lineCount; currLine++)
        {
            if (_lineEntries[currLine].label == _nextLabel)
                break;
        }

        if (currLine == _lineCount)
        {
            /* NOT found */
            return MMS_ERR_INVALID_LABEL;
        }
    }

    int16_t ret = MMScript_ProcessLine(&currLine, &_nextLabel, local_error_callback, node_error_callback, DelayMilliSecondsImpl, log_func);

    /* If negtive, indicates something error, return */
    if (ret < 0)
        return ret;

    /* NON negtive _nextLabel means next label or ended, while -1 for next line */
    if (_nextLabel == -1)
    {
        currLine++;

        while (currLine < _lineCount && _lineEntries[currLine].startOfLine == NULL)
        {
            currLine++;
        }

        if (currLine == _lineCount)
        {
            /* NO found */
            return MMS_ERR_END;
        }

        /* Return the label */
        _nextLabel = _lineEntries[currLine].label;

    }

    return _nextLabel;
}


void MMScript_Rewind()
{
    _stop = 0;
    _nextLabel = -1;
}


void MMScript_Stop()
{
    _stop = 1;
}


void MMScript_Clean()
{
    if (_scriptBuf)
        free(_scriptBuf);

    if (_lineEntries)
        free(_lineEntries);

    _nextLabel = 0;
    _lineCount = 0;
}




