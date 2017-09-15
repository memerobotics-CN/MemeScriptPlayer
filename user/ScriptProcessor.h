/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SCRITP_PROCESSOR_H__
#define __SCRITP_PROCESSOR_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ----------------------------------------------------------------*/

#include "MemeServoAPI/MemeServoAPI.h"


/* Exported types ------------------------------------------------------------*/
typedef void (*MMSCRIPT_LOCAL_ERROR_CALLBACK)(uint8_t node_addr, uint8_t err);
typedef void (*MMSCRIPT_NODE_ERROR_CALLBACK)(uint8_t node_addr, uint8_t err);
typedef void (*MMSCRIPT_LOG)(uint8_t node_addr, const char *msg);
typedef void (*MMSCRIPT_DELAY_MILLI_SECONDS)(uint32_t ms);

/* Exported constants --------------------------------------------------------*/


/* Exported macro ------------------------------------------------------------*/


/* Exec errors */
#define MMS_ERR_END                   (int16_t)0
#define MMS_ERR_UNKNOWN_COMMAND       (int16_t)-1   /* Unknown command */
#define MMS_ERR_INVALID_LABEL         (int16_t)-2   /* Invalid label */
#define MMS_ERR_ERROR_START_PARAM     (int16_t)-3   /* Missing/error parameters for START */
#define MMS_ERR_MISSING_WAIT_PARAM    (int16_t)-4   /* Missing parameters for WAIT */
#define MMS_ERR_MISSING_DELAY_PARAM   (int16_t)-5   /* Missing parameter for DELAY */
#define MMS_ERR_MISSING_GOTO_PARAM    (int16_t)-6   /* Missing parameter for GOTO */
#define MMS_ERR_MISSING_NODE_ID       (int16_t)-7   /* Missing node id for command */
#define MMS_ERR_MISSING_VM_PARAM      (int16_t)-8   /* Missing parameter for VM */
#define MMS_ERR_MISSING_PVM_PARAM     (int16_t)-9   /* Missing parameters for PVM */
#define MMS_ERR_MISSING_AP_PARAM      (int16_t)-10  /* Missing parameter for AP */
#define MMS_ERR_MISSING_PAP_PARAM     (int16_t)-11  /* Missing parameters for PAP */
#define MMS_ERR_MISSING_RP_PARAM      (int16_t)-12  /* Missing parameter for RP */
#define MMS_ERR_MISSING_PRP_PARAM     (int16_t)-13  /* Missing parameters for PRP */

/* Parse errors */
#define MMS_PARSE_ERR_FILE            (int16_t)-101 /* File open error */
#define MMS_PARSE_ERR_MALLOC          (int16_t)-102 /* Malloc failed */
#define MMS_PARSE_ERR_MISSING_LABEL   (int16_t)-103 /* Missing label */
#define MMS_PARSE_ERR_MISSING_COMMAND (int16_t)-104 /* Missing command */


/* Exported functions ------------------------------------------------------- */

/**
  * @brief  Parse script & return the next script line label
  * @note   Call this before calling MMS_ExecOneStep().
  * @param  scriptBuf: Malloced script buffer address, memeory will be managed by script procesor.
  * @param  bufLen: size of scriptBuf.
  * @retval >0 : start script line label
  *         <=0: something error, see parse error codes for detailed info
  */
int16_t MMScript_ParseScript(char *scriptBuf, size_t bufLen);


/**
  * @brief  Execute oneline & return the next script line label
  * @note   This function should called to execute script step by step.
  * @param  local_error_callback: call back function for local error
  * @param  node_error_callback: call back function for servo error
  * @param  DelayMilliSecondsImpl: ms delay function pointer
  * @retval >0: next script line label
  *         =0: ended without any error
  *         <0: something error, see exec error codes for detailed info
  */
int16_t MMScript_ExecOneStep(MMSCRIPT_LOCAL_ERROR_CALLBACK local_error_callback, MMSCRIPT_NODE_ERROR_CALLBACK node_error_callback, MMSCRIPT_DELAY_MILLI_SECONDS DelayMilliSecondsImpl, MMSCRIPT_LOG log_func);


/**
  * @brief  Rewind for restart
  * @param  None
  * @retval None
  */
void MMScript_Rewind();


/**
  * @brief  Stop execution
  * @param  None
  * @retval None
  */
void MMScript_Stop();


/**
  * @brief  Clean allocated space for script parser
  * @param  None
  * @retval None
  */
void MMScript_Clean();

#ifdef __cplusplus
}
#endif
#endif /* __SCRITP_PROCESSOR_H__ */
