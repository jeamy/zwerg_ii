#!/usr/bin/env python3
"""
Compare C++ generated payload with Android PCAP payload
"""

# Android PCAP: ROW=3 from ctrl_20251226_113958.pcapng
pcap_row3 = "080110141801200f28bf82013a0c089c8080808080bc81071003422432303939643762392d323537612d343166632d613161622d376535316165326630333030"

# C++ generated: ROW=3 from user log
cpp_row3 = "080110141801200f28bf82013a0c089c80808080bc81071003422432303939643762392d323537612d343166632d613161622d37653531616532663033303000"

print("="*80)
print("PAYLOAD COMPARISON - ROW=3")
print("="*80)
print()

print(f"PCAP length: {len(pcap_row3)//2} bytes ({len(pcap_row3)} hex digits)")
print(f"C++  length: {len(cpp_row3)//2} bytes ({len(cpp_row3)} hex digits)")
print()

if len(pcap_row3) != len(cpp_row3):
    print("❌ LENGTH MISMATCH!")
    print()
    print(f"PCAP has {len(pcap_row3)//2} bytes")
    print(f"C++  has {len(cpp_row3)//2} bytes")
    print()
    
    # Find where they differ
    min_len = min(len(pcap_row3), len(cpp_row3))
    for i in range(0, min_len, 2):
        pcap_byte = pcap_row3[i:i+2]
        cpp_byte = cpp_row3[i:i+2]
        if pcap_byte != cpp_byte:
            print(f"First difference at byte {i//2}:")
            print(f"  PCAP: 0x{pcap_byte}")
            print(f"  C++:  0x{cpp_byte}")
            break
    
    # Show trailing bytes
    if len(cpp_row3) > len(pcap_row3):
        print()
        print("C++ has extra bytes:")
        extra = cpp_row3[len(pcap_row3):]
        print(f"  {extra}")
else:
    print("✓ Same length")
    print()
    
    # Byte-by-byte compare
    differences = []
    for i in range(0, len(pcap_row3), 2):
        pcap_byte = pcap_row3[i:i+2]
        cpp_byte = cpp_row3[i:i+2]
        if pcap_byte != cpp_byte:
            differences.append((i//2, pcap_byte, cpp_byte))
    
    if differences:
        print(f"❌ Found {len(differences)} byte difference(s):")
        for offset, pcap_b, cpp_b in differences:
            print(f"  Offset {offset}: PCAP=0x{pcap_b} C++=0x{cpp_b}")
    else:
        print("✓ All bytes match!")

print()
print("="*80)

# Decode both to see structure
print()
print("STRUCTURE COMPARISON")
print("="*80)
print()

def show_structure(name, hex_str):
    data = bytes.fromhex(hex_str)
    print(f"{name}:")
    print(f"  Total: {len(data)} bytes")
    print(f"  Offset 15 (selector): 0x{data[15]:02x}")
    if len(data) > 24:
        print(f"  Offset 24: 0x{data[24]:02x}")
    if len(data) > 25:
        print(f"  Offset 25: 0x{data[25]:02x}")
    
    # UUID field
    if len(data) >= 27:
        uuid_start = 27
        uuid_len = data[26] if data[25] == 0x42 else 0
        if uuid_len > 0 and uuid_start + uuid_len <= len(data):
            uuid_bytes = data[uuid_start:uuid_start+uuid_len]
            uuid_str = uuid_bytes.decode('ascii', errors='replace')
            print(f"  UUID (offset 27, len {uuid_len}): '{uuid_str}'")
    print()

show_structure("PCAP (Android)", pcap_row3)
show_structure("C++ (zwergII)", cpp_row3)
