#! /usr/bin/python

# To change this license header, choose License Headers in Project Properties.
# To change this template file, choose Tools | Templates
# and open the template in the editor.

__author__="C40249"
__date__ ="$Apr 16, 2024 3:51:49 PM$"

import time
import sys  # Import sys module to access command line arguments
import serial
import serial.tools.list_ports

# Initialize serial port
#ser = serial.Serial('/dev/ttyS4', 115200, timeout=0.1)  # Adjust this according to your serial configuration

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

def send_command(command, end='\r\n'):
    print(f"\033[32m{command}\033[0m")  # Green text
    ser.write((command + end).encode())
    time.sleep(0.1)  # Allow time for the device to process the command


def wait_for_prompt(expected_prompt="Enter address and length (e.g. 0x1000 128): "):
    buffer = ""
    while True:
        part = ser.read(1).decode('utf-8', errors='ignore')
        buffer += part
        if expected_prompt in buffer:
            break

def wait_for_prompt(expected_prompt="Enter address", timeout=1.0):
    print(f"Waiting for prompt...")
    start_time = time.time()
    buffer = ""
    while True:
        if ser.in_waiting > 0:
            part = ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
            print(f"\033[36m{part}\033[0m")  # Display data as received
            buffer += part

        if expected_prompt in buffer:
            print("Prompt detected.")
            break

        elapsed_time = time.time() - start_time
        if elapsed_time > timeout:
            print(f"Timeout exceeded waiting for prompt '{expected_prompt}'")
            print("******Make sure Badge is in Admin mode**********")
            sys.exit(1)
            break

def send_full_command(address):
    send_command("c", end='')  # Send 'c' to access memory check menu
    wait_for_prompt()
    crc_command = f"0x{address:04X} 2"
    send_command(crc_command)

def read_response():
    print("Reading response...")
    response = ''
    start_time = time.time()
    while True:
        if ser.in_waiting > 0:
            part = ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
            print(f"\033[36m{part}\033[0m")
            response += part
        if "\n>" in response:
            break
        if time.time() - start_time > 1.0:
            print("\033[32mResponse read timeout.\033[0m")
            break
    return response


def print_hex_dump(address, data):
    bytes_per_line = 16  # Number of bytes per line in the hex dump
    for i in range(0, len(data), bytes_per_line):
        line_data = data[i:i + bytes_per_line]
        # Swap each pair of bytes for hex display
        swapped_data = bytearray()
        for j in range(0, len(line_data), 2):
            if j + 1 < len(line_data):
                swapped_data.extend([line_data[j+1], line_data[j]])  # Swap the bytes
            else:
                swapped_data.append(line_data[j])  # In case of an odd number of bytes at the end
        
        hex_parts = [''.join(f"{byte:02X}" for byte in swapped_data[j:j+2]) for j in range(0, len(swapped_data), 2)]
        hex_string = ' '.join(hex_parts)

        # Generate ASCII string using the original byte order
        ascii_string = ''.join(chr(byte) if 32 <= byte <= 126 else '.' for byte in line_data)  # Use line_data, not swapped_data

        print(f"{address + i:08X}: {hex_string:<39} {ascii_string}")


def probe_addresses(start, end, crc_table):
    all_data = bytearray()  # Use bytearray to accumulate binary data
    current_address = start

    while current_address < end:
        send_full_command(current_address)
        response = read_response()
        crc_value = extract_crc_from_response(response)  # Extract CRC from response
        
        if crc_value is not None and crc_value in crc_table:
            original_bytes = crc_table[crc_value]  # Get the original bytes using CRC value
            
            # Convert bytes to a hex string for better readability
            original_bytes_hex = ' '.join(f"{(original_bytes[i+1] << 8) + original_bytes[i]:04X}" for i in range(0, len(original_bytes), 2))
            print(f"Decode CRC {crc_value:04X} -> {original_bytes_hex}")
            
            all_data.extend(original_bytes)  # Add the original bytes to the accumulated data
        else:
            # Handle missing data by adding placeholder bytes
            all_data.extend(b'\xFF\xFF')  # Assume each missing block is 2 bytes

        current_address += 2  # Increment by the smallest unit

    if all_data:
        print_hex_dump(start, all_data)
    else:
        print(f"No data found from address {start:04X} to {end:04X}")

def extract_crc_from_response(response):
    # This function should extract CRC from the response text.
    lines = response.split('\n')
    for line in lines:
        if 'CRC:' in line:
            crc_hex = line.split('CRC:')[1].strip()
            return int(crc_hex, 16)
    return None

def generate_crc_table():
    crc_table = {}
    for i in range(65536):
        data = i.to_bytes(2, byteorder='big')
        crc_value = crc16_ccitt(data)
        crc_table[crc_value] = data
    return crc_table

def crc16_ccitt(data):
    crc = 0xFFFF
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            if crc & 0x8000:
                crc = (crc << 1) ^ 0x1021
            else:
                crc <<= 1
        crc &= 0xFFFF
    return crc

if __name__ == '__main__':

    #Open COM PORT
    port_name = find_serial_port()
    ser = open_serial_port(port_name)
    
    if len(sys.argv) != 3:
        print("Usage: python crc_test.py <start address> <length>")
        sys.exit(1)

    start_address = int(sys.argv[1], 16)  # Convert start address from hex string to integer
    length = int(sys.argv[2], 10)  # Convert length from decimal string to integer

    if length % 2 != 0:
        print("Length must be even, as each CRC covers 2 bytes.")
        sys.exit(1)

    end_address = start_address + length
    crc_table = generate_crc_table()
    
    
    
    
    probe_addresses(start_address, end_address, crc_table)
