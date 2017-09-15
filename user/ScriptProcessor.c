
/**
  * 脚本定义：
  * SCRIPT ::= {LINE}
  * LINE ::= (LABEL ":" (ASSIGN_EXPR | IF_EXPR | ("GOTO" NUMBER) | ("DELAY" NUMBER) | "END" | ACTION)) "CRLF"
  * LABEL ::= NUMBER
  * ASSIGN_EXPR := "LET" VAR "=" (NUMBER | VAR) {("+" | "-") (NUMBER | VAR)}
  * IF_EXPR := ("IF(" (VAR | NUMBER) (">" | "<" | ">=" | "<=" | "==" | "!=") (VAR | NUMBER) ")" "THEN" LABEL)
  * VAR ::= ("A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" | "J" | "K" | "L" | "M" | "N" | "O" | "P" | "Q" | "R" | "S" | "T" | "U" | "V" | "W" | "X" | "Y" | "Z")
  * ACTION ::= ("WAIT", NODE_ID{"," NODE_ID}) | COMMANDS
  * COMMANDS ::= NODE_ID "," COMMAND{";" NODE_ID "," COMMAND}
  * COMMAND ::= ("START" NUMBER) | "STOP" | "HALT" | ("VM" NUMBER) | ("PVM" NUMBER "," NUMBER) | ("AP" NUMBER) | ("PAP" NUMBER "," NUMBER "," NUMBER) | ("RP" NUMBER) | ("PRP" NUMBER "," NUMBER "," NUMBER)
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


#define ERROR_EXIT(err)     \
    do {                    \
        retVal = err;       \
        goto _ERR_EXIT;     \
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
                    while ((ret = MMS_StartServo(node_id, MMS_MODE_KEEP,                      \
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


/* Private variables ---------------------------------------------------------*/

static uint8_t _stop = 0;

static int16_t _nextLabel = 0;
static char *_scriptBuf = NULL;
static LINE_ENTRY *_lineEntries = NULL;
static int _lineCount = 0;


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
static int16_t MMScript_ProcessLine(const char *scriptLine, int16_t *nextLabel, MMSCRIPT_LOCAL_ERROR_CALLBACK local_error_callback, MMSCRIPT_NODE_ERROR_CALLBACK node_error_callback, void (*DelayMilliSecondsImpl)(uint32_t ms), MMSCRIPT_LOG log_func);


/* Private functions ---------------------------------------------------------*/

static int16_t MMScript_ProcessLine(const char *scriptLine, int16_t *nextLabel, MMSCRIPT_LOCAL_ERROR_CALLBACK local_error_callback, MMSCRIPT_NODE_ERROR_CALLBACK node_error_callback, void (*DelayMilliSecondsImpl)(uint32_t ms), MMSCRIPT_LOG log_func)
{
    int16_t retVal = 0;

    const char *p;

    *nextLabel = -1;    /* Default to next line */

    if (scriptLine== NULL)
        return MMS_PARSE_ERR_MALLOC;

    if (strncmp(scriptLine, "LET", 3) == 0)
    {
        //
        // TODO: LET

    }
    else if (strncmp(scriptLine, "IF", 2) == 0)
    {
        //
        // TODO: IF

    }
    else if (strncmp(scriptLine, "GOTO", 4) == 0)
    {
        //
        // PARAMETERS
        int label;

        if (sscanf(scriptLine + 4, "%d", &label) != 1)
            ERROR_EXIT(MMS_ERR_MISSING_GOTO_PARAM);

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
            ERROR_EXIT(MMS_ERR_MISSING_DELAY_PARAM);

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

            if (sscanf(p, "%d", &node_id) != 1)
                ERROR_EXIT(MMS_ERR_MISSING_WAIT_PARAM);

            //
            // Wait specified servo to finish command

            status = MMS_CTRL_STATUS_NO_CONTROL;

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
                ERROR_EXIT(MMS_ERR_MISSING_NODE_ID);

            if (sscanf(p, "%d", &node_id) != 1)
                ERROR_EXIT(MMS_ERR_MISSING_NODE_ID);

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
                    ERROR_EXIT(MMS_ERR_ERROR_START_PARAM);

                if (mode != MMS_MODE_KEEP && mode != MMS_MODE_ZERO && mode != MMS_MODE_RESET)
                    ERROR_EXIT(MMS_ERR_ERROR_START_PARAM);

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
                    ERROR_EXIT(MMS_ERR_MISSING_VM_PARAM);

                FORCE_MOVE(MMS_ProfiledVelocityMove(node_id, velocity, node_error_callback), node_id, local_error_callback, node_error_callback, log_func);
            }
            else if (strncmp(p, "PVM", 3) == 0)
            {
                //
                // PARAMETERS
                int accel, velocity;

                if (sscanf(p + 3, "%d,%d", &accel, &velocity) != 2)
                    ERROR_EXIT(MMS_ERR_MISSING_PVM_PARAM);

                FORCE_EXEC(MMS_SetProfileAcceleration(node_id, accel, node_error_callback), node_id, local_error_callback, node_error_callback, log_func);
                FORCE_MOVE(MMS_ProfiledVelocityMove(node_id, velocity, node_error_callback), node_id, local_error_callback, node_error_callback, log_func);
            }
            else if (strncmp(p, "AP", 2) == 0)
            {
                //
                // PARAMETERS
                long position;

                if (sscanf(p + 2, "%ld", &position) != 1)
                    ERROR_EXIT(MMS_ERR_MISSING_AP_PARAM);

                FORCE_MOVE(MMS_AbsolutePositionMove(node_id, position, node_error_callback), node_id, local_error_callback, node_error_callback, log_func);
            }
            else if (strncmp(p, "PAP", 3) == 0)
            {
                //
                // PARAMETERS
                int accel, velocity;
                long position;

                if (sscanf(p + 3, "%d,%d,%ld", &accel, &velocity, &position) != 3)
                    ERROR_EXIT(MMS_ERR_MISSING_PAP_PARAM);

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
                    ERROR_EXIT(MMS_ERR_MISSING_RP_PARAM);

                FORCE_MOVE(MMS_RelativePositionMove(node_id, position, node_error_callback), node_id, local_error_callback, node_error_callback, log_func);
            }
            else if (strncmp(p, "PRP", 3) == 0)
            {
                //
                // PARAMETERS
                int accel, velocity;
                long position;

                if (sscanf(p + 3, "%d,%d,%ld", &accel, &velocity, &position) != 3)
                    ERROR_EXIT(MMS_ERR_MISSING_PRP_PARAM);

                FORCE_EXEC(MMS_SetProfileAcceleration(node_id, accel, node_error_callback), node_id, local_error_callback, node_error_callback, log_func);
                FORCE_EXEC(MMS_SetProfileVelocity(node_id, velocity, node_error_callback), node_id, local_error_callback, node_error_callback, log_func);
                FORCE_MOVE(MMS_ProfiledRelativePositionMove(node_id, position, node_error_callback), node_id, local_error_callback, node_error_callback, log_func);
            }
            else
            {
                // Unknown command
                ERROR_EXIT(MMS_ERR_UNKNOWN_COMMAND);
            }

            // next COMMAND
            p = strchr(p, ';');

            if (p)
                p++;    /* Skip ';' */
        }
    }

