export interface Subject {
  id: number;
  name: string;
  periodsPerWeek: number;
}

export interface Faculty {
  id: number;
  name: string;
  canTeach: number[];      // Subject IDs
  unavailable: number[];   // Slot IDs
}

export interface TimeSlot {
  id: number;
  day: string;             // e.g. "Mon"
  period: number;          // 1-based index e.g. 1
  name: string;            // e.g. "Mon P1"
}

export interface Dataset {
  subjects: Subject[];
  faculty: Faculty[];
  slots: TimeSlot[];
  totalPeriodsNeeded: number;
  totalSlotsAvailable: number;
  isFeasible: boolean;
}

export interface TimetableEntry {
  subjectId: number;
  facultyId: number;
  slotId: number;
}

export interface GeneratorStats {
  placements: number;
  backtracks: number;
  score: number;
  solutionsFound: number;
  durationMs: number;
}

export interface Block {
  index: number;
  timestamp: number;
  note: string;
  data: string;
  prevHash: string;
  nonce: number;
  hash: string;
}

export interface BlockVerificationStatus {
  index: number;
  valid: boolean;
  problems: string[];
}

export interface VerificationReport {
  allValid: boolean;
  messages: string[];
  blockStatuses: Record<string, BlockVerificationStatus>;
}

export interface StrategyResult {
  name: string;
  description: string;
  success: boolean;
  placements: number;
  backtracks: number;
  score: number;
  solutionsFound: number;
  durationMs: number;
}

export interface SavedDataset {
  id: string;
  name: string;
  rawContent: string;
  updatedAt: string;
}
