import React, { useState, useEffect } from 'react';
import { useApp } from '../context/AppContext';
import { api } from '../services/api';
import {
  Upload,
  Download,
  Plus,
  Trash2,
  Edit2,
  Save,
  CheckCircle2,
  AlertTriangle,
  FileText,
  Database,
  Layers,
  Users,
  Calendar,
  AlertCircle,
  HelpCircle,
} from 'lucide-react';

export const CurriculumDataPage: React.FC = () => {
  const {
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
    showToast,
  } = useApp();

  const [activeSubTab, setActiveSubTab] = useState<'tables' | 'raw' | 'entityEditor'>('tables');
  const [editorContent, setEditorContent] = useState<string>(datasetContent);
  const [validationReport, setValidationReport] = useState<{
    valid: boolean;
    errors: string[];
    warnings: string[];
  } | null>(null);

  const [saveAsName, setSaveAsName] = useState<string>(currentDatasetName);
  const [showSaveModal, setShowSaveModal] = useState<boolean>(false);

  // Sync editor content with current dataset content
  useEffect(() => {
    setEditorContent(datasetContent);
    setSaveAsName(currentDatasetName);
  }, [datasetContent, currentDatasetName]);

  // Run live validation whenever editorContent changes
  useEffect(() => {
    let active = true;
    const runValidate = async () => {
      try {
        const res = await api.validateDataset(editorContent);
        if (active) {
          setValidationReport({
            valid: res.valid,
            errors: res.errors,
            warnings: res.warnings,
          });
        }
      } catch {
        if (active) {
          setValidationReport(null);
        }
      }
    };

    const timer = setTimeout(runValidate, 300);
    return () => {
      active = false;
      clearTimeout(timer);
    };
  }, [editorContent]);

  // Handle uploading .txt file
  const handleFileUpload = (e: React.ChangeEvent<HTMLInputElement>) => {
    const file = e.target.files?.[0];
    if (!file) return;

    const reader = new FileReader();
    reader.onload = async (event) => {
      const text = event.target?.result as string;
      const success = await loadDataset(file.name, text);
      if (success) {
        showToast(`Successfully uploaded and validated '${file.name}'`, 'success');
        setEditorContent(text);
      } else {
        showToast(`Validation failed for '${file.name}'`, 'error');
      }
    };
    reader.readAsText(file);
    e.target.value = '';
  };

  // Handle downloading dataset as .txt
  const handleExportText = () => {
    const blob = new Blob([editorContent], { type: 'text/plain' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = currentDatasetName.endsWith('.txt') ? currentDatasetName : `${currentDatasetName}.txt`;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
    showToast(`Exported '${a.download}'`, 'success');
  };

  // Download template
  const handleDownloadTemplate = () => {
    const template = `# Timetable Chain Curriculum Dataset Template
# Format Specification:
# SUBJECT|id|name|periodsPerWeek
# FACULTY|id|name
# CAN|facultyId|subjectId
# UNAVAILABLE|facultyId|slotId
# SLOT|id|day|period

SUBJECT|1|Mathematics|3
SUBJECT|2|Physics|2
FACULTY|1|Dr. Smith
FACULTY|2|Prof. Johnson
CAN|1|1
CAN|2|2
SLOT|1|Mon|1
SLOT|2|Mon|2
SLOT|3|Tue|1
SLOT|4|Tue|2
SLOT|5|Wed|1
`;
    const blob = new Blob([template], { type: 'text/plain' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = 'curriculum_template.txt';
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
    showToast('Downloaded curriculum_template.txt', 'info');
  };

  const handleApplyRawContent = async () => {
    const success = await loadDataset(currentDatasetName, editorContent);
    if (success) {
      showToast('Applied and parsed curriculum updates', 'success');
    }
  };

  const periodsNeeded = dataset?.totalPeriodsNeeded || 0;
  const slotsAvailable = dataset?.slots.length || 0;
  const isDeficit = periodsNeeded > slotsAvailable;

  return (
    <div className="space-y-6">
      {/* Top Dataset Management Bar */}
      <div className="bg-white dark:bg-[#181C2E] border border-[#E4E7F0] dark:border-[#2A2F47] rounded-2xl p-4 sm:p-5 shadow-xs transition-colors">
        <div className="flex flex-col lg:flex-row lg:items-center justify-between gap-4">
          {/* Dataset Switcher */}
          <div className="flex flex-wrap items-center gap-3">
            <label className="text-sm font-bold text-[#1B1F3B] dark:text-[#E8EAF6] shrink-0">
              Active Dataset:
            </label>
            <select
              value={currentDatasetName}
              onChange={(e) => {
                const found = savedDatasets.find((d) => d.name === e.target.value);
                if (found) {
                  loadDataset(found.name, found.rawContent);
                }
              }}
              className="px-3 py-2 rounded-xl border border-[#CBD5E1] dark:border-[#334155] bg-[#F8F9FD] dark:bg-[#20253B] text-sm font-semibold text-[#1B1F3B] dark:text-[#E8EAF6] focus:outline-none focus:ring-2 focus:ring-[#5B5BD6]"
            >
              {savedDatasets.map((d) => (
                <option key={d.id} value={d.name}>
                  {d.name}
                </option>
              ))}
            </select>

            <button
              onClick={() => setShowSaveModal(true)}
              className="flex items-center space-x-1.5 px-3 py-2 rounded-xl bg-[#F0F2FA] dark:bg-[#20253B] hover:bg-[#E4E7F0] dark:hover:bg-[#2A3150] text-[#1B1F3B] dark:text-[#E8EAF6] text-xs font-semibold border border-[#E4E7F0] dark:border-[#2A2F47] transition-all cursor-pointer"
            >
              <Save className="w-3.5 h-3.5 text-[#5B5BD6]" />
              <span>Save As...</span>
            </button>

            <button
              onClick={createNewDataset}
              className="flex items-center space-x-1.5 px-3 py-2 rounded-xl bg-[#F0F2FA] dark:bg-[#20253B] hover:bg-[#E4E7F0] dark:hover:bg-[#2A3150] text-[#1B1F3B] dark:text-[#E8EAF6] text-xs font-semibold border border-[#E4E7F0] dark:border-[#2A2F47] transition-all cursor-pointer"
            >
              <Plus className="w-3.5 h-3.5 text-emerald-600" />
              <span>Create New</span>
            </button>
          </div>

          {/* Action Buttons */}
          <div className="flex flex-wrap items-center gap-2">
            <label className="flex items-center space-x-1.5 px-3.5 py-2 rounded-xl bg-[#5B5BD6] hover:bg-[#4D4DBF] text-white text-xs font-semibold shadow-xs transition-all cursor-pointer">
              <Upload className="w-3.5 h-3.5" />
              <span>Upload .txt File</span>
              <input
                type="file"
                accept=".txt"
                onChange={handleFileUpload}
                className="hidden"
              />
            </label>

            <button
              onClick={handleExportText}
              className="flex items-center space-x-1.5 px-3 py-2 rounded-xl bg-[#F0F2FA] dark:bg-[#20253B] hover:bg-[#E4E7F0] dark:hover:bg-[#2A3150] text-[#1B1F3B] dark:text-[#E8EAF6] text-xs font-semibold border border-[#E4E7F0] dark:border-[#2A2F47] transition-all cursor-pointer"
            >
              <Download className="w-3.5 h-3.5" />
              <span>Export .txt</span>
            </button>

            <button
              onClick={() => loadSampleDataset(false)}
              className="flex items-center space-x-1 px-3 py-2 rounded-xl bg-[#F0F2FA] dark:bg-[#20253B] hover:bg-[#E4E7F0] dark:hover:bg-[#2A3150] text-[#6B7194] dark:text-[#9AA0C3] text-xs font-semibold border border-[#E4E7F0] dark:border-[#2A2F47] transition-all cursor-pointer"
            >
              <span>Load Sample</span>
            </button>

            <button
              onClick={() => loadSampleDataset(true)}
              className="flex items-center space-x-1 px-3 py-2 rounded-xl bg-[#F0F2FA] dark:bg-[#20253B] hover:bg-[#E4E7F0] dark:hover:bg-[#2A3150] text-[#6B7194] dark:text-[#9AA0C3] text-xs font-semibold border border-[#E4E7F0] dark:border-[#2A2F47] transition-all cursor-pointer"
            >
              <span>Load Full Week</span>
            </button>
          </div>
        </div>
      </div>

      {/* Deficit / Error Warning Banner */}
      {isDeficit && (
        <div className="p-4 rounded-2xl bg-amber-50 dark:bg-amber-950/40 border border-amber-300 dark:border-amber-800 text-amber-800 dark:text-amber-200 text-sm flex items-start space-x-3">
          <AlertTriangle className="w-5 h-5 text-amber-600 dark:text-amber-400 shrink-0 mt-0.5" />
          <div>
            <span className="font-bold block">Capacity Warning: Periods Exceed Available Slots</span>
            <span className="text-xs">
              Required periods ({periodsNeeded}) exceed total weekly time slots ({slotsAvailable}). An impossible constraint schedule cannot be solved.
            </span>
          </div>
        </div>
      )}

      {datasetError && (
        <div className="p-4 rounded-2xl bg-rose-50 dark:bg-rose-950/40 border border-rose-300 dark:border-rose-800 text-rose-800 dark:text-rose-200 text-sm flex items-start space-x-3 font-mono">
          <AlertCircle className="w-5 h-5 text-rose-600 dark:text-rose-400 shrink-0 mt-0.5" />
          <div>
            <span className="font-bold block">Dataset Syntax / Semantic Error:</span>
            <span className="text-xs">{datasetError}</span>
          </div>
        </div>
      )}

      {/* Summary Stat Cards */}
      {dataset && (
        <div className="grid grid-cols-2 sm:grid-cols-4 gap-3.5">
          <div className="bg-white dark:bg-[#181C2E] border border-[#E4E7F0] dark:border-[#2A2F47] rounded-2xl p-4 shadow-xs">
            <span className="text-[11px] font-bold uppercase tracking-wider text-[#9AA0C3] block">
              Subjects
            </span>
            <span className="text-2xl font-black text-[#1B1F3B] dark:text-[#E8EAF6] mt-0.5 block">
              {dataset.subjects.length}
            </span>
            <span className="text-[11px] text-[#6B7194] dark:text-[#9AA0C3]">
              Distinct courses offered
            </span>
          </div>

          <div className="bg-white dark:bg-[#181C2E] border border-[#E4E7F0] dark:border-[#2A2F47] rounded-2xl p-4 shadow-xs">
            <span className="text-[11px] font-bold uppercase tracking-wider text-[#9AA0C3] block">
              Faculty Staff
            </span>
            <span className="text-2xl font-black text-[#1B1F3B] dark:text-[#E8EAF6] mt-0.5 block">
              {dataset.faculty.length}
            </span>
            <span className="text-[11px] text-[#6B7194] dark:text-[#9AA0C3]">
              Instructors with qualifications
            </span>
          </div>

          <div className="bg-white dark:bg-[#181C2E] border border-[#E4E7F0] dark:border-[#2A2F47] rounded-2xl p-4 shadow-xs">
            <span className="text-[11px] font-bold uppercase tracking-wider text-[#9AA0C3] block">
              Weekly Slots
            </span>
            <span className="text-2xl font-black text-[#1B1F3B] dark:text-[#E8EAF6] mt-0.5 block">
              {dataset.slots.length}
            </span>
            <span className="text-[11px] text-[#6B7194] dark:text-[#9AA0C3]">
              Available timetable periods
            </span>
          </div>

          <div className="bg-white dark:bg-[#181C2E] border border-[#E4E7F0] dark:border-[#2A2F47] rounded-2xl p-4 shadow-xs">
            <span className="text-[11px] font-bold uppercase tracking-wider text-[#9AA0C3] block">
              Periods Needed
            </span>
            <span className={`text-2xl font-black mt-0.5 block ${isDeficit ? 'text-amber-600' : 'text-[#5B5BD6] dark:text-[#8AB4F8]'}`}>
              {periodsNeeded}
            </span>
            <span className="text-[11px] text-[#6B7194] dark:text-[#9AA0C3]">
              {isDeficit ? 'Deficit detected' : 'Feasible capacity'}
            </span>
          </div>
        </div>
      )}

      {/* Sub Navigation */}
      <div className="flex items-center space-x-2 border-b border-[#E4E7F0] dark:border-[#2A2F47] pb-1">
        <button
          onClick={() => setActiveSubTab('tables')}
          className={`flex items-center space-x-2 px-4 py-2.5 rounded-xl font-semibold text-xs transition-colors cursor-pointer ${
            activeSubTab === 'tables'
              ? 'bg-[#EEF0FD] dark:bg-[#2C204B] text-[#5B5BD6] dark:text-[#8AB4F8]'
              : 'text-[#6B7194] dark:text-[#9AA0C3] hover:text-[#1B1F3B] dark:hover:text-[#E8EAF6]'
          }`}
        >
          <Layers className="w-4 h-4" />
          <span>Curricular Tables</span>
        </button>

        <button
          onClick={() => setActiveSubTab('raw')}
          className={`flex items-center space-x-2 px-4 py-2.5 rounded-xl font-semibold text-xs transition-colors cursor-pointer ${
            activeSubTab === 'raw'
              ? 'bg-[#EEF0FD] dark:bg-[#2C204B] text-[#5B5BD6] dark:text-[#8AB4F8]'
              : 'text-[#6B7194] dark:text-[#9AA0C3] hover:text-[#1B1F3B] dark:hover:text-[#E8EAF6]'
          }`}
        >
          <FileText className="w-4 h-4" />
          <span>Raw Text Editor</span>
        </button>
      </div>

      {/* Sub Tab 1: Tables View */}
      {activeSubTab === 'tables' && dataset && (
        <div className="space-y-6">
          {/* Table 1: Subjects */}
          <div className="bg-white dark:bg-[#181C2E] border border-[#E4E7F0] dark:border-[#2A2F47] rounded-2xl p-5 shadow-xs transition-colors">
            <h3 className="text-sm font-bold text-[#1B1F3B] dark:text-[#E8EAF6] mb-3">
              1. Academic Subjects ({dataset.subjects.length})
            </h3>
            <div className="overflow-x-auto rounded-xl border border-[#E4E7F0] dark:border-[#2A2F47]">
              <table className="w-full text-left text-sm">
                <thead className="bg-[#F8F9FD] dark:bg-[#121626] text-xs uppercase font-bold text-[#6B7194] dark:text-[#9AA0C3] border-b border-[#E4E7F0] dark:border-[#2A2F47]">
                  <tr>
                    <th className="py-2.5 px-4 w-20">ID</th>
                    <th className="py-2.5 px-4">Subject Name</th>
                    <th className="py-2.5 px-4 w-44">Periods / Week</th>
                  </tr>
                </thead>
                <tbody className="divide-y divide-[#E4E7F0] dark:divide-[#2A2F47]">
                  {dataset.subjects.map((s) => (
                    <tr key={s.id} className="hover:bg-slate-50/50 dark:hover:bg-slate-800/20">
                      <td className="py-2.5 px-4 font-mono font-bold text-[#5B5BD6] dark:text-[#8AB4F8]">
                        #{s.id}
                      </td>
                      <td className="py-2.5 px-4 font-semibold text-[#1B1F3B] dark:text-[#E8EAF6]">
                        {s.name}
                      </td>
                      <td className="py-2.5 px-4 font-mono text-xs">
                        <span className="px-2.5 py-1 rounded-full bg-[#EEF0FD] dark:bg-[#20253B] text-[#5B5BD6] dark:text-[#8AB4F8] font-bold">
                          {s.periodsPerWeek} periods
                        </span>
                      </td>
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>
          </div>

          {/* Table 2: Faculty */}
          <div className="bg-white dark:bg-[#181C2E] border border-[#E4E7F0] dark:border-[#2A2F47] rounded-2xl p-5 shadow-xs transition-colors">
            <h3 className="text-sm font-bold text-[#1B1F3B] dark:text-[#E8EAF6] mb-3">
              2. Faculty Members & Qualifications ({dataset.faculty.length})
            </h3>
            <div className="overflow-x-auto rounded-xl border border-[#E4E7F0] dark:border-[#2A2F47]">
              <table className="w-full text-left text-sm">
                <thead className="bg-[#F8F9FD] dark:bg-[#121626] text-xs uppercase font-bold text-[#6B7194] dark:text-[#9AA0C3] border-b border-[#E4E7F0] dark:border-[#2A2F47]">
                  <tr>
                    <th className="py-2.5 px-4 w-20">ID</th>
                    <th className="py-2.5 px-4 w-48">Faculty Name</th>
                    <th className="py-2.5 px-4">Qualified Subjects (CAN)</th>
                    <th className="py-2.5 px-4">Unavailable Slots</th>
                  </tr>
                </thead>
                <tbody className="divide-y divide-[#E4E7F0] dark:divide-[#2A2F47]">
                  {dataset.faculty.map((f) => {
                    const qualifiedNames = f.canTeach.map((subId) => {
                      const s = dataset.subjects.find((x) => x.id === subId);
                      return s ? s.name : `Subj #${subId}`;
                    });
                    const unavailNames = f.unavailable.map((slId) => {
                      const sl = dataset.slots.find((x) => x.id === slId);
                      return sl ? sl.name : `Slot #${slId}`;
                    });
                    return (
                      <tr key={f.id} className="hover:bg-slate-50/50 dark:hover:bg-slate-800/20">
                        <td className="py-2.5 px-4 font-mono font-bold text-[#5B5BD6] dark:text-[#8AB4F8]">
                          #{f.id}
                        </td>
                        <td className="py-2.5 px-4 font-semibold text-[#1B1F3B] dark:text-[#E8EAF6]">
                          {f.name}
                        </td>
                        <td className="py-2.5 px-4">
                          <div className="flex flex-wrap gap-1.5">
                            {qualifiedNames.length > 0 ? (
                              qualifiedNames.map((name, i) => (
                                <span
                                  key={i}
                                  className="text-[11px] px-2 py-0.5 rounded-md bg-emerald-50 dark:bg-emerald-950/50 text-emerald-700 dark:text-emerald-300 border border-emerald-200 dark:border-emerald-800 font-medium"
                                >
                                  {name}
                                </span>
                              ))
                            ) : (
                              <span className="text-xs text-rose-500 italic">None assigned</span>
                            )}
                          </div>
                        </td>
                        <td className="py-2.5 px-4">
                          <div className="flex flex-wrap gap-1.5">
                            {unavailNames.length > 0 ? (
                              unavailNames.map((name, i) => (
                                <span
                                  key={i}
                                  className="text-[11px] px-2 py-0.5 rounded-md bg-amber-50 dark:bg-amber-950/50 text-amber-700 dark:text-amber-300 border border-amber-200 dark:border-amber-800 font-medium"
                                >
                                  {name}
                                </span>
                              ))
                            ) : (
                              <span className="text-xs text-[#9AA0C3]">All slots available</span>
                            )}
                          </div>
                        </td>
                      </tr>
                    );
                  })}
                </tbody>
              </table>
            </div>
          </div>

          {/* Table 3: Slots */}
          <div className="bg-white dark:bg-[#181C2E] border border-[#E4E7F0] dark:border-[#2A2F47] rounded-2xl p-5 shadow-xs transition-colors">
            <h3 className="text-sm font-bold text-[#1B1F3B] dark:text-[#E8EAF6] mb-3">
              3. Weekly Time Slots ({dataset.slots.length})
            </h3>
            <div className="grid grid-cols-2 sm:grid-cols-3 md:grid-cols-6 gap-2.5">
              {dataset.slots.map((s) => (
                <div
                  key={s.id}
                  className="p-3 rounded-xl bg-[#F8F9FD] dark:bg-[#20253B] border border-[#E4E7F0] dark:border-[#2A2F47] text-center"
                >
                  <span className="text-xs font-bold text-[#1B1F3B] dark:text-[#E8EAF6] block">
                    {s.name}
                  </span>
                  <span className="text-[10px] font-mono text-[#9AA0C3] mt-0.5 block">
                    Slot ID #{s.id}
                  </span>
                </div>
              ))}
            </div>
          </div>
        </div>
      )}

      {/* Sub Tab 2: Raw Text Editor */}
      {activeSubTab === 'raw' && (
        <div className="space-y-4">
          <div className="bg-white dark:bg-[#181C2E] border border-[#E4E7F0] dark:border-[#2A2F47] rounded-2xl p-5 shadow-xs transition-colors space-y-4">
            <div className="flex items-center justify-between">
              <div>
                <h3 className="text-sm font-bold text-[#1B1F3B] dark:text-[#E8EAF6]">
                  Dataset Text Editor
                </h3>
                <p className="text-xs text-[#6B7194] dark:text-[#9AA0C3]">
                  Directly edit or paste records matching the pipe-delimited format (SUBJECT, FACULTY, CAN, UNAVAILABLE, SLOT)
                </p>
              </div>

              <div className="flex items-center space-x-2">
                <button
                  onClick={handleDownloadTemplate}
                  className="flex items-center space-x-1.5 px-3 py-1.5 text-xs font-semibold rounded-lg bg-[#F0F2FA] dark:bg-[#20253B] hover:bg-[#E4E7F0] dark:hover:bg-[#2A3150] text-[#6B7194] dark:text-[#9AA0C3] border border-[#E4E7F0] dark:border-[#2A2F47]"
                >
                  <HelpCircle className="w-3.5 h-3.5" />
                  <span>Download Template</span>
                </button>
                <button
                  onClick={handleApplyRawContent}
                  className="px-4 py-2 rounded-xl bg-[#5B5BD6] hover:bg-[#4D4DBF] text-white text-xs font-semibold shadow-xs cursor-pointer"
                >
                  Apply & Parse Changes
                </button>
              </div>
            </div>

            {/* Live Validator Diagnostics */}
            {validationReport && (
              <div
                className={`p-3.5 rounded-xl border text-xs font-mono transition-all ${
                  validationReport.valid
                    ? 'bg-emerald-50 dark:bg-emerald-950/40 text-emerald-800 dark:text-emerald-200 border-emerald-200 dark:border-emerald-800'
                    : 'bg-rose-50 dark:bg-rose-950/40 text-rose-800 dark:text-rose-200 border-rose-200 dark:border-rose-800'
                }`}
              >
                {validationReport.valid ? (
                  <div className="flex items-center space-x-2">
                    <CheckCircle2 className="w-4 h-4 text-emerald-600 dark:text-emerald-400" />
                    <span>✓ Dataset syntax and referential integrity valid!</span>
                  </div>
                ) : (
                  <div>
                    <div className="font-bold mb-1 flex items-center space-x-1.5">
                      <AlertCircle className="w-4 h-4 text-rose-600 dark:text-rose-400" />
                      <span>Validation Errors:</span>
                    </div>
                    <ul className="list-disc pl-5 space-y-0.5">
                      {validationReport.errors.map((err, i) => (
                        <li key={i}>{err}</li>
                      ))}
                    </ul>
                  </div>
                )}
              </div>
            )}

            {/* Textarea */}
            <textarea
              value={editorContent}
              onChange={(e) => setEditorContent(e.target.value)}
              rows={16}
              className="w-full font-mono text-xs p-4 rounded-xl border border-[#CBD5E1] dark:border-[#334155] bg-[#F8F9FD] dark:bg-[#121626] text-[#1B1F3B] dark:text-[#E8EAF6] focus:outline-none focus:ring-2 focus:ring-[#5B5BD6] leading-relaxed select-all"
            />
          </div>
        </div>
      )}

      {/* Save Modal */}
      {showSaveModal && (
        <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/50 backdrop-blur-xs">
          <div className="bg-white dark:bg-[#181C2E] border border-[#E4E7F0] dark:border-[#2A2F47] rounded-3xl w-full max-w-sm shadow-2xl p-6 space-y-4">
            <h3 className="text-base font-bold text-[#1B1F3B] dark:text-[#E8EAF6]">
              Save Dataset As
            </h3>
            <p className="text-xs text-[#6B7194] dark:text-[#9AA0C3]">
              Enter a name to save this dataset into local browser storage.
            </p>
            <input
              type="text"
              value={saveAsName}
              onChange={(e) => setSaveAsName(e.target.value)}
              placeholder="e.g. spring_2026.txt"
              className="w-full px-3.5 py-2.5 rounded-xl border border-[#CBD5E1] dark:border-[#334155] bg-white dark:bg-[#20253B] text-sm text-[#1B1F3B] dark:text-[#E8EAF6] focus:outline-none focus:ring-2 focus:ring-[#5B5BD6]"
            />
            <div className="flex justify-end space-x-2 pt-2">
              <button
                onClick={() => setShowSaveModal(false)}
                className="px-4 py-2 rounded-xl text-sm font-semibold text-[#6B7194] dark:text-[#9AA0C3] hover:bg-[#F0F2FA] dark:hover:bg-[#20253B]"
              >
                Cancel
              </button>
              <button
                onClick={() => {
                  if (saveAsName.trim()) {
                    saveCurrentDataset(saveAsName.trim());
                    setShowSaveModal(false);
                  }
                }}
                className="px-4 py-2 rounded-xl bg-[#5B5BD6] hover:bg-[#4D4DBF] text-white text-sm font-semibold shadow-xs"
              >
                Save
              </button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
};
