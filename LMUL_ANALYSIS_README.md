# RISC-V Vector LMUL Performance Analysis

This directory contains a comprehensive framework for analyzing RISC-V Vector Length Multiplier (LMUL) performance across different image processing operations.

## Overview

LMUL (Length Multiplier) is a crucial parameter in RISC-V Vector Extension that determines how many vector registers are grouped together for each vector instruction. This analysis compares performance across LMUL values of 1, 2, 4, and 8 (m1, m2, m4, m8) to help determine optimal configurations for different operations.

## What is LMUL?

LMUL controls the effective vector length by grouping multiple vector registers:
- **m1**: Uses 1 register (baseline)
- **m2**: Groups 2 registers (2x potential elements)
- **m4**: Groups 4 registers (4x potential elements)  
- **m8**: Groups 8 registers (8x potential elements)

Higher LMUL values can process more elements per instruction but may face constraints from:
- Memory bandwidth limitations
- Register pressure
- Cache effects
- Loop overhead reduction benefits

## Framework Components

### 1. Core Infrastructure
- `VectorTraits_LMUL.hpp` - Template-based LMUL intrinsic wrapper
- `LMULBenchmark.cpp` - Comprehensive multi-operation benchmark suite

### 2. Operation-Specific Benchmarks
- `AddBenchmark_LMUL.cpp` - Image addition with different LMUL values
- `FlippingBenchmark_LMUL.cpp` - Horizontal/vertical flipping analysis
- `BoxFilterBenchmark_LMUL.cpp` - Convolution operation analysis

### 3. Visualization and Analysis
- `generate_lmul_graph.py` - Comprehensive visualization script
- `plot_lmul_results.py` - Auto-generated plotting script

## Building and Running

### Prerequisites
```bash
# Ensure you have the RISC-V toolchain installed
# Python requirements for visualization
pip install pandas matplotlib seaborn numpy
```

### Build Commands
```bash
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../riscv-toolchain.cmake -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### Running Benchmarks

#### Quick Comprehensive Analysis
```bash
# Run the main LMUL benchmark suite
./LMULBenchmark

# Generate comprehensive graphs
python3 ../generate_lmul_graph.py
```

#### Individual Operation Analysis
```bash
# Image addition analysis
./AddBenchmark_LMUL

# Flipping operations analysis  
./FlippingBenchmark_LMUL

# Box filter analysis
./BoxFilterBenchmark_LMUL
```

## Understanding the Results

### Key Metrics

1. **Execution Time (ms)**: Raw execution time for each LMUL configuration
2. **Throughput (MPix/sec)**: Megapixels processed per second
3. **Speedup vs m1**: Performance improvement relative to LMUL=1
4. **Efficiency (%)**: Actual speedup / theoretical speedup × 100

### Efficiency Interpretation

- **100%**: Perfect scaling (rare, indicates compute-bound operation)
- **75-99%**: Excellent scaling (good LMUL candidate)
- **50-74%**: Good scaling (LMUL beneficial but with limitations)
- **25-49%**: Poor scaling (memory-bound or other constraints)
- **<25%**: Very poor scaling (LMUL may be counterproductive)

### Expected Performance Patterns

#### Compute-Intensive Operations
- **Image Addition**: Expect good scaling, efficiency 70-90%
- **Simple filters**: Good candidates for higher LMUL

#### Memory-Intensive Operations  
- **Box Filter**: May show diminishing returns at m4/m8 due to memory bandwidth
- **Complex convolutions**: Limited by memory access patterns

#### Data Rearrangement Operations
- **Horizontal Flipping**: Complex due to gather/scatter operations
- **Rotations**: May not scale well due to transposition requirements

## Generated Visualizations

The analysis produces several visualizations:

### 1. Performance Dashboard (`lmul_performance_dashboard.png`)
- **Execution Time Bar Chart**: Direct time comparison
- **Throughput Bar Chart**: Processing rate comparison  
- **Speedup Line Plot**: Scaling vs theoretical maximum
- **Efficiency Heatmap**: Color-coded efficiency matrix
- **Performance Summary Table**: Best/worst LMUL per operation

### 2. Data Files
- `lmul_benchmark_results.csv`: Raw benchmark data
- Detailed console output with efficiency analysis

## Example Results Interpretation

```
=== LMUL ANALYSIS ===
Best LMUL for image addition: m4 (12.45x vs scalar)

