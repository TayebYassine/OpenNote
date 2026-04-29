#ifndef AUTOCOMPLETER_H
#define AUTOCOMPLETER_H

#include <QCompleter>
#include <QObject>
#include <QSet>
#include <QStringList>
#include <QStringListModel>

class CodeEditor;

class AutoCompleter : public QObject {
    Q_OBJECT
public:
    explicit AutoCompleter(CodeEditor* editor);

    void updateWordList(const QString& text);
    void showCompletion();
    void hideCompletion();

    bool isPopupVisible() const;

private:
    CodeEditor* m_editor;
    QCompleter* m_completer;
    QStringListModel* m_model;
    QSet<QString> m_wordSet;

    QString wordUnderCursor() const;
    void insertCompletion(const QString& completion);
};

#endif