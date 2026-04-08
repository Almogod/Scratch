import sys
import argparse
import struct
import os

def parse_args():
    parser = argparse.ArgumentParser(description='Convert GGUF weights to C header array.')
    parser.add_argument('--input', type=str, required=True, help='Input GGUF file')
    parser.add_argument('--output', type=str, required=True, help='Output C header file')
    parser.add_argument('--max-size-mb', type=int, default=48, help='Max size in MB')
    parser.add_argument('--quantize', type=str, choices=['int8'], default='int8', help='Quantization type')
    return parser.parse_args()

def main():
    args = parse_args()
    
    if not os.path.exists(args.input):
        print(f"Error: {args.input} not found.")
        sys.exit(1)

    file_size = os.path.getsize(args.input)
    max_bytes = args.max_size_mb * 1024 * 1024

    print(f"Reading {args.input} ({file_size / (1024*1024):.2f} MB)...")
    
    with open(args.input, 'rb') as f:
        # Simple GGUF read
        magic = f.read(4)
        if magic != b'GGUF':
            print("Error: Not a GGUF file.")
            sys.exit(1)
        
        version = struct.unpack('<I', f.read(4))[0]
        print(f"GGUF version: {version}")
        
        # We just need to read the raw bytes up to max_bytes for now
        # In a real scenario, we'd parse the metadata to get layer info
        f.seek(0)
        data = f.read(max_bytes)

    print(f"Writing {args.output}...")
    
    with open(args.output, 'w') as out:
        out.write("#ifndef AIBIOS_MODEL_WEIGHTS_H\n")
        out.write("#define AIBIOS_MODEL_WEIGHTS_H\n\n")
        out.write("#include <Uefi.h>\n\n")
        
        out.write("typedef struct {\n")
        out.write("  UINT32 Magic;\n")
        out.write("  UINT32 Version;\n")
        out.write("  UINT64 TensorCount;\n")
        out.write("  UINT64 KvCount;\n")
        out.write("} MODEL_HEADER;\n\n")
        
        out.write(f"// Model: {args.input}\n")
        out.write(f"// Size: {len(data)} bytes\n\n")
        out.write("static const UINT8 gModelWeights[] = {\n")
        
        # Write bytes in chunks of 12 per line for readability
        for i in range(0, len(data), 12):
            chunk = data[i:i+12]
            line = ", ".join([f"0x{b:02x}" for b in chunk])
            out.write(f"  {line},\n")
        
        out.write("};\n\n")
        out.write("#endif // AIBIOS_MODEL_WEIGHTS_H\n")

    print("Success!")

if __name__ == "__main__":
    main()
