/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Company Name

  @File Name
    filename.c

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

/* This section lists the other files that are included in this file.
 */
#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include "lcd_menu.h"
#include "cryptoauthlib.h"
#include "secure_element.h"
#include "ui.h"
#include "util.h"
#include "ir.h"

extern rx_t rx;
extern char secretPassword[];

uint8_t passwordAttackState = 0;

void UI_CLI_BackupMenu() {

    ATCA_STATUS status;
    char input[256];
    uint8_t plain[16];
    uint8_t cipher[16]; //Cipher text returned here (bytes)
    static uint8_t buffer[161]; //needs to be static or global...??  supports lengths up to 80 bytes
    uint8_t u8_paddingValue;
    uint8_t u8_index = 0;
    uint8_t u8_byteCount;

    char * name = "test";
    sprintf(input, name);
    u8_index = strlen(name);
    
    //Copy secret password to input
    memcpy(&input[u8_index], secretPassword, strlen(secretPassword));
    //Manage byte counter
    u8_index += strlen(secretPassword);
    //Determine padding value
    u8_paddingValue = 16 - (u8_index % 16);
    //Add padding value
    memset(&input[u8_index], u8_paddingValue, u8_paddingValue);
    u8_index += u8_paddingValue;

    u8_byteCount = u8_index; //Copy over total byte count
    u8_index = 0; //Reset index
    //printf("\n\nEncrypted Payload: \n");
    
    sprintf((char *)buffer, "Cipher:");
    
    //Start encrypting data and return cipher text
    while (u8_byteCount != 0) {
        memcpy(plain, &input[u8_index], 16);
        status = calib_aes_encrypt(atcab_get_device(), 4, 0, plain, cipher);
        CHECK_STATUS(status);
        //Copy temp holding buffer to buffer
        UTIL_HexToBuffer(&buffer[(u8_index << 1) + 7], cipher, 16);
        //respond(buffer, true);
        u8_byteCount -= 16;
        u8_index += 16;
    }

    LCD_MENU_BufferToDisplayText((char*)buffer, strlen((char *)buffer), MENU_MODE_RESULTS);

    return;
}

void UI_CLI_MemoryCheck() {
    uint8_t *data = (uint8_t *) 0x20005280; // Just an example address
    size_t length = 128; // Example length
    uint16_t crc = UTIL_crc16_CCITT(data, length);

    // Format the CRC result for display
    static char crcDisplay[32]; // Ensure this buffer is large enough
    sprintf(crcDisplay, "CRC: 0x%04X", crc);
    printf("CRC: 0x%04X", crc);

    LCD_MENU_BufferToDisplayText(crcDisplay, strlen(crcDisplay), MENU_MODE_RESULTS);

}

void UI_CLI_IrPower(void) {

    char input[32] = {0};

    printf("Current IR Power: %ld\n", ir_power);
    printf("New IR Power raw count (631 50%%): ");
    fgets(input, sizeof (input), stdin);

    if (sscanf(input, "%lu", (unsigned long *)&ir_power) == 1) {
        printf("New Power %lu\n", (unsigned long)ir_power);
    } else {
        printf("\nInvalid input format. Expected number\n");
    }

    printf("\n");

}

void UI_CLI_ResetLimit(void) {

    ATCA_STATUS status = ATCA_SUCCESS;
    uint32_t counterValue = 0;
    status = atcab_counter_read(0, &counterValue);
    CHECK_STATUS(status);

    status = atcab_write_bytes_zone(ATCA_ZONE_DATA, 5, 0, (uint8_t*)&counterValue, 4);
    CHECK_STATUS(status);

}

void UI_IrIncrease() {
    static char text[48] = "this is a test of displaying some data";
    ir_power += 20;

    if(ir_power > 600) ir_power = 600;
    
    TCC2_REGS->TCC_CC[0] = ir_power;
    while (TCC2_REGS->TCC_SYNCBUSY != 0U);
    sprintf(text, "Power +     %lu", (unsigned long)ir_power);
    printf("IR Power : %lu", (unsigned long)ir_power);
    printf("\n");
    settingsSubMenu[0].displayText = text;
    //buffer_to_display(text, strlen(text));    
}

void UI_IrDecrease() {
    static char text[48] = "this is a test of displaying some data";
    ir_power -= 20;
    
    if(ir_power > 600) ir_power = 0;
    
    TCC2_REGS->TCC_CC[0] = ir_power;
    while (TCC2_REGS->TCC_SYNCBUSY != 0U);
    sprintf(text, "Power +     %lu", ir_power);
    printf("IR Power : %lu", ir_power);
    printf("\n");
    settingsSubMenu[0].displayText = text;
    //buffer_to_display(text, strlen(text));    
}

void UI_IrMenuSendMessageExit(char * message) {
    IR_SendMessage(message, 0, NULL);
}

void UI_IrMenuSendMessage() {
    static char text[48] = "test 1234";

    LCD_MENU_RegisterDataEntryExit(UI_IrMenuSendMessageExit);
    LCD_MENU_BufferToDisplayText(text, strlen(text), MENU_MODE_DATA_ENTRY);
}

void UI_PasswordAttackStart() {
    passwordAttackState = 1;
}

void UI_PasswordAttackMenu() {

	static char * temp = "Overflow Attack:";

	LCD_MENU_DisplayTextBuffer(temp, 0);
    LCD_MENU_DisplayTextBuffer((char *)ir_packet.buffer, 1);
    LCD_MENU_DisplayTextBuffer(NULL, 2);

	LCD_MENU_EnterSubMenu();

	UI_PasswordAttackStart();

	return;
}

/* *****************************************************************************
 End of File
 */
