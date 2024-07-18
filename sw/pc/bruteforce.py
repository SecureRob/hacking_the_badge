#! /usr/bin/python

import time
import sys
import serial
import re

# Initialize serial port
ser = serial.Serial('/dev/ttyS4', 115200, timeout=0.1)  # Adjust this according to your serial configuration

def send_command(command, end=''):
    print(f"\033[32mSending command: {command} with end: '{end}'\033[0m")
    ser.write((command + end).encode())
    time.sleep(0.1)  # Allow time for the device to process the command

def wait_for_prompt(expected_prompt, timeout_duration=10):
    print(f"Waiting for prompt: {expected_prompt}")
    buffer = ""
    start_time = time.time()
    while time.time() - start_time < timeout_duration:
        if ser.in_waiting > 0:
            part = ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
            buffer += part
            print(f"\033[36mReceived: {buffer}\033[0m")
        if re.search(expected_prompt, buffer):
            print("Prompt detected.")
            return buffer
    print("Failed to detect the expected prompt.")
    return None

def try_codes(start_code, end_code):
    send_command(" ", end='')  # Trigger the main menu
    if not wait_for_prompt(r">"):
        print("Failed to trigger main menu.")
        return False
    
    for code in range(start_code, end_code + 1):
        send_command("1", end='')  # Navigate to the password menu
        if not wait_for_prompt(r"Enter password \(Attempts remaining: \d+\):"):
            print("Password prompt not detected after sending '1'. Retrying...")
            continue  # Skip this iteration and try again

        send_command(str(code), end='\r')  # Send the code with CR LF
        response = wait_for_prompt(r"Error 42|Accepted!")
        if response:
            if "Accepted!" in response:
                print(f"Access granted. Code accepted: {code}")
                return True
            elif "Error 42" in response:
                print(f"Invalid code: {code}. Retrying...")
                continue  # Will trigger sending "1" again in the loop
            else:
                print("Unexpected response received. Exiting...")
                sys.exit(1)
        else:
            print("No valid response detected. Exiting...")
            sys.exit(1)

if __name__ == '__main__':
    if not try_codes(1230, 1240):
        print("Code range exhausted without acceptance.")
