#include "schedule_grid.hpp"
#include "theme.hpp"
#include "toast.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QFileDialog>
#include <QPainter>
#include <QPdfWriter>
#include <QPageSize>
#include <QPageLayout>
#include <fstream>
#include <set>

namespace gui {

namespace {

// Custom widget to render cell chip
class CellChipWidget : public QWidget {
public:
    CellChipWidget(const QString& subject, const QString& faculty, const QString& slotInfo,
                   int subjectId, QWidget* parent = nullptr)
        : QWidget(parent), m_subject(subject), m_faculty(faculty),
          m_slotInfo(slotInfo), m_subjectId(subjectId), m_empty(false) {
        setup();
    }

    CellChipWidget(QWidget* parent = nullptr)
        : QWidget(parent), m_empty(true) {
        setup();
    }

private:
    void setup() {
        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(8, 6, 8, 6);
        layout->setSpacing(2);

        if (m_empty) {
            auto* dash = new QLabel("-", this);
            dash->setAlignment(Qt::AlignCenter);
            dash->setStyleSheet("color: #9AA0C3; font-size: 16px; font-weight: bold;");
            layout->addWidget(dash);
        } else {
            auto* subLabel = new QLabel(m_subject, this);
            subLabel->setStyleSheet(QString("font-weight: 700; font-size: 13px; color: %1;")
                                        .arg(Theme::subjectTextColor(m_subjectId).name()));
            subLabel->setWordWrap(true);

            auto* facLabel = new QLabel(m_faculty, this);
            facLabel->setStyleSheet("font-size: 11px; color: #6B7194; font-weight: 500;");

            auto* slotLabel = new QLabel(m_slotInfo, this);
            slotLabel->setStyleSheet("font-size: 10px; color: #9AA0C3; font-weight: 600;");
            slotLabel->setAlignment(Qt::AlignRight);

            layout->addWidget(subLabel);
            layout->addWidget(facLabel);
            layout->addWidget(slotLabel);
        }
    }

    void paintEvent(QPaintEvent* /*event*/) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        QRect r = rect().adjusted(3, 3, -3, -3);
        if (m_empty) {
            p.setPen(QPen(Theme::borderColor(), 1, Qt::DashLine));
            p.setBrush(Qt::transparent);
            p.drawRoundedRect(r, 8, 8);
        } else {
            p.setPen(QPen(Theme::borderColor(), 1));
            p.setBrush(Theme::subjectColor(m_subjectId));
            p.drawRoundedRect(r, 8, 8);
        }
    }

    QString m_subject;
    QString m_faculty;
    QString m_slotInfo;
    int m_subjectId = 0;
    bool m_empty = true;
};

} // anonymous namespace

ScheduleGrid::ScheduleGrid(bool interactive, QWidget* parent)
    : QWidget(parent), m_interactive(interactive) {
    setupUi();
}

void ScheduleGrid::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(12);

    // Top control bar inside grid
    auto* topLayout = new QHBoxLayout();
    m_headerLabel = new QLabel("0 of 0 required periods assigned", this);
    m_headerLabel->setStyleSheet("font-size: 14px; font-weight: 700; color: #5B5BD6;");
    topLayout->addWidget(m_headerLabel);

    topLayout->addStretch();

    m_exportCsvBtn = new QPushButton("Export CSV", this);
    m_exportCsvBtn->setProperty("class", "secondary-btn");
    connect(m_exportCsvBtn, &QPushButton::clicked, this, &ScheduleGrid::onExportCsvClicked);
    topLayout->addWidget(m_exportCsvBtn);

    m_exportPdfBtn = new QPushButton("Export PDF", this);
    m_exportPdfBtn->setProperty("class", "secondary-btn");
    connect(m_exportPdfBtn, &QPushButton::clicked, this, &ScheduleGrid::onExportPdfClicked);
    topLayout->addWidget(m_exportPdfBtn);

    mainLayout->addLayout(topLayout);

    // Schedule Table
    m_table = new QTableWidget(this);
    m_table->setCornerButtonEnabled(false);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setAttribute(Qt::WA_StyledBackground, true);
    m_table->viewport()->setAttribute(Qt::WA_StyledBackground, true);
    m_table->verticalHeader()->setAttribute(Qt::WA_StyledBackground, true);
    m_table->verticalHeader()->setDefaultSectionSize(72);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setShowGrid(false);

    if (m_interactive) {
        m_table->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(m_table, &QTableWidget::customContextMenuRequested, this, &ScheduleGrid::showContextMenu);
        connect(m_table, &QTableWidget::cellClicked, this, &ScheduleGrid::onCellClicked);
    }

    mainLayout->addWidget(m_table);
}

