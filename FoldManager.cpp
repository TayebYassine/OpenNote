#include "FoldManager.h"
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextDocument>

FoldManager::FoldManager(QTextDocument* document, Language lang)
    : m_document(document), m_language(lang) {}

void FoldManager::setLanguage(Language lang) {
    if (m_language != lang) {
        clear();
        m_language = lang;
        detectFolds();
    }
}

void FoldManager::clear() {
    // Unfold everything first
    for (auto it = m_folds.begin(); it != m_folds.end(); ++it) {
        if (it.value().isFolded) {
            unfoldRegion(it.key());
        }
    }
    m_folds.clear();
}

void FoldManager::detectFolds() {
    if (!m_document)
        return;

    m_folds.clear();

    switch (m_language) {
    case Language::C:
    case Language::Cpp:
    case Language::Java:
    case Language::CSharp:
    case Language::JavaScript:
    case Language::TypeScript:
    case Language::Kotlin:
    case Language::Groovy:
    case Language::Swift:
    case Language::Rust:
    case Language::Go:
    case Language::Zig:
    case Language::CSS:
    case Language::PHP:
    case Language::ActionScript:
    case Language::Dart:
    case Language::Perl:
    case Language::Scala:
    case Language::JSON:
        detectBraceFolds();
        break;

    case Language::Python:
        detectIndentFolds();
        break;

    case Language::Bash:
    case Language::Ruby:
    case Language::Lua:
        detectKeywordFolds();
        break;

    case Language::HTML:
    case Language::XML:
        detectXmlFolds();
        break;

    default:
        //detectBraceFolds();
        break;
    }
}

void FoldManager::detectBraceFolds() {
    if (!m_document)
        return;

    QTextBlock block = m_document->firstBlock();
    int lineNum = 0;

    while (block.isValid()) {
        QString text = block.text();

        // Look for opening braces
        int bracePos = text.indexOf('{');
        if (bracePos >= 0) {
            // Find matching closing brace
            int endLine = findMatchingBrace(lineNum, "{", "}");
            if (endLine > lineNum + 1) {
                // Only fold if more than 1 line
                m_folds.insert(lineNum, FoldRegion(lineNum, endLine));
            }
        }

        block = block.next();
        ++lineNum;
    }
}

void FoldManager::detectIndentFolds() {
    if (!m_document)
        return;

    QTextBlock block = m_document->firstBlock();
    int lineNum = 0;

    QList<QPair<int, int>> indentStack;

    while (block.isValid()) {
        QString text = block.text();

        // Skip empty lines and comments
        QString trimmed = text.trimmed();
        if (trimmed.isEmpty() || trimmed.startsWith('#')) {
            block = block.next();
            ++lineNum;
            continue;
        }

        int indent = getIndentLevel(text);

        // Pop stack until we find parent indent level
        while (!indentStack.isEmpty() && indentStack.last().second >= indent) {
            auto popped = indentStack.takeLast();
            if (lineNum - popped.first > 1) {
                m_folds.insert(popped.first, FoldRegion(popped.first, lineNum - 1));
            }
        }

        // Check if this line ends with ':'
        if (trimmed.endsWith(':') && !trimmed.startsWith('#')) {
            indentStack.append(qMakePair(lineNum, indent));
        }

        block = block.next();
        ++lineNum;
    }

    // Close remaining folds
    while (!indentStack.isEmpty()) {
        auto popped = indentStack.takeLast();
        if (lineNum - popped.first > 1) {
            m_folds.insert(popped.first, FoldRegion(popped.first, lineNum - 1));
        }
    }
}

