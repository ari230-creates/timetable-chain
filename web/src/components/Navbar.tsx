import React from 'react';
import { useApp } from '../context/AppContext';
import { Link2, Sun, Moon, Database, CheckCircle2, AlertTriangle, ShieldCheck } from 'lucide-react';

export const Navbar: React.FC = () => {
  const {
    currentDatasetName,
    blocks,
    verificationReport,
    theme,
    toggleTheme,
    setTab
  } = useApp();

  const isVerified = verificationReport ? verificationReport.allValid : false;
  const hasReport = verificationReport !== null;

  return (
    <header className="h-16 bg-white dark:bg-[#181C2E] border-b border-[#E4E7F0] dark:border-[#2A2F47] px-4 sm:px-6 flex items-center justify-between sticky top-0 z-30 transition-colors">
      {/* Brand */}
      <div className="flex items-center space-x-3">
        <div className="w-10 h-10 rounded-xl bg-[#5B5BD6] flex items-center justify-center text-white shadow-md shadow-[#5B5BD6]/20">
          <Link2 className="w-5 h-5 transform -rotate-45" />
        </div>
        <div>
          <div className="flex items-center space-x-2">
            <span className="font-extrabold text-lg sm:text-xl tracking-tight text-[#1B1F3B] dark:text-[#E8EAF6]">
              TIMETABLE CHAIN
            </span>
            <span className="hidden md:inline-block text-[11px] font-semibold uppercase px-2 py-0.5 rounded-full bg-[#EEF0FD] dark:bg-[#2C204B] text-[#5B5BD6] dark:text-[#8AB4F8] border border-[#D6D8F8] dark:border-[#3B3463]">
              C++17 Core
            </span>
          </div>
          <p className="hidden sm:block text-xs text-[#6B7194] dark:text-[#9AA0C3]">
            Intelligent Timetable Generation & Blockchain Ledger System
          </p>
        </div>
      </div>

      {/* Right Controls */}
      <div className="flex items-center space-x-2 sm:space-x-3">
        {/* Dataset Badge */}
        <button
          onClick={() => setTab('curriculum')}
          title="Click to manage curriculum datasets"
          className="flex items-center space-x-1.5 px-2.5 py-1.5 text-xs font-semibold rounded-lg bg-[#F0F2FA] dark:bg-[#232840] text-[#1B1F3B] dark:text-[#E8EAF6] hover:bg-[#E4E7F0] dark:hover:bg-[#2A3150] transition-colors border border-[#E4E7F0] dark:border-[#2A2F47]"
        >
          <Database className="w-3.5 h-3.5 text-[#5B5BD6]" />
          <span className="max-w-[120px] truncate">{currentDatasetName}</span>
        </button>

        {/* Chain Status Badge */}
        <button
          onClick={() => setTab('blockchain')}
          title="Click to view Blockchain ledger"
          className={`flex items-center space-x-1.5 px-2.5 py-1.5 text-xs font-semibold rounded-lg transition-colors border ${
            hasReport
              ? isVerified
                ? 'bg-emerald-50 dark:bg-emerald-950/40 text-emerald-700 dark:text-emerald-300 border-emerald-200 dark:border-emerald-800'
                : 'bg-rose-50 dark:bg-rose-950/40 text-rose-700 dark:text-rose-300 border-rose-200 dark:border-rose-800'
              : 'bg-[#EEF0FD] dark:bg-[#20253B] text-[#5B5BD6] dark:text-[#8AB4F8] border-[#D6D8F8] dark:border-[#2C3352]'
          }`}
        >
          {hasReport ? (
            isVerified ? (
              <CheckCircle2 className="w-3.5 h-3.5 text-emerald-600 dark:text-emerald-400" />
            ) : (
              <AlertTriangle className="w-3.5 h-3.5 text-rose-600 dark:text-rose-400" />
            )
          ) : (
            <ShieldCheck className="w-3.5 h-3.5 text-[#5B5BD6]" />
          )}
          <span>
            {blocks.length} {blocks.length === 1 ? 'Block' : 'Blocks'}
          </span>
          <span className="hidden sm:inline font-mono text-[10px] opacity-75">
            {hasReport ? (isVerified ? 'VERIFIED' : 'INVALID') : 'PENDING'}
          </span>
        </button>

        {/* Theme Toggle */}
        <button
          onClick={toggleTheme}
          aria-label="Toggle theme"
          className="p-2 rounded-lg bg-[#F0F2FA] dark:bg-[#232840] text-[#6B7194] dark:text-[#9AA0C3] hover:text-[#1B1F3B] dark:hover:text-[#E8EAF6] transition-colors border border-[#E4E7F0] dark:border-[#2A2F47]"
        >
          {theme === 'light' ? <Moon className="w-4 h-4" /> : <Sun className="w-4 h-4 text-amber-400" />}
        </button>
      </div>
    </header>
  );
};
