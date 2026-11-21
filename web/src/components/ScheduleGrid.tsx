import React from 'react';
import { Dataset, TimetableEntry } from '../types';
import { getSubjectColors, exportTimetableToCSV } from '../utils/theme';
import { useApp } from '../context/AppContext';
import { Download, Printer, Plus, X } from 'lucide-react';

interface ScheduleGridProps {
  entries: TimetableEntry[];
  dataset: Dataset;
  interactive?: boolean;
  onSlotClick?: (slotId: number) => void;
  onSlotRemove?: (slotId: number) => void;
  title?: string;
}

export const ScheduleGrid: React.FC<ScheduleGridProps> = ({
  entries,
  dataset,
  interactive = false,
  onSlotClick,
  onSlotRemove,
  title,
}) => {
  const { theme, showToast } = useApp();
  const isDark = theme === 'dark';

  // Group slots by Day and Period
  const standardDayOrder = ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'];
  const uniqueDays = Array.from(new Set(dataset.slots.map((s) => s.day))).sort(
    (a, b) => standardDayOrder.indexOf(a) - standardDayOrder.indexOf(b)
  );

  const maxPeriod = Math.max(1, ...dataset.slots.map((s) => s.period));
  const periods = Array.from({ length: maxPeriod }, (_, i) => i + 1);

  // Map slot lookup: key = `${day}_${period}`
  const slotMap = new Map<string, number>();
  dataset.slots.forEach((s) => {
    slotMap.set(`${s.day}_${s.period}`, s.id);
  });

  // Entry lookup by slotId
  const entryBySlot = new Map<number, TimetableEntry>();
  entries.forEach((e) => {
    entryBySlot.set(e.slotId, e);
  });

  const totalPeriodsAssigned = entries.length;
  const totalPeriodsNeeded = dataset.totalPeriodsNeeded;

  const handlePrint = () => {
    window.print();
  };

  const handleExportCSV = () => {
    exportTimetableToCSV(entries, dataset, `${title ? title.replace(/\s+/g, '_').toLowerCase() : 'timetable'}.csv`);
    showToast('Exported timetable to CSV', 'success');
  };

  return (
    <div className="bg-white dark:bg-[#181C2E] border border-[#E4E7F0] dark:border-[#2A2F47] rounded-2xl p-4 sm:p-5 shadow-sm space-y-4 transition-colors">
      {/* Top Header */}
      <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-3 pb-2 border-b border-[#E4E7F0] dark:border-[#2A2F47] no-print">
        <div className="flex items-center space-x-3">
          <span className="text-sm font-bold text-[#5B5BD6] dark:text-[#8AB4F8]">
            {totalPeriodsAssigned} of {totalPeriodsNeeded} required periods assigned
          </span>
          {totalPeriodsAssigned === totalPeriodsNeeded && (
            <span className="text-[11px] font-bold uppercase tracking-wider px-2 py-0.5 rounded-full bg-emerald-50 dark:bg-emerald-950/60 text-emerald-600 dark:text-emerald-400 border border-emerald-200 dark:border-emerald-800">
              Complete
            </span>
          )}
        </div>

        <div className="flex items-center space-x-2">
          <button
            onClick={handleExportCSV}
            className="flex items-center space-x-1.5 px-3 py-1.5 text-xs font-semibold rounded-lg bg-[#F0F2FA] dark:bg-[#20253B] text-[#1B1F3B] dark:text-[#E8EAF6] hover:bg-[#E4E7F0] dark:hover:bg-[#2A3150] transition-colors border border-[#E4E7F0] dark:border-[#2A2F47]"
          >
            <Download className="w-3.5 h-3.5" />
            <span>Export CSV</span>
          </button>
          <button
            onClick={handlePrint}
            className="flex items-center space-x-1.5 px-3 py-1.5 text-xs font-semibold rounded-lg bg-[#F0F2FA] dark:bg-[#20253B] text-[#1B1F3B] dark:text-[#E8EAF6] hover:bg-[#E4E7F0] dark:hover:bg-[#2A3150] transition-colors border border-[#E4E7F0] dark:border-[#2A2F47]"
          >
            <Printer className="w-3.5 h-3.5" />
            <span>Print / PDF</span>
          </button>
        </div>
      </div>

      {/* Grid Container */}
      <div className="overflow-x-auto">
        <table className="w-full min-w-[640px] border-collapse">
          <thead>
            <tr>
              <th className="w-20 sm:w-28 py-2.5 px-3 text-left text-xs font-bold uppercase tracking-wider text-[#9AA0C3] border-b border-[#E4E7F0] dark:border-[#2A2F47]">
                Day / Period
              </th>
              {periods.map((p) => (
                <th
                  key={p}
                  className="py-2.5 px-3 text-center text-xs font-bold uppercase tracking-wider text-[#6B7194] dark:text-[#9AA0C3] border-b border-[#E4E7F0] dark:border-[#2A2F47]"
                >
                  Period {p}
                </th>
              ))}
            </tr>
          </thead>
          <tbody className="divide-y divide-[#E4E7F0] dark:divide-[#2A2F47]">
            {uniqueDays.map((day) => (
              <tr key={day} className="hover:bg-slate-50/50 dark:hover:bg-slate-800/20 transition-colors">
                <td className="py-3 px-3 font-bold text-sm text-[#1B1F3B] dark:text-[#E8EAF6] align-middle">
                  {day}
                </td>
                {periods.map((period) => {
                  const slotId = slotMap.get(`${day}_${period}`);
                  const slot = slotId ? dataset.slots.find((s) => s.id === slotId) : null;
                  const entry = slotId ? entryBySlot.get(slotId) : null;
                  const subject = entry ? dataset.subjects.find((s) => s.id === entry.subjectId) : null;
                  const faculty = entry ? dataset.faculty.find((f) => f.id === entry.facultyId) : null;

                  if (!slot) {
                    return (
                      <td key={period} className="p-2 text-center align-middle">
                        <div className="h-20 rounded-xl bg-gray-50/50 dark:bg-gray-900/20 border border-dashed border-gray-200 dark:border-gray-800 flex items-center justify-center text-gray-300 dark:text-gray-700 text-xs">
                          N/A
                        </div>
                      </td>
                    );
                  }

                  if (entry && subject && faculty) {
                    const colors = getSubjectColors(subject.id, isDark);
                    return (
                      <td key={period} className="p-2 align-middle">
                        <div
                          style={{
                            backgroundColor: colors.bg,
                            borderColor: colors.border,
                          }}
                          className={`relative h-20 rounded-xl p-2.5 border transition-all flex flex-col justify-between group ${
                            interactive ? 'cursor-pointer hover:shadow-md' : ''
                          }`}
                          onClick={() => interactive && onSlotClick && onSlotClick(slot.id)}
                        >
                          {interactive && onSlotRemove && (
                            <button
                              onClick={(e) => {
                                e.stopPropagation();
                                onSlotRemove(slot.id);
                              }}
                              title="Remove assignment"
                              className="absolute -top-1.5 -right-1.5 w-5 h-5 rounded-full bg-rose-500 hover:bg-rose-600 text-white flex items-center justify-center shadow-xs opacity-0 group-hover:opacity-100 transition-opacity z-10"
                            >
                              <X className="w-3 h-3" />
                            </button>
                          )}
                          <div className="flex items-start justify-between gap-1">
                            <span
                              style={{ color: colors.text }}
                              className="font-bold text-xs sm:text-[13px] leading-tight line-clamp-2"
                            >
                              {subject.name}
                            </span>
                            <span className="text-[10px] font-mono text-[#9AA0C3] shrink-0 font-medium">
                              S{subject.id}
                            </span>
                          </div>

                          <div className="flex items-center justify-between text-[11px] text-[#6B7194] dark:text-[#9AA0C3] mt-1">
                            <span className="truncate font-medium">{faculty.name}</span>
                            <span className="text-[10px] text-[#9AA0C3] font-mono">
                              {slot.toString ? slot.name : `${day} P${period}`}
                            </span>
                          </div>
                        </div>
                      </td>
                    );
                  }

                  // Empty Slot
                  return (
                    <td key={period} className="p-2 align-middle">
                      <div
                        onClick={() => interactive && onSlotClick && onSlotClick(slot.id)}
                        className={`h-20 rounded-xl border border-dashed border-[#CBD5E1] dark:border-[#334155] flex flex-col items-center justify-center text-xs transition-all ${
                          interactive
                            ? 'cursor-pointer hover:border-[#5B5BD6] hover:bg-[#EEF0FD]/40 dark:hover:bg-[#2C204B]/30 text-[#9AA0C3] hover:text-[#5B5BD6]'
                            : 'text-[#9AA0C3]'
                        }`}
                      >
                        {interactive ? (
                          <>
                            <Plus className="w-4 h-4 mb-0.5 opacity-70" />
                            <span className="text-[11px] font-medium">Assign</span>
                          </>
                        ) : (
                          <span className="text-sm font-bold opacity-40">-</span>
                        )}
                        <span className="text-[9px] font-mono opacity-50 mt-0.5">{day} P{period}</span>
                      </div>
                    </td>
                  );
                })}
              </tr>
            ))}
          </tbody>
        </table>
      </div>
    </div>
  );
};