void FoldManager::detectKeywordFolds() {
    if (!m_document)
        return;

    QTextBlock block = m_document->firstBlock();
    int lineNum = 0;

    QMap<QString, QString> keywords;

    // Bash / Shell
    if (m_language == Language::Bash) {
        keywords = {
            {"if", "fi"}, {"case", "esac"}, {"for", "done"},
            {"while", "done"}, {"until", "done"}, {"function", "}"}
        };
    }

    // Ruby
    else if (m_language == Language::Ruby) {
        keywords = {
            {"def", "end"}, {"class", "end"}, {"module", "end"},
            {"if", "end"}, {"unless", "end"}, {"case", "end"},
            {"while", "end"}, {"until", "end"}, {"for", "end"},
            {"begin", "end"}, {"do", "end"}
        };
    }

    // Lua
    else if (m_language == Language::Lua) {
        keywords = {
            {"function", "end"}, {"if", "end"}, {"for", "end"},
            {"while", "end"}, {"repeat", "until"}, {"do", "end"}
        };
    }

    QList<QPair<int, QString>> stack;

    while (block.isValid()) {
        QString text = block.text().trimmed();

        // Check for opening keywords
        for (auto it = keywords.constBegin(); it != keywords.constEnd(); ++it) {
            if (text.startsWith(it.key() + " ") || text.startsWith(it.key() + "\t") ||
                text == it.key()) {
                stack.append(qMakePair(lineNum, it.key()));
                break;
            }
        }

        // Check for closing keywords
        for (auto it = keywords.constBegin(); it != keywords.constEnd(); ++it) {
            if (text.endsWith(it.value()) || text == it.value()) {
                // Find matching opening
                for (int i = stack.size() - 1; i >= 0; --i) {
                    if (keywords.value(stack[i].second) == it.value()) {
                        int startLine = stack[i].first;
                        if (lineNum - startLine > 1) {
                            m_folds.insert(startLine, FoldRegion(startLine, lineNum));
                        }
                        stack.removeAt(i);
                        break;
                    }
                }
                break;
            }
        }

        block = block.next();
        ++lineNum;
    }
}

void FoldManager::detectXmlFolds() {
    if (!m_document)
        return;

    QTextBlock block = m_document->firstBlock();
    int lineNum = 0;

    QRegularExpression openTag(R"(<([a-zA-Z][a-zA-Z0-9]*)[^>]*(?<!/)>)");
    QRegularExpression closeTag(R"(</([a-zA-Z][a-zA-Z0-9]*)>)");
    QRegularExpression selfClosing(R"(<[^>]+/>)");

    QList<QPair<int, QString>> tagStack;

    while (block.isValid()) {
        QString text = block.text();

        // Find all tags in this line
        auto openIt = openTag.globalMatch(text);
        auto closeIt = closeTag.globalMatch(text);

        QList<QPair<int, QString>> opens;
        QList<QPair<int, QString>> closes;

        while (openIt.hasNext()) {
            auto match = openIt.next();
            opens.append(qMakePair(match.capturedStart(), match.captured(1)));
        }

        while (closeIt.hasNext()) {
            auto match = closeIt.next();
            closes.append(qMakePair(match.capturedStart(), match.captured(1)));
        }

        // Process in order of appearance
        int openIdx = 0, closeIdx = 0;

        while (openIdx < opens.size() || closeIdx < closes.size()) {
            bool useOpen = false;

            if (openIdx >= opens.size()) {
                useOpen = false;
            }
            else if (closeIdx >= closes.size()) {
                useOpen = true;
            }
            else {
                useOpen = opens[openIdx].first < closes[closeIdx].first;
            }

            if (useOpen) {
                tagStack.append(qMakePair(lineNum, opens[openIdx].second));
                ++openIdx;
            }
            else {
                // Find matching open tag
                for (int i = tagStack.size() - 1; i >= 0; --i) {
                    if (tagStack[i].second == closes[closeIdx].second) {
                        int startLine = tagStack[i].first;
                        if (lineNum - startLine > 0) {
                            m_folds.insert(startLine, FoldRegion(startLine, lineNum));
                        }
                        tagStack.removeAt(i);
                        break;
                    }
                }
                ++closeIdx;
            }
        }

        block = block.next();
        ++lineNum;
    }
}

