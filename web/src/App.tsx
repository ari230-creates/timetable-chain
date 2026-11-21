import React from 'react';
import { useApp } from './context/AppContext';
import { Navbar } from './components/Navbar';
import { Sidebar } from './components/Sidebar';
import { GeneratorPage } from './pages/GeneratorPage';
import { ManualBuilderPage } from './pages/ManualBuilderPage';
import { BlockchainPage } from './pages/BlockchainPage';
import { CurriculumDataPage } from './pages/CurriculumDataPage';
import { CheckCircle2, AlertCircle, AlertTriangle, Info, X } from 'lucide-react';

const MainContent: React.FC = () => {
  const { tab, toasts, dismissToast } = useApp();

  return (
    <div className="flex flex-col min-h-screen">
      <Navbar />

      <div className="flex flex-1 overflow-hidden">
        <Sidebar />

        {/* Main Workspace Area */}
        <main className="flex-1 overflow-y-auto p-4 sm:p-6 lg:p-8 pb-20 md:pb-8">
          <div className="max-w-7xl mx-auto">
            {tab === 'generator' && <GeneratorPage />}
            {tab === 'manual' && <ManualBuilderPage />}
            {tab === 'blockchain' && <BlockchainPage />}
            {tab === 'curriculum' && <CurriculumDataPage />}
          </div>
        </main>
      </div>

      {/* Toast Notification Container */}
      <div className="fixed bottom-20 md:bottom-6 right-4 sm:right-6 z-50 flex flex-col space-y-2 pointer-events-none max-w-sm w-full">
        {toasts.map((toast) => {
          let bg = 'bg-white dark:bg-[#181C2E] border-[#E4E7F0] dark:border-[#2A2F47] text-[#1B1F3B] dark:text-[#E8EAF6]';
          let icon = <Info className="w-4 h-4 text-[#5B5BD6] shrink-0" />;

          if (toast.type === 'success') {
            bg = 'bg-emerald-50 dark:bg-emerald-950/80 border-emerald-200 dark:border-emerald-800 text-emerald-900 dark:text-emerald-100';
            icon = <CheckCircle2 className="w-4 h-4 text-emerald-600 dark:text-emerald-400 shrink-0" />;
          } else if (toast.type === 'error') {
            bg = 'bg-rose-50 dark:bg-rose-950/80 border-rose-200 dark:border-rose-800 text-rose-900 dark:text-rose-100';
            icon = <AlertCircle className="w-4 h-4 text-rose-600 dark:text-rose-400 shrink-0" />;
          } else if (toast.type === 'warning') {
            bg = 'bg-amber-50 dark:bg-amber-950/80 border-amber-200 dark:border-amber-800 text-amber-900 dark:text-amber-100';
            icon = <AlertTriangle className="w-4 h-4 text-amber-600 dark:text-amber-400 shrink-0" />;
          }

          return (
            <div
              key={toast.id}
              className={`pointer-events-auto p-3.5 rounded-2xl border shadow-lg flex items-center justify-between space-x-3 transition-all animate-in fade-in slide-in-from-bottom-2 ${bg}`}
            >
              <div className="flex items-center space-x-2.5">
                {icon}
                <span className="text-xs font-semibold leading-tight">{toast.text}</span>
              </div>
              <button
                onClick={() => dismissToast(toast.id)}
                className="p-1 opacity-60 hover:opacity-100 rounded-md transition-opacity"
              >
                <X className="w-3.5 h-3.5" />
              </button>
            </div>
          );
        })}
      </div>
    </div>
  );
};

export const App: React.FC = () => {
  return <MainContent />;
};

export default App;
