#include "data_page.hpp"
#include "theme.hpp"
#include "toast.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QFileDialog>
#include <QHeaderView>
#include <QPainter>
#include <QStyleOption>

namespace gui {

DataPage::DataPage(core::Dataset& dataset, QWidget* parent)
    : QWidget(parent), m_dataset(dataset) {
    setupUi();
}

void DataPage::setupUi() {
    setObjectName("DataPage");
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(16);

    // Toolbar Card
    auto* toolbarCard = new QFrame(this);
    toolbarCard->setProperty("class", "card");
    toolbarCard->setGraphicsEffect(Theme::createCardShadow(toolbarCard));
    auto* toolbarLayout = new QHBoxLayout(toolbarCard);
    toolbarLayout->setContentsMargins(16, 14, 16, 14);
    toolbarLayout->setSpacing(12);

    m_loadBtn = new QPushButton("Load Data File...", toolbarCard);
    m_loadBtn->setProperty("class", "primary-btn");
    connect(m_loadBtn, &QPushButton::clicked, this, &DataPage::onLoadClicked);
    toolbarLayout->addWidget(m_loadBtn);

    m_filePathLabel = new QLabel("No data file loaded", toolbarCard);
    m_filePathLabel->setStyleSheet("color: #6B7194; font-size: 13px;");
    toolbarLayout->addWidget(m_filePathLabel);

    toolbarLayout->addStretch();
    mainLayout->addWidget(toolbarCard);

    // Warning Banner (hidden unless periodsNeeded > slotsCount)
    m_warningBanner = new QLabel(this);
    m_warningBanner->setWordWrap(true);
    m_warningBanner->setStyleSheet("background-color: #FEF7E0; color: #B06000; font-weight: 600; padding: 12px; border-radius: 8px; border: 1px solid #FCE8B2;");
    m_warningBanner->hide();
    mainLayout->addWidget(m_warningBanner);

    // Summary Cards (4 stats)
    auto* statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(16);

    auto makeStatCard = [this](const QString& title, QLabel*& valLabel, const QString& subtext) {
        auto* card = new QFrame(this);
        card->setProperty("class", "card");
        card->setGraphicsEffect(Theme::createCardShadow(card));
        auto* l = new QVBoxLayout(card);
        l->setContentsMargins(16, 14, 16, 14);
        l->setSpacing(4);

        auto* t = new QLabel(title, card);
        t->setProperty("class", "stat-title");
        l->addWidget(t);

        valLabel = new QLabel("0", card);
        valLabel->setProperty("class", "stat-value");
        l->addWidget(valLabel);

        auto* s = new QLabel(subtext, card);
        s->setProperty("class", "stat-sub");
        l->addWidget(s);

        return card;
    };

    statsLayout->addWidget(makeStatCard("SUBJECTS", m_subjectsCountLabel, "Distinct courses offered"));
    statsLayout->addWidget(makeStatCard("FACULTY", m_facultyCountLabel, "Teaching staff members"));
    statsLayout->addWidget(makeStatCard("SLOTS", m_slotsCountLabel, "Weekly time slots"));
    statsLayout->addWidget(makeStatCard("PERIODS NEEDED", m_periodsNeededLabel, "Total required periods"));

    mainLayout->addLayout(statsLayout);

    // Tabs for 3 Tables
    auto* tabs = new QTabWidget(this);

    // Table 1: Subjects
    m_subjectsTable = new QTableWidget(this);
    m_subjectsTable->setCornerButtonEnabled(false);
    m_subjectsTable->setColumnCount(3);
    m_subjectsTable->setHorizontalHeaderLabels({"ID", "Subject Name", "Periods Per Week"});
    m_subjectsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_subjectsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tabs->addTab(m_subjectsTable, "Subjects");

    // Table 2: Faculty
    m_facultyTable = new QTableWidget(this);
    m_facultyTable->setCornerButtonEnabled(false);
    m_facultyTable->setColumnCount(4);
    m_facultyTable->setHorizontalHeaderLabels({"ID", "Faculty Name", "Qualified Subjects", "Unavailable Slots"});
    m_facultyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_facultyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tabs->addTab(m_facultyTable, "Faculty & Qualifications");

    // Table 3: Time Slots
    m_slotsTable = new QTableWidget(this);
    m_slotsTable->setCornerButtonEnabled(false);
    m_slotsTable->setColumnCount(4);
    m_slotsTable->setHorizontalHeaderLabels({"ID", "Day", "Period", "Slot Label"});
    m_slotsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_slotsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tabs->addTab(m_slotsTable, "Time Slots");

    mainLayout->addWidget(tabs, 1);
}

void DataPage::openLoadFileDialog() {
    onLoadClicked();
}

void DataPage::onLoadClicked() {
    QString path = QFileDialog::getOpenFileName(this, "Open Curriculum Data File", "", "Text Files (*.txt);;All Files (*)");
    if (!path.isEmpty()) {
        loadFile(path);
    }
}

bool DataPage::loadFile(const QString& path) {
    std::string err;
    if (!m_dataset.loadFromFile(path.toStdString(), err)) {
        Toast::show(this, QString("Failed to load data: %1").arg(QString::fromStdString(err)), ToastType::Error, 5000);
        return false;
    }

    m_filePathLabel->setText(path);
    updateViews();
    Toast::show(this, "Curricular dataset loaded successfully", ToastType::Success);
    Q_EMIT datasetLoaded();
    return true;
}

void DataPage::updateViews() {
    const auto& subjects = m_dataset.getSubjects();
    const auto& faculty = m_dataset.getFaculty();
    const auto& slots = m_dataset.getSlots();
    int needed = m_dataset.totalPeriodsNeeded();
    int available = static_cast<int>(slots.size());

    m_subjectsCountLabel->setText(QString::number(subjects.size()));
    m_facultyCountLabel->setText(QString::number(faculty.size()));
    m_slotsCountLabel->setText(QString::number(available));
    m_periodsNeededLabel->setText(QString::number(needed));

    // Capacity check
    if (needed > available) {
        m_warningBanner->setText(QString("⚠ Capacity Warning: Curricular demand (%1 periods) exceeds available time slots (%2 slots). Timetable generation is mathematically impossible!")
                                     .arg(needed).arg(available));
        m_warningBanner->show();
    } else {
        m_warningBanner->hide();
    }

    // Populate Subjects Table
    m_subjectsTable->setRowCount(static_cast<int>(subjects.size()));
    for (size_t i = 0; i < subjects.size(); ++i) {
        const auto& s = subjects[i];
        m_subjectsTable->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::number(s.id)));
        m_subjectsTable->setItem(static_cast<int>(i), 1, new QTableWidgetItem(QString::fromStdString(s.name)));
        m_subjectsTable->setItem(static_cast<int>(i), 2, new QTableWidgetItem(QString::number(s.periodsPerWeek)));
    }

    // Populate Faculty Table
    m_facultyTable->setRowCount(static_cast<int>(faculty.size()));
    for (size_t i = 0; i < faculty.size(); ++i) {
        const auto& f = faculty[i];
        m_facultyTable->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::number(f.id)));
        m_facultyTable->setItem(static_cast<int>(i), 1, new QTableWidgetItem(QString::fromStdString(f.name)));

        // Can teach subjects
        QString canTeachStr;
        for (int subId : f.canTeachSubjects) {
            const auto* sub = m_dataset.findSubject(subId);
            if (!canTeachStr.isEmpty()) canTeachStr += ", ";
            canTeachStr += sub ? QString::fromStdString(sub->name) : QString("#%1").arg(subId);
        }
        m_facultyTable->setItem(static_cast<int>(i), 2, new QTableWidgetItem(canTeachStr));

        // Unavailable slots
        QString unavailStr;
        for (int slotId : f.unavailableSlots) {
            const auto* slot = m_dataset.findSlot(slotId);
            if (!unavailStr.isEmpty()) unavailStr += ", ";
            unavailStr += slot ? QString::fromStdString(slot->toString()) : QString("#%1").arg(slotId);
        }
        if (unavailStr.isEmpty()) unavailStr = "None (Fully Available)";
        m_facultyTable->setItem(static_cast<int>(i), 3, new QTableWidgetItem(unavailStr));
    }

    // Populate Slots Table
    m_slotsTable->setRowCount(static_cast<int>(slots.size()));
    for (size_t i = 0; i < slots.size(); ++i) {
        const auto& sl = slots[i];
        m_slotsTable->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::number(sl.id)));
        m_slotsTable->setItem(static_cast<int>(i), 1, new QTableWidgetItem(QString::fromStdString(sl.day)));
        m_slotsTable->setItem(static_cast<int>(i), 2, new QTableWidgetItem(QString::number(sl.period)));
        m_slotsTable->setItem(static_cast<int>(i), 3, new QTableWidgetItem(QString::fromStdString(sl.toString())));
    }
}

void DataPage::paintEvent(QPaintEvent* event) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(event);
}

} // namespace gui
