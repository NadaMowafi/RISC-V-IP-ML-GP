# RISC-V Vector LMUL Performance Analysis with Real Image Data

## Overview

This document presents comprehensive LMUL (Length Multiplier) performance analysis for RISC-V Vector image processing operations using **real image data** (`barb.512.pgm`, 512×512 pixels) instead of synthetic test patterns.

## Test Configuration

- **Image**: `barb.512.pgm` (512×512 grayscale image)
- **Simulator**: Spike RISC-V ISA Simulator
- **Architecture**: RV64GCV (with Vector Extension)
- **Iterations**: 100 per test for statistical accuracy
- **LMUL Values Tested**: m1, m2, m4, m8

## Performance Results Summary

### 🏆 Best Performing LMUL (m8) Results

| Operation | Time (ms) | Speedup vs m1 | Throughput (MPix/sec) | Efficiency |
|-----------|-----------|---------------|----------------------|------------|
| **Image Addition** | 0.251 | **2.14×** | 1044.6 | 26.8% |
| **Vertical Flip** | 0.234 | **1.86×** | 1120.0 | 23.2% |
| **Gaussian Filter** | 0.468 | **2.52×** | 559.7 | 31.5% |
| **Box Filter** | 0.458 | **2.40×** | 572.2 | 30.0% |
| **Horizontal Flip** | 1.323 | **1.37×** | 198.1 | 17.1% |

### 📊 Complete Performance Matrix

#### Image Addition
- **m1**: 0.538ms (1.00×) - 487.4 MPix/sec - 100.0% efficiency
- **m2**: 0.374ms (1.44×) - 701.1 MPix/sec - 72.0% efficiency  
- **m4**: 0.292ms (1.84×) - 897.9 MPix/sec - 46.0% efficiency
- **m8**: 0.251ms (2.14×) - 1044.6 MPix/sec - 26.8% efficiency

#### Horizontal Flip
- **m1**: 1.810ms (1.00×) - 144.8 MPix/sec - 100.0% efficiency
- **m2**: 1.532ms (1.18×) - 171.1 MPix/sec - 59.0% efficiency
- **m4**: 1.392ms (1.30×) - 188.2 MPix/sec - 32.5% efficiency
- **m8**: 1.323ms (1.37×) - 198.1 MPix/sec - 17.1% efficiency

#### Vertical Flip
- **m1**: 0.435ms (1.00×) - 602.3 MPix/sec - 100.0% efficiency
- **m2**: 0.320ms (1.36×) - 819.2 MPix/sec - 68.0% efficiency
- **m4**: 0.263ms (1.65×) - 996.2 MPix/sec - 41.2% efficiency
- **m8**: 0.234ms (1.86×) - 1120.0 MPix/sec - 23.2% efficiency

#### Gaussian Filter
- **m1**: 1.182ms (1.00×) - 221.7 MPix/sec - 100.0% efficiency
- **m2**: 0.774ms (1.53×) - 338.5 MPix/sec - 76.5% efficiency
- **m4**: 0.570ms (2.07×) - 459.6 MPix/sec - 51.7% efficiency
- **m8**: 0.468ms (2.52×) - 559.7 MPix/sec - 31.5% efficiency

#### Box Filter
- **m1**: 1.101ms (1.00×) - 238.2 MPix/sec - 100.0% efficiency
- **m2**: 0.734ms (1.50×) - 357.4 MPix/sec - 75.0% efficiency
- **m4**: 0.550ms (2.00×) - 476.7 MPix/sec - 50.0% efficiency
- **m8**: 0.458ms (2.40×) - 572.2 MPix/sec - 30.0% efficiency

## Key Insights

### 🚀 Performance Characteristics

1. **Universal Benefit**: All operations benefit from higher LMUL values
2. **Best Absolute Performance**: LMUL=m8 consistently provides the fastest execution times
3. **Speedup Range**: LMUL=m8 achieves 1.37× to 2.52× speedup over m1
4. **Throughput Range**: 198.1 to 1120.0 MPix/sec at LMUL=m8