void ScheduleGrid::setInteractive(bool interactive) {
    m_interactive = interactive;
    if (interactive) {
        m_table->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(m_table, &QTableWidget::customContextMenuRequested, this, &ScheduleGrid::showContextMenu, Qt::UniqueConnection);
        connect(m_table, &QTableWidget::cellClicked, this, &ScheduleGrid::onCellClicked, Qt::UniqueConnection);
    } else {
        m_table->setContextMenuPolicy(Qt::NoContextMenu);
        disconnect(m_table, &QTableWidget::customContextMenuRequested, this, &ScheduleGrid::showContextMenu);
        disconnect(m_table, &QTableWidget::cellClicked, this, &ScheduleGrid::onCellClicked);
    }
}

void ScheduleGrid::rebuildGridStructure() {
    m_days.clear();
    m_maxPeriod = 0;
    m_dayPeriodToSlot.clear();
    m_slotToRowCol.clear();

    // Standard days of the academic week: Mon, Tue, Wed, Thu, Fri, Sat
    const std::vector<std::string> standardDays = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    std::set<std::string> seenDays;

    for (const auto& slot : m_dataset.getSlots()) {
        if (slot.period > m_maxPeriod) {
            m_maxPeriod = slot.period;
        }
        m_dayPeriodToSlot[{slot.day, slot.period}] = slot.id;
    }

    // Always include Monday through Saturday
    for (const auto& d : standardDays) {
        if (seenDays.insert(d).second) {
            m_days.push_back(d);
        }
    }

    // Also include any custom days present in dataset
    for (const auto& slot : m_dataset.getSlots()) {
        if (seenDays.insert(slot.day).second) {
            m_days.push_back(slot.day);
        }
    }

    if (m_maxPeriod < 3) {
        m_maxPeriod = 3;
    }

    m_table->clear();
    m_table->setRowCount(static_cast<int>(m_days.size()));
    m_table->setColumnCount(m_maxPeriod);

    // Horizontal headers: Period 1, Period 2, ...
    QStringList colHeaders;
    for (int p = 1; p <= m_maxPeriod; ++p) {
        colHeaders << QString("Period %1").arg(p);
    }
    m_table->setHorizontalHeaderLabels(colHeaders);

    // Vertical headers: Days
    QStringList rowHeaders;
    for (size_t r = 0; r < m_days.size(); ++r) {
        rowHeaders << QString::fromStdString(m_days[r]);
        for (int p = 1; p <= m_maxPeriod; ++p) {
            auto it = m_dayPeriodToSlot.find({m_days[r], p});
            if (it != m_dayPeriodToSlot.end()) {
                int slotId = it->second;
                m_slotToRowCol[slotId] = {static_cast<int>(r), p - 1};
            }
        }
    }
    m_table->setVerticalHeaderLabels(rowHeaders);
}

void ScheduleGrid::updateSchedule(const core::Timetable& timetable, const core::Dataset& dataset) {
    m_timetable = timetable;
    m_dataset = dataset;

    rebuildGridStructure();

    int assignedCount = static_cast<int>(m_timetable.getEntries().size());
    int totalNeeded = m_dataset.totalPeriodsNeeded();
    m_headerLabel->setText(QString("%1 of %2 required periods assigned")
                               .arg(assignedCount)
                               .arg(totalNeeded));

    // Fill all cells with empty chip by default
    for (int r = 0; r < m_table->rowCount(); ++r) {
        for (int c = 0; c < m_table->columnCount(); ++c) {
            m_table->setCellWidget(r, c, new CellChipWidget(this));
        }
    }

    // Populate assigned entries
    for (const auto& e : m_timetable.getEntries()) {
        auto it = m_slotToRowCol.find(e.slotId);
        if (it != m_slotToRowCol.end()) {
            int row = it->second.first;
            int col = it->second.second;

            const auto* sub = m_dataset.findSubject(e.subjectId);
            const auto* fac = m_dataset.findFaculty(e.facultyId);
            const auto* slot = m_dataset.findSlot(e.slotId);

            QString subName = sub ? QString::fromStdString(sub->name) : QString("Subj #%1").arg(e.subjectId);
            QString facName = fac ? QString::fromStdString(fac->name) : QString("Fac #%1").arg(e.facultyId);
            QString slotStr = slot ? QString("Slot %1 (%2)").arg(slot->id).arg(QString::fromStdString(slot->toString()))
                                   : QString("Slot %1").arg(e.slotId);

            m_table->setCellWidget(row, col, new CellChipWidget(subName, facName, slotStr, e.subjectId, this));
        }
    }
}

