import React, { useState, useEffect } from 'react';
import { Dataset, TimetableEntry } from '../types';
import { api } from '../services/api';
import { X, CheckCircle2, AlertCircle, Trash2 } from 'lucide-react';

interface SlotAssignmentModalProps {
  isOpen: boolean;
  onClose: () => void;
  slotId: number;
  entries: TimetableEntry[];
  dataset: Dataset;
  datasetContent: string;
  onAssign: (entry: TimetableEntry) => void;
  onRemove: (slotId: number) => void;
}

export const SlotAssignmentModal: React.FC<SlotAssignmentModalProps> = ({
  isOpen,
  onClose,
  slotId,
  entries,
  dataset,
  datasetContent,
  onAssign,
  onRemove,
}) => {
  const currentEntry = entries.find((e) => e.slotId === slotId);
  const slot = dataset.slots.find((s) => s.id === slotId);

  const [selectedSubjectId, setSelectedSubjectId] = useState<number>(() => {
    return currentEntry ? currentEntry.subjectId : (dataset.subjects[0]?.id || 1);
  });

  const [selectedFacultyId, setSelectedFacultyId] = useState<number>(() => {
    return currentEntry ? currentEntry.facultyId : (dataset.faculty[0]?.id || 1);
  });

  const [canPlace, setCanPlace] = useState<boolean>(true);
  const [rejectionReason, setRejectionReason] = useState<string>('');
  const [checking, setChecking] = useState<boolean>(false);

  // Check canPlace whenever subject or faculty selection changes
  useEffect(() => {
    if (!isOpen || !slotId) return;

    let active = true;
    const verifyPlacement = async () => {
      setChecking(true);
      try {
        const res = await api.checkCanPlace({
          content: datasetContent,
          entries,
          subjectId: selectedSubjectId,
          facultyId: selectedFacultyId,
          slotId,
        });
        if (active) {
          setCanPlace(res.valid);
          setRejectionReason(res.reason);
        }
      } catch {
        if (active) {
          setCanPlace(false);
          setRejectionReason('Constraint verification check failed');
        }
      } finally {
        if (active) setChecking(false);
      }
    };

    verifyPlacement();
    return () => {
      active = false;
    };
  }, [isOpen, slotId, selectedSubjectId, selectedFacultyId, datasetContent, entries]);

  if (!isOpen || !slot) return null;

  const handleSave = () => {
    if (!canPlace) return;
    onAssign({
      subjectId: selectedSubjectId,
      facultyId: selectedFacultyId,
      slotId,
    });
    onClose();
  };

  const handleRemove = () => {
    onRemove(slotId);
    onClose();
  };

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/50 backdrop-blur-xs">
      <div className="bg-white dark:bg-[#181C2E] border border-[#E4E7F0] dark:border-[#2A2F47] rounded-3xl w-full max-w-md shadow-2xl overflow-hidden transition-colors">
        {/* Header */}
        <div className="px-6 py-4 border-b border-[#E4E7F0] dark:border-[#2A2F47] flex items-center justify-between">
          <div>
            <h2 className="text-base font-bold text-[#1B1F3B] dark:text-[#E8EAF6]">
              Assign Schedule Slot
            </h2>
            <p className="text-xs text-[#5B5BD6] dark:text-[#8AB4F8] font-semibold mt-0.5">
              {slot.day} Period {slot.period} (Slot #{slot.id})
            </p>
          </div>
          <button
            onClick={onClose}
            className="p-1.5 text-[#9AA0C3] hover:text-[#1B1F3B] dark:hover:text-[#E8EAF6] rounded-xl hover:bg-[#F0F2FA] dark:hover:bg-[#20253B]"
          >
            <X className="w-5 h-5" />
          </button>
        </div>

        {/* Form Body */}
        <div className="p-6 space-y-4">
          <div>
            <label className="block text-xs font-bold uppercase tracking-wider text-[#6B7194] dark:text-[#9AA0C3] mb-1.5">
              Subject
            </label>
            <select
              value={selectedSubjectId}
              onChange={(e) => setSelectedSubjectId(Number(e.target.value))}
              className="w-full px-3.5 py-2.5 rounded-xl border border-[#CBD5E1] dark:border-[#334155] bg-white dark:bg-[#20253B] text-[#1B1F3B] dark:text-[#E8EAF6] text-sm focus:outline-none focus:ring-2 focus:ring-[#5B5BD6]"
            >
              {dataset.subjects.map((sub) => (
                <option key={sub.id} value={sub.id}>
                  {sub.name} ({sub.periodsPerWeek} periods/week)
                </option>
              ))}
            </select>
          </div>

          <div>
            <label className="block text-xs font-bold uppercase tracking-wider text-[#6B7194] dark:text-[#9AA0C3] mb-1.5">
              Faculty Instructor
            </label>
            <select
              value={selectedFacultyId}
              onChange={(e) => setSelectedFacultyId(Number(e.target.value))}
              className="w-full px-3.5 py-2.5 rounded-xl border border-[#CBD5E1] dark:border-[#334155] bg-white dark:bg-[#20253B] text-[#1B1F3B] dark:text-[#E8EAF6] text-sm focus:outline-none focus:ring-2 focus:ring-[#5B5BD6]"
            >
              {dataset.faculty.map((fac) => (
                <option key={fac.id} value={fac.id}>
                  {fac.name}
                </option>
              ))}
            </select>
          </div>

          {/* Real-time Constraint Feedback */}
          <div
            className={`p-3 rounded-xl border text-xs flex items-start space-x-2 transition-all ${
              checking
                ? 'bg-slate-50 dark:bg-slate-800/40 text-[#6B7194] border-slate-200 dark:border-slate-800'
                : canPlace
                ? 'bg-emerald-50 dark:bg-emerald-950/40 text-emerald-700 dark:text-emerald-300 border-emerald-200 dark:border-emerald-800'
                : 'bg-rose-50 dark:bg-rose-950/40 text-rose-700 dark:text-rose-300 border-rose-200 dark:border-rose-800'
            }`}
          >
            {checking ? (
              <span className="animate-pulse">Checking constraints...</span>
            ) : canPlace ? (
              <>
                <CheckCircle2 className="w-4 h-4 text-emerald-600 dark:text-emerald-400 shrink-0 mt-0.5" />
                <span className="font-semibold">Valid placement satisfying all 4 hard constraints.</span>
              </>
            ) : (
              <>
                <AlertCircle className="w-4 h-4 text-rose-600 dark:text-rose-400 shrink-0 mt-0.5" />
                <div>
                  <span className="font-bold block">Constraint Violation:</span>
                  <span className="leading-tight">{rejectionReason}</span>
                </div>
              </>
            )}
          </div>
        </div>

        {/* Footer */}
        <div className="px-6 py-4 border-t border-[#E4E7F0] dark:border-[#2A2F47] flex items-center justify-between">
          {currentEntry ? (
            <button
              onClick={handleRemove}
              className="flex items-center space-x-1.5 px-3 py-2 text-xs font-semibold text-rose-600 hover:text-rose-700 dark:text-rose-400 hover:bg-rose-50 dark:hover:bg-rose-950/40 rounded-xl transition-colors"
            >
              <Trash2 className="w-4 h-4" />
              <span>Remove</span>
            </button>
          ) : (
            <div />
          )}

          <div className="flex items-center space-x-2">
            <button
              onClick={onClose}
              className="px-4 py-2 rounded-xl text-sm font-semibold text-[#6B7194] dark:text-[#9AA0C3] hover:bg-[#F0F2FA] dark:hover:bg-[#20253B]"
            >
              Cancel
            </button>
            <button
              onClick={handleSave}
              disabled={!canPlace || checking}
              className={`px-4 py-2 rounded-xl text-sm font-semibold text-white shadow-xs transition-all ${
                canPlace && !checking
                  ? 'bg-[#5B5BD6] hover:bg-[#4D4DBF] cursor-pointer'
                  : 'bg-slate-300 dark:bg-slate-700 cursor-not-allowed opacity-60'
              }`}
            >
              Assign Slot
            </button>
          </div>
        </div>
      </div>
    </div>
  );
};
