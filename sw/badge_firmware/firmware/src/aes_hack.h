/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Company Name

  @File Name
    filename.h

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */

#ifndef _AES_HACK_H    /* Guard against multiple inclusion */
#define _AES_HACK_H


#include "config/default/definitions.h"

void AES_HACK(void);
uint8_t AES_HACK_FindTargetsBlock(char *input_string);
bool AES_HACK_PlainTextAttack(const char *plainText); 
void AES_HACK_Start(void);
//bool Send_A(const char *str);
//bool Send_B(void);

typedef enum {
    FIND_TARGET,
    BRUTE_FORCE,
    SET_PAD_ATTACK,
    FINISHED
} aes_hack_state_t;

extern volatile bool b_aes_Hack_request;

#endif /* _AES_HACK_H */

/* *****************************************************************************
 End of File
 */
