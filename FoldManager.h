#ifndef OPENNOTE_LINUX_FOLDMANAGER_H
#define OPENNOTE_LINUX_FOLDMANAGER_H
#pragma once

#include <QMap>
#include <QSet>
#include <QTextBlock>
#include "Language.h"

class QTextDocument;

struct FoldRegion {
    int startLine;
    int endLine;
    bool isFolded;

    FoldRegion() : startLine(-1), endLine(-1), isFolded(false) {}
    FoldRegion(int start, int end) : startLine(start), endLine(end), isFolded(false) {}

    bool isValid() const { return startLine >= 0 && endLine > startLine; }
};

class FoldManager {
public:
    explicit FoldManager(QTextDocument* document, Language lang = Language::None);

    void setLanguage(Language lang);
    Language language() const { return m_language; }

    void detectFolds();

    FoldRegion getFoldAt(int line) const;

    bool isFoldStart(int line) const;

    bool isFolded(int line) const;

    void toggleFold(int line);

    void foldRegion(int line);
    void unfoldRegion(int line);

    QList<int> foldableLines() const { return m_folds.keys(); }

    bool isLineInFoldedRegion(int line) const;

    void clear();

private:
    void detectBraceFolds();
    void detectIndentFolds();
    void detectKeywordFolds();
    void detectXmlFolds();

    int findMatchingBrace(int startLine, const QString& openChar, const QString& closeChar);
    int getIndentLevel(const QString& line) const;

    void applyFold(int startLine, int endLine, bool fold);

    QTextDocument* m_document;
    Language m_language;
    QMap<int, FoldRegion> m_folds; // Map: start line -> fold region
};

#endif // OPENNOTE_LINUX_FOLDMANAGER_H
