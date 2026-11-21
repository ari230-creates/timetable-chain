import React from 'react';
import { useApp, TabType } from '../context/AppContext';
import { Cpu, Edit3, Link2, Database } from 'lucide-react';

interface NavItem {
  id: TabType;
  label: string;
  sub: string;
  icon: React.ComponentType<{ className?: string }>;
}

const NAV_ITEMS: NavItem[] = [
  {
    id: 'generator',
    label: 'Generator',
    sub: 'CSP Backtracking Solver',
    icon: Cpu,
  },
  {
    id: 'manual',
    label: 'Manual Builder',
    sub: 'Interactive Schedule Editor',
    icon: Edit3,
  },
  {
    id: 'blockchain',
    label: 'Blockchain',
    sub: 'PoW & 3-Layer Verification',
    icon: Link2,
  },
  {
    id: 'curriculum',
    label: 'Curriculum Data',
    sub: 'Dataset Management & Parser',
    icon: Database,
  },
];

export const Sidebar: React.FC = () => {
  const { tab, setTab } = useApp();

  return (
    <>
      {/* Desktop & Tablet Sidebar */}
      <aside className="w-64 bg-white dark:bg-[#181C2E] border-r border-[#E4E7F0] dark:border-[#2A2F47] flex flex-col justify-between p-4 shrink-0 hidden md:flex transition-colors">
        <div className="space-y-1.5">
          <p className="px-3 py-1.5 text-[11px] font-bold uppercase tracking-wider text-[#9AA0C3]">
            Navigation
          </p>
          {NAV_ITEMS.map((item) => {
            const active = tab === item.id;
            const Icon = item.icon;
            return (
              <button
                key={item.id}
                onClick={() => setTab(item.id)}
                className={`w-full flex items-center space-x-3 px-3.5 py-3 rounded-xl text-left transition-all ${
                  active
                    ? 'bg-[#EEF0FD] dark:bg-[#2C204B] text-[#5B5BD6] dark:text-[#8AB4F8] font-semibold shadow-xs'
                    : 'text-[#6B7194] dark:text-[#9AA0C3] hover:bg-[#F0F2FA] dark:hover:bg-[#20253B] hover:text-[#1B1F3B] dark:hover:text-[#E8EAF6]'
                }`}
              >
                <div
                  className={`p-2 rounded-lg ${
                    active
                      ? 'bg-[#5B5BD6] text-white shadow-xs'
                      : 'bg-[#F0F2FA] dark:bg-[#232840] text-[#6B7194] dark:text-[#9AA0C3]'
                  }`}
                >
                  <Icon className="w-4 h-4" />
                </div>
                <div>
                  <div className="text-sm font-semibold leading-tight">{item.label}</div>
                  <div className="text-[11px] opacity-75 font-normal leading-tight">{item.sub}</div>
                </div>
              </button>
            );
          })}
        </div>

        {/* Project Info Card in Sidebar */}
        <div className="p-3.5 rounded-xl bg-[#F6F7FB] dark:bg-[#20253B] border border-[#E4E7F0] dark:border-[#2A2F47] text-xs space-y-1.5">
          <div className="font-semibold text-[#1B1F3B] dark:text-[#E8EAF6] flex items-center justify-between">
            <span>Proof-of-Work</span>
            <span className="font-mono text-[#5B5BD6] dark:text-[#8AB4F8]">Diff: 3 (000...)</span>
          </div>
          <p className="text-[11px] text-[#6B7194] dark:text-[#9AA0C3]">
            Cryptographic SHA-256 blocks with full 3-layer verification.
          </p>
        </div>
      </aside>

      {/* Mobile Bottom Navigation Bar */}
      <nav className="md:hidden fixed bottom-0 left-0 right-0 h-16 bg-white dark:bg-[#181C2E] border-t border-[#E4E7F0] dark:border-[#2A2F47] flex items-center justify-around z-30 px-2 transition-colors">
        {NAV_ITEMS.map((item) => {
          const active = tab === item.id;
          const Icon = item.icon;
          return (
            <button
              key={item.id}
              onClick={() => setTab(item.id)}
              className={`flex flex-col items-center justify-center flex-1 py-1.5 text-xs transition-colors ${
                active
                  ? 'text-[#5B5BD6] dark:text-[#8AB4F8] font-bold'
                  : 'text-[#6B7194] dark:text-[#9AA0C3]'
              }`}
            >
              <Icon className="w-5 h-5 mb-0.5" />
              <span className="text-[10px] leading-tight">{item.label}</span>
            </button>
          );
        })}
      </nav>
    </>
  );
};
