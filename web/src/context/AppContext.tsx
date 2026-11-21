import React, { createContext, useContext, useState, useEffect, ReactNode, useCallback } from 'react';
import { Dataset, TimetableEntry, GeneratorStats, Block, VerificationReport, SavedDataset } from '../types';
import { api } from '../services/api';

export type TabType = 'generator' | 'manual' | 'blockchain' | 'curriculum';

export interface ToastMessage {
  id: string;
  type: 'info' | 'success' | 'warning' | 'error';
  text: string;
}

interface AppContextType {
  tab: TabType;
  setTab: (tab: TabType) => void;
  theme: 'light' | 'dark';
  toggleTheme: () => void;

  // Dataset State
  currentDatasetName: string;
  datasetContent: string;
  dataset: Dataset | null;
  datasetError: string | null;
  savedDatasets: SavedDataset[];
  loadDataset: (name: string, content: string) => Promise<boolean>;
  saveCurrentDataset: (name: string) => void;
  deleteSavedDataset: (id: string) => void;
  createNewDataset: () => void;
  loadSampleDataset: (fullWeek?: boolean) => Promise<void>;

  // Timetable State (Generator)
  generatedTimetable: TimetableEntry[];
  generatorStats: GeneratorStats | null;
  generatorMode: 'Plain' | 'MRV' | 'Best';
  setGeneratorMode: (mode: 'Plain' | 'MRV' | 'Best') => void;
  isGenerating: boolean;
  runGenerate: (mineAfter?: boolean) => Promise<boolean>;

  // Timetable State (Manual Builder)
  manualTimetable: TimetableEntry[];
  setManualTimetable: React.Dispatch<React.SetStateAction<TimetableEntry[]>>;
  clearManualTimetable: () => void;
  manualValidation: { valid: boolean; problems: string[]; penaltyScore: number } | null;
  validateManualSchedule: () => Promise<boolean>;
  commitManualToBlockchain: (note: string) => Promise<boolean>;

  // Blockchain State
  blocks: Block[];
  verificationReport: VerificationReport | null;
  isChainVerified: boolean;
  isVerifying: boolean;
  verifyChain: () => Promise<VerificationReport>;
  tamperChain: (targetIndex: number, remine: boolean, customData?: string) => Promise<void>;
  resetChain: () => void;
  exportChainFile: () => Promise<string>;
  importChainFile: (content: string) => Promise<boolean>;

  // UI / Toasts
  toasts: ToastMessage[];
  showToast: (text: string, type?: 'info' | 'success' | 'warning' | 'error') => void;
  dismissToast: (id: string) => void;
}

const AppContext = createContext<AppContextType | undefined>(undefined);

const GENESIS_BLOCK: Block = {
  index: 0,
  timestamp: 1704067200,
  note: 'Genesis Block',
  data: 'GENESIS',
  prevHash: '0',
  nonce: 3770,
  hash: '00025fcebd821b950eea1df93c0e89b667d58f2eccd3f0f8014d2bbca12394f2'
};

const DEFAULT_SAMPLE_CONTENT = `SUBJECT|1|Data Structures|2
SUBJECT|2|Algorithms|2
SUBJECT|3|Databases|2
SUBJECT|4|Operating Systems|1
SUBJECT|5|Networks|1
FACULTY|1|Dr. Rao
FACULTY|2|Prof. Devi
FACULTY|3|Dr. Iyer
FACULTY|4|Prof. Khan
CAN|1|1
CAN|1|2
CAN|2|3
CAN|2|4
CAN|3|4
CAN|3|5
CAN|4|2
CAN|4|5
UNAVAILABLE|2|4
UNAVAILABLE|2|5
UNAVAILABLE|2|6
UNAVAILABLE|2|7
UNAVAILABLE|2|8
UNAVAILABLE|2|9
UNAVAILABLE|1|6
SLOT|1|Mon|1
SLOT|2|Mon|2
SLOT|3|Mon|3
SLOT|4|Tue|1
SLOT|5|Tue|2
SLOT|6|Tue|3
SLOT|7|Wed|1
SLOT|8|Wed|2
SLOT|9|Wed|3`;