void ScheduleGrid::onCellClicked(int row, int col) {
    if (!m_interactive) return;
    if (row < 0 || row >= static_cast<int>(m_days.size())) return;
    int period = col + 1;
    auto it = m_dayPeriodToSlot.find({m_days[row], period});
    int slotId = -1;
    if (it != m_dayPeriodToSlot.end()) {
        slotId = it->second;
    } else {
        // Dynamically register slot ID for Thursday/Friday/Saturday
        int nextId = 1;
        for (const auto& s : m_dataset.getSlots()) {
            if (s.id >= nextId) nextId = s.id + 1;
        }
        core::TimeSlot newSlot(nextId, m_days[row], period);
        m_dataset.addSlot(newSlot);
        m_dayPeriodToSlot[{m_days[row], period}] = nextId;
        m_slotToRowCol[nextId] = {row, col};
        slotId = nextId;
    }
    if (slotId != -1) {
        Q_EMIT cellSelected(slotId);
    }
}

void ScheduleGrid::showContextMenu(const QPoint& pos) {
    if (!m_interactive) return;
    QModelIndex index = m_table->indexAt(pos);
    if (!index.isValid()) return;

    int row = index.row();
    int col = index.column();
    int period = col + 1;
    auto it = m_dayPeriodToSlot.find({m_days[row], period});
    if (it == m_dayPeriodToSlot.end()) return;

    int slotId = it->second;
    if (m_timetable.isSlotUsed(slotId)) {
        QMenu menu(this);
        QAction* removeAct = menu.addAction("Remove Assignment");
        QAction* selected = menu.exec(m_table->viewport()->mapToGlobal(pos));
        if (selected == removeAct) {
            Q_EMIT cellRemoveRequested(slotId);
        }
    }
}

void ScheduleGrid::onExportCsvClicked() {
    QString path = QFileDialog::getSaveFileName(this, "Export Timetable to CSV", "timetable.csv", "CSV Files (*.csv)");
    if (!path.isEmpty()) {
        if (exportToCsv(path)) {
            Toast::show(this, "Timetable exported to CSV successfully", ToastType::Success);
        } else {
            Toast::show(this, "Failed to write CSV file", ToastType::Error);
        }
    }
}

void ScheduleGrid::onExportPdfClicked() {
    QString path = QFileDialog::getSaveFileName(this, "Export Timetable to PDF", "timetable.pdf", "PDF Files (*.pdf)");
    if (!path.isEmpty()) {
        if (exportToPdf(path)) {
            Toast::show(this, "Timetable exported to PDF successfully", ToastType::Success);
        } else {
            Toast::show(this, "Failed to generate PDF file", ToastType::Error);
        }
    }
}

bool ScheduleGrid::exportToCsv(const QString& path) {
    std::ofstream out(path.toStdString());
    if (!out.is_open()) return false;

    out << "SlotID,Day,Period,Subject,Faculty\n";
    for (const auto& e : m_timetable.getEntries()) {
        const auto* sub = m_dataset.findSubject(e.subjectId);
        const auto* fac = m_dataset.findFaculty(e.facultyId);
        const auto* slot = m_dataset.findSlot(e.slotId);

        out << e.slotId << ","
            << (slot ? slot->day : "") << ","
            << (slot ? std::to_string(slot->period) : "") << ",\""
            << (sub ? sub->name : "") << "\",\""
            << (fac ? fac->name : "") << "\"\n";
    }
    return true;
}

