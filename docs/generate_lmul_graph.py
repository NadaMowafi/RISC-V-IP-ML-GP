#!/usr/bin/env python3
"""
RISC-V Vector LMUL Performance Analysis and Visualization
Generates comprehensive graphs comparing m1, m2, m4, m8 performance across different operations.
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns
from matplotlib.patches import Rectangle
import argparse
import sys

def load_benchmark_data(csv_file='lmul_benchmark_results.csv'):
    """Load benchmark results from CSV file."""
    try:
        df = pd.read_csv(csv_file)
        return df
    except FileNotFoundError:
        print(f"Error: {csv_file} not found. Please run the LMUL benchmarks first.")
        sys.exit(1)

def create_performance_dashboard(df, output_file='lmul_performance_dashboard.png'):
    """Create comprehensive performance dashboard with multiple visualizations."""
    
    # Set up the plotting style
    plt.style.use('default')
    sns.set_palette("husl")
    fig = plt.figure(figsize=(20, 15))
    
    # Create a complex grid layout
    gs = fig.add_gridspec(3, 4, hspace=0.3, wspace=0.3)
    
    operations = df['Operation'].unique()
    lmul_values = ['m1', 'm2', 'm4', 'm8']
    colors = ['#2E86AB', '#A23B72', '#F18F01', '#C73E1D']
    
    # Main title
    fig.suptitle('RISC-V Vector LMUL Performance Analysis\nComprehensive Benchmark Results', 
                 fontsize=20, fontweight='bold', y=0.98)
    
    # 1. Execution Time Comparison (Bar Chart)
    ax1 = fig.add_subplot(gs[0, :2])
    x = np.arange(len(operations))
    width = 0.2
    for i, lmul in enumerate(lmul_values):
        times = [df[(df['Operation'] == op) & (df['LMUL'] == lmul)]['Time_ms'].values[0] 
                 for op in operations]
        bars = ax1.bar(x + i*width, times, width, label=lmul, color=colors[i], alpha=0.8)
        
        # Add value labels on bars
        for bar, time in zip(bars, times):
            height = bar.get_height()
            ax1.text(bar.get_x() + bar.get_width()/2., height + 0.01*max(times),
                    f'{time:.1f}', ha='center', va='bottom', fontsize=8)
    
    ax1.set_title('Execution Time by LMUL', fontweight='bold', fontsize=14)
    ax1.set_xlabel('Operation', fontweight='bold')
    ax1.set_ylabel('Time (ms)', fontweight='bold')
    ax1.set_xticks(x + width * 1.5)
    ax1.set_xticklabels(operations, rotation=15, ha='right')
    ax1.legend(title='LMUL', fontweight='bold')
    ax1.grid(True, alpha=0.3)
    
    # 2. Throughput Comparison (Bar Chart)
    ax2 = fig.add_subplot(gs[0, 2:])
    for i, lmul in enumerate(lmul_values):
        throughputs = [df[(df['Operation'] == op) & (df['LMUL'] == lmul)]['Throughput_MPix_sec'].values[0] 
                       for op in operations]
        bars = ax2.bar(x + i*width, throughputs, width, label=lmul, color=colors[i], alpha=0.8)
        
        # Add value labels on bars
        for bar, tput in zip(bars, throughputs):
            height = bar.get_height()
            ax2.text(bar.get_x() + bar.get_width()/2., height + 0.01*max(throughputs),
                    f'{tput:.1f}', ha='center', va='bottom', fontsize=8)
    
    ax2.set_title('Throughput by LMUL', fontweight='bold', fontsize=14)
    ax2.set_xlabel('Operation', fontweight='bold')
    ax2.set_ylabel('Throughput (MPix/sec)', fontweight='bold')
    ax2.set_xticks(x + width * 1.5)
    ax2.set_xticklabels(operations, rotation=15, ha='right')
    ax2.legend(title='LMUL', fontweight='bold')
    ax2.grid(True, alpha=0.3)
    
    # 3. Speedup vs m1 (Line Plot)
    ax3 = fig.add_subplot(gs[1, :2])
    lmul_nums = [1, 2, 4, 8]
    for i, op in enumerate(operations):
        speedups = [df[(df['Operation'] == op) & (df['LMUL'] == f'm{lmul}')]['Speedup_vs_m1'].values[0] 
                    for lmul in lmul_nums]
        ax3.plot(lmul_nums, speedups, 'o-', label=op, linewidth=3, markersize=8, alpha=0.8)
    
    # Add theoretical perfect scaling line
    ax3.plot(lmul_nums, lmul_nums, 'k--', label='Theoretical (Perfect)', linewidth=2, alpha=0.7)
    ax3.set_title('Speedup vs LMUL=1', fontweight='bold', fontsize=14)
    ax3.set_xlabel('LMUL Value', fontweight='bold')
    ax3.set_ylabel('Speedup Factor', fontweight='bold')
    ax3.set_xticks(lmul_nums)
    ax3.set_xticklabels([f'm{lmul}' for lmul in lmul_nums])
    ax3.legend(fontweight='bold')
    ax3.grid(True, alpha=0.3)
    
    # 4. Efficiency Heatmap
    ax4 = fig.add_subplot(gs[1, 2:])
    efficiency_matrix = []
    for op in operations:
        op_efficiencies = []
        for lmul in lmul_nums:
            if lmul == 1:
                efficiency = 100.0  # m1 is 100% efficient relative to itself
            else:
                actual_speedup = df[(df['Operation'] == op) & (df['LMUL'] == f'm{lmul}')]['Speedup_vs_m1'].values[0]
                theoretical_speedup = lmul
                efficiency = (actual_speedup / theoretical_speedup) * 100.0
            op_efficiencies.append(efficiency)
        efficiency_matrix.append(op_efficiencies)
    
    efficiency_df = pd.DataFrame(efficiency_matrix, 
                                index=operations, 
                                columns=[f'm{lmul}' for lmul in lmul_nums])
    
    sns.heatmap(efficiency_df, annot=True, fmt='.1f', cmap='RdYlGn', 
                center=100, vmin=0, vmax=200, ax=ax4, cbar_kws={'label': 'Efficiency (%)'})
    ax4.set_title('LMUL Efficiency Heatmap', fontweight='bold', fontsize=14)
    ax4.set_xlabel('LMUL Value', fontweight='bold')
    ax4.set_ylabel('Operation', fontweight='bold')
    
    # 5. Performance per LMUL (Grouped Bar Chart)
    ax5 = fig.add_subplot(gs[2, :2])
    lmul_positions = np.arange(len(lmul_values))
    op_width = 0.15
    for i, op in enumerate(operations):
        speedups = [df[(df['Operation'] == op) & (df['LMUL'] == lmul)]['Speedup_vs_m1'].values[0] 
                    for lmul in lmul_values]
        bars = ax5.bar(lmul_positions + i*op_width, speedups, op_width, 
                       label=op, alpha=0.8)
        
        # Add value labels
        for bar, speedup in zip(bars, speedups):
            height = bar.get_height()
            ax5.text(bar.get_x() + bar.get_width()/2., height + 0.02,
                    f'{speedup:.1f}x', ha='center', va='bottom', fontsize=8)
    
    ax5.set_title('Speedup Comparison by LMUL', fontweight='bold', fontsize=14)
    ax5.set_xlabel('LMUL Value', fontweight='bold')
    ax5.set_ylabel('Speedup vs m1', fontweight='bold')
    ax5.set_xticks(lmul_positions + op_width * (len(operations)-1) / 2)
    ax5.set_xticklabels(lmul_values)
    ax5.legend(title='Operation', fontweight='bold', bbox_to_anchor=(1.05, 1), loc='upper left')
    ax5.grid(True, alpha=0.3)
    
    # 6. Performance Summary Table
    ax6 = fig.add_subplot(gs[2, 2:])
    ax6.axis('tight')
    ax6.axis('off')
    
    # Create summary statistics table
    summary_data = []
    for op in operations:
        op_data = df[df['Operation'] == op]
        best_lmul = op_data.loc[op_data['Speedup_vs_m1'].idxmax(), 'LMUL']
        best_speedup = op_data['Speedup_vs_m1'].max()
        worst_lmul = op_data.loc[op_data['Speedup_vs_m1'].idxmin(), 'LMUL']
        worst_speedup = op_data['Speedup_vs_m1'].min()
        
        summary_data.append([
            op,
            f"{best_lmul} ({best_speedup:.2f}x)",
            f"{worst_lmul} ({worst_speedup:.2f}x)",
            f"{best_speedup/worst_speedup:.2f}x"
        ])
    
    summary_df = pd.DataFrame(summary_data, 
                             columns=['Operation', 'Best LMUL', 'Worst LMUL', 'Range'])
    
    table = ax6.table(cellText=summary_df.values,
                     colLabels=summary_df.columns,
                     cellLoc='center',
                     loc='center',
                     bbox=[0, 0, 1, 1])
    
    table.auto_set_font_size(False)
    table.set_fontsize(10)
    table.scale(1, 2)
    
    # Style the table
    for i in range(len(summary_df.columns)):
        table[(0, i)].set_facecolor('#4CAF50')
        table[(0, i)].set_text_props(weight='bold', color='white')
    
    ax6.set_title('Performance Summary', fontweight='bold', fontsize=14, pad=20)
    
    # Add footer with analysis notes
    footer_text = (
        "Analysis Notes:\n"
        "• Higher LMUL values process more elements per instruction\n"
        "• Efficiency <100% indicates memory bandwidth or register pressure limitations\n"
        "• Best LMUL varies by operation due to different computational characteristics"
    )
    fig.text(0.02, 0.02, footer_text, fontsize=10, style='italic', 
             bbox=dict(boxstyle="round,pad=0.3", facecolor="lightgray", alpha=0.5))
    
    plt.tight_layout()
    plt.savefig(output_file, dpi=300, bbox_inches='tight', facecolor='white')
    print(f"Performance dashboard saved as: {output_file}")
    
    return fig

def print_detailed_analysis(df):
    """Print detailed text analysis of the benchmark results."""
    print("\n" + "="*70)
    print("RISC-V VECTOR LMUL PERFORMANCE ANALYSIS")
    print("="*70)
    
    operations = df['Operation'].unique()
    lmul_values = ['m1', 'm2', 'm4', 'm8']
    
    for op in operations:
        print(f"\n{op.upper()}:")
        print("-" * len(op) + "--")
        
        op_data = df[df['Operation'] == op]
        
        # Find best and worst LMUL
        best_idx = op_data['Speedup_vs_m1'].idxmax()
        worst_idx = op_data['Speedup_vs_m1'].idxmin()
        
        best_lmul = op_data.loc[best_idx, 'LMUL']
        best_speedup = op_data.loc[best_idx, 'Speedup_vs_m1']
        best_time = op_data.loc[best_idx, 'Time_ms']
        
        worst_lmul = op_data.loc[worst_idx, 'LMUL']
        worst_speedup = op_data.loc[worst_idx, 'Speedup_vs_m1']
        worst_time = op_data.loc[worst_idx, 'Time_ms']
        
        print(f"  Best Performance: {best_lmul} ({best_speedup:.2f}x speedup, {best_time:.2f}ms)")
        print(f"  Worst Performance: {worst_lmul} ({worst_speedup:.2f}x speedup, {worst_time:.2f}ms)")
        print(f"  Performance Range: {best_speedup/worst_speedup:.2f}x difference")
        
        # Calculate efficiencies
        print(f"  LMUL Efficiencies:")
        for lmul in lmul_values:
            lmul_data = op_data[op_data['LMUL'] == lmul]
            if len(lmul_data) > 0:
                speedup = lmul_data['Speedup_vs_m1'].values[0]
                lmul_num = int(lmul[1:])
                theoretical = lmul_num
                efficiency = (speedup / theoretical) * 100 if lmul_num > 1 else 100
                print(f"    {lmul}: {efficiency:.1f}% (actual: {speedup:.2f}x, theoretical: {theoretical:.1f}x)")
    
    # Overall analysis
    print("\n" + "="*70)
    print("OVERALL ANALYSIS")
    print("="*70)
    
    # Find the most scalable operation
    scalability_scores = {}
    for op in operations:
        op_data = df[df['Operation'] == op]
        m8_speedup = op_data[op_data['LMUL'] == 'm8']['Speedup_vs_m1'].values[0]
        scalability_scores[op] = m8_speedup
    
    most_scalable = max(scalability_scores.keys(), key=lambda x: scalability_scores[x])
    least_scalable = min(scalability_scores.keys(), key=lambda x: scalability_scores[x])
    
    print(f"Most LMUL-scalable operation: {most_scalable} ({scalability_scores[most_scalable]:.2f}x with m8)")
    print(f"Least LMUL-scalable operation: {least_scalable} ({scalability_scores[least_scalable]:.2f}x with m8)")
    
    # LMUL recommendations
    print(f"\nLMUL RECOMMENDATIONS:")
    for op in operations:
        op_data = df[df['Operation'] == op]
        best_lmul = op_data.loc[op_data['Speedup_vs_m1'].idxmax(), 'LMUL']
        print(f"  {op}: Use {best_lmul} for optimal performance")

def main():
    parser = argparse.ArgumentParser(description='Generate RISC-V Vector LMUL performance analysis')
    parser.add_argument('--csv', default='lmul_benchmark_results.csv', 
                       help='CSV file with benchmark results')
    parser.add_argument('--output', default='lmul_performance_dashboard.png',
                       help='Output image file')
    parser.add_argument('--no-display', action='store_true',
                       help='Do not display the plot')
    
    args = parser.parse_args()
    
    # Load data
    df = load_benchmark_data(args.csv)
    
    # Generate visualizations
    fig = create_performance_dashboard(df, args.output)
    
    # Print detailed analysis
    print_detailed_analysis(df)
    
    # Display plot if requested
    if not args.no_display:
        plt.show()

if __name__ == "__main__":
    main() 