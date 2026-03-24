#ifndef OPENNOTE_LINUX_SYNTAXHIGHLIGHTER_H
#define OPENNOTE_LINUX_SYNTAXHIGHLIGHTER_H
#pragma once

#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QVector>

#include "Language.h"
#include "SyntaxTheme.h"

class SyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit SyntaxHighlighter(QTextDocument* parent,
                               Language lang = Language::None,
                               SyntaxTheme theme = openNoteDefaultTheme());

    void setLanguage(Language lang);

    Language language() const { return m_language; }

    void applyTheme(const SyntaxTheme& theme);

protected:
    void highlightBlock(const QString& text) override;

private:
    struct Rule {
        QRegularExpression pattern;
        QTextCharFormat format;
        int captureGroup = 0;
    };

    void buildRules();

    void addKeywords(const QStringList& words, const QTextCharFormat& fmt);

    void initFormatsFromTheme();

    Language m_language = Language::None;
    SyntaxTheme m_theme;
    QVector<Rule> m_rules;

    // Multi-line comment / string block state
    QTextCharFormat m_mlCommentFmt;
    QRegularExpression m_mlCommentStart;
    QRegularExpression m_mlCommentEnd;
    QTextCharFormat m_mlStringFmt;
    QRegularExpression m_mlStringDelim;

    // Shared formats
    QTextCharFormat m_keywordFmt;
    QTextCharFormat m_typeFmt;
    QTextCharFormat m_stringFmt;
    QTextCharFormat m_commentFmt;
    QTextCharFormat m_numberFmt;
    QTextCharFormat m_preprocFmt;
    QTextCharFormat m_funcFmt;
    QTextCharFormat m_decoratorFmt;
    QTextCharFormat m_tagFmt;
    QTextCharFormat m_attrFmt;
};

#endif  // OPENNOTE_LINUX_SYNTAXHIGHLIGHTER_H