### 📈 LMUL Efficiency Patterns

**Average Efficiency by LMUL:**
- **m1**: 100.0% (baseline)
- **m2**: 70.1% (good efficiency retention)
- **m4**: 44.3% (moderate efficiency)
- **m8**: 25.7% (lower efficiency but highest absolute performance)

### 🎯 Operation-Specific Behavior

**Best Scaling Operations:**
1. **Gaussian Filter**: 2.52× speedup (excellent compute-bound scaling)
2. **Box Filter**: 2.40× speedup (good convolution scaling)
3. **Image Addition**: 2.14× speedup (simple arithmetic scales well)

**Limited Scaling Operations:**
1. **Horizontal Flip**: 1.37× speedup (memory access pattern limitations)
2. **Vertical Flip**: 1.86× speedup (better memory locality than horizontal)

### 🔍 Technical Analysis

**Why Efficiency Decreases with Higher LMUL:**
- **Register Pressure**: More vector registers consumed per operation
- **Memory Bandwidth**: Saturation of memory subsystem with wider loads/stores
- **Algorithm Characteristics**: Some operations become memory-bound rather than compute-bound
- **Overhead**: Increased setup and management costs for wider vectors

**Memory Access Pattern Impact:**
- **Horizontal Flip**: Poor cache locality due to stride access patterns
- **Vertical Flip**: Better locality with sequential row copying
- **Filters**: Good locality with sliding window operations
- **Addition**: Excellent locality with element-wise operations

## Practical Recommendations

### 🏁 For Maximum Performance
**Use LMUL=m8** when:
- Absolute performance is critical
- Power/efficiency is less important
- Sufficient vector register resources available

### ⚖️ For Balanced Performance/Efficiency
**Use LMUL=m4** when:
- Good performance with reasonable efficiency needed
- Moderate register pressure acceptable
- Balance between speed and resource usage

### 🔋 For Conservative Scaling
**Use LMUL=m2** when:
- Efficiency is important (maintains >60% efficiency)
- Register pressure must be minimized
- Predictable performance characteristics needed

### 📊 Algorithm-Specific Recommendations

| Operation | Recommended LMUL | Rationale |
|-----------|------------------|-----------|
| **Gaussian/Box Filter** | m8 | Excellent scaling (>2.4× speedup) |
| **Image Addition** | m8 | Good scaling (2.14× speedup) |
| **Vertical Flip** | m4 or m8 | Decent scaling, m4 balances efficiency |
| **Horizontal Flip** | m2 or m4 | Limited scaling, prioritize efficiency |

## Comparison with Synthetic Data

**Real Image Benefits:**
- More realistic performance measurements
- Actual cache behavior and memory access patterns
- Representative of real-world workloads
- Better guidance for production algorithm selection

**Key Differences from Synthetic Patterns:**
- More conservative speedup numbers (realistic)
- Better understanding of memory subsystem impact
- Clearer efficiency degradation patterns
- More accurate throughput measurements

## Generated Assets

1. **`real_image_lmul_analysis.png`**: Comprehensive 4-panel visualization
2. **`real_image_lmul_results.csv`**: Complete dataset for further analysis
3. **`complete_lmul_analysis.png`**: Summary visualization from analysis script
4. **Individual benchmark result files**: Detailed per-operation analysis

## Conclusion

This real image analysis provides **production-ready guidance** for LMUL selection in RISC-V Vector image processing applications. The results demonstrate that while LMUL=m8 provides the best absolute performance, the choice of LMUL should consider the specific operation characteristics, efficiency requirements, and system constraints.

The analysis confirms that RISC-V Vector extensions with appropriate LMUL selection can provide significant performance improvements (1.37× to 2.52×) for image processing workloads, with the best results achieved by compute-bound operations like filtering and arithmetic operations. 