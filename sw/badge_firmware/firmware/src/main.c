/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include "lcd_menu.h"
#include "cryptoauthlib.h"
#include "secure_element.h"
#include "ui.h"
#include "util.h"
#include "crc_attack.h"
#include "lcd_menu.h"
#include "multi_comm.h"

#include "ir.h"
#include "aes_hack.h"

void MAIN_Init(void);

#define PASSWORD_ATTEMPT_LIMIT 100

//Step Password 3.2.1

rx_t rx;

#define MAIN_MENU_ACTIVE_LED() PWM_Set();
#define MAIN_MENU_EXIT_LED() PWM_Clear();

char message[32] = "no messages";
//Step AES 4.4
char secretPassword[] = "0123a";

extern uint8_t passwordAttackState;

void UI_CLI_CheckMessages(void) {

    uint16_t len = strlen(message);

    MAIN_MENU_EXIT_LED(); //exiting main menu

    for (uint16_t i = 0; i != len; i++) {
        printf("%c", message[i]);
    }

    MAIN_MENU_ACTIVE_LED();
}

void UI_CLI_Backup(void) {
    //Step AES 4.3
}

void UI_CLI_Checksum(void) {

    static uint8_t buffer[161];
    memset(buffer, 0x00, sizeof (buffer));

    //Step CRC 1.3

    return;

}

void UI_CLI_Password(void) {
    //Step Limit 1


    // Step Password 3.3

    //Step Password 3.4

    return;
}

void UI_IrPasswordAttack() {

    //Step Password IR Attack 1


    return;

}

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main(void) {

    /* Initialize all modules */
    SYS_Initialize(NULL);
    MAIN_Init();

    uint8_t writeMenu = 1;
    ir_packet.state = WAITING_FOR_SYNC;

    while (true) {
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        SYS_Tasks();

        if (writeMenu) {

            if (rx.HideAdminMode == false) {
                printf("\n\033[37;41mAdmin Menu\033[0m\n");
            } else {
                printf("\nMenu\n");
            }

            //Step Password 3.1

            printf("2 - Reset Limit\n");
            printf("5 - IR Power\n");
            printf("m - Check Messages\n");
            printf("s - Check State\n");

            if (!rx.HideAdminMode) {
                //Step CRC 1
                //Step AES 4.1
                printf("x - Exit Admin\n");
            }

            printf("> ");

            writeMenu = 0;

        }

        MAIN_MENU_ACTIVE_LED();

        if (MULTI_COMM_GetUserInput(1)) {

            rx.receivedCount = 0; //Reset receive counter

            printf("\n");

            switch (rx.receiveBuffer[0]) {
                case 's':
                    if (!rx.HideAdminMode) {
                        MULTI_COMM_Print("State = Admin\n", true);
                    } else {
                        MULTI_COMM_Print("State = User\n", true);
                    }
                    break;
                case 'm':
                    UI_CLI_CheckMessages();
                    break;
                case '1':
                    //Step Password 3.2
                    break;
                case '2':
                    UI_CLI_ResetLimit();
                    break;
                case '5':
                    UI_CLI_IrPower();
                    break;
                case 'c':
                    if (!rx.HideAdminMode) {
                        //Step CRC 1.2
                    }
                    break;
                case 'b':
                    if (!rx.HideAdminMode) {
                        //Step 4.2
                    }
                    break;
                case 'x':
                    if (!rx.HideAdminMode) {
                        rx.HideAdminMode = true;
                        printf("Exiting...\n");
                    }
                    break;
                default:
                    break;
            }
            writeMenu = 1;
        }
    }
    return ( EXIT_FAILURE);
}

void MAIN_Init(void) {

    IR_EnableReceive();

    EIC_CallbackRegister(0, LCD_MENU_UpButtonPressCallback, (uintptr_t) NULL);
    EIC_CallbackRegister(1, LCD_MENU_DownButtonPressCallback, (uintptr_t) NULL);
    EIC_CallbackRegister(2, LCD_MENU_LeftButtonPressCallback, (uintptr_t) NULL);
    EIC_CallbackRegister(3, LCD_MENU_RightButtonPressCallback, (uintptr_t) NULL);
    EIC_CallbackRegister(4, LCD_MENU_EnterButtonPressCallback, (uintptr_t) NULL);

    SERCOM2_USART_ReadCallbackRegister(IR_Receive, (uintptr_t) NULL);

    NVIC_SetPriority(9, 1); //increase sercom 0 (ecc608 to run in display interrupt)

    //Init menu system
    LCD_MENU_Init();

    SERCOM5_REGS->USART_INT.SERCOM_INTENSET = SERCOM_USART_INT_INTENSET_RXC(1);
    PORT_REGS->GROUP[0].PORT_PMUX[6] = 0x0U;

    SYSTICK_TimerStart();

    ATCA_STATUS status = ATCA_SUCCESS;

    status = atcab_init(&atecc608_0_init_data);
    CHECK_STATUS(status);

    uint8_t revision[4];
    status = atcab_info(revision);
    CHECK_STATUS(status);
    printf("Device Revision.\n");
    UTIL_PrintHexString(revision, 4);
    printf("\n");

    uint8_t config[128];
    status = atcab_read_config_zone(config);
    CHECK_STATUS(status);
    //printf("608 Config Zone: \n"); print_array(config, 128);

    bool islocked = false;
    status = atcab_is_locked(LOCK_ZONE_CONFIG, &islocked);

    if (!islocked) { //Provision Part
        SECURE_ELEMENT_WriteConfig(0xc0);
    }

    status = atcab_is_locked(LOCK_ZONE_DATA, &islocked);
    if (!islocked) { //Provision Part

        SECURE_ELEMENT_WriteData();
    }

    rx.HideAdminMode = true;
}

/*******************************************************************************
 End of File
 */