_ERR_EXIT:
    return retVal;
}


/* Public functions ---------------------------------------------------------*/

int16_t MMScript_ParseScript(const char *scriptFile)
{
    FILE *fh = fopen(scriptFile, "rb");

    if (fh == NULL)
        return MMS_PARSE_ERR_FILE;

    fseek(fh, 0, SEEK_END);
    size_t bytes = ftell(fh);

    if (bytes == 0)
    {
        fclose(fh);
        return MMS_PARSE_ERR_FILE;
    }

    if (_scriptBuf != NULL)
    {
        SAFE_FREE(_scriptBuf);
    }

    _scriptBuf = (char*)malloc(bytes + 1);

    if (_scriptBuf == NULL)
    {
        fclose(fh);
        return MMS_PARSE_ERR_MALLOC;
    }

    fseek(fh, 0, SEEK_SET);

    if (fread(_scriptBuf, 1, bytes, fh) != bytes)
    {
        fclose(fh);
        SAFE_FREE(_scriptBuf);
        return MMS_PARSE_ERR_FILE;
    }

    fclose(fh);

    _scriptBuf[bytes] = '\0';


    /**
      * Count lines
      */

    _lineCount = 1;

    for (size_t i=0; i<bytes; i++)
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

    for (size_t i=0; i<bytes; i++)
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

    fclose(fh);

    _stop = 0;
    _nextLabel = _lineEntries[0].label; /* Label of first line */

    return _nextLabel;
}


int16_t MMScript_ExecOneStep(MMSCRIPT_LOCAL_ERROR_CALLBACK local_error_callback, MMSCRIPT_NODE_ERROR_CALLBACK node_error_callback, MMSCRIPT_DELAY_MILLI_SECONDS DelayMilliSecondsImpl, MMSCRIPT_LOG log_func)
{
    int i;

    if (_lineCount == 0)
        return 0;

    /* Find line with label _nextLabel */

    if (_nextLabel == -1)
    {
        i = 0;
        _nextLabel = _lineEntries[0].label;
    }
    else
    {
        for (i=0; i<_lineCount; i++)
        {
            if (_lineEntries[i].label == _nextLabel)
                break;
        }

        if (i == _lineCount)
        {
            /* NOT found */
            return MMS_ERR_INVALID_LABEL;
        }
    }

    int16_t ret = MMScript_ProcessLine(_lineEntries[i].startOfLine, &_nextLabel, local_error_callback, node_error_callback, DelayMilliSecondsImpl, log_func);

    /* If negtive, indicates something error, return */
    if (ret < 0)
        return ret;

    /* NON negtive _nextLabel means next label or ended, while -1 for next line */
    if (_nextLabel == -1)
    {
        i++;

        if (i == _lineCount)
        {
            /* NO found */
            return MMS_ERR_END;
        }

        /* Return the label */
        _nextLabel = _lineEntries[i].label;

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




