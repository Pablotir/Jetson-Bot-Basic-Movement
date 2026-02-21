#!/usr/bin/env python3
import serial
import time

SERIAL_PORT = '/dev/ttyUSB0'
BAUD_RATE = 115200

def send_command(ser, command):
    print(f"Sending: {command}")
    ser.write(f"{command}\n".encode('utf-8'))
    ser.flush()
    time.sleep(0.5)
    
    while ser.in_waiting:
        line = ser.readline().decode('utf-8', errors='ignore').strip()
        if line:
            print(f"  → {line}")

def main():
    print("=== Arduino Motor Diagnostic Script - MECANUM DRIVE ===")
    print(f"Connecting to {SERIAL_PORT}...")
    
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        time.sleep(2)
        
        print("Connected! Reading initial messages...")
        time.sleep(1)
        while ser.in_waiting:
            print(ser.readline().decode('utf-8', errors='ignore').strip())
        
        while True:
            print("\n" + "="*60)
            print("=== MECANUM MOTOR DIAGNOSTIC MENU ===")
            print("="*60)
            print("\n--- INDIVIDUAL MOTOR TESTS (Find Correct Directions) ---")
            print("1.  Test Motor 1 (BL) - Positive command")
            print("2.  Test Motor 1 (BL) - Negative command")
            print("3.  Test Motor 2 (BR) - Positive command")
            print("4.  Test Motor 2 (BR) - Negative command")
            print("5.  Test Motor 3 (FL) - Positive command")
            print("6.  Test Motor 3 (FL) - Negative command")
            print("7.  Test Motor 4 (FR) - Positive command")
            print("8.  Test Motor 4 (FR) - Negative command")
            
            print("\n--- VERIFIED MOVEMENT PATTERNS (Mecanum) ---")
            print("10. STRAFE RIGHT:  B 1 -80 2 80 3 -80 4 80")
            print("11. STRAFE LEFT:   B 1 80 2 -80 3 80 4 -80")
            print("12. BACKWARD:      B 1 -80 2 -80 3 80 4 80")
            print("13. FORWARD:       B 1 80 2 80 3 -80 4 -80")
            
            print("\n--- ROTATION TESTS (CALIBRATED: 720 wheel° = 90 robot°) ---")
            print("14. TURN LEFT 45°:   B 1 -360 2 360 3 360 4 -360")
            print("15. TURN RIGHT 45°:  B 1 360 2 -360 3 -360 4 360")
            print("16. TURN LEFT 90°:   B 1 -720 2 720 3 720 4 -720")
            print("17. TURN RIGHT 90°:  B 1 720 2 -720 3 -720 4 720")
            print("18. TURN LEFT 180°:  B 1 -1440 2 1440 3 1440 4 -1440")
            print("19. TURN RIGHT 180°: B 1 1440 2 -1440 3 -1440 4 1440")
            
            print("\n--- UTILITY ---")
            print("20. Read Encoder Values (5 seconds)")
            print("21. RESET Encoders (zero all positions)")
            print("22. STOP (Emergency)")
            print("23. Custom Command")
            print("0.  Exit")
            
            choice = input("\nEnter choice: ").strip()
            
            if choice == '1':
                print("\n→ Motor 1 (BL) with POSITIVE command (+80)")
                send_command(ser, "B 1 80")
                time.sleep(2)
                
            elif choice == '2':
                print("\n→ Motor 1 (BL) with NEGATIVE command (-80)")
                send_command(ser, "B 1 -80")
                time.sleep(2)
                
            elif choice == '3':
                print("\n→ Motor 2 (BR) with POSITIVE command (+80)")
                send_command(ser, "B 2 80")
                time.sleep(2)
                
            elif choice == '4':
                print("\n→ Motor 2 (BR) with NEGATIVE command (-80)")
                send_command(ser, "B 2 -80")
                time.sleep(2)
                
            elif choice == '5':
                print("\n→ Motor 3 (FL) with POSITIVE command (+80)")
                send_command(ser, "B 3 80")
                time.sleep(2)
                
            elif choice == '6':
                print("\n→ Motor 3 (FL) with NEGATIVE command (-80)")
                send_command(ser, "B 3 -80")
                time.sleep(2)
                
            elif choice == '7':
                print("\n→ Motor 4 (FR) with POSITIVE command (+80)")
                send_command(ser, "B 4 80")
                time.sleep(2)
                
            elif choice == '8':
                print("\n→ Motor 4 (FR) with NEGATIVE command (-80)")
                send_command(ser, "B 4 -80")
                time.sleep(2)
            
            elif choice == '10':
                print("\n→ STRAFE RIGHT: Moving robot sideways to the right")
                send_command(ser, "B 1 -80 2 80 3 -80 4 80")
                time.sleep(3)
                
            elif choice == '11':
                print("\n→ STRAFE LEFT: Moving robot sideways to the left")
                send_command(ser, "B 1 80 2 -80 3 80 4 -80")
                time.sleep(3)
                
            elif choice == '12':
                print("\n→ BACKWARD: Moving robot straight backward")
                send_command(ser, "B 1 -80 2 -80 3 80 4 80")
                time.sleep(3)
                
            elif choice == '13':
                print("\n→ FORWARD: Moving robot straight forward")
                send_command(ser, "B 1 80 2 80 3 -80 4 -80")
                time.sleep(3)
                
            elif choice == '14':
                print("\n→ TURN LEFT 45°: Rotating robot counter-clockwise")
                send_command(ser, "B 1 -285 2 285 3 285 4 -285")
                time.sleep(3)
                
            elif choice == '15':
                print("\n→ TURN RIGHT 45°: Rotating robot clockwise")
                send_command(ser, "B 1 285 2 -285 3 -285 4 285")
                time.sleep(3)
                
            elif choice == '16':
                print("\n→ TURN LEFT 90°: Quarter turn counter-clockwise")
                send_command(ser, "B 1 -570 2 570 3 570 4 -570")
                time.sleep(3)
                
            elif choice == '17':
                print("\n→ TURN RIGHT 90°: Quarter turn clockwise")
                send_command(ser, "B 1 570 2 -570 3 -570 4 570")
                time.sleep(3)
                
            elif choice == '18':
                print("\n→ TURN LEFT 180°: Half turn counter-clockwise")
                send_command(ser, "B 1 -1140 2 1140 3 1140 4 -1140")
                time.sleep(4)
                
            elif choice == '19':
                print("\n→ TURN RIGHT 180°: Half turn clockwise")
                send_command(ser, "B 1 1140 2 -1410 3 -1140 4 1140")
                time.sleep(4)
            
            elif choice == '20':
                print("\n→ Reading encoder values for 5 seconds...")
                start = time.time()
                while time.time() - start < 5:
                    while ser.in_waiting:
                        line = ser.readline().decode('utf-8', errors='ignore').strip()
                        if line.startswith('DATA'):
                            print(f"  {line}")
                    time.sleep(0.1)
                    
            elif choice == '21':
                print("\n→ RESET ENCODERS - Zeroing all positions")
                send_command(ser, "RESET")
                time.sleep(1)
                
            elif choice == '22':
                print("\n→ EMERGENCY STOP!")
                send_command(ser, "STOP")
                
            elif choice == '23':
                cmd = input("Enter custom command: ")
                send_command(ser, cmd)
                time.sleep(3)
                
            elif choice == '0':
                print("\n→ Sending STOP and exiting...")
                send_command(ser, "STOP")
                break
                
            else:
                print("Invalid choice!")
            
            time.sleep(0.5)
            print("\nResponses:")
            timeout = time.time() + 2
            while time.time() < timeout:
                while ser.in_waiting:
                    line = ser.readline().decode('utf-8', errors='ignore').strip()
                    if line and not line.startswith('DATA'):
                        print(f"  {line}")
                time.sleep(0.1)
        
        ser.close()
        print("Disconnected.")
        
    except serial.SerialException as e:
        print(f"Error: {e}")
        print("Make sure Arduino is connected and Serial Monitor is closed!")
    except KeyboardInterrupt:
        print("\n\nInterrupted! Sending STOP...")
        if 'ser' in locals() and ser.is_open:
            send_command(ser, "STOP")
            ser.close()

if __name__ == '__main__':
    main()
