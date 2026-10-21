#!/usr/bin/env python3
"""
Unit tests for the Python Log Parser
"""

import os
import struct
import zlib
import unittest
from log_parser import parse_log_file, export_csv, BLOCK_SIZE, BLOCK_MAGIC, BLOCK_VERSION, RECORDS_PER_BLOCK

def generate_mock_record(seq: int, ts: int) -> bytes:
    # <IIhhhhhhhhhH (28 bytes)
    return struct.pack("<IIhhhhhhhhhH", seq, ts, 2048, 2000, 2100, 100, 200, 16000, 10, -10, 0, 0)

def generate_mock_block(first_seq: int, record_count: int, session_id: int) -> bytes:
    records_bytes = bytearray()
    for i in range(record_count):
        records_bytes.extend(generate_mock_record(first_seq + i, 10000 * (first_seq + i)))
    # pad up to 17 records
    for i in range(RECORDS_PER_BLOCK - record_count):
        records_bytes.extend(bytes(28))

    header = struct.pack("<IBBHII", BLOCK_MAGIC, BLOCK_VERSION, 1, record_count, first_seq, session_id)
    padding = bytes(16)
    payload = header + records_bytes + padding
    crc = zlib.crc32(payload) & 0xFFFFFFFF
    return payload + struct.pack("<I", crc)

class TestLogParser(unittest.TestCase):
    def setUp(self):
        self.test_bin = "test_run.bin"
        self.test_csv = "test_run.csv"

    def tearDown(self):
        for path in (self.test_bin, self.test_csv):
            if os.path.exists(path):
                os.remove(path)

    def test_valid_blocks(self):
        # 2 full blocks = 34 records
        with open(self.test_bin, "wb") as f:
            f.write(generate_mock_block(0, 17, 0x12345678))
            f.write(generate_mock_block(17, 17, 0x12345678))

        records, stats = parse_log_file(self.test_bin)
        self.assertEqual(stats["total_blocks"], 2)
        self.assertEqual(stats["valid_blocks"], 2)
        self.assertEqual(stats["corrupted_blocks"], 0)
        self.assertEqual(stats["total_records"], 34)
        self.assertEqual(stats["sequence_gaps"], 0)
        self.assertEqual(stats["last_seq"], 33)

        export_csv(records, self.test_csv)
        self.assertTrue(os.path.exists(self.test_csv))

    def test_corrupted_crc_block(self):
        with open(self.test_bin, "wb") as f:
            blk1 = generate_mock_block(0, 17, 0x12345678)
            f.write(blk1)
            # Corrupted block
            corrupted = bytearray(generate_mock_block(17, 17, 0x12345678))
            corrupted[10] ^= 0xFF
            f.write(corrupted)

        records, stats = parse_log_file(self.test_bin)
        self.assertEqual(stats["total_blocks"], 2)
        self.assertEqual(stats["valid_blocks"], 1)
        self.assertEqual(stats["corrupted_blocks"], 1)
        self.assertEqual(stats["total_records"], 17)

    def test_sequence_gap_detection(self):
        with open(self.test_bin, "wb") as f:
            f.write(generate_mock_block(0, 17, 0x12345678))
            # Next block starts at seq 25 (jump from 16 to 25 -> gap)
            f.write(generate_mock_block(25, 17, 0x12345678))

        records, stats = parse_log_file(self.test_bin)
        self.assertEqual(stats["valid_blocks"], 2)
        self.assertEqual(stats["sequence_gaps"], 1)

if __name__ == "__main__":
    unittest.main()
