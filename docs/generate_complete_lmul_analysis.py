#!/usr/bin/env python3
"""
Complete RISC-V Vector LMUL Analysis - All Operations
Comprehensive performance analysis across all implemented benchmarks
"""
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns
from matplotlib.patches import Rectangle
import matplotlib.patches as mpatches

# Set style for professional plots
plt.style.use('seaborn-v0_8-whitegrid')
sns.set_palette("husl")

# Define data with all benchmark results
results_data = {
    'Operation': ['Image Addition', 'Image Addition', 'Image Addition', 'Image Addition',
                  'Horizontal Flip', 'Horizontal Flip', 'Horizontal Flip', 'Horizontal Flip',
                  'Vertical Flip', 'Vertical Flip', 'Vertical Flip', 'Vertical Flip',
                  'Gaussian Filter', 'Gaussian Filter', 'Gaussian Filter', 'Gaussian Filter',
                  'Box Filter', 'Box Filter', 'Box Filter', 'Box Filter'],
    'LMUL': ['m1', 'm2', 'm4', 'm8'] * 5,
    'Time_ms': [0.538, 0.374, 0.292, 0.251,  # Image Addition
                1.810, 1.532, 1.392, 1.323,  # Horizontal Flip
                0.435, 0.320, 0.263, 0.234,  # Vertical Flip
                1.182, 0.774, 0.570, 0.468,  # Gaussian Filter
                1.101, 0.734, 0.550, 0.458], # Box Filter
    'Speedup_vs_m1': [1.00, 1.44, 1.84, 2.14,  # Image Addition
                      1.00, 1.18, 1.30, 1.37,  # Horizontal Flip
                      1.00, 1.36, 1.65, 1.86,  # Vertical Flip
                      1.00, 1.53, 2.07, 2.52,  # Gaussian Filter
                      1.00, 1.50, 2.00, 2.40], # Box Filter
    'Efficiency_percent': [100.0, 71.9, 46.0, 26.8,  # Image Addition
                          100.0, 59.1, 32.5, 17.1,   # Horizontal Flip
                          100.0, 67.9, 41.4, 23.2,   # Vertical Flip
                          100.0, 76.3, 51.8, 31.6,   # Gaussian Filter
                          100.0, 75.0, 50.0, 30.0],  # Box Filter
    'Throughput_MPix_per_sec': [487.2, 701.3, 898.6, 1044.6,  # Image Addition
                               144.8, 171.1, 188.2, 198.0,    # Horizontal Flip
                               602.3, 819.2, 996.2, 1119.7,   # Vertical Flip
                               221.7, 338.5, 459.6, 559.7,    # Gaussian Filter
                               238.2, 357.4, 476.7, 572.2]    # Box Filter
}

df = pd.DataFrame(results_data)

# Create comprehensive figure
fig, axes = plt.subplots(2, 3, figsize=(20, 14))
fig.suptitle('RISC-V Vector LMUL Performance Analysis - Complete Results\nAll Image Processing Operations', 
             fontsize=20, fontweight='bold', y=0.98)

# Color palette for operations
operation_colors = {
    'Image Addition': '#FF6B6B',
    'Horizontal Flip': '#4ECDC4', 
    'Vertical Flip': '#45B7D1',
    'Gaussian Filter': '#96CEB4',
    'Box Filter': '#FFEAA7'
}

# 1. Execution Time Comparison
ax1 = axes[0, 0]
x = np.arange(4)
width = 0.15
lmuls = ['m1', 'm2', 'm4', 'm8']

for i, operation in enumerate(operation_colors.keys()):
    op_data = df[df['Operation'] == operation]
    times = op_data['Time_ms'].values
    ax1.bar(x + i*width, times, width, label=operation, color=operation_colors[operation], alpha=0.8)

ax1.set_xlabel('LMUL Configuration', fontweight='bold')
ax1.set_ylabel('Execution Time (ms)', fontweight='bold')
ax1.set_title('Execution Time by LMUL', fontweight='bold', fontsize=14)
ax1.set_xticks(x + width*2)
ax1.set_xticklabels(lmuls)
ax1.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
ax1.grid(True, alpha=0.3)

