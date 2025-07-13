# RISC-V Vector LMUL Performance Analysis Framework

A comprehensive image processing library and performance analysis framework for RISC-V processors with Vector Extension (RVV). This project provides detailed LMUL (Length Multiplier) performance analysis across multiple image processing operations.

## 🎯 Project Overview

This framework evaluates RISC-V Vector Extension performance across different LMUL configurations (m1, m2, m4, m8) for image processing operations, providing data-driven insights for optimal vector optimization.

## 📊 Key Performance Results

### Current Benchmarks (512×512 images)
- **Image Addition**: Up to 36.75x speedup vs scalar (LMUL=m8)
- **Vertical Flip**: Up to 8.55x speedup vs scalar (LMUL=m8)  
- **Gaussian Filter**: Up to 82.84x speedup vs scalar (production algorithms)
- **Box Filter**: Up to 61.00x speedup vs scalar (production algorithms)
- **Horizontal Flip**: Limited scaling due to memory access patterns

## 🏗️ Project Structure

```
riscv-image-processing/
├── lib/                          # Core vector library
│   ├── include/                  # Vector operation headers
│   │   ├── VectorTraits_LMUL.hpp # LMUL-specific vector traits
│   │   ├── VectorTraits.hpp      # Base vector traits
│   │   └── *_Vector.hpp          # Algorithm implementations
│   └── src/                      # Vector operation implementations
├── examples/                     # Performance benchmarks
│   ├── *Benchmark_LMUL.cpp      # LMUL comparison benchmarks
│   ├── *Benchmark.cpp           # Standard benchmarks
│   └── filters_test.cpp         # Algorithm validation
├── build/                        # Performance results (CSV files)
├── docs/                        # Analysis charts and scripts
├── models/                      # Image data structures
├── tests/                       # Reference implementations
└── utils/                       # I/O utilities
```

## 🚀 Quick Start

### Prerequisites
- RISC-V GCC toolchain with Vector Extension support
- CMake 3.15+
- Spike RISC-V simulator (for testing)

### Build Instructions

```bash
# Clone the repository
git clone <repository-url>
cd riscv-image-processing

# Build the project
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../riscv-toolchain.cmake
make -j$(nproc)
```

### Running Benchmarks

```bash
# Run LMUL analysis benchmarks
spike_run ./AddBenchmark_LMUL
spike_run ./FlippingBenchmark_LMUL
spike_run ./GaussianBenchmark_LMUL
spike_run ./BoxFilterBenchmark_LMUL

# Run production algorithm benchmarks
spike_run ./filters_test
```

## 📈 Performance Analysis

### LMUL Scaling Characteristics
- **LMUL=m1**: Baseline performance, 100% efficiency
- **LMUL=m2**: ~70% efficiency, good balance
- **LMUL=m4**: ~44% efficiency, higher performance
- **LMUL=m8**: ~26% efficiency, maximum throughput

### Operation Categories
1. **Compute-bound** (Addition, Filters): Excellent LMUL scaling
2. **Memory-bound** (Horizontal flip): Limited scaling
3. **Sequential access** (Vertical flip): Good scaling

## 🔬 Key Technical Insights

### Register Pressure Analysis
- **Well-designed algorithms** perform excellently across all LMUL values
- **Register pressure only affects poorly optimized code** with excessive unrolling
- **Spike simulator accurately models** finite vector register constraints

### Algorithm Design Guidelines
- Use **conservative vectorization approaches** for LMUL=m8
- Avoid **excessive loop unrolling** (causes severe penalties)
- **Profile on target hardware** for final optimization decisions

## 📋 Documentation

### Analysis Reports
- `COMPLETE_LMUL_RESULTS.md` - Comprehensive performance analysis
- `REGISTER_PRESSURE_ANALYSIS.md` - Register pressure investigation
- `REALISTIC_LMUL_EXPECTATIONS.md` - Simulator vs hardware comparison
- `VECTOR_INTRINSICS_GUIDE.md` - RISC-V Vector programming guide

### Performance Data
- `build/*.csv` - Raw performance measurements
- `docs/*.png` - Performance analysis charts

## 🛠️ Core Technologies

- **RISC-V Vector Extension (RVV)** - Hardware acceleration
- **Template-based design** - Type-safe vector operations  
- **Comprehensive LMUL support** - m1, m2, m4, m8 configurations
- **Production-quality algorithms** - Real-world performance validation

## 📊 Current Status

✅ **Complete LMUL analysis framework**  
✅ **5 different image processing operations**  
✅ **Production algorithm implementations**  
✅ **Comprehensive performance validation**  
✅ **Professional documentation and visualization**

## 🤝 Contributing

This framework provides a foundation for RISC-V Vector Extension performance research. Contributions welcome for:
- Additional image processing algorithms
- Hardware-specific optimizations
- Extended LMUL analysis
- Performance comparison studies

## 📄 License

[Specify your license here]

---

**Note**: This framework focuses on LMUL performance analysis. For production deployment, always profile on target hardware as simulator results may differ from actual silicon performance.
