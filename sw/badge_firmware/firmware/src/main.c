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
#define MAX_PASSWORD_LENGTH 4 * AES_BLOCK_SIZE

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
    ATCA_STATUS status;
    char input[256];
    uint8_t plain[16];
    uint8_t cipher[16]; //Cipher text returned here (bytes)
    static uint8_t buffer[161]; //needs to be static or global...??  supports lengths up to 80 bytes
    uint8_t u8_paddingValue;
    uint8_t u8_index = 0;
    uint8_t u8_byteCount;

    memset(rx.receiveBuffer, 0x00, sizeof (rx.receiveBuffer));
    MULTI_COMM_Print("Enter backup name: ", true);

    while ((u8_index = MULTI_COMM_GetUserInput(0)) == 0);
    
    //remove line feed
    if(rx.receiveBuffer[u8_index] == LINE_FEED) {
        rx.receivedCount--;
        u8_index--; 
    }

    //Copy user input to input
    memcpy(input, rx.receiveBuffer, u8_index);
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
    
    sprintf((char *)buffer, "Cipher: ");
    
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

    //Respond
    MULTI_COMM_Print((char *)buffer, true);
    
    ir_packet.valid_packet = false; //Clear invalidate message
    return;
	
}

void UI_CLI_Checksum(void) {

    static uint8_t buffer[161];
    memset(buffer, 0x00, sizeof(buffer));

    //Step CRC 1.3
    MULTI_COMM_Print("Enter address and length: ", true);
    while (!MULTI_COMM_GetUserInput(0));
    unsigned long start_address;
    unsigned long length;
    uint16_t crc = 0;
    // Parse the input
    if (sscanf((char *)rx.receiveBuffer, "%lx %lu", &start_address, &length) == 2) {
        // Check if the command is 'readcrc'
        crc = UTIL_crc16_CCITT((uint8_t *)start_address, length);
        sprintf((char *)buffer, "CRC: %04X", crc);
    } else {
        sprintf((char *)buffer, "Invalid input format. Expected 'start_address length'");
    }
    MULTI_COMM_Print((char *)buffer, true);

    return;

}

void UI_CLI_Password(void) {
    //Step Limit 1
    char buffer[50];
    ATCA_STATUS status = ATCA_SUCCESS;
    uint32_t counterValue = 0;
    
    status = atcab_counter_read(0, &counterValue);
    CHECK_STATUS(status);

    uint32_t last_good_counter = 0;
    atcab_read_bytes_zone(ATCA_ZONE_DATA, 5, 0, (uint8_t *)&last_good_counter, 4);
    uint8_t attemptsLeft = PASSWORD_ATTEMPT_LIMIT - (counterValue - last_good_counter);

    if (attemptsLeft == 0) {
        printf("\033[37;41mPassword Attempt Limit Reached\033[0m\n");
        return;
    }
    
    // Step Password 3.3
    // Prompts the user to enter a password.
    MULTI_COMM_Print("Enter password: ", true);
    //Step Limit 2

    uint8_t u8_index = 0; // Index for accessing buffer positions.
    char input_char = 0; // Variable to hold each character read from the USART.
    memset(rx.receiveBuffer, 0x00, sizeof (rx.receiveBuffer));
    // Continuously read characters until the password buffer is full.
    while ((u8_index < MAX_PASSWORD_LENGTH)) {
        // Wait until data is ready to be received from USART.
        while (!MULTI_COMM_ReceiverIsReady());
        // Read a byte from USART.
        input_char = MULTI_COMM_ReadByte(NULL);
        u8_index++;
        // Check if the received character is a line feed
        if (input_char == LINE_FEED) {
            rx.receiveBuffer[u8_index] = '\0'; // Null-terminate the string to end input.
            break;
        } else {
            // Store the received character into the buffer and increment the index.
            rx.receiveBuffer[u8_index - 1] = input_char;
        }
    }

    //Step Password 3.4
    // Compare the received input with the predefined secret password.
    if (strcmp((char *)rx.receiveBuffer, secretPassword) == 0) {
        // If the password is correct, acknowledge access and enable admin mode.
        MULTI_COMM_Print("Password Accepted!\n", true);
        rx.HideAdminMode = false;
        //Step Limit 3
//Reset limit
status = atcab_write_bytes_zone(ATCA_ZONE_DATA, 5, 0, (uint8_t *)&counterValue, 4);
CHECK_STATUS(status);
        
    } else {
        // If the password is incorrect, deny access.
        MULTI_COMM_Print("Invalid Password: ", true);
        //Step Limit 4
    }


    return;
}

void UI_IrPasswordAttack() {

//Step Password IR Attack 1
    switch (passwordAttackState) {
    case 0:
        //waiting to start
        break;
    case 1:
        printf("\nPassword Attack on IR:\n");
        passwordAttackState = 2;
        break;
    case 2:
        //Force menu to known state
        IR_SendMessage("\n", 0, NULL);
    	SYSTICK_DelayMs(500);
        IR_SendMessage("\n", 0, NULL);
        SYSTICK_DelayMs(500);
        passwordAttackState = 3;
        break;
    case 3:
        IR_SendMessage("1", 3, "password");
        passwordAttackState = 4;
        break;
    case 4: 
        IR_SendMessage("012345678901234567890123456789012345678901234567890123456789123", 3, "Invalid");
        passwordAttackState = 5;
        break;
    case 5:
        //Check if now in Admin mode
        IR_SendMessage("s", 3, "State");
        passwordAttackState = 0;
        printf("Done\n");
        break;
    default:
        printf("error");
        while (1);
        break;
    }
    
    if(passwordAttackState != 0 && ir_packet.valid_packet == false) {
        strcpy((char *)ir_packet.buffer, "waiting...");
    }
    
    
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
                printf("b - Backup Admin Password\n");  //Adding menu item for Backup Admin Password
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
					if(!rx.HideAdminMode) {
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
                	if(!rx.HideAdminMode) {
                        //Step CRC 1.2
                        UI_CLI_Checksum();
                	}
                    break;
                case 'b':
                    if (!rx.HideAdminMode) {
                        //Step 4.2
                        UI_CLI_Backup(); //Call backup admin password item menu
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

