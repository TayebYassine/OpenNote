#include "PreferencesDialog.h"

#include <QApplication>
#include <QColorDialog>
#include <QDialogButtonBox>
#include <QFontDatabase>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPalette>
#include <QScrollArea>
#include <QSplitter>
#include <QVBoxLayout>

#include "Language.h"
#include "SyntaxHighlighter.h"

static const char* PREVIEW_CODE = R"(import java.util.Scanner;

public class UserInputExample {
    static class User {
        private String name;
        private int age;

        // Constructor
        public User(String name, int age) {
            this.name = name;
            this.age = age;
        }

        // Method to display user info
        public void printInfo() {
            System.out.println("User Info:");
            System.out.println("Name: " + name);
            System.out.println("Age: " + age);
        }
    }

    public static void main(String[] args) {
        // Create a Scanner object to read input from the console
        Scanner scanner = new Scanner(System.in);

        System.out.print("Enter your age (an integer): ");
        int age = scanner.nextInt();
        scanner.nextLine(); // consume newline

        System.out.print("Enter your full name: ");
        String fullName = scanner.nextLine();

        // Create a User object
        User user = new User(fullName, age);

        System.out.println("\nHello, " + fullName);
        user.printInfo();

        scanner.close();
    }
})";

PreferencesDialog::PreferencesDialog(QWidget* parent)
    : QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint) {
    setWindowTitle("Preferences");
    setModal(true);
    setMinimumSize(660, 520);
    setupUi();
    loadFromDatabase();
}

void PreferencesDialog::setupUi() {
    auto* root = new QVBoxLayout(this);
    root->setSpacing(0);
    root->setContentsMargins(0, 0, 0, 0);

    m_tabs = new QTabWidget(this);
    m_tabs->setDocumentMode(false);
    m_tabs->addTab(buildViewTab(), "View");
    m_tabs->addTab(buildEditorTab(), "Editor");
    m_tabs->addTab(buildSyntaxHighlightingTab(), "Syntax Highlighting");

    root->addWidget(m_tabs);

    auto* btnBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply,
        this);
    btnBox->setContentsMargins(12, 8, 12, 12);
    root->addWidget(btnBox);

    connect(btnBox, &QDialogButtonBox::accepted, this, [this] {
        apply();
        accept();
    });
    connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(btnBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this,
            &PreferencesDialog::apply);
}

QWidget* PreferencesDialog::buildViewTab() {
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    layout->setSpacing(16);
    layout->setContentsMargins(20, 20, 20, 20);

    // Display group
    auto* displayGroup = new QGroupBox("Display", page);
    auto* displayLayout = new QVBoxLayout(displayGroup);
    m_showLineNumbers = new QCheckBox("Display line numbers");
    m_highlightCurrentLine = new QCheckBox("Highlight current line");
    m_showStatusBar = new QCheckBox("Display status bar");
    displayLayout->addWidget(m_showLineNumbers);
    displayLayout->addWidget(m_highlightCurrentLine);
    displayLayout->addWidget(m_showStatusBar);
    layout->addWidget(displayGroup);

    // Text Wrapping group
    auto* wrapGroup = new QGroupBox("Text Wrapping", page);
    auto* wrapLayout = new QVBoxLayout(wrapGroup);
    m_wordWrap = new QCheckBox("Enable text wrapping");
    m_wrapAtWord = new QCheckBox("Do not split words over two lines");
    wrapLayout->addWidget(m_wordWrap);
    wrapLayout->addWidget(m_wrapAtWord);
    connect(m_wordWrap, &QCheckBox::toggled, m_wrapAtWord,
            &QCheckBox::setEnabled);
    layout->addWidget(wrapGroup);

    layout->addStretch();
    return page;
}

