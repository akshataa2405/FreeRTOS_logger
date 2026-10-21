#!/usr/bin/env python3
"""
Automated PySerial Test Suite for the STM32 FreeRTOS Logger Shell
"""

import sys
import time
import argparse

def run_shell_tests(port: str, baud: int = 115200):
    try:
        import serial
    except ImportError:
        print("Error: pyserial is required. Run 'pip install pyserial'")
        sys.exit(1)

    print(f"Connecting to {port} @ {baud}...")
    ser = serial.Serial(port, baud, timeout=1.0)
    time.sleep(0.5)

    def send_cmd(cmd: str) -> str:
        ser.write((cmd + "\r\n").encode("utf-8"))
        time.sleep(0.2)
        response = ser.read_all().decode("utf-8", errors="replace")
        return response

    test_commands = [
        ("help", "Available Commands"),
        ("status", "System Status"),
        ("version", "STM32 FreeRTOS Data Logger"),
        ("rate 150", "Sampling rate set to 150 Hz"),
        ("status", "150 Hz"),
        ("rate 100", "Sampling rate set to 100 Hz"),
    ]

    passed = 0
    failed = 0

    for cmd, expected in test_commands:
        print(f"Testing command: '{cmd}'...")
        res = send_cmd(cmd)
        if expected in res:
            print(f"  [PASS] Found '{expected}'")
            passed += 1
        else:
            print(f"  [FAIL] Expected '{expected}' in response, got:\n{res}")
            failed += 1

    ser.close()
    print(f"\nShell Test Results: {passed} Passed, {failed} Failed")
    return failed == 0

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Test UART Shell on STM32")
    parser.add_argument("--port", default="/dev/tty.usbmodem14103", help="Serial port device")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate")
    args = parser.parse_args()

    success = run_shell_tests(args.port, args.baud)
    sys.exit(0 if success else 1)