bool ScheduleGrid::exportToPdf(const QString& path) {
    QPdfWriter pdfWriter(path);
    pdfWriter.setPageSize(QPageSize(QPageSize::A4));
    pdfWriter.setPageOrientation(QPageLayout::Landscape);

    QPainter painter(&pdfWriter);
    painter.setRenderHint(QPainter::Antialiasing);

    // Title & Header
    QFont titleFont("Helvetica", 16, QFont::Bold);
    painter.setFont(titleFont);
    painter.setPen(QColor("#1B1F3B"));
    painter.drawText(600, 600, "Timetable Chain - Academic Schedule");

    QFont subtitleFont("Helvetica", 10, QFont::Normal);
    painter.setFont(subtitleFont);
    painter.setPen(QColor("#6B7194"));
    painter.drawText(600, 950, QString("Generated by C++ Constraint Solver • Total Periods Assigned: %1 of %2")
                                  .arg(m_timetable.getEntries().size())
                                  .arg(m_dataset.totalPeriodsNeeded()));

    // Draw Grid
    int startX = 600;
    int startY = 1300;
    int cellWidth = 2600;
    int cellHeight = 1400;

    int cols = m_maxPeriod;
    int rows = static_cast<int>(m_days.size());

    // Column headers (Period 1, 2, ...)
    QFont headerFont("Helvetica", 10, QFont::Bold);
    painter.setFont(headerFont);
    for (int c = 0; c < cols; ++c) {
        QRect headerRect(startX + 1400 + c * cellWidth, startY - 450, cellWidth, 400);
        painter.setPen(QColor("#5B5BD6"));
        painter.drawText(headerRect, Qt::AlignCenter, QString("Period %1").arg(c + 1));
    }

    // Rows and cells
    for (int r = 0; r < rows; ++r) {
        // Day label
        QRect dayRect(startX, startY + r * cellHeight, 1300, cellHeight);
        painter.setFont(headerFont);
        painter.setPen(QColor("#1B1F3B"));
        painter.drawText(dayRect, Qt::AlignVCenter | Qt::AlignLeft, QString::fromStdString(m_days[r]));

        for (int c = 0; c < cols; ++c) {
            QRect cellRect(startX + 1400 + c * cellWidth, startY + r * cellHeight, cellWidth - 100, cellHeight - 100);
            int period = c + 1;
            auto it = m_dayPeriodToSlot.find({m_days[r], period});
            if (it != m_dayPeriodToSlot.end()) {
                int slotId = it->second;
                const auto* entry = m_timetable.findEntryBySlot(slotId);
                if (entry) {
                    painter.setPen(QPen(QColor("#5B5BD6"), 2));
                    painter.setBrush(QColor("#EEF0FD"));
                    painter.drawRoundedRect(cellRect, 80, 80);

                    const auto* sub = m_dataset.findSubject(entry->subjectId);
                    const auto* fac = m_dataset.findFaculty(entry->facultyId);

                    painter.setFont(QFont("Helvetica", 10, QFont::Bold));
                    painter.setPen(QColor("#1B1F3B"));
                    painter.drawText(cellRect.adjusted(80, 80, -80, -400), Qt::AlignLeft | Qt::AlignVCenter,
                                     sub ? QString::fromStdString(sub->name) : "");

                    painter.setFont(QFont("Helvetica", 9, QFont::Normal));
                    painter.setPen(QColor("#6B7194"));
                    painter.drawText(cellRect.adjusted(80, 500, -80, -80), Qt::AlignLeft | Qt::AlignVCenter,
                                     fac ? QString::fromStdString(fac->name) : "");
                } else {
                    painter.setPen(QPen(QColor("#E4E7F0"), 1, Qt::DashLine));
                    painter.setBrush(Qt::NoBrush);
                    painter.drawRoundedRect(cellRect, 80, 80);

                    painter.setFont(QFont("Helvetica", 12, QFont::Normal));
                    painter.setPen(QColor("#9AA0C3"));
                    painter.drawText(cellRect, Qt::AlignCenter, "-");
                }
            }
        }
    }

    painter.end();
    return true;
}

} // namespace gui
