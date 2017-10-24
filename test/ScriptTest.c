
#include <assert.h>
#include <string.h>
#include <stdio.h>


#include "ScriptProcessor.h"

#define SCRIPT_ASSERT(test_id, result)\
{\
  if (!(result))\
    printf("test %d FAILED: (%s)\n", test_id, #result);\
  else\
    printf("test %d OK: (%s)\n", test_id, #result);\
}


int main(int argc, char **argv)
{
  char script1[] = "1: GOTO 10\r\n";
  char script2[] = "1: A=1+3*6-2\r\n2: IF A==22 THEN 2\r\n";

  int16_t ret;

  
  /* Script 1 */
  printf("Tests start.\n\n");

  printf("script: %s\n", script1);

  printf("test %d: %s\n", 1, "MMScript_ParseScript");
  ret = MMScript_ParseScript(script1, strlen(script1) + 1);
  SCRIPT_ASSERT(1, ret == 1);

  printf("test %d: %s\n", 2, "GOTO");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  SCRIPT_ASSERT(1, ret == 10);

  /* Script 2 */
  
  
  printf("\nAll tests done.");
  return 0;
}
