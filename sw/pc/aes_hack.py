import time
import serial
import re
import string
import serial.tools.list_ports
import sys


def extract_encrypted_payload(data):
    match = re.search(r'[0-9a-f]{32}', data)
    if match:
        return match.group()
    else:
        return None


def find_targets_block(input_string):
    counter = 16
    block = 0
    while len(input_string) >= counter:
        counter = counter + 16
        block = block + 1
    return block


def find_serial_port():
    ports = serial.tools.list_ports.comports()
    if not ports:
        print("No serial ports found.")
        return None
    else:
        print("Available serial ports:")
        for i, port in enumerate(ports):
            print(f"{i+1}. {port.device}")
        choice = input("Enter the number of the serial port to open: ")
        try:
            port_index = int(choice) - 1
            selected_port = ports[port_index]
            return selected_port.device
        except (ValueError, IndexError):
            print("Invalid choice. Please enter a valid number.")
            return None

def open_serial_port(port_name):
    try:
        ser_port = serial.Serial(port_name, baudrate=115200, timeout=.1)
        print(f"Serial port {port_name} opened successfully.")
        return ser_port
    except serial.SerialException as e:
        print(f"Failed to open serial port {port_name}: {e}")
        return None


def main():

    #Open COM PORT
    port_name = find_serial_port()
    ser = open_serial_port(port_name)
    
    library =  string.digits + string.ascii_lowercase + string.ascii_uppercase # + string.punctuation
    
    #ser = serial.Serial('COM61', 115200, timeout=.1)  # Replace 'COM1' with your serial port
    print("Serial port opened.")
    ser.write(('\n').encode())
    time.sleep(0.5)
    ser.write(('\n').encode())

    try:
        
        # Initialize variables to store received encrypted payloads
        encrypted_payloads = []
        counter = 1
        while True:
            

            # Send 'b' to trigger the device to send encrypted payload
            ser.write(b'b')

            #print("Sent 'b'")

            # Read from serial port until "Enter backup name" is received
            while True:
                data = ser.readline().decode().strip()
                if "Enter backup name" in data:
                    #print("<<" + data)
                    break  # Exit the loop once the desired string is received

            # Send 'a' followed by enter
            data = 'a' * counter + '\n'
            length = len(data) - 1
            counter = counter + 1
            print(">>" + data[:-1])
            ser.write(data.encode())



            # Read from serial port until "Admin Menu" is received
            while True:
                data = ser.readline().decode().strip()
                if ">" in data:
                    #print(data)
                    break  # Exit the loop once the desired string is received

                #print(data)  # Print received data for debugging purposes
                encrypted_payload = extract_encrypted_payload(data)
                if encrypted_payload:
                    print('Found Encrypted Data:' + encrypted_payload)
                    encrypted_payloads.append(encrypted_payload)
                            
            padBlocksNeeded  = 0
            if(len(encrypted_payloads) > 0):                
                for item in encrypted_payloads:
                    padBlocksNeeded = padBlocksNeeded + 1 
                break;
            
            encrypted_payloads = []

        charactersToHack = (len(encrypted_payloads) * 16) - 1
        print("\r\nWe need to hack " + str(charactersToHack) + " characters.")
        dataWeKnow = 'aaaaaaaaaaaaaaaa' * padBlocksNeeded + '\n'
        dataWeKnow = dataWeKnow[1:]
        print("Starting padding needed for hack: " + dataWeKnow[:-1])
        hackedCharacters = []
        
        TargetEncryptedBlock = ''
        indexOfBlock = 0
        serialTxData = dataWeKnow[:-1] #"aaaaaaaaaaaaaaa"

        indexOfBlock = find_targets_block(dataWeKnow[:-1])
        
        while charactersToHack != 0:

            state = "Find Target Encrypted Block"  # Initial state
            while True:
                print('\r\nCurrent State: ', state)
                # State transitions and actions
                if state == "Find Target Encrypted Block":
                    #Print out recovered data
                    recovered = ""
                    for x in hackedCharacters:
                        recovered = recovered + x                    
                    print("______==_________=====_______=====_______===________Recovered: " + recovered )
                    
                    encrypted_payloads = []
                    # Send 'b' to trigger the device to send encrypted payload
                    ser.write(b'b')

                    # Read from serial port until "Enter backup name" is received
                    while True:
                        data = ser.readline().decode().strip()
                        if "Enter backup name" in data:
                            print(data)
                            break  # Exit the loop once the desired string is received

                    print(dataWeKnow[:-1])    
                    ser.write(dataWeKnow.encode())

                    #indexOfBlock = find_targets_block(dataWeKnow[:-1])
                    #indexOfBlock = 2 # test
                    print("Care about block: " + str(indexOfBlock))                    

                    # Read from serial port until "Admin Menu" is received
                    while True:
                        data = ser.readline().decode().strip()
                        if ">" in data:
                            #print(data)
                            TargetEncryptedBlock = encrypted_payloads[indexOfBlock]
                            print('\r\nTarget Encrypted Outcome: ' + TargetEncryptedBlock)
                            state = "Brute Force"
                            #print('\r\nChanged State: ', state)
                            break  # Exit the loop once the desired string is received

                        #print(data)  # Print received data for debugging purposes
                        encrypted_payload = extract_encrypted_payload(data)
                        if encrypted_payload:
                            print('Found Encrypted Data:' + encrypted_payload)
                            encrypted_payloads.append(encrypted_payload)
                            
                    
                    
                    
                
                elif state == "Brute Force":
                    
                    hackIndex = 0
                    print("Currently in Brute Force.")
                    
                    for char in library: #---------------------------------------------------FOR LOOP
                        encrypted_payloads = []
                        
                        ser.write(b'b')

                        while True: #start loop -------------------------
                            data = ser.readline().decode().strip()      #
                            if "Enter backup name" in data:             #
                                #print("<<" + data)                      #
                                break #break loop -----------------------
                           
                            
                        #serialTxData = serialTxData + char + '\n'
                        #print('\r\n\nTrying...' + serialTxData + char)
                        print('\r\nTrying...>>' + serialTxData + char)    
                        ser.write((serialTxData + char + '\n').encode())

                        hackIndex = find_targets_block(serialTxData + char)

                        while True:  #start loop ----------------------------------
                            data = ser.readline().decode().strip()                #
                            if ">" in data:
                                #print(encrypted_payloads[indexOfBlock] + " vs " + TargetEncryptedBlock)
                                if encrypted_payloads[indexOfBlock] == TargetEncryptedBlock:
                                    print('**HACKED!**')
                                    charactersToHack -= 1
                                    #print(dataWeKnow + " " + str(len(dataWeKnow)))
                                    dataWeKnow = dataWeKnow[1:]
                                    #print(dataWeKnow + " " + str(len(dataWeKnow)))
                                    hackedCharacters.append(char)
                                    if len(hackedCharacters) == charactersToHack: #look into this
                                        state = "FINISHED"
                                    else:                           
                                        state = "Set Pad Attack"
                                    break  #break loop --------------------------------
                                else:
                                    break
                            if data:
                                encrypted_payload = extract_encrypted_payload(data)
                                if encrypted_payload and encrypted_payload != serialTxData[:-1]:
                                    print('  Found Encrypted Data:' + encrypted_payload)
                                    encrypted_payloads.append(encrypted_payload)
                                
                            
                                 

                        if state == "Set Pad Attack":
                            break  # break from FOR LOOP

                    if state == "Brute Force":  # If we get here and no state change the brute force failed
                            print('\r\nAttack exhausted.')
                            recovered = ""
                            for x in hackedCharacters:
                                recovered = recovered + x
                            
                            print("Recovered: " + recovered)
                                  
                            ser.close()
                            sys.exit(0)                    
                
                elif state == "Set Pad Attack":
                    print("Currently in Set Pad Attack state.")
                    print(serialTxData + " <~~~~~~~~~")  
                    serialTxData = serialTxData[1:] + hackedCharacters[len(hackedCharacters)- 1]
                    print(serialTxData + " <~~~~~~~~~")                    
                    state = "Find Target Encrypted Block"

                elif state == "FINISHED":
                    recovered = ""
                    for x in hackedCharacters:
                        recovered = recovered + x
                    
                    print("Recovered: " + recovered)
                    
                    ser.close()
                    sys.exit(0)
                    
                    break
                
                else:
                    print("Unknown state:", state)
                    break  # Exit the state machine if an unknown state is encountered

            


            

    except KeyboardInterrupt:
        print("Exiting program.")
        ser.close()

if __name__ == "__main__":
    main()