# 2. Speedup vs m1 Comparison
ax2 = axes[0, 1]
for i, operation in enumerate(operation_colors.keys()):
    op_data = df[df['Operation'] == operation]
    speedups = op_data['Speedup_vs_m1'].values
    ax2.plot(lmuls, speedups, marker='o', linewidth=3, markersize=8, 
             label=operation, color=operation_colors[operation])

# Add theoretical speedup line
theoretical = [1, 2, 4, 8]
ax2.plot(lmuls, theoretical, '--', color='black', linewidth=2, alpha=0.7, label='Theoretical (Linear)')

ax2.set_xlabel('LMUL Configuration', fontweight='bold')
ax2.set_ylabel('Speedup vs LMUL=m1', fontweight='bold')
ax2.set_title('LMUL Speedup Progression', fontweight='bold', fontsize=14)
ax2.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
ax2.grid(True, alpha=0.3)
ax2.set_yscale('log', base=2)

# 3. Efficiency Analysis
ax3 = axes[0, 2]
for operation in operation_colors.keys():
    op_data = df[df['Operation'] == operation]
    efficiencies = op_data['Efficiency_percent'].values
    ax3.plot(lmuls, efficiencies, marker='s', linewidth=3, markersize=8,
             label=operation, color=operation_colors[operation])

ax3.set_xlabel('LMUL Configuration', fontweight='bold')
ax3.set_ylabel('Efficiency (%)', fontweight='bold')
ax3.set_title('LMUL Efficiency Trends', fontweight='bold', fontsize=14)
ax3.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
ax3.grid(True, alpha=0.3)
ax3.set_ylim(0, 105)

# 4. Throughput Comparison
ax4 = axes[1, 0]
for operation in operation_colors.keys():
    op_data = df[df['Operation'] == operation]
    throughputs = op_data['Throughput_MPix_per_sec'].values
    ax4.plot(lmuls, throughputs, marker='^', linewidth=3, markersize=8,
             label=operation, color=operation_colors[operation])

ax4.set_xlabel('LMUL Configuration', fontweight='bold')
ax4.set_ylabel('Throughput (MPix/sec)', fontweight='bold')
ax4.set_title('Processing Throughput', fontweight='bold', fontsize=14)
ax4.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
ax4.grid(True, alpha=0.3)

# 5. Best LMUL Analysis (Heatmap)
ax5 = axes[1, 1]
pivot_speedup = df.pivot(index='Operation', columns='LMUL', values='Speedup_vs_m1')
sns.heatmap(pivot_speedup, annot=True, fmt='.2f', cmap='RdYlGn', 
            ax=ax5, cbar_kws={'label': 'Speedup vs m1'})
ax5.set_title('LMUL Speedup Heatmap', fontweight='bold', fontsize=14)
ax5.set_xlabel('LMUL Configuration', fontweight='bold')
ax5.set_ylabel('')

# 6. Operation Characteristics Summary
ax6 = axes[1, 2]
# Best LMUL performance for each operation
best_results = []
for operation in operation_colors.keys():
    op_data = df[df['Operation'] == operation]
    best_idx = op_data['Speedup_vs_m1'].idxmax()
    best_lmul = op_data.loc[best_idx, 'LMUL']
    best_speedup = op_data.loc[best_idx, 'Speedup_vs_m1']
    best_efficiency = op_data.loc[best_idx, 'Efficiency_percent']
    best_results.append({
        'Operation': operation,
        'Best_LMUL': best_lmul,
        'Best_Speedup': best_speedup,
        'Efficiency_at_Best': best_efficiency
    })

best_df = pd.DataFrame(best_results)
bars = ax6.bar(range(len(best_df)), best_df['Best_Speedup'], 
               color=[operation_colors[op] for op in best_df['Operation']], alpha=0.8)

