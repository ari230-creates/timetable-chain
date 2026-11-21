import React, { useState } from 'react';
import { useApp } from '../context/AppContext';
import { Block } from '../types';
import { TamperModal } from '../components/TamperModal';
import { BlockDetailModal } from '../components/BlockDetailModal';
import {
  ShieldCheck,
  ShieldAlert,
  Link as LinkIcon,
  Download,
  Upload,
  RotateCcw,
  Eye,
  Copy,
  Check,
  AlertTriangle,
  Loader2,
  Lock,
} from 'lucide-react';

export const BlockchainPage: React.FC = () => {
  const {
    blocks,
    dataset,
    verificationReport,
    isVerifying,
    verifyChain,
    resetChain,
    exportChainFile,
    importChainFile,
    showToast,
  } = useApp();

  const [tamperModalOpen, setTamperModalOpen] = useState(false);
  const [selectedBlock, setSelectedBlock] = useState<Block | null>(null);
  const [copiedHash, setCopiedHash] = useState<string | null>(null);

  const handleCopy = (text: string) => {
    navigator.clipboard.writeText(text);
    setCopiedHash(text);
    showToast('Hash copied to clipboard', 'info');
    setTimeout(() => setCopiedHash(null), 2000);
  };

  const handleExport = async () => {
    try {
      const content = await exportChainFile();
      const blob = new Blob([content], { type: 'text/plain' });
      const url = URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url;
      a.download = `timetable_chain_${Date.now()}.txt`;
      document.body.appendChild(a);
      a.click();
      document.body.removeChild(a);
      URL.revokeObjectURL(url);
      showToast('Exported blockchain ledger file', 'success');
    } catch {
      showToast('Failed to export chain', 'error');
    }
  };

  const handleImportFile = (e: React.ChangeEvent<HTMLInputElement>) => {
    const file = e.target.files?.[0];
    if (!file) return;
    const reader = new FileReader();
    reader.onload = async (event) => {
      const content = event.target?.result as string;
      await importChainFile(content);
    };
    reader.readAsText(file);
    e.target.value = '';
  };

  const shortenHash = (hash: string) => {
    if (!hash || hash.length <= 16) return hash;
    return `${hash.slice(0, 8)}...${hash.slice(-6)}`;
  };

  return (
    <div className="space-y-6">
      {/* Top Toolbar Card */}
      <div className="bg-white dark:bg-[#181C2E] border border-[#E4E7F0] dark:border-[#2A2F47] rounded-2xl p-4 sm:p-5 shadow-xs transition-colors">
        <div className="flex flex-col lg:flex-row lg:items-center justify-between gap-4">
          <div className="flex flex-wrap items-center gap-2.5">
            <button
              onClick={() => verifyChain()}
              disabled={isVerifying}
              className="flex items-center space-x-2 px-4 py-2.5 rounded-xl bg-[#5B5BD6] hover:bg-[#4D4DBF] text-white font-semibold text-sm shadow-xs transition-all disabled:opacity-50 cursor-pointer"
            >
              {isVerifying ? (
                <Loader2 className="w-4 h-4 animate-spin" />
              ) : (
                <ShieldCheck className="w-4 h-4" />
              )}
              <span>{isVerifying ? 'Verifying All 3 Layers...' : 'Verify Blockchain'}</span>
            </button>

            <button
              onClick={() => setTamperModalOpen(true)}
              className="flex items-center space-x-2 px-4 py-2.5 rounded-xl bg-rose-50 dark:bg-rose-950/40 hover:bg-rose-100 dark:hover:bg-rose-900/40 text-rose-700 dark:text-rose-300 font-semibold text-sm border border-rose-200 dark:border-rose-800 transition-all cursor-pointer"
            >
              <ShieldAlert className="w-4 h-4 text-rose-600 dark:text-rose-400" />
              <span>Tamper Demo</span>
            </button>
          </div>

          <div className="flex flex-wrap items-center gap-2">
            <button
              onClick={handleExport}
              className="flex items-center space-x-1.5 px-3 py-2 rounded-xl bg-[#F0F2FA] dark:bg-[#20253B] hover:bg-[#E4E7F0] dark:hover:bg-[#2A3150] text-[#1B1F3B] dark:text-[#E8EAF6] text-xs font-semibold border border-[#E4E7F0] dark:border-[#2A2F47] transition-all cursor-pointer"
            >
              <Download className="w-3.5 h-3.5" />
              <span>Save Chain</span>
            </button>

            <label className="flex items-center space-x-1.5 px-3 py-2 rounded-xl bg-[#F0F2FA] dark:bg-[#20253B] hover:bg-[#E4E7F0] dark:hover:bg-[#2A3150] text-[#1B1F3B] dark:text-[#E8EAF6] text-xs font-semibold border border-[#E4E7F0] dark:border-[#2A2F47] transition-all cursor-pointer">
              <Upload className="w-3.5 h-3.5" />
              <span>Load Chain</span>
              <input
                type="file"
                accept=".txt"
                onChange={handleImportFile}
                className="hidden"
              />
            </label>

            <button
              onClick={() => {
                if (window.confirm('Reset blockchain to Genesis block? Unsaved blocks will be discarded.')) {
                  resetChain();
                }
              }}
              className="flex items-center space-x-1.5 px-3 py-2 rounded-xl bg-[#F0F2FA] dark:bg-[#20253B] hover:bg-rose-50 dark:hover:bg-rose-950/40 text-[#6B7194] dark:text-[#9AA0C3] hover:text-rose-600 dark:hover:text-rose-400 text-xs font-semibold border border-[#E4E7F0] dark:border-[#2A2F47] transition-all cursor-pointer"
            >
              <RotateCcw className="w-3.5 h-3.5" />
              <span>Reset</span>
            </button>
          </div>
        </div>
      </div>

      {/* 3-Layer Verification Banner */}
      {verificationReport && (
        <div
          className={`p-4 rounded-2xl border text-sm transition-all ${
            verificationReport.allValid
              ? 'bg-emerald-50 dark:bg-emerald-950/40 text-emerald-800 dark:text-emerald-200 border-emerald-200 dark:border-emerald-800'
              : 'bg-rose-50 dark:bg-rose-950/40 text-rose-800 dark:text-rose-200 border-rose-200 dark:border-rose-800'
          }`}
        >
          <div className="flex items-start space-x-3">
            {verificationReport.allValid ? (
              <ShieldCheck className="w-5 h-5 text-emerald-600 dark:text-emerald-400 shrink-0 mt-0.5" />
            ) : (
              <AlertTriangle className="w-5 h-5 text-rose-600 dark:text-rose-400 shrink-0 mt-0.5" />
            )}
            <div className="space-y-1">
              <div className="font-bold text-base">
                {verificationReport.allValid
                  ? '✓ Chain intact and every timetable valid'
                  : `Verification Failed: ${verificationReport.messages.length} problem(s) detected across 3 layers:`}
              </div>
              <p className="text-xs opacity-90 leading-relaxed">
                {verificationReport.allValid
                  ? 'Layer 1 (Structural Integrity & PoW), Layer 2 (Cryptographic prevHash Linkage), and Layer 3 (Curricular Timetable Semantics) all passed with zero violations.'
                  : 'Ledger integrity violation detected! The append-only chain was tampered or contains semantic rule contradictions.'}
              </p>
              {!verificationReport.allValid && (
                <ul className="list-disc pl-5 text-xs space-y-1 font-mono pt-1 text-rose-900 dark:text-rose-200">
                  {verificationReport.messages.map((msg, idx) => (
                    <li key={idx}>{msg}</li>
                  ))}
                </ul>
              )}
            </div>
          </div>
        </div>
      )}

      {/* Blocks Sequence */}
      <div className="space-y-0">
        {blocks.map((block, index) => {
          const status = verificationReport?.blockStatuses[block.index.toString()];
          const isBlockValid = status ? status.valid : null;
          const hasProblems = status && status.problems.length > 0;

          return (
            <React.Fragment key={block.index}>
              {/* Vertical Chain Connector if not first block */}
              {index > 0 && (
                <div className="flex flex-col items-center justify-center my-1 select-none">
                  <div className="w-0.5 h-3 bg-[#CBD5E1] dark:bg-[#334155]" />
                  <div className="w-7 h-7 rounded-lg border-2 border-[#5B5BD6] bg-white dark:bg-[#181C2E] flex items-center justify-center text-[#5B5BD6] shadow-xs">
                    <LinkIcon className="w-3.5 h-3.5 transform -rotate-45" />
                  </div>
                  <div className="w-0.5 h-3 bg-[#CBD5E1] dark:bg-[#334155]" />
                </div>
              )}

              {/* Block Card */}
              <div
                className={`bg-white dark:bg-[#181C2E] rounded-2xl p-5 shadow-xs transition-all border ${
                  isBlockValid === true
                    ? 'border-emerald-300 dark:border-emerald-800'
                    : isBlockValid === false
                    ? 'border-rose-400 dark:border-rose-800 ring-2 ring-rose-400/20'
                    : 'border-[#E4E7F0] dark:border-[#2A2F47]'
                }`}
              >
                <div className="flex flex-col md:flex-row md:items-center justify-between gap-4 pb-3 border-b border-[#E4E7F0] dark:border-[#2A2F47]">
                  <div className="flex items-center space-x-3">
                    <div className="w-10 h-10 rounded-xl bg-[#5B5BD6] text-white flex items-center justify-center font-bold text-sm shadow-xs">
                      #{block.index}
                    </div>
                    <div>
                      <div className="flex items-center space-x-2">
                        <h3 className="font-bold text-sm text-[#1B1F3B] dark:text-[#E8EAF6]">
                          {block.note}
                        </h3>
                        {block.data === 'GENESIS' && (
                          <span className="text-[10px] font-bold uppercase tracking-wider px-2 py-0.5 rounded-full bg-purple-50 dark:bg-purple-950/60 text-[#5B5BD6] dark:text-[#8AB4F8] border border-purple-200 dark:border-purple-800">
                            Genesis Anchor
                          </span>
                        )}
                      </div>
                      <p className="text-xs text-[#6B7194] dark:text-[#9AA0C3] mt-0.5">
                        Timestamp: {new Date(block.timestamp * 1000).toLocaleString()} • Nonce: <span className="font-mono font-semibold">{block.nonce}</span>
                      </p>
                    </div>
                  </div>

                  <div className="flex items-center space-x-2">
                    {/* Verification Badge */}
                    {status ? (
                      status.valid ? (
                        <span className="flex items-center space-x-1 px-2.5 py-1 text-xs font-bold rounded-lg bg-emerald-50 dark:bg-emerald-950/60 text-emerald-700 dark:text-emerald-300 border border-emerald-200 dark:border-emerald-800">
                          <ShieldCheck className="w-3.5 h-3.5 text-emerald-600 dark:text-emerald-400" />
                          <span>VERIFIED</span>
                        </span>
                      ) : (
                        <span className="flex items-center space-x-1 px-2.5 py-1 text-xs font-bold rounded-lg bg-rose-50 dark:bg-rose-950/60 text-rose-700 dark:text-rose-300 border border-rose-200 dark:border-rose-800">
                          <AlertTriangle className="w-3.5 h-3.5 text-rose-600 dark:text-rose-400" />
                          <span>INVALID</span>
                        </span>
                      )
                    ) : (
                      <span className="px-2.5 py-1 text-xs font-bold rounded-lg bg-slate-100 dark:bg-slate-800 text-[#6B7194] dark:text-[#9AA0C3]">
                        PENDING
                      </span>
                    )}

                    <button
                      onClick={() => setSelectedBlock(block)}
                      className="flex items-center space-x-1.5 px-3 py-1.5 text-xs font-semibold rounded-lg bg-[#F0F2FA] dark:bg-[#20253B] text-[#1B1F3B] dark:text-[#E8EAF6] hover:bg-[#E4E7F0] dark:hover:bg-[#2A3150] transition-colors border border-[#E4E7F0] dark:border-[#2A2F47] cursor-pointer"
                    >
                      <Eye className="w-3.5 h-3.5 text-[#5B5BD6]" />
                      <span>Inspect</span>
                    </button>
                  </div>
                </div>

                {/* Hashes & Linkage */}
                <div className="grid grid-cols-1 md:grid-cols-2 gap-3 pt-3 text-xs">
                  <div>
                    <span className="text-[#9AA0C3] font-semibold uppercase text-[10px] block mb-1">
                      Current Hash (SHA-256)
                    </span>
                    <button
                      onClick={() => handleCopy(block.hash)}
                      title={`Full Hash: ${block.hash}\nClick to copy`}
                      className="w-full flex items-center justify-between p-2 rounded-xl bg-[#EEF0FD]/70 dark:bg-[#20253B] border border-[#D6D8F8] dark:border-[#2C3352] text-[#1B1F3B] dark:text-[#E8EAF6] font-mono hover:bg-[#EEF0FD] dark:hover:bg-[#2A3150] transition-colors text-left"
                    >
                      <span className="truncate">{shortenHash(block.hash)}</span>
                      {copiedHash === block.hash ? (
                        <Check className="w-3.5 h-3.5 text-emerald-600 shrink-0 ml-1" />
                      ) : (
                        <Copy className="w-3.5 h-3.5 text-[#5B5BD6] opacity-60 shrink-0 ml-1" />
                      )}
                    </button>
                  </div>

                  <div>
                    <span className="text-[#9AA0C3] font-semibold uppercase text-[10px] block mb-1">
                      Previous Hash (Pointer)
                    </span>
                    <button
                      onClick={() => handleCopy(block.prevHash)}
                      title={`Full PrevHash: ${block.prevHash}\nClick to copy`}
                      className="w-full flex items-center justify-between p-2 rounded-xl bg-slate-50 dark:bg-[#20253B]/50 border border-[#E4E7F0] dark:border-[#2A2F47] text-[#6B7194] dark:text-[#9AA0C3] font-mono hover:bg-slate-100 dark:hover:bg-[#20253B] transition-colors text-left"
                    >
                      <span className="truncate">{shortenHash(block.prevHash)}</span>
                      {copiedHash === block.prevHash ? (
                        <Check className="w-3.5 h-3.5 text-emerald-600 shrink-0 ml-1" />
                      ) : (
                        <Copy className="w-3.5 h-3.5 text-[#6B7194] opacity-50 shrink-0 ml-1" />
                      )}
                    </button>
                  </div>
                </div>

                {/* Per-block Error Callout if invalid */}
                {hasProblems && (
                  <div className="mt-3 p-3 rounded-xl bg-rose-50 dark:bg-rose-950/40 border border-rose-200 dark:border-rose-800 text-xs text-rose-800 dark:text-rose-200 font-mono space-y-0.5">
                    {status.problems.map((p, idx) => (
                      <div key={idx} className="flex items-start space-x-1.5">
                        <AlertTriangle className="w-3.5 h-3.5 text-rose-600 dark:text-rose-400 shrink-0 mt-0.5" />
                        <span>{p}</span>
                      </div>
                    ))}
                  </div>
                )}
              </div>
            </React.Fragment>
          );
        })}
      </div>

      {/* Tamper Modal */}
      <TamperModal
        isOpen={tamperModalOpen}
        onClose={() => setTamperModalOpen(false)}
      />

      {/* Block Inspector Modal */}
      <BlockDetailModal
        isOpen={selectedBlock !== null}
        onClose={() => setSelectedBlock(null)}
        block={selectedBlock}
        dataset={dataset}
      />
    </div>
  );
};
