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

#ifndef _UI_H    /* Guard against multiple inclusion */
#define _UI_H

void UI_CLI_BackupMenu();
void UI_CLI_MemoryCheck();
void UI_CLI_IrPower(void);
void UI_CLI_ResetLimit(void);
void UI_IrDecrease();
void UI_IrIncrease();
void UI_IrMenuSendMessageExit(char * message);
void UI_IrMenuSendMessage();
void UI_PasswordAttackStart();
void UI_PasswordAttackMenu();

#endif /* _UI_H */

/* *****************************************************************************
 End of File
 */
