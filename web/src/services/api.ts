import { Dataset, TimetableEntry, GeneratorStats, Block, VerificationReport, StrategyResult } from '../types';

const API_BASE = '/api';

export const api = {
  async getHealth() {
    const res = await fetch(`${API_BASE}/health`);
    if (!res.ok) throw new Error('Backend health check failed');
    return res.json();
  },

  async getSampleDataset(): Promise<{ name: string; content: string }> {
    const res = await fetch(`${API_BASE}/sample`);
    if (!res.ok) throw new Error('Failed to load sample dataset');
    return res.json();
  },

  async getSampleFullWeekDataset(): Promise<{ name: string; content: string }> {
    const res = await fetch(`${API_BASE}/sample-full-week`);
    if (!res.ok) throw new Error('Failed to load full week sample dataset');
    return res.json();
  },

  async parseDataset(content: string): Promise<{ valid: boolean; error: string; dataset?: Dataset }> {
    const res = await fetch(`${API_BASE}/dataset/parse`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ content }),
    });
    return res.json();
  },

  async validateDataset(content: string): Promise<{
    valid: boolean;
    errors: string[];
    warnings: string[];
    stats: {
      subjects: number;
      faculty: number;
      slots: number;
      periodsNeeded: number;
      slotsAvailable: number;
    };
  }> {
    const res = await fetch(`${API_BASE}/dataset/validate`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ content }),
    });
    return res.json();
  },

  async generateTimetable(params: {
    content: string;
    mode: 'Plain' | 'MRV' | 'Best';
    mineAfter?: boolean;
    note?: string;
    prevHash?: string;
    index?: number;
  }): Promise<{
    success: boolean;
    entries?: TimetableEntry[];
    serialized?: string;
    stats: GeneratorStats;
    minedBlock?: Block;
    error?: string;
  }> {
    const res = await fetch(`${API_BASE}/generate`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(params),
    });
    return res.json();
  },

  async compareStrategies(content: string): Promise<{
    results: StrategyResult[];
  }> {
    const res = await fetch(`${API_BASE}/compare`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ content }),
    });
    return res.json();
  },

  async checkCanPlace(params: {
    content: string;
    entries: TimetableEntry[];
    subjectId: number;
    facultyId: number;
    slotId: number;
  }): Promise<{
    valid: boolean;
    reason: string;
  }> {
    const res = await fetch(`${API_BASE}/manual/can-place`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(params),
    });
    return res.json();
  },

  async validateSchedule(params: {
    content: string;
    entries: TimetableEntry[];
  }): Promise<{
    valid: boolean;
    problems: string[];
    penaltyScore: number;
    serialized: string;
  }> {
    const res = await fetch(`${API_BASE}/manual/validate`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(params),
    });
    return res.json();
  },

  async mineBlock(params: {
    index: number;
    timestamp?: number;
    note: string;
    data: string;
    prevHash: string;
  }): Promise<Block> {
    const res = await fetch(`${API_BASE}/blockchain/mine`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(params),
    });
    return res.json();
  },

  async verifyBlockchain(params: {
    content: string;
    blocks: Block[];
  }): Promise<VerificationReport> {
    const res = await fetch(`${API_BASE}/blockchain/verify`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(params),
    });
    return res.json();
  },

  async tamperBlock(params: {
    blocks: Block[];
    targetIndex: number;
    remine: boolean;
    customData?: string;
  }): Promise<{
    success: boolean;
    tamperedIndex: number;
    remined: boolean;
    blocks: Block[];
  }> {
    const res = await fetch(`${API_BASE}/blockchain/tamper`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(params),
    });
    return res.json();
  },

  async saveBlockchain(blocks: Block[]): Promise<{ content: string }> {
    const res = await fetch(`${API_BASE}/blockchain/save`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ blocks }),
    });
    return res.json();
  },

  async loadBlockchain(content: string): Promise<{ success: boolean; blocks?: Block[]; error?: string }> {
    const res = await fetch(`${API_BASE}/blockchain/load`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ content }),
    });
    return res.json();
  },
};
