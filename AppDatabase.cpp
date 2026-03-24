#include "AppDatabase.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

AppDatabase& AppDatabase::instance() {
    static AppDatabase inst;
    return inst;
}

QString AppDatabase::dbPath() const {
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
        "/session.json";
}

static QJsonObject themeToJson(const SyntaxTheme& theme) {
    QJsonObject obj;
    for (auto it = theme.constBegin(); it != theme.constEnd(); ++it) {
        QJsonObject ts;
        ts["color"] = it.value().color;
        ts["bold"] = it.value().bold;
        ts["italic"] = it.value().italic;
        obj[it.key()] = ts;
    }
    return obj;
}

static SyntaxTheme themeFromJson(const QJsonObject& obj) {
    SyntaxTheme theme = openNoteDefaultTheme();
    for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
        const QJsonObject ts = it.value().toObject();
        TokenStyle style;
        style.color = ts.value("color").toString(theme.value(it.key()).color);
        style.bold = ts.value("bold").toBool(theme.value(it.key()).bold);
        style.italic = ts.value("italic").toBool(theme.value(it.key()).italic);
        theme[it.key()] = style;
    }
    return theme;
}

void AppDatabase::load() {
    QFile file(dbPath());
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) return;

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) return;

    const QJsonObject root = doc.object();

    // Font
    m_editorFontFamily = root.value("editorFontFamily").toString();
    m_editorFontSize = root.value("editorFontSize").toInt(12);

    // View
    m_showLineNumbers = root.value("showLineNumbers").toBool(true);
    m_highlightCurrentLine = root.value("highlightCurrentLine").toBool(true);
    m_showStatusBar = root.value("showStatusBar").toBool(true);
    m_wordWrap = root.value("wordWrap").toBool(false);
    m_wrapAtWord = root.value("wrapAtWord").toBool(true);

    // Editor behaviour
    m_tabSize = qMax(1, qMin(root.value("tabSize").toInt(4), 16));
    m_spacesInsteadOfTabs = root.value("spacesInsteadOfTabs").toBool(false);
    m_autoIndent = root.value("autoIndent").toBool(true);
    m_openRecentFiles = root.value("openRecentFiles").toBool(true);
    m_autoSave = root.value("autoSave").toBool(false);
    m_autoSaveIntervalMin = qMax(1, root.value("autoSaveIntervalMin").toInt(5));

    // Recent
    m_recentDir = root.value("recentDirectory").toString();
    m_recentFiles.clear();
    for (const auto& v : root.value("recentFiles").toArray())
        m_recentFiles.append(v.toString());

    // Bookmarks
    m_bookmarks.clear();
    const QJsonObject bmsObj = root.value("bookmarks").toObject();
    for (auto it = bmsObj.constBegin(); it != bmsObj.constEnd(); ++it) {
        QList<int> lines;
        for (const auto& v : it.value().toArray()) lines.append(v.toInt());
        m_bookmarks.insert(it.key(), lines);
    }

    // Syntax theme
    if (root.contains("syntaxTheme"))
        m_syntaxTheme = themeFromJson(root.value("syntaxTheme").toObject());
}

void AppDatabase::save() const {
    QDir().mkpath(QFileInfo(dbPath()).absolutePath());

    QJsonObject root;

    // Font
    root["editorFontFamily"] = m_editorFontFamily;
    root["editorFontSize"] = m_editorFontSize;

    // View
    root["showLineNumbers"] = m_showLineNumbers;
    root["highlightCurrentLine"] = m_highlightCurrentLine;
    root["showStatusBar"] = m_showStatusBar;
    root["wordWrap"] = m_wordWrap;
    root["wrapAtWord"] = m_wrapAtWord;

    // Editor
    root["tabSize"] = m_tabSize;
    root["spacesInsteadOfTabs"] = m_spacesInsteadOfTabs;
    root["autoIndent"] = m_autoIndent;
    root["openRecentFiles"] = m_openRecentFiles;
    root["autoSave"] = m_autoSave;
    root["autoSaveIntervalMin"] = m_autoSaveIntervalMin;

    // Recent
    root["recentDirectory"] = m_recentDir;
    QJsonArray rfArr;
    for (const auto& f : m_recentFiles) rfArr.append(f);
    root["recentFiles"] = rfArr;

    // Bookmarks
    QJsonObject bmsObj;
    for (auto it = m_bookmarks.constBegin(); it != m_bookmarks.constEnd(); ++it) {
        QJsonArray arr;
        for (int line : it.value()) arr.append(line);
        bmsObj[it.key()] = arr;
    }
    root["bookmarks"] = bmsObj;

    // Syntax theme
    root["syntaxTheme"] = themeToJson(m_syntaxTheme);

    QFile file(dbPath());
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
}

void AppDatabase::setRecentDirectory(const QString& dir) { m_recentDir = dir; }
void AppDatabase::clearRecentFiles() { m_recentFiles.clear(); }

void AppDatabase::addRecentFile(const QString& path) {
    m_recentFiles.removeAll(path);
    m_recentFiles.prepend(path);
    while (m_recentFiles.size() > MAX_RECENT_FILES) m_recentFiles.removeLast();
}

void AppDatabase::removeRecentFile(const QString& path) {
    m_recentFiles.removeAll(path);
}

QList<int> AppDatabase::bookmarksForFile(const QString& filePath) const {
    return m_bookmarks.value(filePath);
}

void AppDatabase::setBookmarks(const QString& filePath,
                               const QList<int>& lines) {
    if (lines.isEmpty())
        m_bookmarks.remove(filePath);
    else
        m_bookmarks.insert(filePath, lines);
}

void AppDatabase::clearBookmarksForFile(const QString& filePath) {
    m_bookmarks.remove(filePath);
}

void AppDatabase::setEditorFont(const QString& family, int pointSize) {
    m_editorFontFamily = family;
    m_editorFontSize = pointSize;
}
