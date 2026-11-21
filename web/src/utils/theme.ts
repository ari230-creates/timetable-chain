export const getSubjectColors = (subjectId: number, isDark: boolean = false) => {
  const lightBg = [
    '#E8F0FE', // 1: Soft Blue
    '#F3E8FD', // 2: Soft Purple
    '#E6F4EA', // 3: Soft Green
    '#FEF7E0', // 4: Soft Amber
    '#FCE8E6', // 5: Soft Coral
    '#E0F2FE', // 6: Soft Cyan
  ];

  const lightText = [
    '#1A73E8',
    '#9334E6',
    '#137333',
    '#B06000',
    '#C5221F',
    '#0284C7',
  ];

  const darkBg = [
    '#1E294B',
    '#2C204B',
    '#1B382B',
    '#38321B',
    '#3D1E1E',
    '#1A3245',
  ];

  const darkText = [
    '#8AB4F8',
    '#D7AEFB',
    '#81C995',
    '#FDD663',
    '#F28B82',
    '#78D9EC',
  ];

  const idx = ((subjectId > 0 ? subjectId - 1 : 0) % 6);
  return {
    bg: isDark ? darkBg[idx] : lightBg[idx],
    text: isDark ? darkText[idx] : lightText[idx],
    border: isDark ? '#334155' : '#E2E8F0',
  };
};

export const exportTimetableToCSV = (
  timetable: { subjectId: number; facultyId: number; slotId: number }[],
  dataset: {
    subjects: { id: number; name: string }[];
    faculty: { id: number; name: string }[];
    slots: { id: number; day: string; period: number; name: string }[];
  },
  filename = 'timetable.csv'
) => {
  const headers = ['Slot ID', 'Day', 'Period', 'Subject ID', 'Subject Name', 'Faculty ID', 'Faculty Name'];
  const rows = timetable.map((e) => {
    const s = dataset.subjects.find((sub) => sub.id === e.subjectId);
    const f = dataset.faculty.find((fac) => fac.id === e.facultyId);
    const slot = dataset.slots.find((sl) => sl.id === e.slotId);
    return [
      e.slotId,
      slot?.day || '',
      slot?.period || '',
      e.subjectId,
      `"${s?.name || ''}"`,
      e.facultyId,
      `"${f?.name || ''}"`,
    ].join(',');
  });

  const csvContent = 'data:text/csv;charset=utf-8,' + [headers.join(','), ...rows].join('\n');
  const encodedUri = encodeURI(csvContent);
  const link = document.createElement('a');
  link.setAttribute('href', encodedUri);
  link.setAttribute('download', filename);
  document.body.appendChild(link);
  link.click();
  document.body.removeChild(link);
};
