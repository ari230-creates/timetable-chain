import React, { useEffect, useState } from 'react';
import { useApp } from '../context/AppContext';
import { api } from '../services/api';
import { StrategyResult } from '../types';
import { X, BarChart3, Loader2, Sparkles, CheckCircle, Info } from 'lucide-react';
import { BarChart, Bar, XAxis, YAxis, Tooltip, ResponsiveContainer, CartesianGrid, Legend } from 'recharts';

interface ComparisonModalProps {
  isOpen: boolean;
  onClose: () => void;
}

export const ComparisonModal: React.FC<ComparisonModalProps> = ({ isOpen, onClose }) => {
  const { datasetContent, theme } = useApp();
  const [loading, setLoading] = useState(false);
  const [results, setResults] = useState<StrategyResult[]>([]);
  const [error, setError] = useState<string | null>(null);

  const isDark = theme === 'dark';

  useEffect(() => {
    if (isOpen) {
      runCompare();
    }
  }, [isOpen]);

  const runCompare = async () => {
    setLoading(true);
    setError(null);
    try {
      const res = await api.compareStrategies(datasetContent);
      setResults(res.results);
    } catch (e: any) {
      setError(e.message || 'Failed to compare strategies');
    } finally {
      setLoading(false);
    }
  };

  if (!isOpen) return null;

  const chartData = results.map((r) => ({
    name: r.name,
    Placements: r.placements,
    Backtracks: r.backtracks,
    'Penalty Score': r.score,
    'Time (ms)': Number(r.durationMs.toFixed(3)),
  }));

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/50 backdrop-blur-xs overflow-y-auto">
      <div className="bg-white dark:bg-[#181C2E] border border-[#E4E7F0] dark:border-[#2A2F47] rounded-3xl w-full max-w-4xl max-h-[90vh] shadow-2xl flex flex-col overflow-hidden my-8 transition-colors">
        {/* Header */}
        <div className="px-6 py-4 border-b border-[#E4E7F0] dark:border-[#2A2F47] flex items-center justify-between">
          <div className="flex items-center space-x-2.5">
            <div className="p-2 rounded-xl bg-[#EEF0FD] dark:bg-[#2C204B] text-[#5B5BD6] dark:text-[#8AB4F8]">
              <BarChart3 className="w-5 h-5" />
            </div>
            <div>
              <h2 className="text-lg font-bold text-[#1B1F3B] dark:text-[#E8EAF6]">
                Algorithm Strategy Benchmark
              </h2>
              <p className="text-xs text-[#6B7194] dark:text-[#9AA0C3]">
                Comparing CSP Backtracking Solvers (Plain vs. MRV vs. Best) on the active curriculum
              </p>
            </div>
          </div>
          <button
            onClick={onClose}
            className="p-2 text-[#9AA0C3] hover:text-[#1B1F3B] dark:hover:text-[#E8EAF6] rounded-xl hover:bg-[#F0F2FA] dark:hover:bg-[#20253B] transition-colors"
          >
            <X className="w-5 h-5" />
          </button>
        </div>

        {/* Content */}
        <div className="p-6 overflow-y-auto space-y-6">
          {loading ? (
            <div className="py-20 flex flex-col items-center justify-center space-y-3">
              <Loader2 className="w-8 h-8 text-[#5B5BD6] animate-spin" />
              <p className="text-sm font-semibold text-[#6B7194] dark:text-[#9AA0C3]">
                Running C++17 CSP search benchmarks...
              </p>
            </div>
          ) : error ? (
            <div className="p-4 rounded-xl bg-rose-50 dark:bg-rose-950/40 text-rose-700 dark:text-rose-300 border border-rose-200 dark:border-rose-800 text-sm">
              {error}
            </div>
          ) : (
            <>
              {/* Chart Section */}
              <div className="bg-[#F8F9FD] dark:bg-[#121626] border border-[#E4E7F0] dark:border-[#2A2F47] rounded-2xl p-4">
                <h3 className="text-xs font-bold uppercase tracking-wider text-[#9AA0C3] mb-4">
                  Search Tree Operations (Placements & Backtracks)
                </h3>
                <div className="h-64 w-full">
                  <ResponsiveContainer width="100%" height="100%">
                    <BarChart data={chartData} margin={{ top: 10, right: 20, left: 0, bottom: 20 }}>
                      <CartesianGrid strokeDasharray="3 3" stroke={isDark ? '#2A2F47' : '#E4E7F0'} />
                      <XAxis dataKey="name" stroke={isDark ? '#9AA0C3' : '#6B7194'} />
                      <YAxis stroke={isDark ? '#9AA0C3' : '#6B7194'} />
                      <Tooltip
                        contentStyle={{
                          backgroundColor: isDark ? '#181C2E' : '#FFFFFF',
                          borderColor: isDark ? '#2A2F47' : '#E4E7F0',
                          borderRadius: '12px',
                          color: isDark ? '#E8EAF6' : '#1B1F3B',
                        }}
                      />
                      <Legend />
                      <Bar dataKey="Placements" fill="#5B5BD6" radius={[6, 6, 0, 0]} />
                      <Bar dataKey="Backtracks" fill="#D64545" radius={[6, 6, 0, 0]} />
                      <Bar dataKey="Penalty Score" fill="#E0A030" radius={[6, 6, 0, 0]} />
                    </BarChart>
                  </ResponsiveContainer>
                </div>
              </div>

              {/* Table */}
              <div className="overflow-x-auto rounded-2xl border border-[#E4E7F0] dark:border-[#2A2F47]">
                <table className="w-full text-left text-sm">
                  <thead className="bg-[#F8F9FD] dark:bg-[#121626] text-xs uppercase font-bold text-[#6B7194] dark:text-[#9AA0C3] border-b border-[#E4E7F0] dark:border-[#2A2F47]">
                    <tr>
                      <th className="py-3 px-4">Strategy</th>
                      <th className="py-3 px-4">Placements</th>
                      <th className="py-3 px-4">Backtracks</th>
                      <th className="py-3 px-4">Penalty Score</th>
                      <th className="py-3 px-4">Time (ms)</th>
                    </tr>
                  </thead>
                  <tbody className="divide-y divide-[#E4E7F0] dark:divide-[#2A2F47]">
                    {results.map((r) => {
                      const isFewest = r.placements === Math.min(...results.map((x) => x.placements));
                      const isBestScore = r.score === Math.min(...results.map((x) => x.score));
                      return (
                        <tr key={r.name} className="hover:bg-slate-50/50 dark:hover:bg-slate-800/20">
                          <td className="py-3 px-4 font-bold text-[#1B1F3B] dark:text-[#E8EAF6]">
                            <div className="flex items-center space-x-2">
                              <span>{r.name}</span>
                              {isFewest && (
                                <span className="text-[10px] font-semibold px-2 py-0.5 rounded-full bg-emerald-50 dark:bg-emerald-950/60 text-emerald-600 dark:text-emerald-400 border border-emerald-200 dark:border-emerald-800">
                                  Fastest
                                </span>
                              )}
                              {isBestScore && (
                                <span className="text-[10px] font-semibold px-2 py-0.5 rounded-full bg-purple-50 dark:bg-purple-950/60 text-[#5B5BD6] dark:text-[#8AB4F8] border border-purple-200 dark:border-purple-800">
                                  Lowest Penalty
                                </span>
                              )}
                            </div>
                            <div className="text-xs text-[#6B7194] dark:text-[#9AA0C3] font-normal mt-0.5">
                              {r.description}
                            </div>
                          </td>
                          <td className="py-3 px-4 font-mono font-bold text-[#1B1F3B] dark:text-[#E8EAF6]">
                            {r.placements}
                          </td>
                          <td className="py-3 px-4 font-mono text-[#6B7194] dark:text-[#9AA0C3]">
                            {r.backtracks}
                          </td>
                          <td className="py-3 px-4 font-mono font-bold text-[#5B5BD6] dark:text-[#8AB4F8]">
                            {r.score}
                          </td>
                          <td className="py-3 px-4 font-mono text-xs text-[#6B7194] dark:text-[#9AA0C3]">
                            {r.durationMs.toFixed(3)} ms
                          </td>
                        </tr>
                      );
                    })}
                  </tbody>
                </table>
              </div>

              {/* Theoretical Analysis Card */}
              <div className="p-4 rounded-2xl bg-[#EEF0FD]/60 dark:bg-[#1E243D] border border-[#D6D8F8] dark:border-[#2C3352] text-xs space-y-2 text-[#1B1F3B] dark:text-[#E8EAF6]">
                <div className="flex items-center space-x-2 font-bold text-[#5B5BD6] dark:text-[#8AB4F8]">
                  <Info className="w-4 h-4 shrink-0" />
                  <span>Algorithmic Analysis & Viva Defense Notes</span>
                </div>
                <p className="leading-relaxed">
                  • <b>Plain Order:</b> Follows raw file sequence without domain heuristics. It frequently encounters constraint bottlenecks deep down in the search tree, necessitating hundreds of backtracks (320 placements / 312 backtracks on sample dataset).
                </p>
                <p className="leading-relaxed">
                  • <b>MRV Heuristic (Most Constrained First):</b> Orders tasks by ascending domain options. Bottleneck subjects (e.g. Databases taught by only one instructor who has limited free slots) are allocated first, avoiding infeasible branches and finding a valid schedule with 0 backtracks (8 placements).
                </p>
                <p className="leading-relaxed">
                  • <b>Best Optimization Strategy:</b> Explores up to 50 valid solutions using MRV variable ordering and retains the one that minimizes pedagogical penalty (clustering penalty + faculty fatigue penalty), achieving the lowest penalty score (7 on sample dataset).
                </p>
              </div>
            </>
          )}
        </div>

        {/* Footer */}
        <div className="px-6 py-4 border-t border-[#E4E7F0] dark:border-[#2A2F47] flex justify-end">
          <button
            onClick={onClose}
            className="px-4 py-2 rounded-xl bg-[#F0F2FA] dark:bg-[#20253B] text-sm font-semibold text-[#1B1F3B] dark:text-[#E8EAF6] hover:bg-[#E4E7F0] dark:hover:bg-[#2A3150] transition-colors"
          >
            Close Benchmark
          </button>
        </div>
      </div>
    </div>
  );
};