QWidget* PreferencesDialog::buildEditorTab() {
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    layout->setSpacing(16);
    layout->setContentsMargins(20, 20, 20, 20);

    // Tab Stops group
    auto* tabGroup = new QGroupBox("Tab Stops", page);
    auto* tabLayout = new QVBoxLayout(tabGroup);

    auto* tabWidthRow = new QHBoxLayout;
    tabWidthRow->addWidget(new QLabel("Tab width:"));
    m_tabSize = new QSpinBox;
    m_tabSize->setRange(1, 16);
    m_tabSize->setFixedWidth(60);
    tabWidthRow->addWidget(m_tabSize);
    tabWidthRow->addStretch();
    tabLayout->addLayout(tabWidthRow);

    m_spacesInsteadOfTabs = new QCheckBox("Insert spaces instead of tabs");
    m_autoIndent = new QCheckBox("Enable automatic indentation");
    tabLayout->addWidget(m_spacesInsteadOfTabs);
    tabLayout->addWidget(m_autoIndent);
    layout->addWidget(tabGroup);

    // Files group
    auto* filesGroup = new QGroupBox("Files", page);
    auto* filesLayout = new QVBoxLayout(filesGroup);

    m_openRecentFiles = new QCheckBox("Open recent files on startup");
    filesLayout->addWidget(m_openRecentFiles);

    m_autoSave = new QCheckBox("Autosave files");
    filesLayout->addWidget(m_autoSave);

    auto* intervalRow = new QHBoxLayout;
    intervalRow->addWidget(
        new QLabel("    Time in minutes between each autosave:"));
    m_autoSaveInterval = new QSpinBox;
    m_autoSaveInterval->setRange(1, 120);
    m_autoSaveInterval->setFixedWidth(60);
    intervalRow->addWidget(m_autoSaveInterval);
    intervalRow->addStretch();
    filesLayout->addLayout(intervalRow);

    connect(m_autoSave, &QCheckBox::toggled, m_autoSaveInterval,
            &QSpinBox::setEnabled);
    layout->addWidget(filesGroup);

    layout->addStretch();
    return page;
}

QWidget* PreferencesDialog::buildSyntaxHighlightingTab() {
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    layout->setSpacing(10);
    layout->setContentsMargins(12, 12, 12, 12);

    auto* themeRow = new QHBoxLayout;
    themeRow->addWidget(new QLabel("Color Theme:"));

    m_themeCombo = new QComboBox(page);
    m_themeCombo->setMinimumWidth(200);

    for (const auto& info : predefinedThemes()) {
        m_themeCombo->addItem(info.name, themeToVariant(info.theme));
    }

    m_themeCombo->insertSeparator(m_themeCombo->count());
    m_themeCombo->addItem("Custom…", QVariant());

    themeRow->addWidget(m_themeCombo);
    themeRow->addStretch();
    layout->addLayout(themeRow);

    auto* line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("color: #444;");
    layout->addWidget(line);

    auto* topRow = new QHBoxLayout;

    // Token list
    m_tokenList = new QListWidget(page);
    m_tokenList->setFixedWidth(200);
    const QMap<QString, QString> names = tokenDisplayNames();
    for (const QString& key : tokenKeys()) {
        auto* item = new QListWidgetItem(names.value(key, key));
        item->setData(Qt::UserRole, key);
        m_tokenList->addItem(item);
    }
    topRow->addWidget(m_tokenList);

    // Style controls
    auto* styleWidget = new QWidget(page);
    auto* styleLayout = new QVBoxLayout(styleWidget);
    styleLayout->setSpacing(12);
    styleLayout->setContentsMargins(12, 4, 4, 4);

    auto* colorRow = new QHBoxLayout;

    colorRow->addWidget(new QLabel("Color:"));
    m_colorBtn = new QPushButton;
    m_colorBtn->setFixedSize(48, 24);
    m_colorBtn->setToolTip("Click to change color");

    m_defaultColorBtn = new QPushButton("Default");
    m_defaultColorBtn->setToolTip("Click to reset default color");

    colorRow->addWidget(m_colorBtn);
    //colorRow->addWidget(m_defaultColorBtn); // removed

    colorRow->addStretch();
    styleLayout->addLayout(colorRow);

    m_boldChk = new QCheckBox("Bold");
    m_italicChk = new QCheckBox("Italic");
    styleLayout->addWidget(m_boldChk);
    styleLayout->addWidget(m_italicChk);
    styleLayout->addStretch();

    topRow->addWidget(styleWidget, 1);
    layout->addLayout(topRow);

    auto* previewLabel = new QLabel("Preview:");
    previewLabel->setStyleSheet("font-weight: bold;");
    layout->addWidget(previewLabel);

    // Preview mode selector
    auto* previewModeRow = new QHBoxLayout;
    previewModeRow->addWidget(new QLabel("Preview Mode:"));

    m_previewModeCombo = new QComboBox(page);
    m_previewModeCombo->addItem("Dark Theme", "dark");
    m_previewModeCombo->addItem("Light Theme", "light");
    m_previewModeCombo->setFixedWidth(150);

    // Auto-detect system theme and set combobox
    if (isSystemDarkTheme()) {
        m_previewModeCombo->setCurrentIndex(0); // Dark
    }
    else {
        m_previewModeCombo->setCurrentIndex(1); // Light
    }

    previewModeRow->addWidget(m_previewModeCombo);
    previewModeRow->addStretch();
    layout->addLayout(previewModeRow);

    // Single preview
    m_preview = new QPlainTextEdit(page);
    m_preview->setReadOnly(true);
    m_preview->setPlainText(PREVIEW_CODE);
    m_preview->setMinimumHeight(200);
    QFont previewFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    previewFont.setPointSize(10);
    m_preview->setFont(previewFont);

    // Set initial style based on detected theme
    if (m_previewModeCombo->currentIndex() == 0) {
        m_preview->setStyleSheet(
            "QPlainTextEdit { background:#1e1e1e; color:#d4d4d4; border:1px solid "
            "#444; }");
    }
    else {
        m_preview->setStyleSheet(
            "QPlainTextEdit { background:#ffffff; color:#1e1e1e; border:1px solid "
            "#ccc; }");
    }

    layout->addWidget(m_preview);

    // Attach highlighter to preview document
    m_highlighter = new SyntaxHighlighter(m_preview->document(), Language::Java,
                                          m_editTheme);

    connect(m_tokenList, &QListWidget::currentRowChanged, this,
            &PreferencesDialog::onTokenSelected);
    connect(m_colorBtn, &QPushButton::clicked, this,
            &PreferencesDialog::onColorButtonClicked);
    //connect(m_defaultColorBtn, &QPushButton::clicked, this,
    //        &PreferencesDialog::onDefaultColorButtonClicked); // removed
    connect(m_boldChk, &QCheckBox::toggled, this,
            &PreferencesDialog::onBoldToggled);
    connect(m_italicChk, &QCheckBox::toggled, this,
            &PreferencesDialog::onItalicToggled);

    connect(m_themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PreferencesDialog::onThemeChanged);

    connect(m_previewModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PreferencesDialog::onPreviewModeChanged);

    m_colorBtn->setEnabled(false);
    m_boldChk->setEnabled(false);
    m_italicChk->setEnabled(false);

    return page;
}

