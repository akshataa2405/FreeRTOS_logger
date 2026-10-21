#!/usr/bin/env python3
"""
STM32 FreeRTOS Data Logger - Host Log Parser
Validates block structure, verifies CRC-32, checks sequence continuity,
and exports timestamped records to CSV format.
"""

import sys
import struct
import zlib
import argparse
from typing import List, Dict, Any, Tuple

BLOCK_SIZE = 512
BLOCK_MAGIC = 0xDA7A1068
BLOCK_VERSION = 0x01
RECORDS_PER_BLOCK = 17
RECORD_SIZE = 28

# Block header: magic(4B), version(1B), flags(1B), record_count(2B), first_seq(4B), session_id(4B) -> 16 bytes
BLOCK_HEADER_FMT = "<IBBHII"
# Record: seq(4B), timestamp_us(4B), adc_mean(2B), adc_min(2B), adc_max(2B), ax(2B), ay(2B), az(2B), gx(2B), gy(2B), gz(2B), flags(2B) -> 28 bytes
RECORD_FMT = "<IIhhhhhhhhhH"

def parse_record(data: bytes) -> Dict[str, Any]:
    fields = struct.unpack(RECORD_FMT, data)
    return {
        "seq": fields[0],
        "timestamp_us": fields[1],
        "adc_mean": fields[2],
        "adc_min": fields[3],
        "adc_max": fields[4],
        "ax": fields[5],
        "ay": fields[6],
        "az": fields[7],
        "gx": fields[8],
        "gy": fields[9],
        "gz": fields[10],
        "flags": fields[11],
    }

def parse_log_file(filepath: str, verbose: bool = False) -> Tuple[List[Dict[str, Any]], Dict[str, Any]]:
    records: List[Dict[str, Any]] = []
    stats = {
        "total_blocks": 0,
        "valid_blocks": 0,
        "corrupted_blocks": 0,
        "total_records": 0,
        "sequence_gaps": 0,
        "last_seq": None,
        "session_id": None,
    }

    with open(filepath, "rb") as f:
        block_idx = 0
        expected_seq = None

        while True:
            block_data = f.read(BLOCK_SIZE)
            if not block_data:
                break
            if len(block_data) < BLOCK_SIZE:
                if verbose:
                    print(f"Warning: Incomplete trailing block {block_idx} ({len(block_data)} bytes), skipping.")
                break

            stats["total_blocks"] += 1
            payload = block_data[:508]
            expected_crc = struct.unpack("<I", block_data[508:512])[0]
            computed_crc = zlib.crc32(payload) & 0xFFFFFFFF

            if computed_crc != expected_crc:
                stats["corrupted_blocks"] += 1
                if verbose:
                    print(f"Block {block_idx}: CRC mismatch! Expected 0x{expected_crc:08X}, computed 0x{computed_crc:08X}")
                block_idx += 1
                continue

            magic, version, flags, record_count, first_seq, session_id = struct.unpack(BLOCK_HEADER_FMT, block_data[:16])

            if magic != BLOCK_MAGIC:
                stats["corrupted_blocks"] += 1
                if verbose:
                    print(f"Block {block_idx}: Invalid magic 0x{magic:08X}")
                block_idx += 1
                continue

            if stats["session_id"] is None:
                stats["session_id"] = session_id

            stats["valid_blocks"] += 1

            for r in range(record_count):
                rec_offset = 16 + r * RECORD_SIZE
                rec_bytes = block_data[rec_offset:rec_offset + RECORD_SIZE]
                rec = parse_record(rec_bytes)

                if expected_seq is not None and rec["seq"] != expected_seq:
                    gap_size = rec["seq"] - expected_seq
                    stats["sequence_gaps"] += 1
                    if verbose:
                        print(f"Sequence gap at record {rec['seq']}: expected {expected_seq} (lost {gap_size} records)")

                expected_seq = rec["seq"] + 1
                stats["last_seq"] = rec["seq"]
                stats["total_records"] += 1
                records.append(rec)

            block_idx += 1

    return records, stats

def export_csv(records: List[Dict[str, Any]], csv_path: str):
    import csv
    if not records:
        print("No records to export.")
        return

    fieldnames = ["seq", "timestamp_us", "adc_mean", "adc_min", "adc_max", "ax", "ay", "az", "gx", "gy", "gz", "flags"]
    with open(csv_path, "w", newline="") as csvfile:
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(records)

def main():
    parser = argparse.ArgumentParser(description="STM32 FreeRTOS Logger Binary File Parser")
    parser.add_argument("logfile", help="Path to binary log file (e.g. log_0001.bin)")
    parser.add_argument("--csv", help="Path to export CSV file", default=None)
    parser.add_argument("-v", "--verbose", action="store_true", help="Enable verbose logging")
    args = parser.parse_args()

    records, stats = parse_log_file(args.logfile, verbose=args.verbose)

    print("\n--- Parsing Summary ---")
    print(f"Total Blocks Read:    {stats['total_blocks']}")
    print(f"Valid Blocks:         {stats['valid_blocks']}")
    print(f"Corrupted Blocks:     {stats['corrupted_blocks']}")
    print(f"Total Valid Records:  {stats['total_records']}")
    print(f"Sequence Gaps:        {stats['sequence_gaps']}")
    print(f"Session ID:           {hex(stats['session_id']) if stats['session_id'] else 'None'}")
    print(f"Last Sequence Number: {stats['last_seq']}")
    print("-----------------------\n")

    if args.csv:
        export_csv(records, args.csv)
        print(f"Exported {len(records)} records to {args.csv}")

if __name__ == "__main__":
    main()
