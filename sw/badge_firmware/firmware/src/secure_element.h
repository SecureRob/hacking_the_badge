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

#ifndef _EXAMPLE_FILE_NAME_H    /* Guard against multiple inclusion */
#define _EXAMPLE_FILE_NAME_H

extern const uint8_t mcu_key[];
extern ATCAIfaceCfg atecc608_0_init_data; //i2c

uint8_t SECURE_ELEMENT_WriteConfig(uint8_t addr);
void SECURE_ELEMENT_WriteData();

#endif /* _EXAMPLE_FILE_NAME_H */

/* *****************************************************************************
 End of File
 */