void PreferencesDialog::loadFromDatabase() {
    const AppDatabase& db = AppDatabase::instance();

    // View
    m_showLineNumbers->setChecked(db.showLineNumbers());
    m_highlightCurrentLine->setChecked(db.highlightCurrentLine());
    m_showStatusBar->setChecked(db.showStatusBar());
    m_wordWrap->setChecked(db.wordWrap());
    m_wrapAtWord->setChecked(db.wrapAtWord());
    m_wrapAtWord->setEnabled(db.wordWrap());

    // Editor
    m_tabSize->setValue(db.tabSize());
    m_spacesInsteadOfTabs->setChecked(db.spacesInsteadOfTabs());
    m_autoIndent->setChecked(db.autoIndent());
    m_openRecentFiles->setChecked(db.openRecentFiles());
    m_autoSave->setChecked(db.autoSave());
    m_autoSaveInterval->setValue(db.autoSaveIntervalMin());
    m_autoSaveInterval->setEnabled(db.autoSave());

    // Font & Colors
    m_loadingTheme = true;
    m_editTheme = db.syntaxTheme();

    bool matched = false;
    for (int i = 0; i < m_themeCombo->count(); ++i) {
        QVariant data = m_themeCombo->itemData(i);
        if (!data.isValid() || data.isNull()) continue;

        SyntaxTheme predefined = variantToTheme(data);
        if (predefined == m_editTheme) {
            m_themeCombo->setCurrentIndex(i);
            matched = true;
            break;
        }
    }

    if (!matched) {
        m_themeCombo->setCurrentIndex(m_themeCombo->count() - 1);
    }

    refreshPreview();
    m_loadingTheme = false;

    if (m_tokenList->count() > 0) m_tokenList->setCurrentRow(0);
}