export const AppProvider: React.FC<{ children: ReactNode }> = ({ children }) => {
  const [tab, setTab] = useState<TabType>('generator');
  const [theme, setTheme] = useState<'light' | 'dark'>(() => {
    return (localStorage.getItem('ttc_theme') as 'light' | 'dark') || 'light';
  });

  // Toasts
  const [toasts, setToasts] = useState<ToastMessage[]>([]);
  const showToast = useCallback((text: string, type: 'info' | 'success' | 'warning' | 'error' = 'info') => {
    const id = Math.random().toString(36).substring(2, 9);
    setToasts((prev) => [...prev, { id, type, text }]);
    setTimeout(() => {
      setToasts((prev) => prev.filter((t) => t.id !== id));
    }, 3500);
  }, []);

  const dismissToast = (id: string) => {
    setToasts((prev) => prev.filter((t) => t.id !== id));
  };

  const toggleTheme = () => {
    const next = theme === 'light' ? 'dark' : 'light';
    setTheme(next);
    localStorage.setItem('ttc_theme', next);
    if (next === 'dark') {
      document.documentElement.classList.add('dark');
    } else {
      document.documentElement.classList.remove('dark');
    }
  };

  useEffect(() => {
    if (theme === 'dark') {
      document.documentElement.classList.add('dark');
    } else {
      document.documentElement.classList.remove('dark');
    }
  }, [theme]);

  // Dataset State
  const [currentDatasetName, setCurrentDatasetName] = useState<string>('sample.txt');
  const [datasetContent, setDatasetContent] = useState<string>(DEFAULT_SAMPLE_CONTENT);
  const [dataset, setDataset] = useState<Dataset | null>(null);
  const [datasetError, setDatasetError] = useState<string | null>(null);
  const [savedDatasets, setSavedDatasets] = useState<SavedDataset[]>(() => {
    const stored = localStorage.getItem('ttc_saved_datasets');
    if (stored) {
      try { return JSON.parse(stored); } catch { return []; }
    }
    return [
      {
        id: 'default-sample',
        name: 'sample.txt',
        rawContent: DEFAULT_SAMPLE_CONTENT,
        updatedAt: new Date().toISOString()
      }
    ];
  });

  // Generator State
  const [generatedTimetable, setGeneratedTimetable] = useState<TimetableEntry[]>([]);
  const [generatorStats, setGeneratorStats] = useState<GeneratorStats | null>(null);
  const [generatorMode, setGeneratorMode] = useState<'Plain' | 'MRV' | 'Best'>('MRV');
  const [isGenerating, setIsGenerating] = useState(false);

  // Manual Builder State
  const [manualTimetable, setManualTimetable] = useState<TimetableEntry[]>([]);
  const [manualValidation, setManualValidation] = useState<{ valid: boolean; problems: string[]; penaltyScore: number } | null>(null);

  // Blockchain State
  const [blocks, setBlocks] = useState<Block[]>(() => {
    const stored = localStorage.getItem('ttc_blocks');
    if (stored) {
      try { return JSON.parse(stored); } catch { return [GENESIS_BLOCK]; }
    }
    return [GENESIS_BLOCK];
  });
  const [verificationReport, setVerificationReport] = useState<VerificationReport | null>(null);
  const [isVerifying, setIsVerifying] = useState(false);

  // Persist blocks
  useEffect(() => {
    localStorage.setItem('ttc_blocks', JSON.stringify(blocks));
  }, [blocks]);

  // Persist saved datasets
  useEffect(() => {
    localStorage.setItem('ttc_saved_datasets', JSON.stringify(savedDatasets));
  }, [savedDatasets]);

  // Parse current dataset whenever content changes
  const loadDataset = useCallback(async (name: string, content: string): Promise<boolean> => {
    try {
      const res = await api.parseDataset(content);
      if (res.valid && res.dataset) {
        setDatasetContent(content);
        setCurrentDatasetName(name);
        setDataset(res.dataset);
        setDatasetError(null);
        setGeneratedTimetable([]);
        setGeneratorStats(null);
        setManualTimetable([]);
        setManualValidation(null);
        setVerificationReport(null);
        return true;
      } else {
        setDatasetError(res.error || 'Failed to parse dataset');
        return false;
      }
    } catch (e: any) {
      setDatasetError(e.message || 'Network error while parsing dataset');
      return false;
    }
  }, []);

  // Initial load
  useEffect(() => {
    loadDataset(currentDatasetName, datasetContent);
  }, []);

  const saveCurrentDataset = (name: string) => {
    const existingIndex = savedDatasets.findIndex((d) => d.name === name);
    const updated = {
      id: existingIndex >= 0 ? savedDatasets[existingIndex].id : Math.random().toString(36).substring(2, 9),
      name,
      rawContent: datasetContent,
      updatedAt: new Date().toISOString()
    };
    if (existingIndex >= 0) {
      setSavedDatasets((prev) => prev.map((d, i) => (i === existingIndex ? updated : d)));
    } else {
      setSavedDatasets((prev) => [...prev, updated]);
    }
    setCurrentDatasetName(name);
    showToast(`Dataset '${name}' saved`, 'success');
  };

  const deleteSavedDataset = (id: string) => {
    setSavedDatasets((prev) => prev.filter((d) => d.id !== id));
    showToast('Dataset deleted', 'info');
  };

  const createNewDataset = () => {
    const blank = `# New Curriculum Dataset
SUBJECT|1|Course Name|2
FACULTY|1|Instructor Name
CAN|1|1
SLOT|1|Mon|1
SLOT|2|Mon|2
`;
    loadDataset('untitled.txt', blank);
    showToast('New blank dataset created', 'info');
  };

  const loadSampleDataset = async (fullWeek = false) => {
    try {
      const data = fullWeek ? await api.getSampleFullWeekDataset() : await api.getSampleDataset();
      await loadDataset(data.name, data.content);
      showToast(`Loaded ${data.name}`, 'success');
    } catch {
      showToast('Could not fetch sample dataset from server', 'error');
    }
  };

  // Timetable Generator
  const runGenerate = async (mineAfter = false): Promise<boolean> => {
    if (!dataset) return false;
    setIsGenerating(true);
    try {
      const prevHash = blocks.length > 0 ? blocks[blocks.length - 1].hash : '0';
      const nextIndex = blocks.length;
      const note = `Generated Schedule (${generatorMode})`;

      const res = await api.generateTimetable({
        content: datasetContent,
        mode: generatorMode,
        mineAfter,
        note,
        prevHash,
        index: nextIndex
      });

      if (res.success && res.entries) {
        setGeneratedTimetable(res.entries);
        setGeneratorStats(res.stats);
        if (mineAfter && res.minedBlock) {
          setBlocks((prev) => [...prev, res.minedBlock!]);
          setVerificationReport(null);
          showToast(`Generated and mined Block #${res.minedBlock.index} into Blockchain!`, 'success');
        } else {
          showToast(`Timetable generated successfully (${res.stats.score} penalty score)`, 'success');
        }
        return true;
      } else {
        showToast(res.error || 'Failed to generate timetable', 'error');
        return false;
      }
    } catch (e: any) {
      showToast(e.message || 'Generation failed', 'error');
      return false;
    } finally {
      setIsGenerating(false);
    }
  };

  // Manual Builder
  const clearManualTimetable = () => {
    setManualTimetable([]);
    setManualValidation(null);
    showToast('Cleared manual schedule', 'info');
  };

  const validateManualSchedule = async (): Promise<boolean> => {
    try {
      const res = await api.validateSchedule({
        content: datasetContent,
        entries: manualTimetable
      });
      setManualValidation({
        valid: res.valid,
        problems: res.problems,
        penaltyScore: res.penaltyScore
      });
      if (res.valid) {
        showToast(`Schedule is completely valid! (Penalty: ${res.penaltyScore})`, 'success');
        return true;
      } else {
        showToast(`Schedule has ${res.problems.length} constraint violation(s)`, 'error');
        return false;
      }
    } catch (e: any) {
      showToast(e.message || 'Validation error', 'error');
      return false;
    }
  };

  const commitManualToBlockchain = async (note: string): Promise<boolean> => {
    const isValid = await validateManualSchedule();
    if (!isValid) return false;

    try {
      const prevHash = blocks.length > 0 ? blocks[blocks.length - 1].hash : '0';
      const nextIndex = blocks.length;
      const res = await api.validateSchedule({
        content: datasetContent,
        entries: manualTimetable
      });

      const mined = await api.mineBlock({
        index: nextIndex,
        note: note || 'Manual Schedule',
        data: res.serialized,
        prevHash
      });

      setBlocks((prev) => [...prev, mined]);
      setVerificationReport(null);
      showToast(`Mined Block #${mined.index} from Manual Schedule!`, 'success');
      return true;
    } catch (e: any) {
      showToast(e.message || 'Failed to mine block', 'error');
      return false;
    }
  };

  // Blockchain Operations
  const verifyChain = async (): Promise<VerificationReport> => {
    setIsVerifying(true);
    try {
      const report = await api.verifyBlockchain({
        content: datasetContent,
        blocks
      });
      setVerificationReport(report);
      if (report.allValid) {
        showToast('✓ Blockchain verified: all 3 layers intact!', 'success');
      } else {
        showToast(`⚠ Verification failed: ${report.messages.length} problem(s) detected`, 'error');
      }
      return report;
    } catch (e: any) {
      const failReport: VerificationReport = {
        allValid: false,
        messages: [e.message || 'Network error during verification'],
        blockStatuses: {}
      };
      setVerificationReport(failReport);
      showToast('Verification failed to execute', 'error');
      return failReport;
    } finally {
      setIsVerifying(false);
    }
  };

  const tamperChain = async (targetIndex: number, remine: boolean, customData?: string) => {
    try {
      const res = await api.tamperBlock({
        blocks,
        targetIndex,
        remine,
        customData
      });
      if (res.success) {
        setBlocks(res.blocks);
        setVerificationReport(null);
        showToast(
          remine
            ? `Block #${targetIndex} tampered & re-mined (Clever Attacker). Re-run verification!`
            : `Block #${targetIndex} data corrupted without re-mining (Careless Attacker). Re-run verification!`,
          'warning'
        );
      }
    } catch (e: any) {
      showToast(e.message || 'Tamper failed', 'error');
    }
  };

  const resetChain = () => {
    setBlocks([GENESIS_BLOCK]);
    setVerificationReport(null);
    showToast('Blockchain reset to Genesis block', 'info');
  };

  const exportChainFile = async (): Promise<string> => {
    const res = await api.saveBlockchain(blocks);
    return res.content;
  };

  const importChainFile = async (content: string): Promise<boolean> => {
    try {
      const res = await api.loadBlockchain(content);
      if (res.success && res.blocks) {
        setBlocks(res.blocks);
        setVerificationReport(null);
        showToast(`Loaded chain with ${res.blocks.length} blocks`, 'success');
        return true;
      } else {
        showToast(res.error || 'Failed to parse chain file', 'error');
        return false;
      }
    } catch (e: any) {
      showToast(e.message || 'Failed to load chain', 'error');
      return false;
    }
  };

  const isChainVerified = verificationReport ? verificationReport.allValid : false;

  return (
    <AppContext.Provider
      value={{
        tab,
        setTab,
        theme,
        toggleTheme,

        currentDatasetName,
        datasetContent,
        dataset,
        datasetError,
        savedDatasets,
        loadDataset,
        saveCurrentDataset,
        deleteSavedDataset,
        createNewDataset,
        loadSampleDataset,

        generatedTimetable,
        generatorStats,
        generatorMode,
        setGeneratorMode,
        isGenerating,
        runGenerate,

        manualTimetable,
        setManualTimetable,
        clearManualTimetable,
        manualValidation,
        validateManualSchedule,
        commitManualToBlockchain,

        blocks,
        verificationReport,
        isChainVerified,
        isVerifying,
        verifyChain,
        tamperChain,
        resetChain,
        exportChainFile,
        importChainFile,

        toasts,
        showToast,
        dismissToast,
      }}
    >
      {children}
    </AppContext.Provider>
  );
};

export const useApp = () => {
  const ctx = useContext(AppContext);
  if (!ctx) throw new Error('useApp must be used within AppProvider');
  return ctx;
};
