import argparse
import sys
import struct

def generate_mock_weights(output_path, max_size_mb, quantize):
    print(f"Generating block mock weights at {output_path} with Max Size {max_size_mb}MB and Quantization {quantize}...")
    
    # Simple struct matching expectations:
    # 22 layers, 2048 embed dim, 32000 vocab size.
    # To save space and compile time for the mock, we can output a small array.
    # But the C code uses `sizeof(gModelWeights)`, so let's output a decently sized array 
    # but not too large to avoid huge compile times, or just enough to satisfy the struct check.
    # Actually, let's keep it small for testing: just 1MB of 0s.
    
    mock_array_size = 1024 * 1024  # 1 MB
    
    with open(output_path, "w") as f:
        f.write("#ifndef AIBIOS_MODEL_WEIGHTS_H\n")
        f.write("#define AIBIOS_MODEL_WEIGHTS_H\n\n")
        f.write("#include <Uefi.h>\n\n")
        
        f.write("// MOCK WEIGHTS FOR FAST COMPILATION\n")
        f.write(f"STATIC CONST UINT8 gModelWeights[{mock_array_size}] = {{\n")
        f.write("  0\n") # Fill with 0s, GCC will zero-initialize the rest
        f.write("};\n\n")
        
        f.write("#endif // AIBIOS_MODEL_WEIGHTS_H\n")
        
    print(f"Mock weights written successfully to {output_path}")

def main():
    parser = argparse.ArgumentParser(description="Convert GGUF to C Array")
    parser.add_argument("--input", type=str, help="Input GGUF file path")
    parser.add_argument("--output", type=str, required=True, help="Output C header file path")
    parser.add_argument("--max-size-mb", type=int, default=48, help="Max size in MB")
    parser.add_argument("--quantize", type=str, default="int8", help="Quantization level")
    parser.add_argument("--mock", action="store_true", help="Generate mock header instead of parsing GGUF")
    
    args = parser.parse_args()
    
    if args.mock:
        generate_mock_weights(args.output, args.max_size_mb, args.quantize)
        sys.exit(0)
        
    if not args.input:
        print("Error: --input is required unless --mock is used.")
        sys.exit(1)
        
    print("Full GGUF parsing not implemented in mock yet. Use --mock flag.")
    sys.exit(1)

if __name__ == "__main__":
    main()