void PreferencesDialog::saveToDatabase() {
    AppDatabase& db = AppDatabase::instance();

    db.setShowLineNumbers(m_showLineNumbers->isChecked());
    db.setHighlightCurrentLine(m_highlightCurrentLine->isChecked());
    db.setShowStatusBar(m_showStatusBar->isChecked());
    db.setWordWrap(m_wordWrap->isChecked());
    db.setWrapAtWord(m_wrapAtWord->isChecked());

    db.setTabSize(m_tabSize->value());
    db.setSpacesInsteadOfTabs(m_spacesInsteadOfTabs->isChecked());
    db.setAutoIndent(m_autoIndent->isChecked());
    db.setOpenRecentFiles(m_openRecentFiles->isChecked());
    db.setAutoSave(m_autoSave->isChecked());
    db.setAutoSaveIntervalMin(m_autoSaveInterval->value());

    db.setSyntaxTheme(m_editTheme);
    db.save();
}

void PreferencesDialog::apply() {
    saveToDatabase();
    emit settingsChanged();
}

void PreferencesDialog::refreshPreview() {
    if (m_highlighter) m_highlighter->applyTheme(m_editTheme);
}

void PreferencesDialog::updateColorButton() {
    if (m_currentToken.isEmpty()) return;
    const QString color = m_editTheme.value(m_currentToken).color;
    m_colorBtn->setStyleSheet(QString("QPushButton { background: %1; border: 1px "
            "solid #555; border-radius: 3px; }")
        .arg(color));
}

void PreferencesDialog::onTokenSelected(int row) {
    if (row < 0) return;
    m_currentToken = m_tokenList->item(row)->data(Qt::UserRole).toString();

    const TokenStyle& ts = m_editTheme[m_currentToken];

    // Block signals while loading values to avoid recursive updates
    QSignalBlocker b1(m_boldChk), b2(m_italicChk);
    m_boldChk->setChecked(ts.bold);
    m_italicChk->setChecked(ts.italic);

    m_colorBtn->setEnabled(true);
    m_boldChk->setEnabled(true);
    m_italicChk->setEnabled(true);
    updateColorButton();
}

void PreferencesDialog::onColorButtonClicked() {
    if (m_currentToken.isEmpty()) return;
    const QColor initial = QColor(m_editTheme[m_currentToken].color);
    const QColor chosen =
        QColorDialog::getColor(initial, this, "Pick Token Color");
    if (!chosen.isValid()) return;

    m_editTheme[m_currentToken].color = chosen.name(QColor::HexRgb);
    m_themeCombo->setCurrentIndex(m_themeCombo->count() - 1);
    updateColorButton();
    refreshPreview();
}

void PreferencesDialog::onDefaultColorButtonClicked() {
    if (m_currentToken.isEmpty()) return;

    SyntaxTheme defaultTheme = openNoteDefaultTheme();

    m_editTheme[m_currentToken].color = defaultTheme[m_currentToken].color;
    m_themeCombo->setCurrentIndex(m_themeCombo->count() - 1);
    updateColorButton();
    refreshPreview();
}

void PreferencesDialog::onThemeChanged(int index) {
    if (m_loadingTheme) return; // Prevent recursive updates during load

    QVariant data = m_themeCombo->itemData(index);
    if (!data.isValid() || data.isNull()) {
        return;
    }

    m_editTheme = variantToTheme(data);

    if (!m_currentToken.isEmpty()) {
        QSignalBlocker b1(m_boldChk), b2(m_italicChk);
        const TokenStyle& ts = m_editTheme[m_currentToken];
        m_boldChk->setChecked(ts.bold);
        m_italicChk->setChecked(ts.italic);
        updateColorButton();
    }

    refreshPreview();
}

void PreferencesDialog::onBoldToggled(bool on) {
    if (m_currentToken.isEmpty()) return;
    m_editTheme[m_currentToken].bold = on;
    m_themeCombo->setCurrentIndex(m_themeCombo->count() - 1);
    refreshPreview();
}

void PreferencesDialog::onItalicToggled(bool on) {
    if (m_currentToken.isEmpty()) return;
    m_editTheme[m_currentToken].italic = on;
    m_themeCombo->setCurrentIndex(m_themeCombo->count() - 1);
    refreshPreview();
}

void PreferencesDialog::onPreviewModeChanged(int index) {
    if (!m_preview) return;

    if (index == 0) {
        // Dark theme
        m_preview->setStyleSheet(
            "QPlainTextEdit { background:#1e1e1e; color:#d4d4d4; border:1px solid "
            "#444; }");
    }
    else {
        // Light theme
        m_preview->setStyleSheet(
            "QPlainTextEdit { background:#ffffff; color:#1e1e1e; border:1px solid "
            "#ccc; }");
    }
}