int FoldManager::findMatchingBrace(int startLine, const QString& openChar,
                                   const QString& closeChar) {
    if (!m_document)
        return -1;

    QTextBlock block = m_document->findBlockByNumber(startLine);
    if (!block.isValid())
        return -1;

    int depth = 0;
    int lineNum = startLine;
    bool firstLine = true;

    while (block.isValid()) {
        QString text = block.text();

        // On first line, start after the opening brace
        int startPos = 0;
        if (firstLine) {
            startPos = text.indexOf(openChar);
            if (startPos >= 0)
                startPos++;
            firstLine = false;
        }

        for (int i = startPos; i < text.length(); ++i) {
            if (text.mid(i, openChar.length()) == openChar) {
                depth++;
            }
            else if (text.mid(i, closeChar.length()) == closeChar) {
                if (depth == 0) {
                    return lineNum;
                }
                depth--;
            }
        }

        block = block.next();
        ++lineNum;
    }

    return -1;
}

int FoldManager::getIndentLevel(const QString& line) const {
    int indent = 0;
    for (QChar c : line) {
        if (c == ' ')
            indent++;
        else if (c == '\t')
            indent += 4; // Treat tab as 4 spaces
        else
            break;
    }
    return indent;
}

FoldRegion FoldManager::getFoldAt(int line) const {
    return m_folds.value(line, FoldRegion());
}

bool FoldManager::isFoldStart(int line) const { return m_folds.contains(line); }

bool FoldManager::isFolded(int line) const {
    return m_folds.value(line).isFolded;
}

void FoldManager::toggleFold(int line) {
    if (!m_folds.contains(line))
        return;

    if (m_folds[line].isFolded) {
        unfoldRegion(line);
    }
    else {
        foldRegion(line);
    }
}

void FoldManager::foldRegion(int line) {
    if (!m_folds.contains(line))
        return;
    if (m_folds[line].isFolded)
        return;

    FoldRegion& region = m_folds[line];
    applyFold(region.startLine, region.endLine, true);
    region.isFolded = true;
}

void FoldManager::unfoldRegion(int line) {
    if (!m_folds.contains(line))
        return;
    if (!m_folds[line].isFolded)
        return;

    FoldRegion& region = m_folds[line];

    // First, mark this region as unfolded
    region.isFolded = false;

    // Make blocks visible, but check if they're part of another folded region
    for (int i = region.startLine + 1; i <= region.endLine; ++i) {
        QTextBlock block = m_document->findBlockByNumber(i);
        if (!block.isValid())
            continue;

        // Only make visible if not inside another folded region
        bool isInOtherFold = false;
        for (auto it = m_folds.constBegin(); it != m_folds.constEnd(); ++it) {
            if (it.key() == line)
                continue; // Skip the region we're unfolding

            const FoldRegion& otherRegion = it.value();
            if (otherRegion.isFolded && i > otherRegion.startLine &&
                i <= otherRegion.endLine) {
                isInOtherFold = true;
                break;
            }
        }

        if (!isInOtherFold) {
            block.setVisible(true);
        }
    }

    // Trigger document layout update
    m_document->markContentsDirty(
        m_document->findBlockByNumber(region.startLine).position(),
        m_document->findBlockByNumber(region.endLine).position() +
        m_document->findBlockByNumber(region.endLine).length());
}

void FoldManager::applyFold(int startLine, int endLine, bool fold) {
    if (!m_document)
        return;

    // Hide/show blocks from startLine + 1 to endLine (inclusive)
    for (int i = startLine + 1; i <= endLine; ++i) {
        QTextBlock block = m_document->findBlockByNumber(i);
        if (block.isValid()) {
            block.setVisible(!fold);
        }
    }

    // Trigger document layout update
    m_document->markContentsDirty(
        m_document->findBlockByNumber(startLine).position(),
        m_document->findBlockByNumber(endLine).position() +
        m_document->findBlockByNumber(endLine).length());
}

bool FoldManager::isLineInFoldedRegion(int line) const {
    for (auto it = m_folds.constBegin(); it != m_folds.constEnd(); ++it) {
        const FoldRegion& region = it.value();
        if (region.isFolded && line > region.startLine && line <= region.endLine) {
            return true;
        }
    }
    return false;
}
