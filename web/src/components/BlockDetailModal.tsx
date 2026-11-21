import React, { useState } from 'react';
import { Block, Dataset, TimetableEntry } from '../types';
import { ScheduleGrid } from './ScheduleGrid';
import { X, Copy, Check, ShieldCheck, Database, Calendar, Hash } from 'lucide-react';

interface BlockDetailModalProps {
  isOpen: boolean;
  onClose: () => void;
  block: Block | null;
  dataset: Dataset | null;
}

export const BlockDetailModal: React.FC<BlockDetailModalProps> = ({
  isOpen,
  onClose,
  block,
  dataset,
}) => {
  const [copiedHash, setCopiedHash] = useState<boolean>(false);
  const [copiedPrev, setCopiedPrev] = useState<boolean>(false);

  if (!isOpen || !block) return null;

  const handleCopy = (text: string, type: 'hash' | 'prev') => {
    navigator.clipboard.writeText(text);
    if (type === 'hash') {
      setCopiedHash(true);
      setTimeout(() => setCopiedHash(false), 2000);
    } else {
      setCopiedPrev(true);
      setTimeout(() => setCopiedPrev(false), 2000);
    }
  };

  // Deserialize timetable data if not GENESIS
  let entries: TimetableEntry[] = [];
  if (block.data && block.data !== 'GENESIS') {
    const tokens = block.data.split(';').filter((s) => s.trim().length > 0);
    for (const t of tokens) {
      const parts = t.split(',').map((x) => parseInt(x.trim(), 10));
      if (parts.length >= 3 && !isNaN(parts[0]) && !isNaN(parts[1]) && !isNaN(parts[2])) {
        entries.push({
          subjectId: parts[0],
          facultyId: parts[1],
          slotId: parts[2],
        });
      }
    }
  }

  const dateStr = new Date(block.timestamp * 1000).toLocaleString();
  const rawPayload = `${block.index}|${block.timestamp}|${block.note}|${block.data}|${block.prevHash}|${block.nonce}`;

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/50 backdrop-blur-xs overflow-y-auto">
      <div className="bg-white dark:bg-[#181C2E] border border-[#E4E7F0] dark:border-[#2A2F47] rounded-3xl w-full max-w-3xl max-h-[90vh] shadow-2xl overflow-hidden flex flex-col my-8 transition-colors">
        {/* Header */}
        <div className="px-6 py-4 border-b border-[#E4E7F0] dark:border-[#2A2F47] flex items-center justify-between">
          <div className="flex items-center space-x-3">
            <div className="w-10 h-10 rounded-xl bg-[#5B5BD6] text-white flex items-center justify-center font-bold">
              #{block.index}
            </div>
            <div>
              <h2 className="text-base font-bold text-[#1B1F3B] dark:text-[#E8EAF6]">
                Block #{block.index}: {block.note}
              </h2>
              <p className="text-xs text-[#6B7194] dark:text-[#9AA0C3]">
                Mined {dateStr} • Nonce: {block.nonce}
              </p>
            </div>
          </div>
          <button
            onClick={onClose}
            className="p-1.5 text-[#9AA0C3] hover:text-[#1B1F3B] dark:hover:text-[#E8EAF6] rounded-xl hover:bg-[#F0F2FA] dark:hover:bg-[#20253B]"
          >
            <X className="w-5 h-5" />
          </button>
        </div>

        {/* Body */}
        <div className="p-6 overflow-y-auto space-y-5 text-sm">
          {/* Hashes Section */}
          <div className="space-y-3">
            <div>
              <div className="flex items-center justify-between text-xs font-bold uppercase tracking-wider text-[#6B7194] dark:text-[#9AA0C3] mb-1">
                <span>Block SHA-256 Hash</span>
                <button
                  onClick={() => handleCopy(block.hash, 'hash')}
                  className="flex items-center space-x-1 text-[11px] text-[#5B5BD6] dark:text-[#8AB4F8] hover:underline"
                >
                  {copiedHash ? <Check className="w-3.5 h-3.5" /> : <Copy className="w-3.5 h-3.5" />}
                  <span>{copiedHash ? 'Copied' : 'Copy Full Hash'}</span>
                </button>
              </div>
              <div className="p-3 rounded-xl bg-[#F8F9FD] dark:bg-[#121626] border border-[#E4E7F0] dark:border-[#2A2F47] font-mono text-xs text-[#1B1F3B] dark:text-[#E8EAF6] break-all select-all">
                {block.hash}
              </div>
            </div>

            <div>
              <div className="flex items-center justify-between text-xs font-bold uppercase tracking-wider text-[#6B7194] dark:text-[#9AA0C3] mb-1">
                <span>Previous Block Hash (prevHash pointer)</span>
                <button
                  onClick={() => handleCopy(block.prevHash, 'prev')}
                  className="flex items-center space-x-1 text-[11px] text-[#5B5BD6] dark:text-[#8AB4F8] hover:underline"
                >
                  {copiedPrev ? <Check className="w-3.5 h-3.5" /> : <Copy className="w-3.5 h-3.5" />}
                  <span>{copiedPrev ? 'Copied' : 'Copy'}</span>
                </button>
              </div>
              <div className="p-3 rounded-xl bg-[#F8F9FD] dark:bg-[#121626] border border-[#E4E7F0] dark:border-[#2A2F47] font-mono text-xs text-[#6B7194] dark:text-[#9AA0C3] break-all select-all">
                {block.prevHash}
              </div>
            </div>
          </div>

          {/* Cryptographic Preimage Payload */}
          <div>
            <span className="text-xs font-bold uppercase tracking-wider text-[#6B7194] dark:text-[#9AA0C3] block mb-1">
              Raw Cryptographic Hash Payload (Preimage)
            </span>
            <div className="p-3 rounded-xl bg-[#F8F9FD] dark:bg-[#121626] border border-[#E4E7F0] dark:border-[#2A2F47] font-mono text-[11px] text-[#6B7194] dark:text-[#9AA0C3] break-all select-all leading-relaxed">
              {rawPayload}
            </div>
          </div>

          {/* Timetable View */}
          {entries.length > 0 && dataset ? (
            <div>
              <span className="text-xs font-bold uppercase tracking-wider text-[#6B7194] dark:text-[#9AA0C3] block mb-2">
                Stored Timetable Schedule ({entries.length} Entries)
              </span>
              <ScheduleGrid
                entries={entries}
                dataset={dataset}
                interactive={false}
                title={`Block #${block.index} Schedule`}
              />
            </div>
          ) : block.data === 'GENESIS' ? (
            <div className="p-4 rounded-xl bg-purple-50 dark:bg-purple-950/40 text-[#5B5BD6] dark:text-[#8AB4F8] border border-purple-200 dark:border-purple-800 text-xs">
              <b>Genesis Block:</b> Serves as the primordial trust anchor for the Timetable Chain ledger with previous hash pointer "0".
            </div>
          ) : null}
        </div>

        {/* Footer */}
        <div className="px-6 py-4 border-t border-[#E4E7F0] dark:border-[#2A2F47] flex justify-end">
          <button
            onClick={onClose}
            className="px-4 py-2 rounded-xl bg-[#F0F2FA] dark:bg-[#20253B] text-sm font-semibold text-[#1B1F3B] dark:text-[#E8EAF6] hover:bg-[#E4E7F0] dark:hover:bg-[#2A3150] transition-colors"
          >
            Close
          </button>
        </div>
      </div>
    </div>
  );
};
