import React, { useState } from 'react';
import { useApp } from '../context/AppContext';
import { ScheduleGrid } from '../components/ScheduleGrid';
import { SlotAssignmentModal } from '../components/SlotAssignmentModal';
import { TimetableEntry } from '../types';
import { Trash2, CheckCircle2, Link2, AlertCircle, Sparkles } from 'lucide-react';

export const ManualBuilderPage: React.FC = () => {
  const {
    dataset,
    datasetContent,
    manualTimetable,
    setManualTimetable,
    clearManualTimetable,
    manualValidation,
    validateManualSchedule,
    commitManualToBlockchain,
    setTab,
  } = useApp();

  const [activeSlotId, setActiveSlotId] = useState<number | null>(null);
  const [commitNote, setCommitNote] = useState<string>('Department Review Schedule');
  const [showCommitModal, setShowCommitModal] = useState<boolean>(false);
  const [committing, setCommitting] = useState<boolean>(false);

  if (!dataset) {
    return (
      <div className="p-8 text-center space-y-4">
        <div className="p-4 rounded-2xl bg-amber-50 dark:bg-amber-950/40 text-amber-800 dark:text-amber-200 border border-amber-200 dark:border-amber-800 max-w-md mx-auto text-sm">
          No curriculum dataset loaded.
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

  const handleSlotClick = (slotId: number) => {
    setActiveSlotId(slotId);
  };

  const handleSlotRemove = (slotId: number) => {
    setManualTimetable((prev) => prev.filter((e) => e.slotId !== slotId));
  };

  const handleAssignEntry = (entry: TimetableEntry) => {
    setManualTimetable((prev) => {
      const filtered = prev.filter((e) => e.slotId !== entry.slotId);
      return [...filtered, entry];
    });
  };

  const handleCommitConfirm = async () => {
    setCommitting(true);
    const success = await commitManualToBlockchain(commitNote);
    setCommitting(false);
    if (success) {
      setShowCommitModal(false);
    }
  };

  const isValidSchedule = manualValidation && manualValidation.valid;

  return (
    <div className="space-y-6">
      {/* Toolbar Card */}
      <div className="bg-white dark:bg-[#181C2E] border border-[#E4E7F0] dark:border-[#2A2F47] rounded-2xl p-4 sm:p-5 shadow-xs transition-colors">
        <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-4">
          <div>
            <h2 className="text-sm font-bold text-[#1B1F3B] dark:text-[#E8EAF6]">
              Interactive Timetable Builder
            </h2>
            <p className="text-xs text-[#6B7194] dark:text-[#9AA0C3] mt-0.5">
              Click any slot in the grid to assign a subject and qualified faculty member.
            </p>
          </div>

          <div className="flex flex-wrap items-center gap-2">
            <button
              onClick={clearManualTimetable}
              className="flex items-center space-x-1.5 px-3 py-2 rounded-xl bg-[#F0F2FA] dark:bg-[#20253B] hover:bg-rose-50 dark:hover:bg-rose-950/40 text-[#6B7194] dark:text-[#9AA0C3] hover:text-rose-600 dark:hover:text-rose-400 font-semibold text-xs border border-[#E4E7F0] dark:border-[#2A2F47] transition-all cursor-pointer"
            >
              <Trash2 className="w-3.5 h-3.5" />
              <span>Clear All</span>
            </button>

            <button
              onClick={() => validateManualSchedule()}
              className="flex items-center space-x-1.5 px-3.5 py-2 rounded-xl bg-[#F0F2FA] dark:bg-[#20253B] hover:bg-[#E4E7F0] dark:hover:bg-[#2A3150] text-[#1B1F3B] dark:text-[#E8EAF6] font-semibold text-xs border border-[#E4E7F0] dark:border-[#2A2F47] transition-all cursor-pointer"
            >
              <CheckCircle2 className="w-3.5 h-3.5 text-[#5B5BD6]" />
              <span>Validate Schedule</span>
            </button>

            <button
              onClick={() => setShowCommitModal(true)}
              disabled={!isValidSchedule}
              className={`flex items-center space-x-1.5 px-4 py-2 rounded-xl font-semibold text-xs text-white shadow-xs transition-all ${
                isValidSchedule
                  ? 'bg-[#5B5BD6] hover:bg-[#4D4DBF] cursor-pointer'
                  : 'bg-slate-300 dark:bg-slate-700 cursor-not-allowed opacity-60'
              }`}
            >
              <Link2 className="w-3.5 h-3.5" />
              <span>Commit to Blockchain</span>
            </button>
          </div>
        </div>
      </div>

      {/* Validation Status Banner */}
      {manualValidation && (
        <div
          className={`p-4 rounded-2xl border text-sm transition-all ${
            manualValidation.valid
              ? 'bg-emerald-50 dark:bg-emerald-950/40 text-emerald-800 dark:text-emerald-200 border-emerald-200 dark:border-emerald-800'
              : 'bg-rose-50 dark:bg-rose-950/40 text-rose-800 dark:text-rose-200 border-rose-200 dark:border-rose-800'
          }`}
        >
          <div className="flex items-start space-x-3">
            {manualValidation.valid ? (
              <CheckCircle2 className="w-5 h-5 text-emerald-600 dark:text-emerald-400 shrink-0 mt-0.5" />
            ) : (
              <AlertCircle className="w-5 h-5 text-rose-600 dark:text-rose-400 shrink-0 mt-0.5" />
            )}
            <div className="space-y-1">
              <div className="font-bold">
                {manualValidation.valid
                  ? `✓ Schedule satisfies all curriculum constraints (Penalty Score: ${manualValidation.penaltyScore})`
                  : `Schedule has ${manualValidation.problems.length} constraint violation(s):`}
              </div>
              {!manualValidation.valid && (
                <ul className="list-disc pl-5 text-xs space-y-0.5 opacity-90">
                  {manualValidation.problems.map((prob, idx) => (
                    <li key={idx}>{prob}</li>
                  ))}
                </ul>
              )}
            </div>
          </div>
        </div>
      )}

      {/* Grid */}
      <ScheduleGrid
        entries={manualTimetable}
        dataset={dataset}
        interactive={true}
        onSlotClick={handleSlotClick}
        onSlotRemove={handleSlotRemove}
        title="Manual Timetable"
      />

      {/* Slot Assignment Modal */}
      {activeSlotId !== null && (
        <SlotAssignmentModal
          isOpen={activeSlotId !== null}
          onClose={() => setActiveSlotId(null)}
          slotId={activeSlotId}
          entries={manualTimetable}
          dataset={dataset}
          datasetContent={datasetContent}
          onAssign={handleAssignEntry}
          onRemove={handleSlotRemove}
        />
      )}

      {/* Commit Dialog */}
      {showCommitModal && (
        <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/50 backdrop-blur-xs">
          <div className="bg-white dark:bg-[#181C2E] border border-[#E4E7F0] dark:border-[#2A2F47] rounded-3xl w-full max-w-md shadow-2xl overflow-hidden p-6 space-y-4">
            <div className="flex items-center space-x-2.5">
              <div className="p-2 rounded-xl bg-purple-50 dark:bg-purple-950/60 text-[#5B5BD6]">
                <Link2 className="w-5 h-5" />
              </div>
              <div>
                <h3 className="text-base font-bold text-[#1B1F3B] dark:text-[#E8EAF6]">
                  Commit Schedule to Blockchain
                </h3>
                <p className="text-xs text-[#6B7194] dark:text-[#9AA0C3]">
                  Mines a new Proof-of-Work block with this schedule
                </p>
              </div>
            </div>

            <div>
              <label className="block text-xs font-bold uppercase tracking-wider text-[#6B7194] dark:text-[#9AA0C3] mb-1.5">
                Block Note / Audit Memo
              </label>
              <input
                type="text"
                value={commitNote}
                onChange={(e) => setCommitNote(e.target.value)}
                placeholder="e.g. Approved Department Schedule"
                className="w-full px-3.5 py-2.5 rounded-xl border border-[#CBD5E1] dark:border-[#334155] bg-white dark:bg-[#20253B] text-[#1B1F3B] dark:text-[#E8EAF6] text-sm focus:outline-none focus:ring-2 focus:ring-[#5B5BD6]"
              />
            </div>

            <div className="p-3 rounded-xl bg-[#F8F9FD] dark:bg-[#20253B] text-xs text-[#6B7194] dark:text-[#9AA0C3] space-y-1">
              <div className="flex justify-between">
                <span>Proof-of-Work Target:</span>
                <span className="font-mono font-bold text-[#5B5BD6]">3 leading zeros (000...)</span>
              </div>
              <div className="flex justify-between">
                <span>Verification Engine:</span>
                <span className="font-bold text-emerald-600 dark:text-emerald-400">3-Layer Verified</span>
              </div>
            </div>

            <div className="flex justify-end space-x-2 pt-2">
              <button
                onClick={() => setShowCommitModal(false)}
                className="px-4 py-2 rounded-xl text-sm font-semibold text-[#6B7194] dark:text-[#9AA0C3] hover:bg-[#F0F2FA] dark:hover:bg-[#20253B]"
              >
                Cancel
              </button>
              <button
                onClick={handleCommitConfirm}
                disabled={committing}
                className="px-4 py-2 rounded-xl text-sm font-semibold text-white bg-[#5B5BD6] hover:bg-[#4D4DBF] shadow-xs cursor-pointer"
              >
                {committing ? 'Mining Block...' : 'Mine & Commit'}
              </button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
};
