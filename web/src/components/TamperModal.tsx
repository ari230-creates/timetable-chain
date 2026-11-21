import React, { useState } from 'react';
import { useApp } from '../context/AppContext';
import { X, ShieldAlert, AlertTriangle, Zap } from 'lucide-react';

interface TamperModalProps {
  isOpen: boolean;
  onClose: () => void;
}

export const TamperModal: React.FC<TamperModalProps> = ({ isOpen, onClose }) => {
  const { blocks, tamperChain } = useApp();

  const [selectedBlockIndex, setSelectedBlockIndex] = useState<number>(() => {
    return blocks.length > 1 ? 1 : 0;
  });
  const [remine, setRemine] = useState<boolean>(false);
  const [customData, setCustomData] = useState<string>('');
  const [isTampering, setIsTampering] = useState<boolean>(false);

  if (!isOpen) return null;

  const handleExecute = async () => {
    setIsTampering(true);
    await tamperChain(selectedBlockIndex, remine, customData.trim() || undefined);
    setIsTampering(false);
    onClose();
  };

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/50 backdrop-blur-xs">
      <div className="bg-white dark:bg-[#181C2E] border border-[#E4E7F0] dark:border-[#2A2F47] rounded-3xl w-full max-w-lg shadow-2xl overflow-hidden p-6 space-y-5 transition-colors">
        {/* Header */}
        <div className="flex items-center justify-between pb-3 border-b border-[#E4E7F0] dark:border-[#2A2F47]">
          <div className="flex items-center space-x-2.5">
            <div className="p-2 rounded-xl bg-rose-50 dark:bg-rose-950/60 text-rose-600 dark:text-rose-400">
              <ShieldAlert className="w-5 h-5" />
            </div>
            <div>
              <h2 className="text-base font-bold text-rose-600 dark:text-rose-400">
                Blockchain Tamper Demonstration
              </h2>
              <p className="text-xs text-[#6B7194] dark:text-[#9AA0C3]">
                Simulate malicious data alteration to demonstrate 3-layer cryptographic defense
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

        {/* Options */}
        <div className="space-y-4">
          <div>
            <label className="block text-xs font-bold uppercase tracking-wider text-[#6B7194] dark:text-[#9AA0C3] mb-1.5">
              Target Block to Tamper
            </label>
            <select
              value={selectedBlockIndex}
              onChange={(e) => setSelectedBlockIndex(Number(e.target.value))}
              className="w-full px-3.5 py-2.5 rounded-xl border border-[#CBD5E1] dark:border-[#334155] bg-white dark:bg-[#20253B] text-[#1B1F3B] dark:text-[#E8EAF6] text-sm focus:outline-none focus:ring-2 focus:ring-[#5B5BD6]"
            >
              {blocks.map((b) => (
                <option key={b.index} value={b.index}>
                  Block #{b.index} — {b.note} ({b.data === 'GENESIS' ? 'GENESIS' : 'Timetable Payload'})
                </option>
              ))}
            </select>
          </div>

          <div className="flex items-start space-x-3 p-3 rounded-xl bg-slate-50 dark:bg-[#20253B]/50 border border-[#E4E7F0] dark:border-[#2A2F47]">
            <input
              type="checkbox"
              id="remine-check"
              checked={remine}
              onChange={(e) => setRemine(e.target.checked)}
              className="mt-1 w-4 h-4 rounded text-[#5B5BD6] focus:ring-[#5B5BD6] cursor-pointer"
            />
            <label htmlFor="remine-check" className="text-xs text-[#1B1F3B] dark:text-[#E8EAF6] cursor-pointer">
              <span className="font-bold block">Also re-mine the block (Clever Attacker)</span>
              <span className="text-[#6B7194] dark:text-[#9AA0C3] block mt-0.5">
                Recalculates nonce and hash to satisfy Proof-of-Work (3 leading zeroes).
              </span>
            </label>
          </div>

          {/* Educational Explainer */}
          <div className="p-3.5 rounded-xl bg-[#F8F9FD] dark:bg-[#20253B] border border-[#E4E7F0] dark:border-[#2A2F47] text-xs text-[#6B7194] dark:text-[#9AA0C3] space-y-2">
            <div className="font-bold text-[#1B1F3B] dark:text-[#E8EAF6] flex items-center space-x-1.5">
              <Zap className="w-3.5 h-3.5 text-amber-500" />
              <span>Attack Mechanics & Expected Verification Outcome:</span>
            </div>
            <p className="leading-relaxed">
              • <b>Careless Attacker (Unchecked):</b> Corrupts data without re-mining. Layer 1 verification immediately fails on Block #{selectedBlockIndex} due to a <b>Hash Mismatch</b> (stored hash != recalculated SHA-256).
            </p>
            <p className="leading-relaxed">
              • <b>Clever Attacker (Checked):</b> Re-computes valid PoW for Block #{selectedBlockIndex}. However, verification still fails at <b>Block #{selectedBlockIndex + 1}</b> due to a <b>Broken Cryptographic Link</b> (prevHash does not match altered block's new hash).
            </p>
          </div>
        </div>

        {/* Footer */}
        <div className="flex justify-end space-x-2 pt-2 border-t border-[#E4E7F0] dark:border-[#2A2F47]">
          <button
            onClick={onClose}
            className="px-4 py-2 rounded-xl text-sm font-semibold text-[#6B7194] dark:text-[#9AA0C3] hover:bg-[#F0F2FA] dark:hover:bg-[#20253B]"
          >
            Cancel
          </button>
          <button
            onClick={handleExecute}
            disabled={isTampering}
            className="px-4 py-2 rounded-xl text-sm font-semibold text-white bg-rose-600 hover:bg-rose-700 shadow-xs cursor-pointer"
          >
            {isTampering ? 'Tampering...' : 'Tamper Block'}
          </button>
        </div>
      </div>
    </div>
  );
};
