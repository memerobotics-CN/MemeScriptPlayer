
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
  (void)argc;
  (void)argv;

  char script1[] = "1: GOTO 10\r\n";
  char script2[] = "1: LET A=1 +3* 6-2\r\n2: IF (A==22) THEN 1\r\n3: GOTO 1\r\n";
  char script3[] = "1: LET A 1+3*6-2\r\n";
  char script4[] = "1: LET A==1+3*6-2\r\n";
  char script5[] = "1: LET AA=1+3*6-2\r\n";
  char script6[] = "1: LET =1+3*6-2\r\n";
  char script7[] = "1: LET A = 1 + 3* 6 - 2\r\n";
  char script8[] = "1: CALL 3\r\n2: LET A=1+3*6-2\r\n3: RET\r\n";
  char script9[] = "1: LET A = 1\r\n2: CALL 4\r\n3: END\r\n4: LET A = 2\r\n5: RET\r\n";
  char script10[] = "1: LET A = 1\r\n2: CALL 4\r\n3: END\r\n4: CALL 6\r\n5: RET\r\n6: LET A = 2\r\n7: RET";
  char script11[] =
  " 1: LET A = 1\r\n"
  " 2: CALL 4\r\n"
  " 3: END\r\n"
  " 4: CALL 6\r\n"
  " 5: RET\r\n"
  " 6: CALL 8\r\n"
  " 7: RET\r\n"
  " 8: CALL 10\r\n"
  " 9: RET\r\n"
  "10: CALL 12\r\n"
  "11: RET\r\n"
  "12: CALL 14\r\n"
  "13: RET\r\n"
  "14: CALL 16\r\n"
  "15: RET\r\n"
  "16: CALL 18\r\n"
  "17: RET\r\n"
  "18: CALL 20\r\n"
  "19: RET\r\n"
  "18: CALL 20\r\n"
  "19: RET\r\n"
  "20: CALL 22\r\n"
  "21: RET\r\n"
  "22: LET A = 2\r\n"
  "24: RET";

  char script12[] =
  " 1: LET A = 1\r\n"
  " 2: CALL 4\r\n"
  " 3: END\r\n"
  " 4: CALL 6\r\n"
  " 5: RET\r\n"
  " 6: CALL 8\r\n"
  " 7: RET\r\n"
  " 8: CALL 10\r\n"
  " 9: RET\r\n"
  "10: CALL 12\r\n"
  "11: RET\r\n"
  "12: CALL 14\r\n"
  "13: RET\r\n"
  "14: CALL 16\r\n"
  "15: RET\r\n"
  "16: CALL 18\r\n"
  "17: RET\r\n"
  "18: CALL 20\r\n"
  "19: RET\r\n"
  "18: CALL 20\r\n"
  "19: RET\r\n"
  "20: CALL 22\r\n"
  "21: RET\r\n"
  "22: CALL 24\r\n"
  "23: RET\r\n"
  "24: LET A = 2\r\n"
  "25: RET";

  char script13[] =
  " 1: RET\r\n";

  
  int16_t ret;

  printf("Tests start.\n\n");

  /* Script 1 */
  printf("\n---------------------------------------\n");
  printf("script 1 : \n%s\n", script1);

  printf("test %d: %s\n", 1, "MMScript_ParseScript");
  ret = MMScript_ParseScript(script1, strlen(script1) + 1);
  SCRIPT_ASSERT(1, ret == 1);

  printf("test %d: %s\n", 2, "GOTO");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  SCRIPT_ASSERT(2, ret == 10);


  /* Script 2 */
  printf("\n---------------------------------------\n");
  printf("script 2 : \n%s\n", script2);

  printf("test %d: %s\n", 1, "MMScript_ParseScript");
  ret = MMScript_ParseScript(script2, strlen(script2) + 1);
  SCRIPT_ASSERT(1, ret == 1);

  printf("test %d: %s\n", 2, "LET");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(2, ret == 2);

  printf("test %d: %s\n", 3, "IF");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(3, ret == 1);


  /* Script 3 */
  printf("\n---------------------------------------\n");
  printf("script 3 : \n%s\n", script3);

  printf("test %d: %s\n", 1, "MMScript_ParseScript");
  ret = MMScript_ParseScript(script3, strlen(script3) + 1);
  SCRIPT_ASSERT(1, ret == 1);

  printf("test %d: %s\n", 2, "LET");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(2, ret == MMS_ERR_MISSING_LET_EQUAL);


  /* Script 4 */
  printf("\n---------------------------------------\n");
  printf("script 4 : \n%s\n", script4);

  printf("test %d: %s\n", 1, "MMScript_ParseScript");
  ret = MMScript_ParseScript(script4, strlen(script4) + 1);
  SCRIPT_ASSERT(1, ret == 1);

  printf("test %d: %s\n", 2, "LET");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(2, ret == MMS_ERR_INVALID_LET_EQUAL);


  /* Script 5 */
  printf("\n---------------------------------------\n");
  printf("script 5 : \n%s\n", script5);

  printf("test %d: %s\n", 1, "MMScript_ParseScript");
  ret = MMScript_ParseScript(script5, strlen(script5) + 1);
  SCRIPT_ASSERT(1, ret == 1);

  printf("test %d: %s\n", 2, "LET");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(2, ret == MMS_ERR_INVALID_LET_VARNAME);


  /* Script 6 */
  printf("\n---------------------------------------\n");
  printf("script 6 : \n%s\n", script6);

  printf("test %d: %s\n", 1, "MMScript_ParseScript");
  ret = MMScript_ParseScript(script6, strlen(script6) + 1);
  SCRIPT_ASSERT(1, ret == 1);

  printf("test %d: %s\n", 2, "LET");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(2, ret == MMS_ERR_MISSING_LET_VARNAME);


  /* Script 7 */
  printf("\n---------------------------------------\n");
  printf("script 7 : \n%s\n", script7);

  printf("test %d: %s\n", 1, "MMScript_ParseScript");
  ret = MMScript_ParseScript(script7, strlen(script7) + 1);
  SCRIPT_ASSERT(1, ret == 1);

  printf("test %d: %s\n", 2, "LET");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(2, ret == 0);

  /* Script 8 */
  printf("\n---------------------------------------\n");
  printf("script 8 : \n%s\n", script8);

  printf("test %d: %s\n", 1, "MMScript_ParseScript");
  ret = MMScript_ParseScript(script8, strlen(script8) + 1);
  SCRIPT_ASSERT(1, ret == 1);

  printf("test %d: %s\n", 2, "CALL");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(2, ret == 3);

  printf("test %d: %s\n", 3, "RET");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(2, ret == 2);

  printf("test %d: %s\n", 4, "LET");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(2, ret == 3);

  /* Script 9 */
  printf("\n---------------------------------------\n");
  printf("script 9 : \n%s\n", script9);

  printf("test %d: %s\n", 1, "MMScript_ParseScript");
  ret = MMScript_ParseScript(script9, strlen(script9) + 1);
  SCRIPT_ASSERT(1, ret == 1);

  printf("test %d: %s\n", 2, "LET");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(2, ret == 2);

  printf("test %d: %s\n", 3, "CALL");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(2, ret == 4);

  printf("test %d: %s\n", 4, "LET");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(2, ret == 5);

  printf("test %d: %s\n", 5, "RET");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(2, ret == 3);

  printf("test %d: %s\n", 6, "END");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(2, ret == 0);

  /* Script 10 */
  printf("\n---------------------------------------\n");
  printf("script 10 : \n%s\n", script10);

  printf("test %d: %s\n", 1, "MMScript_ParseScript");
  ret = MMScript_ParseScript(script10, strlen(script10) + 1);
  SCRIPT_ASSERT(1, ret == 1);

  printf("test %d: %s\n", 2, "LET");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(2, ret == 2);

  printf("test %d: %s\n", 3, "CALL");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(2, ret == 4);

  printf("test %d: %s\n", 4, "CALL");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(2, ret == 6);

  printf("test %d: %s\n", 5, "LET");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(2, ret == 7);

  printf("test %d: %s\n", 6, "RET");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(2, ret == 5);

  printf("test %d: %s\n", 7, "RET");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(2, ret == 3);

  printf("test %d: %s\n", 8, "END");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(2, ret == 0);

  /* Script 11 */
  printf("\n---------------------------------------\n");
  printf("script 11 : \n%s\n", script11);

  printf("test %d: %s\n", 1, "MMScript_ParseScript");
  ret = MMScript_ParseScript(script11, strlen(script11) + 1);
  SCRIPT_ASSERT(1, ret == 1);

  printf("test %d: %s\n", 2, "LET");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(2, ret == 2);

  printf("test %d: %s\n", 3, "CALL");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(3, ret == 4);

  printf("test %d: %s\n", 4, "CALL");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(4, ret == 6);

  printf("test %d: %s\n", 5, "CALL");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(5, ret == 8);

  printf("test %d: %s\n", 6, "CALL");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(6, ret == 10);

  printf("test %d: %s\n", 7, "CALL");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(7, ret == 12);

  printf("test %d: %s\n", 8, "CALL");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(8, ret == 14);

  printf("test %d: %s\n", 9, "CALL");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(9, ret == 16);

  printf("test %d: %s\n", 10, "CALL");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(10, ret == 18);

  printf("test %d: %s\n", 11, "CALL");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(11, ret == 20);

  printf("test %d: %s\n", 12, "CALL");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(12, ret == 22);

  printf("test %d: %s\n", 13, "LET");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(13, ret == 24);

  printf("test %d: %s\n", 14, "RET");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(14, ret == 21);

  printf("test %d: %s\n", 15, "RET");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(15, ret == 19);

  printf("test %d: %s\n", 16, "RET");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(16, ret == 17);

  printf("test %d: %s\n", 17, "RET");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(17, ret == 15);

  printf("test %d: %s\n", 18, "RET");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(18, ret == 13);

  printf("test %d: %s\n", 19, "RET");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(19, ret == 11);

  printf("test %d: %s\n", 20, "RET");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(20, ret == 9);

  printf("test %d: %s\n", 21, "RET");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(21, ret == 7);

  printf("test %d: %s\n", 22, "RET");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(22, ret == 5);

  printf("test %d: %s\n", 23, "RET");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(23, ret == 3);

  printf("test %d: %s\n", 24, "END");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(24, ret == 0);

  /* Script 12 */
  printf("\n---------------------------------------\n");
  printf("script 12 : \n%s\n", script12);

  printf("test %d: %s\n", 1, "MMScript_ParseScript");
  ret = MMScript_ParseScript(script12, strlen(script12) + 1);
  SCRIPT_ASSERT(1, ret == 1);

  printf("test %d: %s\n", 2, "LET");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(2, ret == 2);

  printf("test %d: %s\n", 3, "CALL");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(3, ret == 4);

  printf("test %d: %s\n", 4, "CALL");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(4, ret == 6);

  printf("test %d: %s\n", 5, "CALL");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(5, ret == 8);

  printf("test %d: %s\n", 6, "CALL");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(6, ret == 10);

  printf("test %d: %s\n", 7, "CALL");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(7, ret == 12);

  printf("test %d: %s\n", 8, "CALL");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(8, ret == 14);

  printf("test %d: %s\n", 9, "CALL");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(9, ret == 16);

  printf("test %d: %s\n", 10, "CALL");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(10, ret == 18);

  printf("test %d: %s\n", 11, "CALL");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(11, ret == 20);

  printf("test %d: %s\n", 12, "CALL");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(12, ret == 22);

  printf("test %d: %s\n", 13, "CALL");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(13, ret == MMS_ERR_FULL_STACK);


    /* Script 13 */
  printf("\n---------------------------------------\n");
  printf("script 13 : \n%s\n", script13);

  printf("test %d: %s\n", 1, "MMScript_ParseScript");
  ret = MMScript_ParseScript(script13, strlen(script13) + 1);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(1, ret == 1);

  printf("test %d: %s\n", 2, "RET");
  ret = MMScript_ExecOneStep(NULL, NULL, NULL, NULL);
  printf("return value = %d\n", ret);
  SCRIPT_ASSERT(2, ret == MMS_ERR_EMPTY_STACK);
  
  
  printf("\nAll tests done.");
  return 0;
}