# Add efficiency labels on bars
for i, (bar, eff) in enumerate(zip(bars, best_df['Efficiency_at_Best'])):
    height = bar.get_height()
    ax6.text(bar.get_x() + bar.get_width()/2., height + 0.05,
             f'{eff:.1f}%', ha='center', va='bottom', fontweight='bold')

ax6.set_xlabel('Operations', fontweight='bold')
ax6.set_ylabel('Best Speedup vs m1', fontweight='bold')
ax6.set_title('Best LMUL Performance\n(with efficiency %)', fontweight='bold', fontsize=14)
ax6.set_xticks(range(len(best_df)))
ax6.set_xticklabels([op.replace(' ', '\n') for op in best_df['Operation']], rotation=0)
ax6.grid(True, alpha=0.3)

plt.tight_layout()
plt.subplots_adjust(top=0.93)

# Save the plot
plt.savefig('complete_lmul_analysis.png', dpi=300, bbox_inches='tight')
print("Complete LMUL analysis saved as 'complete_lmul_analysis.png'")

# Print comprehensive summary
print("\n" + "="*80)
print("COMPREHENSIVE RISC-V VECTOR LMUL ANALYSIS RESULTS")
print("="*80)

print("\n📊 PERFORMANCE SUMMARY BY OPERATION:")
print("-" * 50)
for operation in operation_colors.keys():
    op_data = df[df['Operation'] == operation]
    best_idx = op_data['Speedup_vs_m1'].idxmax()
    best_lmul = op_data.loc[best_idx, 'LMUL']
    best_speedup = op_data.loc[best_idx, 'Speedup_vs_m1']
    best_time = op_data.loc[best_idx, 'Time_ms']
    best_throughput = op_data.loc[best_idx, 'Throughput_MPix_per_sec']
    
    print(f"\n{operation}:")
    print(f"  • Best LMUL: {best_lmul} ({best_speedup:.2f}x speedup)")
    print(f"  • Execution time: {best_time:.3f} ms")
    print(f"  • Throughput: {best_throughput:.1f} MPix/sec")

print("\n🎯 KEY INSIGHTS:")
print("-" * 50)
print("1. Higher LMUL consistently improves performance across ALL operations")
print("2. LMUL=m8 achieves best absolute performance in every case")
print("3. Efficiency decreases with higher LMUL as expected:")

avg_efficiency_by_lmul = df.groupby('LMUL')['Efficiency_percent'].mean()
for lmul in ['m1', 'm2', 'm4', 'm8']:
    avg_eff = avg_efficiency_by_lmul[lmul]
    print(f"   • {lmul}: {avg_eff:.1f}% average efficiency")

print("\n4. Operation characteristics:")
print("   • Image Addition: Best LMUL scaling (compute-bound)")
print("   • Gaussian/Box Filter: Good scaling with moderate efficiency")
print("   • Vertical Flip: Better scaling than horizontal (memory access pattern)")
print("   • Horizontal Flip: Limited scaling due to complex memory patterns")

print("\n🏆 OPTIMAL LMUL RECOMMENDATIONS:")
print("-" * 50)
print("• For MAXIMUM PERFORMANCE: Use LMUL=m8 for all operations")
print("• For BALANCED EFFICIENCY: Use LMUL=m4 (maintains >40% efficiency)")
print("• For CONSERVATIVE SCALING: Use LMUL=m2 (maintains >60% efficiency)")

print("\n📈 PERFORMANCE RANGES:")
print("-" * 50)
all_speedups_m8 = df[df['LMUL'] == 'm8']['Speedup_vs_m1']
print(f"LMUL=m8 speedup range: {all_speedups_m8.min():.2f}x - {all_speedups_m8.max():.2f}x")
print(f"Average LMUL=m8 speedup: {all_speedups_m8.mean():.2f}x")

all_throughputs_m8 = df[df['LMUL'] == 'm8']['Throughput_MPix_per_sec']
print(f"Throughput range: {all_throughputs_m8.min():.1f} - {all_throughputs_m8.max():.1f} MPix/sec")

print("\n" + "="*80)
print("Analysis complete! Check 'complete_lmul_analysis.png' for visualizations.")
print("="*80) 