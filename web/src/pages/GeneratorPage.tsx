import React, { useState } from 'react';
import { useApp } from '../context/AppContext';
import { ScheduleGrid } from '../components/ScheduleGrid';
import { ComparisonModal } from '../components/ComparisonModal';
import { Play, Link2, BarChart2, Loader2, Sparkles, CheckCircle, Clock } from 'lucide-react';

export const GeneratorPage: React.FC = () => {
  const {
    dataset,
    datasetError,
    generatorMode,
    setGeneratorMode,
    isGenerating,
    runGenerate,
    generatedTimetable,
    generatorStats,
    setTab,
  } = useApp();

  const [compareModalOpen, setCompareModalOpen] = useState(false);

  if (!dataset) {
    return (
      <div className="p-8 text-center space-y-4">
        <div className="p-4 rounded-2xl bg-amber-50 dark:bg-amber-950/40 text-amber-800 dark:text-amber-200 border border-amber-200 dark:border-amber-800 max-w-md mx-auto text-sm">
          {datasetError || 'No curriculum dataset loaded.'}
        </div>
        <button
          onClick={() => setTab('curriculum')}
          className="px-4 py-2 rounded-xl bg-[#5B5BD6] text-white font-semibold text-sm shadow-md"
        >
          Go to Curriculum Data
        </button>
      </div>
    );
  }

  return (
    <div className="space-y-6">
      {/* Top Controls Card */}
      <div className="bg-white dark:bg-[#181C2E] border border-[#E4E7F0] dark:border-[#2A2F47] rounded-2xl p-4 sm:p-5 shadow-xs transition-colors">
        <div className="flex flex-col lg:flex-row lg:items-center justify-between gap-4">
          <div className="flex flex-wrap items-center gap-3">
            <label className="text-sm font-bold text-[#1B1F3B] dark:text-[#E8EAF6] shrink-0">
              Search Strategy:
            </label>
            <select
              value={generatorMode}
              onChange={(e) => setGeneratorMode(e.target.value as 'Plain' | 'MRV' | 'Best')}
              disabled={isGenerating}
              className="px-3.5 py-2 rounded-xl border border-[#CBD5E1] dark:border-[#334155] bg-[#F8F9FD] dark:bg-[#20253B] text-sm font-semibold text-[#1B1F3B] dark:text-[#E8EAF6] focus:outline-none focus:ring-2 focus:ring-[#5B5BD6]"
            >
              <option value="MRV">MRV (Most Constrained First) — Recommended</option>
              <option value="Best">Best (Explore 50 Solutions, Min Penalty)</option>
              <option value="Plain">Plain (Data File Order)</option>
            </select>
          </div>

          <div className="flex flex-wrap items-center gap-2.5">
            <button
              onClick={() => runGenerate(false)}
              disabled={isGenerating}
              className="flex items-center space-x-2 px-4 py-2.5 rounded-xl bg-[#5B5BD6] hover:bg-[#4D4DBF] text-white font-semibold text-sm shadow-xs transition-all disabled:opacity-50 cursor-pointer"
            >
              {isGenerating ? <Loader2 className="w-4 h-4 animate-spin" /> : <Play className="w-4 h-4 fill-white" />}
              <span>{isGenerating ? 'Generating...' : 'Generate'}</span>
            </button>

            <button
              onClick={() => runGenerate(true)}
              disabled={isGenerating}
              className="flex items-center space-x-2 px-4 py-2.5 rounded-xl bg-purple-700 hover:bg-purple-800 text-white font-semibold text-sm shadow-xs transition-all disabled:opacity-50 cursor-pointer"
            >
              {isGenerating ? <Loader2 className="w-4 h-4 animate-spin" /> : <Link2 className="w-4 h-4" />}
              <span>Generate & Mine Block</span>
            </button>

            <button
              onClick={() => setCompareModalOpen(true)}
              className="flex items-center space-x-2 px-4 py-2.5 rounded-xl bg-[#F0F2FA] dark:bg-[#20253B] hover:bg-[#E4E7F0] dark:hover:bg-[#2A3150] text-[#1B1F3B] dark:text-[#E8EAF6] font-semibold text-sm border border-[#E4E7F0] dark:border-[#2A2F47] transition-all cursor-pointer"
            >
              <BarChart2 className="w-4 h-4 text-[#5B5BD6]" />
              <span>Compare Strategies</span>
            </button>
          </div>
        </div>
      </div>

      {/* Execution Stats Cards */}
      {generatorStats && (
        <div className="grid grid-cols-2 sm:grid-cols-3 lg:grid-cols-5 gap-3.5">
          <div className="bg-white dark:bg-[#181C2E] border border-[#E4E7F0] dark:border-[#2A2F47] rounded-2xl p-4 shadow-xs">
            <span className="text-[11px] font-bold uppercase tracking-wider text-[#9AA0C3] block">
              Placements
            </span>
            <span className="text-2xl font-black text-[#1B1F3B] dark:text-[#E8EAF6] mt-0.5 block">
              {generatorStats.placements}
            </span>
            <span className="text-[11px] text-[#6B7194] dark:text-[#9AA0C3]">
              Decisions evaluated
            </span>
          </div>

          <div className="bg-white dark:bg-[#181C2E] border border-[#E4E7F0] dark:border-[#2A2F47] rounded-2xl p-4 shadow-xs">
            <span className="text-[11px] font-bold uppercase tracking-wider text-[#9AA0C3] block">
              Backtracks
            </span>
            <span className="text-2xl font-black text-rose-600 dark:text-rose-400 mt-0.5 block">
              {generatorStats.backtracks}
            </span>
            <span className="text-[11px] text-[#6B7194] dark:text-[#9AA0C3]">
              Dead ends pruned
            </span>
          </div>

          <div className="bg-white dark:bg-[#181C2E] border border-[#E4E7F0] dark:border-[#2A2F47] rounded-2xl p-4 shadow-xs">
            <span className="text-[11px] font-bold uppercase tracking-wider text-[#9AA0C3] block">
              Penalty Score
            </span>
            <span className="text-2xl font-black text-[#5B5BD6] dark:text-[#8AB4F8] mt-0.5 block">
              {generatorStats.score}
            </span>
            <span className="text-[11px] text-[#6B7194] dark:text-[#9AA0C3]">
              Lower is better
            </span>
          </div>

          <div className="bg-white dark:bg-[#181C2E] border border-[#E4E7F0] dark:border-[#2A2F47] rounded-2xl p-4 shadow-xs">
            <span className="text-[11px] font-bold uppercase tracking-wider text-[#9AA0C3] block">
              Solutions Found
            </span>
            <span className="text-2xl font-black text-emerald-600 dark:text-emerald-400 mt-0.5 block">
              {generatorStats.solutionsFound}
            </span>
            <span className="text-[11px] text-[#6B7194] dark:text-[#9AA0C3]">
              Feasible schedules
            </span>
          </div>

          <div className="bg-white dark:bg-[#181C2E] border border-[#E4E7F0] dark:border-[#2A2F47] rounded-2xl p-4 shadow-xs col-span-2 sm:col-span-1">
            <span className="text-[11px] font-bold uppercase tracking-wider text-[#9AA0C3] block">
              Execution Time
            </span>
            <span className="text-2xl font-black text-[#1B1F3B] dark:text-[#E8EAF6] mt-0.5 block">
              {generatorStats.durationMs.toFixed(3)} <span className="text-sm font-normal">ms</span>
            </span>
            <span className="text-[11px] text-[#6B7194] dark:text-[#9AA0C3]">
              Native C++17 runtime
            </span>
          </div>
        </div>
      )}

      {/* Timetable Grid View */}
      {generatedTimetable.length > 0 ? (
        <ScheduleGrid
          entries={generatedTimetable}
          dataset={dataset}
          title={`Generated Schedule (${generatorMode})`}
        />
      ) : (
        <div className="bg-white dark:bg-[#181C2E] border border-dashed border-[#CBD5E1] dark:border-[#334155] rounded-3xl p-12 text-center space-y-3">
          <div className="w-12 h-12 rounded-2xl bg-[#EEF0FD] dark:bg-[#2C204B] text-[#5B5BD6] dark:text-[#8AB4F8] flex items-center justify-center mx-auto">
            <Sparkles className="w-6 h-6" />
          </div>
          <h3 className="text-base font-bold text-[#1B1F3B] dark:text-[#E8EAF6]">
            No Schedule Generated Yet
          </h3>
          <p className="text-xs text-[#6B7194] dark:text-[#9AA0C3] max-w-md mx-auto">
            Select a search strategy (Plain, MRV, or Best) and click <b>Generate</b> to solve the Constraint Satisfaction Problem using the native C++17 core.
          </p>
          <div className="pt-2">
            <button
              onClick={() => runGenerate(false)}
              className="px-4 py-2 rounded-xl bg-[#5B5BD6] hover:bg-[#4D4DBF] text-white font-semibold text-sm shadow-sm cursor-pointer"
            >
              Generate with MRV Strategy
            </button>
          </div>
        </div>
      )}

      {/* Comparison Modal */}
      <ComparisonModal
        isOpen={compareModalOpen}
        onClose={() => setCompareModalOpen(false)}
      />
    </div>
  );
};
