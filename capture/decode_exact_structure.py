#!/usr/bin/env python3
"""
Decode exact Android payload structure byte-by-byte
"""

# Android PCAP: ROW=3
android_hex = "080110141801200f28bf82013a0c089c8080808080bc81071003422432303939643762392d323537612d343166632d613161622d376535316165326630333030"

# C++ generated: ROW=3
cpp_hex = "080110141801200f28bf82013a0c089c80808080bc81071003422432303939643762392d323537612d343166632d613161622d37653531616532663033303000"

android_bytes = bytes.fromhex(android_hex)
cpp_bytes = bytes.fromhex(cpp_hex)

print("="*80)
print("BYTE-BY-BYTE COMPARISON")
print("="*80)
print()
print(f"Android: {len(android_bytes)} bytes")
print(f"C++:     {len(cpp_bytes)} bytes")
print()

# Show side-by-side
print("Off  Android  C++      Diff")
print("-" * 40)
max_len = max(len(android_bytes), len(cpp_bytes))
for i in range(max_len):
    android_val = f"{android_bytes[i]:02x}" if i < len(android_bytes) else "--"
    cpp_val = f"{cpp_bytes[i]:02x}" if i < len(cpp_bytes) else "--"
    
    marker = ""
    if i < len(android_bytes) and i < len(cpp_bytes):
        if android_bytes[i] != cpp_bytes[i]:
            marker = " <<<< DIFF"
    elif i >= len(android_bytes):
        marker = " <<<< EXTRA in C++"
    elif i >= len(cpp_bytes):
        marker = " <<<< MISSING in C++"
    
    # Decode special offsets
    annotation = ""
    if i == 15:
        annotation = " # Selector"
    elif i == 24:
        annotation = " # Field tag or value?"
    elif i == 25:
        annotation = " # Value or UUID field?"
    elif i == 26:
        annotation = " # UUID length?"
    elif i == 27:
        annotation = " # UUID start?"
    
    print(f"{i:3d}  {android_val:>6}  {cpp_val:>6}  {marker}{annotation}")

print()
print("="*80)
print("KEY OBSERVATIONS")
print("="*80)
print()

# Show critical offsets
print("Offset 15 (Selector):")
print(f"  Android: 0x{android_bytes[15]:02x}")
print(f"  C++:     0x{cpp_bytes[15]:02x}")
print()

print("Offset 24-27 (Value + UUID field area):")
for i in range(24, min(28, len(android_bytes), len(cpp_bytes))):
    print(f"  [{i}] Android: 0x{android_bytes[i]:02x}  C++: 0x{cpp_bytes[i]:02x}")
print()

# Decode as ASCII from offset 27 onwards for UUID
print("UUID comparison (from offset 27):")
android_uuid = android_bytes[27:63].decode('ascii', errors='replace')
cpp_uuid = cpp_bytes[27:63].decode('ascii', errors='replace')
print(f"  Android: '{android_uuid}'")
print(f"  C++:     '{cpp_uuid}'")
print()

if android_uuid != cpp_uuid:
    print("❌ UUID MISMATCH!")
else:
    print("✓ UUID matches")