LMUL efficiency progression:
  m1: 100.0% efficiency (actual: 1.00x, theoretical: 1.0x)
  m2: 85.2% efficiency (actual: 1.70x, theoretical: 2.0x)
  m4: 78.1% efficiency (actual: 3.12x, theoretical: 4.0x)  
  m8: 65.3% efficiency (actual: 5.22x, theoretical: 8.0x)
```

**Interpretation**: 
- m4 provides best absolute performance
- Efficiency decreases with higher LMUL (typical pattern)
- m8 still provides good speedup despite lower efficiency

## Performance Optimization Guidelines

### When to Use Higher LMUL

✅ **Good Candidates:**
- Simple arithmetic operations
- Independent pixel processing
- Compute-bound algorithms
- Regular memory access patterns

❌ **Poor Candidates:**
- Complex memory access patterns
- Operations requiring frequent branching
- Memory-bandwidth limited operations
- Algorithms with data dependencies

### LMUL Selection Strategy

1. **Start with m2**: Good balance of performance and efficiency
2. **Try m4**: Often optimal for compute-intensive operations
3. **Consider m8**: Only if m4 shows good efficiency (>70%)
4. **Profile memory usage**: Higher LMUL may cause cache thrashing

## Advanced Analysis

### Custom Operation Analysis

To add your own operation to the analysis:

1. **Create LMUL implementation**:
```cpp
template<int LMUL>
std::vector<std::vector<uint8_t>> my_operation_LMUL(const std::vector<std::vector<uint8_t>>& img) {
    using Traits = VectorTraits_LMUL<uint8_t, LMUL>;
    // Implementation using Traits
}
```

2. **Add benchmark variants**:
```cpp
// Benchmark each LMUL value
auto lmul1_fn = [&]() { my_operation_LMUL<1>(image); };
auto lmul2_fn = [&]() { my_operation_LMUL<2>(image); };
// etc.
```

3. **Integrate into analysis framework**

### Understanding Hardware Constraints

Different RISC-V implementations may show varying LMUL performance due to:
- **Vector register file size**: Determines maximum usable LMUL
- **Memory subsystem**: Bandwidth and latency characteristics
- **Cache hierarchy**: Size and associativity affecting larger vector operations
- **Execution units**: Number and capability of vector ALUs

## Troubleshooting

### Common Issues

1. **Build Errors**:
   - Ensure RISC-V toolchain is properly configured
   - Check that vector intrinsics are available

2. **Poor m8 Performance**:
   - Normal for memory-bound operations
   - May indicate insufficient memory bandwidth

3. **Unexpected Results**:
   - Verify image size matches expectations
   - Check for adequate warmup iterations
   - Consider system background load

### Performance Validation

Compare results against expectations:
- Simple operations should show good scaling to m2/m4
- Complex operations may plateau at m2
- Memory-intensive operations may show minimal improvement

## Contributing

To extend this analysis framework:

1. Add new operations to `VectorTraits_LMUL.hpp`
2. Create corresponding benchmark files
3. Update visualization scripts for new metrics
4. Document expected performance characteristics

## References

- [RISC-V Vector Extension Specification](https://github.com/riscv/riscv-v-spec)
- [Xuantie+900 Series RVV-1.0 Intrinsic Manual](./Xuantie+900+Series+RVV-1.0+Intrinsic+Manual.pdf)
- [RISC-V Vector Programming Guide](https://github.com/riscv-non-isa/rvv-intrinsic-doc)

---

**Note**: LMUL performance is highly dependent on the specific RISC-V implementation, memory subsystem, and workload characteristics. These results should be validated on your target hardware configuration. 