#include "SyntaxHighlighter.h"

#include <QApplication>
#include <QFont>

#include "AppDatabase.h"

SyntaxHighlighter::SyntaxHighlighter(QTextDocument* parent, Language lang,
                                     SyntaxTheme theme)
    : QSyntaxHighlighter(parent), m_language(lang), m_theme(std::move(theme)) {
    initFormatsFromTheme();
    buildRules();
}

void SyntaxHighlighter::setLanguage(Language lang) {
    m_language = lang;
    buildRules();
    rehighlight();
}

void SyntaxHighlighter::applyTheme(const SyntaxTheme& theme) {
    m_theme = theme;
    initFormatsFromTheme();
    buildRules();
    rehighlight();
}

void SyntaxHighlighter::initFormatsFromTheme() {
    auto apply = [](QTextCharFormat& fmt, const TokenStyle& ts) {
        fmt = QTextCharFormat(); // reset
        fmt.setForeground(QColor(ts.color));
        fmt.setFontWeight(ts.bold ? QFont::Bold : QFont::Normal);
        fmt.setFontItalic(ts.italic);
    };

    const SyntaxTheme def = openNoteDefaultTheme();
    apply(m_keywordFmt, m_theme.value("keyword", def["keyword"]));
    apply(m_typeFmt, m_theme.value("type", def["type"]));
    apply(m_stringFmt, m_theme.value("string", def["string"]));
    apply(m_commentFmt, m_theme.value("comment", def["comment"]));
    apply(m_numberFmt, m_theme.value("number", def["number"]));
    apply(m_preprocFmt, m_theme.value("preprocessor", def["preprocessor"]));
    apply(m_funcFmt, m_theme.value("function", def["function"]));
    apply(m_decoratorFmt, m_theme.value("decorator", def["decorator"]));
    apply(m_tagFmt, m_theme.value("tag", def["tag"]));
    apply(m_attrFmt, m_theme.value("attribute", def["attribute"]));
}

void SyntaxHighlighter::addKeywords(const QStringList& words,
                                    const QTextCharFormat& fmt) {
    m_rules.append(
        {QRegularExpression("\\b(" + words.join('|') + ")\\b"), fmt, 0});
}

void SyntaxHighlighter::buildRules() {
    m_rules.clear();
    m_mlCommentStart = QRegularExpression();
    m_mlCommentEnd = QRegularExpression();
    m_mlStringDelim = QRegularExpression();

    // To be honest, I used AI to help me create the token rules (since it would take months for me if I wrote this manually).
    switch (m_language) {
    case Language::C: {
        m_rules.append({QRegularExpression(R"(^\s*#\s*\w+)"), m_preprocFmt});
        addKeywords(
            {
                "auto", "break", "case", "const", "continue", "default", "do",
                "else", "enum", "extern", "for", "goto", "if", "inline", "register",
                "restrict", "return", "sizeof", "static", "struct", "switch",
                "typedef", "union", "void", "volatile", "while", "false", "true",
                "NULL", "nullptr",
                // C11
                "_Alignas", "_Alignof", "_Atomic", "_Bool", "_Complex", "_Generic",
                "_Imaginary", "_Noreturn", "_Static_assert", "_Thread_local"
            },
            m_keywordFmt);
        addKeywords({
                        "int", "long", "short", "char", "bool",
                        "float", "double", "unsigned", "signed", "size_t",
                        "ptrdiff_t", "wchar_t", "int8_t", "int16_t", "int32_t",
                        "int64_t", "uint8_t", "uint16_t", "uint32_t", "uint64_t",
                        "intptr_t", "uintptr_t", "intmax_t", "uintmax_t"
                    },
                    m_typeFmt);
        m_rules.append(
            {
                QRegularExpression(
                    R"(\b(0[xX][0-9a-fA-F]+[lLuU]*|\d+\.?\d*([eE][+-]?\d+)?[fFlLuU]*)\b)"),
                m_numberFmt
            });
        m_rules.append({QRegularExpression(R"("(?:[^"\\]|\\.)*")"), m_stringFmt});
        m_rules.append({QRegularExpression(R"('(?:[^'\\]|\\.)*')"), m_stringFmt});
        m_rules.append(
            {QRegularExpression(R"(\b([A-Za-z_]\w*)(?=\s*\())"), m_funcFmt});
        m_rules.append({QRegularExpression(R"(//.*)"), m_commentFmt});
        m_mlCommentFmt = m_commentFmt;
        m_mlCommentStart = QRegularExpression(R"(/\*)");
        m_mlCommentEnd = QRegularExpression(R"(\*/)");
        break;
    }

    case Language::Cpp: {
        m_rules.append({QRegularExpression(R"(^\s*#\s*\w+)"), m_preprocFmt});
        // C++ attributes  [[nodiscard]], [[maybe_unused]], etc.
        m_rules.append({QRegularExpression(R"(\[\[[^\]]*\]\])"), m_decoratorFmt});
        addKeywords({
                        "alignas",
                        "alignof",
                        "auto",
                        "break",
                        "case",
                        "catch",
                        "class",
                        "concept",
                        "const",
                        "const_cast",
                        "consteval",
                        "constexpr",
                        "constinit",
                        "continue",
                        "co_await",
                        "co_return",
                        "co_yield",
                        "default",
                        "delete",
                        "do",
                        "dynamic_cast",
                        "else",
                        "enum",
                        "explicit",
                        "export",
                        "extern",
                        "false",
                        "final",
                        "for",
                        "friend",
                        "goto",
                        "if",
                        "import",
                        "inline",
                        "module",
                        "mutable",
                        "namespace",
                        "new",
                        "noexcept",
                        "nullptr",
                        "operator",
                        "override",
                        "private",
                        "protected",
                        "public",
                        "register",
                        "reinterpret_cast",
                        "requires",
                        "return",
                        "sizeof",
                        "static",
                        "static_assert",
                        "static_cast",
                        "struct",
                        "switch",
                        "template",
                        "this",
                        "thread_local",
                        "throw",
                        "true",
                        "try",
                        "typedef",
                        "typeid",
                        "typename",
                        "union",
                        "using",
                        "virtual",
                        "void",
                        "volatile",
                        "while",
                        "decltype"
                    },
                    m_keywordFmt);
        addKeywords(
            {
                "bool", "char", "char8_t", "char16_t", "char32_t",
                "double", "float", "int", "long", "short",
                "signed", "unsigned", "wchar_t", "int8_t", "int16_t",
                "int32_t", "int64_t", "uint8_t", "uint16_t", "uint32_t",
                "uint64_t", "size_t", "ptrdiff_t", "intptr_t", "uintptr_t",
                "intmax_t", "uintmax_t", "nullptr_t"
            },
            m_typeFmt);
        m_rules.append(
            {
                QRegularExpression(
                    R"(\b(0[xXbB][0-9a-fA-F_]+[lLuU]*|\d[\d_]*\.?[\d_]*([eE][+-]?[\d_]+)?[fFlLuU]*)\b)"),
                m_numberFmt
            });
        m_rules.append({QRegularExpression(R"("(?:[^"\\]|\\.)*")"), m_stringFmt});
        m_rules.append({QRegularExpression(R"('(?:[^'\\]|\\.)*')"), m_stringFmt});
        m_rules.append(
            {QRegularExpression(R"(\b([A-Za-z_]\w*)(?=\s*\())"), m_funcFmt});
        m_rules.append({QRegularExpression(R"(//.*)"), m_commentFmt});
        m_mlCommentFmt = m_commentFmt;
        m_mlCommentStart = QRegularExpression(R"(/\*)");
        m_mlCommentEnd = QRegularExpression(R"(\*/)");
        break;
    }

    case Language::CSharp: {
        m_rules.append(
            {
                QRegularExpression(
                    R"(^\\s*#\\s*(if|else|elif|endif|define|undef|warning|error|line|region|endregion|pragma|nullable))"),
                m_preprocFmt
            });
        addKeywords(
            {
                "abstract", "as", "base", "bool", "break", "byte", "case", "catch",
                "char", "checked", "class", "const", "continue", "decimal",
                "default", "delegate", "do", "double", "else", "enum", "event",
                "explicit", "extern", "false", "finally", "fixed", "float", "for",
                "foreach", "goto", "if", "implicit", "in", "int", "interface",
                "internal", "is", "lock", "long", "namespace", "new", "null",
                "object", "operator", "out", "override", "params", "private",
                "protected", "public", "readonly", "ref", "return", "sbyte",
                "sealed", "short", "sizeof", "stackalloc", "static", "string",
                "struct", "switch", "this", "throw", "true", "try", "typeof", "uint",
                "ulong", "unchecked", "unsafe", "ushort", "using", "virtual", "void",
                "volatile", "while",
                // C# 8.0+
                "async", "await", "when", "nameof", "var", "dynamic", "yield",
                "record", "init", "required", "file", "scoped", "with"
            },
            m_keywordFmt);
        addKeywords(
            {
                "add", "alias", "ascending", "async", "await", "by",
                "descending", "dynamic", "equals", "from", "get", "global",
                "group", "into", "join", "let", "nameof", "on",
                "orderby", "partial", "remove", "select", "set", "value",
                "var", "when", "where", "yield"
            },
            m_keywordFmt);
        addKeywords({
                        "Boolean",
                        "Byte",
                        "SByte",
                        "Char",
                        "Decimal",
                        "Double",
                        "Single",
                        "Int32",
                        "UInt32",
                        "Int64",
                        "UInt64",
                        "Int16",
                        "UInt16",
                        "Object",
                        "String",
                        "Void",
                        "DateTime",
                        "TimeSpan",
                        "Guid",
                        "Uri",
                        "Task",
                        "ValueTask",
                        "List",
                        "Dictionary",
                        "HashSet",
                        "Queue",
                        "Stack",
                        "IEnumerable",
                        "IEnumerator",
                        "ICollection",
                        "IList",
                        "IDictionary",
                        "IComparable",
                        "IEquatable",
                        "IDisposable",
                        "Exception",
                        "ArgumentException",
                        "InvalidOperationException",
                        "NullReferenceException",
                        "NotImplementedException"
                    },
                    m_typeFmt);
        m_rules.append({
            QRegularExpression(
                R"(\\[[A-Za-z_]\\w*(?:Attribute)?(?:\\(.*?\\))?\\])"),
            m_decoratorFmt
        });
        m_rules.append(
            {
                QRegularExpression(
                    R"(\\b(0[xX][0-9a-fA-F_]+[lLuU]*|\\d[\\d_]*\\.?[\\d_]*([eE][+-]?[\\d_]+)?[mMfFdDlLuU]*|0[bB][01_]+)\\b)"),
                m_numberFmt
            });
        m_rules.append(
            {QRegularExpression(R"(@\"(?:[^\"]|\"\")*\")"), m_stringFmt});
        m_rules.append(
            {QRegularExpression(R"(\"(?:[^\"\\\\]|\\\\.)*\")"), m_stringFmt});
        m_rules.append(
            {QRegularExpression(R"('(?:[^'\\\\]|\\\\.)*')"), m_stringFmt});
        m_rules.append({QRegularExpression(R"(\\$\"[^\"]*\")"), m_stringFmt});
        m_rules.append(
            {QRegularExpression(R"(\b([A-Za-z_]\w*)(?=\s*\())"), m_funcFmt});
        m_rules.append({QRegularExpression(R"(//.*)"), m_commentFmt});
        m_mlCommentStart = QRegularExpression(R"(/\\*)");
        m_mlCommentEnd = QRegularExpression(R"(\\*/)");
        m_rules.append({QRegularExpression(R"(///.*)"), m_preprocFmt});
        break;
    }

    case Language::Java: {
        addKeywords(
            {
                "abstract", "assert", "break", "case", "catch",
                "class", "const", "continue", "default", "do",
                "else", "enum", "extends", "final", "finally",
                "for", "goto", "if", "implements", "import",
                "instanceof", "interface", "native", "new", "package",
                "private", "protected", "public", "return", "static",
                "strictfp", "super", "switch", "synchronized", "this",
                "throw", "throws", "transient", "try", "void",
                "volatile", "while", "true", "false", "null",
                "var", "record", "sealed", "permits", "yield",
                "non-sealed"
            },
            m_keywordFmt);
        addKeywords({
                        "boolean", "byte", "char", "double", "float", "int", "long",
                        "short"
                    },
                    m_typeFmt);
        // PascalCase class names
        m_rules.append(
            {QRegularExpression(R"(\b[A-Z][A-Za-z0-9_]*\b)"), m_typeFmt});
        m_rules.append(
            {QRegularExpression(R"(\b\d+\.?\d*[fFdDlL]?\b)"), m_numberFmt});
        m_rules.append({QRegularExpression(R"("(?:[^"\\]|\\.)*")"), m_stringFmt});
        m_rules.append({QRegularExpression(R"('(?:[^'\\]|\\.)*')"), m_stringFmt});
        m_rules.append(
            {QRegularExpression(R"(\b([A-Za-z_]\w*)(?=\s*\())"), m_funcFmt});
        m_rules.append({QRegularExpression(R"(@[A-Za-z_]\w*)"), m_decoratorFmt});
        m_rules.append({QRegularExpression(R"(//.*)"), m_commentFmt});
        m_mlCommentFmt = m_commentFmt;
        m_mlCommentStart = QRegularExpression(R"(/\*)");
        m_mlCommentEnd = QRegularExpression(R"(\*/)");
        break;
    }

    case Language::Kotlin: {
        addKeywords(
            {
                "abstract", "actual", "annotation", "as", "break",
                "by", "catch", "class", "companion", "const",
                "constructor", "continue", "crossinline", "data", "do",
                "dynamic", "else", "enum", "expect", "external",
                "false", "final", "finally", "for", "fun",
                "get", "if", "import", "in", "infix",
                "init", "inline", "inner", "interface", "internal",
                "is", "it", "lateinit", "noinline", "null",
                "object", "open", "operator", "out", "override",
                "package", "private", "protected", "public", "reified",
                "return", "sealed", "set", "super", "suspend",
                "tailrec", "this", "throw", "true", "try",
                "typealias", "val", "value", "var", "vararg",
                "when", "where", "while"
            },
            m_keywordFmt);
        addKeywords({
                        "Any", "Boolean", "Byte", "Char",
                        "Double", "Float", "Int", "Long",
                        "Nothing", "Number", "Short", "String",
                        "Unit", "Array", "BooleanArray", "ByteArray",
                        "CharArray", "DoubleArray", "FloatArray", "IntArray",
                        "LongArray", "ShortArray", "List", "MutableList",
                        "Map", "MutableMap", "Set", "MutableSet",
                        "Pair", "Triple", "Sequence", "Result",
                        "Lazy"
                    },
                    m_typeFmt);
        m_rules.append({QRegularExpression(R"(@[A-Za-z_]\w*)"), m_decoratorFmt});
        m_rules.append(
            {
                QRegularExpression(R"(\b\d+\.?\d*([eE][+-]?\d+)?[fFlLuU]?\b)"),
                m_numberFmt
            });
        m_rules.append(
            {QRegularExpression(R"(0[xX][0-9a-fA-F_]+)"), m_numberFmt});
        m_rules.append({QRegularExpression(R"("(?:[^"\\]|\\.)*")"), m_stringFmt});
        m_rules.append({QRegularExpression(R"('(?:[^'\\]|\\.)*')"), m_stringFmt});
        // Raw / triple-quoted strings (single-line portion)
        m_rules.append({QRegularExpression(R"("""[^"]*""")"), m_stringFmt});
        m_rules.append(
            {QRegularExpression(R"(\b([A-Za-z_]\w*)(?=\s*\())"), m_funcFmt});
        m_rules.append({QRegularExpression(R"(//.*)"), m_commentFmt});
        m_mlCommentFmt = m_commentFmt;
        m_mlCommentStart = QRegularExpression(R"(/\*)");
        m_mlCommentEnd = QRegularExpression(R"(\*/)");
        break;
    }

    case Language::Groovy: {
        addKeywords(
            {
                "as", "assert", "break", "case", "catch", "class", "const",
                "continue", "def", "default", "do", "else", "enum", "extends",
                "false", "finally", "for", "goto", "if", "implements", "import",
                "in", "instanceof", "interface", "new", "null", "package", "return",
                "super", "switch", "this", "throw", "throws", "trait", "true", "try",
                "while", "abstract", "final", "native", "private", "protected",
                "public", "static", "strictfp", "synchronized", "transient",
                "volatile",
                // Groovy specific
                "it", "with", "times", "upto", "downto", "step", "each",
                "eachWithIndex", "collect", "find", "findAll", "grep", "inject",
                "every", "any", "groupBy"
            },
            m_keywordFmt);
        addKeywords(
            {
                "boolean", "byte", "char", "double", "float", "int",
                "long", "short", "void", "Boolean", "Byte", "Character",
                "Double", "Float", "Integer", "Long", "Short", "String",
                "Object", "Class", "Closure", "GString", "List", "Map",
                "Set", "Range", "Pattern", "Matcher", "BigDecimal", "BigInteger",
                "Date", "File", "URL", "URI"
            },
            m_typeFmt);
        addKeywords({
                        "println", "print", "printf", "sprintf",
                        "sleep", "inspect", "dump", "toString",
                        "equals", "hashCode", "getClass", "invokeMethod",
                        "getProperty", "setProperty", "hasProperty", "respondsTo",
                        "is", "asType", "asBoolean", "use",
                        "tap", "withTraits"
                    },
                    m_funcFmt);
        m_rules.append(
            {
                QRegularExpression(
                    R"(\b(0[xX][0-9a-fA-F]+|0[0-7]+|0[bB][01]+|\d+\.?\d*([eE][+-]?\d+)?[gGdDfF]?)\b)"),
                m_numberFmt
            });
        m_rules.append({QRegularExpression(R"("""[\s\S]*?""")"), m_stringFmt});
        m_rules.append({
            QRegularExpression(R"("(?:[^"\\$]|\\.|\$[A-Za-z_{])*")"),
            m_stringFmt
        });
        m_rules.append({QRegularExpression(R"('(?:[^'\\]|\\.)*')"), m_stringFmt});
        m_rules.append({
            QRegularExpression(R"(/[^/\\]*/[gimxsuU]?)"),
            m_stringFmt
        }); // regex
        // m_rules.append({QRegularExpression(R"(\$[A-Za-z_]\w*|\$\{[^}]*\})"),
        // m_specialFmt});
        m_rules.append(
            {QRegularExpression(R"(\b([A-Za-z_]\w*)(?=\s*\())"), m_funcFmt});
        m_rules.append({QRegularExpression(R"(//.*)"), m_commentFmt});
        m_rules.append({QRegularExpression(R"(#(?!\!).*)"), m_commentFmt});
        m_mlCommentStart = QRegularExpression(R"(/\*)");
        m_mlCommentEnd = QRegularExpression(R"(\*/)");
        m_rules.append({QRegularExpression(R"(^#!.*)"), m_preprocFmt});
        m_rules.append(
            {QRegularExpression(R"(@[A-Za-z_]\w*(?:\(.*?\))?)"), m_decoratorFmt});
        break;
    }

    case Language::Python: {
        addKeywords(
            {
                "and", "as", "assert", "async", "await", "break",
                "class", "continue", "def", "del", "elif", "else",
                "except", "False", "finally", "for", "from", "global",
                "if", "import", "in", "is", "lambda", "None",
                "nonlocal", "not", "or", "pass", "raise", "return",
                "True", "try", "type", "while", "with", "yield"
            },
            m_keywordFmt);
        addKeywords(
            {
                // Built-in types & type-hint generics
                "bool", "bytes", "bytearray", "complex", "dict", "float",
                "frozenset", "int", "list", "memoryview", "object", "range", "set",
                "str", "tuple",
                // typing module (commonly used without import in annotations)
                "Any", "Callable", "ClassVar", "Final", "Generic", "Iterator",
                "Literal", "Optional", "Protocol", "Sequence", "TypeVar", "Union",
                // Built-in functions
                "abs", "aiter", "all", "anext", "any", "bin", "breakpoint",
                "callable", "chr", "compile", "delattr", "dir", "divmod",
                "enumerate", "eval", "exec", "filter", "format", "getattr",
                "globals", "hasattr", "hash", "help", "hex", "id", "input",
                "isinstance", "issubclass", "iter", "len", "locals", "map", "max",
                "min", "next", "oct", "open", "ord", "pow", "print", "property",
                "repr", "reversed", "round", "setattr", "slice", "sorted",
                "staticmethod", "sum", "super", "type", "vars", "zip"
            },
            m_typeFmt);
        m_rules.append({
            QRegularExpression(R"(\b\d+\.?\d*([eEjJ][+-]?\d*)?\b)"),
            m_numberFmt
        });
        m_rules.append(
            {QRegularExpression(R"(0[xX][0-9a-fA-F_]+)"), m_numberFmt});
        m_rules.append({QRegularExpression(R"(0[oO][0-7_]+)"), m_numberFmt});
        m_rules.append({QRegularExpression(R"(0[bB][01_]+)"), m_numberFmt});
        // f-strings / b-strings / r-strings (prefix-agnostic single-line)
        m_rules.append({
            QRegularExpression(R"([fFbBrRuU]{0,2}'(?:[^'\\]|\\.)*')"),
            m_stringFmt
        });
        m_rules.append({
            QRegularExpression(R"([fFbBrRuU]{0,2}"(?:[^"\\]|\\.)*")"),
            m_stringFmt
        });
        m_rules.append(
            {QRegularExpression(R"(\b([A-Za-z_]\w*)(?=\s*\())"), m_funcFmt});
        m_rules.append(
            {QRegularExpression(R"(@[A-Za-z_][\w.]*)"), m_decoratorFmt});
        m_rules.append({QRegularExpression(R"(#.*)"), m_commentFmt});
        // Triple-quoted strings (block state 2)
        m_mlStringFmt = m_stringFmt;
        m_mlStringDelim = QRegularExpression(R"(\"\"\"|\'\'\')");
        break;
    }

    case Language::Ruby: {
        addKeywords(
            {
                "BEGIN", "END", "alias", "and", "begin",
                "break", "case", "class", "def", "defined?",
                "do", "else", "elsif", "end", "ensure",
                "false", "for", "if", "in", "module",
                "next", "nil", "not", "or", "redo",
                "rescue", "retry", "return", "self", "super",
                "then", "true", "undef", "unless", "until",
                "when", "while", "yield", "__FILE__", "__LINE__",
                "__ENCODING__", "__method__", "__dir__", "__callee__"
            },
            m_keywordFmt);
        addKeywords({
                        "attr_accessor",
                        "attr_reader",
                        "attr_writer",
                        "include",
                        "extend",
                        "prepend",
                        "protected",
                        "private",
                        "public",
                        "require",
                        "require_relative",
                        "puts",
                        "print",
                        "p",
                        "pp",
                        "raise",
                        "fail",
                        "abort",
                        "exit",
                        "gets",
                        "freeze",
                        "frozen?"
                    },
                    m_funcFmt);
        // Symbols  :name  or  :"string"
        m_rules.append(
            {QRegularExpression(R"(:[A-Za-z_]\w*[?!]?)"), m_preprocFmt});
        // Instance / class variables  @name  @@name
        m_rules.append({QRegularExpression(R"(@@?[A-Za-z_]\w*)"), m_typeFmt});
        // Global variables  $name
        m_rules.append({QRegularExpression(R"(\$[A-Za-z_]\w*)"), m_typeFmt});
        // Constants (PascalCase / UPPER_SNAKE)
        m_rules.append(
            {QRegularExpression(R"(\b[A-Z][A-Za-z0-9_]*\b)"), m_typeFmt});
        m_rules.append(
            {QRegularExpression(R"(\b\d+\.?\d*([eE][+-]?\d+)?\b)"), m_numberFmt});
        m_rules.append(
            {QRegularExpression(R"(0[xX][0-9a-fA-F_]+)"), m_numberFmt});
        m_rules.append({QRegularExpression(R"("(?:[^"\\]|\\.)*")"), m_stringFmt});
        m_rules.append({QRegularExpression(R"('(?:[^'\\]|\\.)*')"), m_stringFmt});
        m_rules.append(
            {QRegularExpression(R"(\b([A-Za-z_]\w*[?!]?)(?=\s*\())"), m_funcFmt});
        m_rules.append({QRegularExpression(R"(#.*)"), m_commentFmt});
        break;
    }

    case Language::Lua: {
        addKeywords({
                        "and", "break", "do", "else", "elseif", "end",
                        "false", "for", "function", "goto", "if", "in",
                        "local", "nil", "not", "or", "repeat", "return",
                        "then", "true", "until", "while"
                    },
                    m_keywordFmt);
        addKeywords(
            {
                // Standard library tables / built-in functions
                "assert", "collectgarbage", "dofile", "error",
                "getmetatable", "ipairs", "load", "loadfile",
                "next", "pairs", "pcall", "print",
                "rawequal", "rawget", "rawlen", "rawset",
                "require", "select", "setmetatable", "tonumber",
                "tostring", "type", "xpcall", "string",
                "table", "math", "io", "os",
                "coroutine", "package", "utf8"
            },
            m_typeFmt);
        m_rules.append(
            {QRegularExpression(R"(\b\d+\.?\d*([eE][+-]?\d+)?\b)"), m_numberFmt});
        m_rules.append({QRegularExpression(R"(0[xX][0-9a-fA-F]+)"), m_numberFmt});
        m_rules.append({QRegularExpression(R"("(?:[^"\\]|\\.)*")"), m_stringFmt});
        m_rules.append({QRegularExpression(R"('(?:[^'\\]|\\.)*')"), m_stringFmt});
        // Long strings  [[ ... ]]  or  [=[ ... ]=]  (single-line only)
        m_rules.append({QRegularExpression(R"(\[=*\[.*?\]=*\])"), m_stringFmt});
        m_rules.append(
            {QRegularExpression(R"(\b([A-Za-z_]\w*)(?=\s*\())"), m_funcFmt});
        m_rules.append({QRegularExpression(R"(--.*)"), m_commentFmt});
        m_mlCommentFmt = m_commentFmt;
        m_mlCommentStart = QRegularExpression(R"(--\[\[)");
        m_mlCommentEnd = QRegularExpression(R"(\]\])");
        break;
    }

    case Language::PHP: {
        // PHP open/close tags
        m_rules.append({QRegularExpression(R"(<\?(?:php|=)?)"), m_preprocFmt});
        m_rules.append({QRegularExpression(R"(\?>)"), m_preprocFmt});
        addKeywords({
                        "abstract", "and", "as", "break",
                        "callable", "case", "catch", "class",
                        "clone", "const", "continue", "declare",
                        "default", "do", "echo", "else",
                        "elseif", "enddeclare", "endfor", "endforeach",
                        "endif", "endswitch", "endwhile", "enum",
                        "extends", "final", "finally", "fn",
                        "for", "foreach", "function", "global",
                        "goto", "if", "implements", "include",
                        "include_once", "instanceof", "insteadof", "interface",
                        "list", "match", "namespace", "new",
                        "null", "or", "print", "private",
                        "protected", "public", "readonly", "require",
                        "require_once", "return", "static", "switch",
                        "throw", "trait", "try", "unset",
                        "use", "var", "while", "xor",
                        "yield", "true", "false"
                    },
                    m_keywordFmt);
        addKeywords({
                        "array", "bool", "callable", "float", "int", "iterable",
                        "mixed", "never", "null", "object", "self", "static",
                        "parent", "string", "void"
                    },
                    m_typeFmt);
        // Variables  $name
        m_rules.append({QRegularExpression(R"(\$[A-Za-z_]\w*)"), m_typeFmt});
        // Magic constants
        m_rules.append(
            {
                QRegularExpression(
                    R"(__(?:FILE|LINE|DIR|FUNCTION|CLASS|TRAIT|METHOD|NAMESPACE|COMPILER_HALT_OFFSET)__)"),
                m_preprocFmt
            });
        m_rules.append(
            {QRegularExpression(R"(\b\d+\.?\d*([eE][+-]?\d+)?\b)"), m_numberFmt});
        m_rules.append(
            {QRegularExpression(R"(0[xX][0-9a-fA-F_]+)"), m_numberFmt});
        m_rules.append({QRegularExpression(R"("(?:[^"\\]|\\.)*")"), m_stringFmt});
        m_rules.append({QRegularExpression(R"('(?:[^'\\]|\\.)*')"), m_stringFmt});
        m_rules.append(
            {QRegularExpression(R"(\b([A-Za-z_]\w*)(?=\s*\())"), m_funcFmt});
        m_rules.append({QRegularExpression(R"(//.*)"), m_commentFmt});
        m_rules.append({
            QRegularExpression(R"(#(?!\!).*)"),
            m_commentFmt
        }); // # but not shebang
        m_mlCommentFmt = m_commentFmt;
        m_mlCommentStart = QRegularExpression(R"(/\*)");
        m_mlCommentEnd = QRegularExpression(R"(\*/)");
        break;
    }

    case Language::JavaScript:
    case Language::TypeScript: {
        QStringList kw = {
            "async", "await", "break", "case", "catch", "class",
            "const", "continue", "debugger", "default", "delete", "do",
            "else", "export", "extends", "false", "finally", "for",
            "from", "function", "if", "import", "in", "instanceof",
            "let", "new", "null", "of", "return", "static",
            "super", "switch", "this", "throw", "true", "try",
            "typeof", "undefined", "var", "void", "while", "with",
            "yield", "get", "set"
        };
        QStringList types = {
            // JS built-ins
            "Array", "ArrayBuffer", "BigInt", "Boolean", "DataView", "Date",
            "Error", "EvalError", "Float32Array", "Float64Array", "Function",
            "Generator", "GlobalThis", "Int8Array", "Int16Array", "Int32Array",
            "JSON", "Map", "Math", "Number", "Object", "Promise", "Proxy",
            "RangeError", "ReferenceError", "Reflect", "RegExp", "Set", "String",
            "Symbol", "SyntaxError", "TypeError", "Uint8Array", "Uint16Array",
            "Uint32Array", "URIError", "WeakMap", "WeakRef", "WeakSet",
            // primitives / annotations
            "any", "boolean", "number", "string", "symbol", "object", "unknown",
            "void", "never", "bigint"
        };
        if (m_language == Language::TypeScript) {
            kw << "abstract" << "as" << "declare" << "enum" << "implements"
                << "interface" << "module" << "namespace" << "override"
                << "private" << "protected" << "public" << "readonly"
                << "satisfies" << "type" << "infer" << "keyof" << "typeof";
            // TS utility types
            types << "Partial" << "Required" << "Readonly" << "Record"
                << "Pick" << "Omit" << "Exclude" << "Extract"
                << "NonNullable" << "ReturnType" << "Parameters"
                << "ConstructorParameters" << "InstanceType" << "Awaited"
                << "Uppercase" << "Lowercase" << "Capitalize" << "Uncapitalize";
        }
        addKeywords(kw, m_keywordFmt);
        addKeywords(types, m_typeFmt);
        m_rules.append({
            QRegularExpression(R"(\b\d+\.?\d*([eE][+-]?\d+)?(n)?\b)"),
            m_numberFmt
        });
        m_rules.append(
            {QRegularExpression(R"(0[xX][0-9a-fA-F_]+)"), m_numberFmt});
        m_rules.append({QRegularExpression(R"("(?:[^"\\]|\\.)*")"), m_stringFmt});
        m_rules.append({QRegularExpression(R"('(?:[^'\\]|\\.)*')"), m_stringFmt});
        m_rules.append({
            QRegularExpression(R"(`(?:[^`\\]|\\.)*`)"),
            m_stringFmt
        }); // template literals
        m_rules.append(
            {QRegularExpression(R"(\b([A-Za-z_$]\w*)(?=\s*\())"), m_funcFmt});
        m_rules.append({QRegularExpression(R"(//.*)"), m_commentFmt});
        m_mlCommentFmt = m_commentFmt;
        m_mlCommentStart = QRegularExpression(R"(/\*)");
        m_mlCommentEnd = QRegularExpression(R"(\*/)");
        break;
    }

    case Language::HTML: {
        m_rules.append({
            QRegularExpression(R"(<\?[^?]*\?>)"),
            m_preprocFmt
        }); // processing instructions
        m_rules.append({QRegularExpression(R"(<!DOCTYPE[^>]*>)"), m_preprocFmt});
        m_rules.append(
            {QRegularExpression(R"(</?[A-Za-z][A-Za-z0-9_-]*)"), m_tagFmt});
        m_rules.append({QRegularExpression(R"(>|/>)"), m_tagFmt});
        m_rules.append(
            {
                QRegularExpression(R"(\b[A-Za-z_:][A-Za-z0-9_:-]*(?=\s*=))"),
                m_attrFmt
            });
        m_rules.append({QRegularExpression(R"("(?:[^"\\]|\\.)*")"), m_stringFmt});
        m_rules.append({QRegularExpression(R"('(?:[^'\\]|\\.)*')"), m_stringFmt});
        m_rules.append({
            QRegularExpression(R"(&[A-Za-z0-9#]+;)"),
            m_preprocFmt
        }); // entities
        m_mlCommentFmt = m_commentFmt;
        m_mlCommentStart = QRegularExpression(R"(<!--)");
        m_mlCommentEnd = QRegularExpression(R"(-->)");
        break;
    }

    case Language::CSS: {
        m_rules.append(
            {QRegularExpression(R"(@[A-Za-z-]+)"), m_preprocFmt}); // at-rules
        // CSS custom properties  --var-name
        m_rules.append({QRegularExpression(R"(--[\w-]+)"), m_attrFmt});
        // Property names  ( word: )
        m_rules.append({QRegularExpression(R"([\w-]+(?=\s*:))"), m_attrFmt});
        // Hex colours
        m_rules.append(
            {QRegularExpression(R"(#[0-9a-fA-F]{3,8}\b)"), m_numberFmt});
        m_rules.append({QRegularExpression(R"("(?:[^"\\]|\\.)*")"), m_stringFmt});
        m_rules.append({QRegularExpression(R"('(?:[^'\\]|\\.)*')"), m_stringFmt});
        // Numbers with optional units
        m_rules.append(
            {
                QRegularExpression(
                    R"(\b\d+\.?\d*(px|em|rem|%|vh|vw|vmin|vmax|dvh|dvw|svh|svw|pt|pc|cm|mm|in|s|ms|deg|rad|turn|fr|ch|ex|lh|rlh)?\b)"),
                m_numberFmt
            });
        // Common value keywords
        addKeywords(
            {
                "absolute", "auto", "block", "border-box",
                "center", "contain", "content-box", "cover",
                "ease", "ease-in", "ease-in-out", "ease-out",
                "fixed", "flex", "flex-end", "flex-start",
                "grid", "hidden", "inherit", "initial",
                "inline", "inline-block", "inline-flex", "inline-grid",
                "left", "linear", "none", "normal",
                "nowrap", "pointer", "relative", "revert",
                "right", "scroll", "solid", "space-around",
                "space-between", "space-evenly", "static", "sticky",
                "stretch", "unset", "visible", "wrap"
            },
            m_keywordFmt);
        // !important
        m_rules.append({QRegularExpression(R"(!important)"), m_preprocFmt});
        // Pseudo-classes / pseudo-elements
        m_rules.append({QRegularExpression(R"(:+[A-Za-z-]+)"), m_funcFmt});
        m_mlCommentFmt = m_commentFmt;
        m_mlCommentStart = QRegularExpression(R"(/\*)");
        m_mlCommentEnd = QRegularExpression(R"(\*/)");
        break;
    }

    case Language::XML: {
        m_rules.append({QRegularExpression(R"(<\?[^?]*\?>)"), m_preprocFmt});
        m_rules.append({QRegularExpression(R"(<!DOCTYPE[^>]*>)"), m_preprocFmt});
        m_rules.append({
            QRegularExpression(R"(</?\s*[A-Za-z_:][A-Za-z0-9_:.-]*)"),
            m_tagFmt
        });
        m_rules.append({QRegularExpression(R"(>|/>)"), m_tagFmt});
        m_rules.append(
            {
                QRegularExpression(R"(\b[A-Za-z_:][A-Za-z0-9_:.-]*(?=\s*=))"),
                m_attrFmt
            });
        m_rules.append({QRegularExpression(R"("(?:[^"\\]|\\.)*")"), m_stringFmt});
        m_rules.append({QRegularExpression(R"('(?:[^'\\]|\\.)*')"), m_stringFmt});
        m_rules.append({
            QRegularExpression(R"(&[A-Za-z0-9#]+;)"),
            m_preprocFmt
        }); // entities
        m_rules.append({QRegularExpression(R"(<!\[CDATA\[)"), m_preprocFmt});
        m_rules.append({QRegularExpression(R"(\]\]>)"), m_preprocFmt});
        m_mlCommentFmt = m_commentFmt;
        m_mlCommentStart = QRegularExpression(R"(<!--)");
        m_mlCommentEnd = QRegularExpression(R"(-->)");
        break;
    }

    case Language::JSON: {
        // Object keys (quoted string before colon)
        m_rules.append(
            {QRegularExpression(R"("(?:[^"\\]|\\.)*"(?=\s*:))"), m_attrFmt});
        // String values
        m_rules.append({QRegularExpression(R"("(?:[^"\\]|\\.)*")"), m_stringFmt});
        // Numbers (integer, float, scientific, negative)
        m_rules.append({
            QRegularExpression(R"(-?\b\d+\.?\d*([eE][+-]?\d+)?\b)"),
            m_numberFmt
        });
        addKeywords({"true", "false", "null"}, m_keywordFmt);
        break;
    }

    case Language::YAML: {
        // Document start/end markers
        QTextCharFormat docMarkerFmt;
        docMarkerFmt.setForeground(QColor("#569cd6"));
        docMarkerFmt.setFontWeight(QFont::Bold);
        m_rules.append(
            {QRegularExpression(R"(^(?:---|\.\.\.)[ \t]*$)"), docMarkerFmt});

        // Anchors  &name  and aliases  *name
        m_rules.append(
            {QRegularExpression(R"([&*][A-Za-z_][\w-]*)"), m_preprocFmt});
        // Tags  !!str  !tag
        m_rules.append(
            {QRegularExpression(R"(!{1,2}[A-Za-z_/][\w/]*)"), m_preprocFmt});

        // Mapping keys (unquoted or quoted, followed by colon)
        m_rules.append({
            QRegularExpression(
                R"(^\s*(?:"[^"]*"|'[^']*'|[^:#{}\[\],\n]+)(?=\s*:))"),
            m_attrFmt
        });

        // Booleans / null
        addKeywords({"true", "false", "yes", "no", "on", "off", "null"},
                    m_keywordFmt);
        m_rules.append({
            QRegularExpression(R"((?:^|[\s,\[{])~(?:$|[\s,\]}]))"),
            m_keywordFmt
        }); // ~ null

        // Numbers
        m_rules.append({
            QRegularExpression(R"(\b-?\d+\.?\d*([eE][+-]?\d+)?\b)"),
            m_numberFmt
        });
        m_rules.append(
            {QRegularExpression(R"(\b0x[0-9a-fA-F]+\b)"), m_numberFmt});
        m_rules.append({QRegularExpression(R"(\b0o[0-7]+\b)"), m_numberFmt});

        // Strings
        m_rules.append({QRegularExpression(R"("(?:[^"\\]|\\.)*")"), m_stringFmt});
        m_rules.append({QRegularExpression(R"('(?:[^']|\\')*')"), m_stringFmt});

        // Comments
        m_rules.append({QRegularExpression(R"(#.*)"), m_commentFmt});
        break;
    }

    case Language::TOML: {
        // Table headers  [section]  and  [[array-of-tables]]
        QTextCharFormat sectionFmt;
        sectionFmt.setForeground(QColor("#569cd6"));
        sectionFmt.setFontWeight(QFont::Bold);
        m_rules.append(
            {QRegularExpression(R"(^\s*\[\[?[^\]]+\]\]?\s*$)"), sectionFmt});

        // Keys (bare or quoted, before = )
        m_rules.append(
            {
                QRegularExpression(R"(^\s*(?:"[^"]*"|'[^']*'|[\w.-]+)(?=\s*=))"),
                m_attrFmt
            });

        // Booleans
        addKeywords({"true", "false"}, m_keywordFmt);

        // Dates / times (ISO 8601)
        m_rules.append(
            {
                QRegularExpression(
                    R"(\d{4}-\d{2}-\d{2}(?:[ T]\d{2}:\d{2}:\d{2}(?:\.\d+)?(?:Z|[+-]\d{2}:\d{2})?)?)"),
                m_numberFmt
            });
        // Numbers (int, float, hex, octal, binary, inf, nan)
        m_rules.append(
            {
                QRegularExpression(
                    R"(\b[-+]?(?:0x[0-9a-fA-F_]+|0o[0-7_]+|0b[01_]+|\d[\d_]*\.?[\d_]*(?:[eE][-+]?\d[\d_]*)?)\b)"),
                m_numberFmt
            });
        m_rules.append({QRegularExpression(R"(\b(?:inf|nan)\b)"), m_numberFmt});

        // Multiline basic / literal strings (single-line portion)
        m_rules.append(
            {QRegularExpression(R"("""(?:[^"\\]|\\.)*""")"), m_stringFmt});
        m_rules.append({QRegularExpression(R"('''[^']*''')"), m_stringFmt});
        m_rules.append({QRegularExpression(R"("(?:[^"\\]|\\.)*")"), m_stringFmt});
        m_rules.append({
            QRegularExpression(R"('[^']*')"),
            m_stringFmt
        }); // literal (no escapes)

        // Comments
        m_rules.append({QRegularExpression(R"(#.*)"), m_commentFmt});
        break;
    }

    case Language::Bash: {
        addKeywords(
            {
                "if", "then", "else", "elif", "fi", "for",
                "while", "until", "do", "done", "case", "esac",
                "function", "return", "exit", "in", "break", "continue",
                "local", "readonly", "export", "unset", "source", "alias",
                "echo", "printf", "read", "test", "true", "false",
                "select", "time", "coproc", "declare", "typeset", "let",
                "eval", "exec", "trap", "wait", "shift", "getopts",
                "mapfile", "readarray"
            },
            m_keywordFmt);
        // Common external commands (highlight in func colour)
        m_rules.append(
            {
                QRegularExpression(
                    R"(\b(awk|basename|cat|cd|chmod|chown|cp|curl|cut|date|diff|dirname|env|find|grep|head|kill|less|ln|ls|mkdir|more|mv|nano|ps|pwd|rm|rmdir|rsync|scp|sed|sleep|sort|ssh|tail|tar|touch|tr|uniq|vi|vim|wget|which|xargs|zip)\b)"),
                m_funcFmt
            });
        // Variables  $VAR  ${VAR}  $1  $@  $#  $?
        m_rules.append(
            {
                QRegularExpression(R"(\$(\{[^}]+\}|[A-Za-z_]\w*|\d|[@#?!$*\-]))"),
                m_typeFmt
            });
        // Strings
        m_rules.append(
            {
                QRegularExpression(R"("(?:[^"\\$]|\\.|\$(?:[^{]|\{[^}]*\}))*")"),
                m_stringFmt
            });
        m_rules.append({QRegularExpression(R"('[^']*')"), m_stringFmt});
        // Numbers
        m_rules.append({QRegularExpression(R"(\b\d+\b)"), m_numberFmt});
        // Comments (# but not shebang on line 1 - shebang is still highlighted as
        // comment)
        m_rules.append({QRegularExpression(R"(#.*)"), m_commentFmt});
        break;
    }

    case Language::Rust: {
        addKeywords(
            {
                "as", "async", "await", "break", "const", "continue", "crate", "dyn",
                "else", "enum", "extern", "false", "fn", "for", "if", "impl", "in",
                "let", "loop", "match", "mod", "move", "mut", "pub", "ref", "return",
                "self", "Self", "static", "struct", "super", "trait", "true", "type",
                "unsafe", "use", "where", "while", "yield",
                // Enum variants used everywhere
                "Some", "None", "Ok", "Err"
            },
            m_keywordFmt);
        addKeywords(
            {
                "bool", "char", "f32", "f64", "i8", "i16", "i32", "i64", "i128",
                "isize", "u8", "u16", "u32", "u64", "u128", "usize", "str",
                // std types
                "Arc", "Box", "BTreeMap", "BTreeSet", "Cell", "Cow", "HashMap",
                "HashSet", "Mutex", "Option", "Rc", "RefCell", "Result", "RwLock",
                "String", "Vec"
            },
            m_typeFmt);
        // Macros:  name!
        m_rules.append(
            {QRegularExpression(R"(\b[A-Za-z_]\w*!)"), m_decoratorFmt});
        // Attributes:  #[...]  or  #![...]
        m_rules.append({QRegularExpression(R"(#!?\[[^\]]*\])"), m_preprocFmt});
        // Lifetimes:  'a
        m_rules.append({QRegularExpression(R"('[a-z_]\w*(?!'))"), m_preprocFmt});
        m_rules.append(
            {
                QRegularExpression(
                    R"(\b(0[xXoObB][\d_a-fA-F]+|\d[\d_]*\.?[\d_]*([eE][+-]?[\d_]+)?)\b)"),
                m_numberFmt
            });
        m_rules.append(
            {QRegularExpression(R"(b?"(?:[^"\\]|\\.)*")"), m_stringFmt});
        m_rules.append(
            {QRegularExpression(R"(b?'(?:[^'\\]|\\.)')"), m_stringFmt});
        m_rules.append({QRegularExpression(R"(//.*)"), m_commentFmt});
        m_mlCommentFmt = m_commentFmt;
        m_mlCommentStart = QRegularExpression(R"(/\*)");
        m_mlCommentEnd = QRegularExpression(R"(\*/)");
        break;
    }

    case Language::Go: {
        addKeywords(
            {
                "break", "case", "chan", "const", "continue", "default",
                "defer", "else", "fallthrough", "for", "func", "go",
                "goto", "if", "import", "interface", "map", "package",
                "range", "return", "select", "struct", "switch", "type",
                "var", "true", "false", "nil", "iota"
            },
            m_keywordFmt);
        addKeywords({
                        "any", "bool", "byte", "complex64", "complex128",
                        "comparable", "error", "float32", "float64", "int",
                        "int8", "int16", "int32", "int64", "rune",
                        "string", "uint", "uint8", "uint16", "uint32",
                        "uint64", "uintptr"
                    },
                    m_typeFmt);
        // Built-in functions
        m_rules.append(
            {
                QRegularExpression(
                    R"(\b(append|cap|clear|close|complex|copy|delete|imag|len|make|max|min|new|panic|print|println|real|recover)\b)"),
                m_funcFmt
            });
        m_rules.append({
            QRegularExpression(R"(\b\d+\.?\d*([eE][+-]?\d+)?(i)?\b)"),
            m_numberFmt
        });
        m_rules.append(
            {QRegularExpression(R"(0[xX][0-9a-fA-F_]+)"), m_numberFmt});
        m_rules.append({QRegularExpression(R"("(?:[^"\\]|\\.)*")"), m_stringFmt});
        m_rules.append(
            {QRegularExpression(R"(`[^`]*`)"), m_stringFmt}); // raw strings
        m_rules.append({QRegularExpression(R"('(?:[^'\\]|\\.)*')"), m_stringFmt});
        m_rules.append(
            {QRegularExpression(R"(\b([A-Za-z_]\w*)(?=\s*\())"), m_funcFmt});
        m_rules.append({QRegularExpression(R"(//.*)"), m_commentFmt});
        m_mlCommentFmt = m_commentFmt;
        m_mlCommentStart = QRegularExpression(R"(/\*)");
        m_mlCommentEnd = QRegularExpression(R"(\*/)");
        break;
    }

    case Language::Swift: {
        addKeywords({
                        "actor",
                        "any",
                        "as",
                        "associatedtype",
                        "async",
                        "await",
                        "break",
                        "case",
                        "catch",
                        "class",
                        "continue",
                        "convenience",
                        "default",
                        "defer",
                        "deinit",
                        "didSet",
                        "do",
                        "dynamic",
                        "else",
                        "enum",
                        "extension",
                        "fallthrough",
                        "false",
                        "fileprivate",
                        "final",
                        "for",
                        "func",
                        "get",
                        "guard",
                        "if",
                        "import",
                        "in",
                        "indirect",
                        "infix",
                        "init",
                        "inout",
                        "internal",
                        "is",
                        "isolated",
                        "lazy",
                        "let",
                        "mutating",
                        "nil",
                        "nonisolated",
                        "nonmutating",
                        "open",
                        "operator",
                        "optional",
                        "override",
                        "postfix",
                        "precedencegroup",
                        "prefix",
                        "private",
                        "protocol",
                        "public",
                        "reasync",
                        "repeat",
                        "required",
                        "rethrows",
                        "return",
                        "self",
                        "Self",
                        "set",
                        "some",
                        "static",
                        "struct",
                        "subscript",
                        "super",
                        "switch",
                        "throws",
                        "true",
                        "try",
                        "typealias",
                        "unowned",
                        "var",
                        "weak",
                        "where",
                        "while",
                        "willSet"
                    },
                    m_keywordFmt);
        addKeywords(
            {
                "Any", "AnyObject", "Bool", "Character", "Double",
                "Float", "Float16", "Int", "Int8", "Int16",
                "Int32", "Int64", "Never", "Optional", "Result",
                "Set", "String", "Substring", "UInt", "UInt8",
                "UInt16", "UInt32", "UInt64", "Void", "Array",
                "Dictionary", "StaticString", "Unicode"
            },
            m_typeFmt);
        // Attributes  @available  @objc  @State  etc.
        m_rules.append({QRegularExpression(R"(@\w+)"), m_decoratorFmt});
        m_rules.append(
            {QRegularExpression(R"(\b\d+\.?\d*([eE][+-]?\d+)?\b)"), m_numberFmt});
        m_rules.append(
            {QRegularExpression(R"(0[xX][0-9a-fA-F_]+)"), m_numberFmt});
        m_rules.append({QRegularExpression(R"(0[oO][0-7_]+)"), m_numberFmt});
        m_rules.append({QRegularExpression(R"(0[bB][01_]+)"), m_numberFmt});
        m_rules.append({QRegularExpression(R"("(?:[^"\\]|\\.)*")"), m_stringFmt});
        m_rules.append(
            {QRegularExpression(R"(\b([A-Za-z_]\w*)(?=\s*\())"), m_funcFmt});
        m_rules.append({QRegularExpression(R"(//.*)"), m_commentFmt});
        m_mlCommentFmt = m_commentFmt;
        m_mlCommentStart = QRegularExpression(R"(/\*)");
        m_mlCommentEnd = QRegularExpression(R"(\*/)");
        break;
    }

    case Language::Zig: {
        addKeywords({
                        "addrspace", "align", "allowzero",
                        "and", "anyframe", "anytype",
                        "asm", "async", "await",
                        "break", "catch", "comptime",
                        "const", "continue", "defer",
                        "else", "enum", "errdefer",
                        "error", "export", "extern",
                        "fn", "for", "if",
                        "inline", "noalias", "nosuspend",
                        "null", "opaque", "or",
                        "orelse", "packed", "pub",
                        "resume", "return", "linksection",
                        "struct", "suspend", "switch",
                        "test", "threadlocal", "try",
                        "union", "unreachable", "usingnamespace",
                        "var", "volatile", "while"
                    },
                    m_keywordFmt);
        addKeywords({
                        "bool", "f16", "f32",
                        "f64", "f80", "f128",
                        "i8", "i16", "i32",
                        "i64", "i128", "isize",
                        "u8", "u16", "u32",
                        "u64", "u128", "usize",
                        "c_char", "c_short", "c_ushort",
                        "c_int", "c_uint", "c_long",
                        "c_ulong", "c_longlong", "c_ulonglong",
                        "c_longdouble", "anyopaque", "void",
                        "noreturn", "type", "anyerror",
                        "comptime_int", "comptime_float"
                    },
                    m_typeFmt);
        m_rules.append({QRegularExpression(R"(@[A-Za-z_]\w*)"), m_decoratorFmt});
        m_rules.append(
            {
                QRegularExpression(
                    R"(\b(0[xX][0-9a-fA-F_]+|0[oO][0-7_]+|0[bB][01_]+|\d[\d_]*\.?[\d_]*([eE][+-]?[\d_]+)?)\b)"),
                m_numberFmt
            });
        m_rules.append({QRegularExpression(R"("(?:[^"\\]|\\.)*")"), m_stringFmt});
        m_rules.append({QRegularExpression(R"(\\\\[^\n]*)"), m_stringFmt});
        m_rules.append({QRegularExpression(R"('(?:[^'\\]|\\.)*')"), m_stringFmt});
        m_rules.append({QRegularExpression(R"(//.*)"), m_commentFmt});
        m_rules.append({QRegularExpression(R"(///.*)"), m_preprocFmt});
        m_rules.append({QRegularExpression(R"(//!.*)"), m_preprocFmt});
        m_rules.append({
            QRegularExpression(R"(\b([a-zA-Z_]\w*)\s*:\s*[A-Z])"),
            m_attrFmt, 1
        });
        break;
    }

    case Language::Pascal: {
        addKeywords({
                        "and",
                        "array",
                        "as",
                        "asm",
                        "begin",
                        "case",
                        "class",
                        "const",
                        "constructor",
                        "destructor",
                        "dispinterface",
                        "div",
                        "do",
                        "downto",
                        "else",
                        "end",
                        "except",
                        "exports",
                        "file",
                        "finalization",
                        "finally",
                        "for",
                        "function",
                        "goto",
                        "if",
                        "implementation",
                        "in",
                        "inherited",
                        "initialization",
                        "inline",
                        "interface",
                        "is",
                        "label",
                        "library",
                        "mod",
                        "nil",
                        "not",
                        "object",
                        "of",
                        "or",
                        "out",
                        "packed",
                        "procedure",
                        "program",
                        "property",
                        "raise",
                        "record",
                        "repeat",
                        "resourcestring",
                        "set",
                        "shl",
                        "shr",
                        "string",
                        "then",
                        "threadvar",
                        "to",
                        "try",
                        "type",
                        "unit",
                        "until",
                        "uses",
                        "var",
                        "while",
                        "with",
                        "xor",
                        "absolute",
                        "abstract",
                        "assembler",
                        "automated",
                        "cdecl",
                        "contains",
                        "default",
                        "deprecated",
                        "dispid",
                        "dynamic",
                        "export",
                        "external",
                        "far",
                        "forward",
                        "implements",
                        "index",
                        "local",
                        "message",
                        "name",
                        "near",
                        "nodefault",
                        "overload",
                        "override",
                        "package",
                        "pascal",
                        "platform",
                        "private",
                        "protected",
                        "public",
                        "published",
                        "read",
                        "readonly",
                        "register",
                        "reintroduce",
                        "requires",
                        "resident",
                        "safecall",
                        "stdcall",
                        "stored",
                        "varargs",
                        "virtual",
                        "write",
                        "writeonly"
                    },
                    m_keywordFmt);
        addKeywords({
                        "integer", "shortint", "smallint", "longint",
                        "int64", "byte", "word", "longword",
                        "cardinal", "uint64", "boolean", "bytebool",
                        "wordbool", "longbool", "char", "widechar",
                        "ansichar", "string", "ansistring", "widestring",
                        "unicodestring", "shortstring", "pchar", "pwidechar",
                        "pansichar", "single", "double", "extended",
                        "comp", "currency", "real", "real48",
                        "pointer", "variant", "olevariant", "text",
                        "file", "textfile", "variant", "olevariant",
                        "idispatch", "iunknown", "iinterface"
                    },
                    m_typeFmt);
        addKeywords(
            {
                "writeln", "write", "readln", "read", "str",
                "val", "chr", "ord", "pred", "succ",
                "high", "low", "inc", "dec", "abs",
                "sqr", "sqrt", "sin", "cos", "arctan",
                "exp", "ln", "round", "trunc", "frac",
                "int", "odd", "random", "randomize", "assign",
                "reset", "rewrite", "append", "close", "eof",
                "eoln", "seek", "seekeof", "seekeoln", "blockread",
                "blockwrite", "truncate", "rename", "erase", "getdir",
                "chdir", "mkdir", "rmdir", "fillchar", "move",
                "sizeof", "length", "copy", "concat", "pos",
                "delete", "insert", "upcase", "lowercase", "trim",
                "strtoint", "strtofloat", "inttostr", "floattostr", "format",
                "sprintf"
            },
            m_funcFmt);
        m_rules.append(
            {QRegularExpression(R"(\b\d+\.?\d*([eE][+-]?\d+)?\b)"), m_numberFmt});
        m_rules.append({
            QRegularExpression(R"(\$[0-9a-fA-F]+)"),
            m_numberFmt
        }); // Hex with $
        m_rules.append({
            QRegularExpression(R"('(?:[^']|'')*')"),
            m_stringFmt
        }); // Pascal uses single quotes
        m_rules.append({QRegularExpression(R"(\{[^}]*\})"), m_commentFmt});
        m_rules.append({QRegularExpression(R"(\(\*[^*]*\*\))"), m_commentFmt});
        m_mlCommentStart = QRegularExpression(R"(\(\*)");
        m_mlCommentEnd = QRegularExpression(R"(\*\))");
        m_rules.append(
            {QRegularExpression(R"(\{\$[A-Za-z][^}]*\})"), m_preprocFmt});
        m_rules.append({QRegularExpression(R"(\b\d+\s*:)"), m_preprocFmt});
        break;
    }

    case Language::Haskell: {
        addKeywords({
                        "case",
                        "class",
                        "data",
                        "default",
                        "deriving",
                        "do",
                        "else",
                        "foreign",
                        "if",
                        "import",
                        "in",
                        "infix",
                        "infixl",
                        "infixr",
                        "instance",
                        "let",
                        "module",
                        "newtype",
                        "of",
                        "then",
                        "type",
                        "where",
                        "_",
                        "as",
                        "qualified",
                        "hiding",
                        "forall",
                        "mdo",
                        "proc",
                        "rec",
                        "family",
                        "pattern",
                        "static",
                        "stock",
                        "anyclass",
                        "via",
                        "derivingStrategies",
                        "role",
                        "nominal",
                        "representational",
                        "phantom"
                    },
                    m_keywordFmt);
        addKeywords(
            {
                "Int", "Integer", "Float", "Double", "Char",
                "String", "Bool", "Maybe", "Either", "Ordering",
                "IO", "Read", "Show", "Eq", "Ord",
                "Enum", "Bounded", "Num", "Real", "Integral",
                "Fractional", "Floating", "RealFrac", "RealFloat", "Monad",
                "Functor", "Applicative", "Monoid", "Semigroup", "Traversable",
                "Foldable", "Vector", "Text", "ByteString", "Map",
                "Set", "HashMap"
            },
            m_typeFmt);
        addKeywords(
            {
                "return", "pure", "putStrLn", "print", "show",
                "read", "error", "undefined", "seq", "const",
                "flip", "curry", "uncurry", "id", "otherwise",
                "not", "isNothing", "isJust", "fromJust", "maybe",
                "either", "fst", "snd", "head", "tail",
                "init", "last", "null", "length", "map",
                "filter", "foldl", "foldr", "scanl", "scanr",
                "iterate", "repeat", "replicate", "cycle", "take",
                "drop", "splitAt", "takeWhile", "dropWhile", "span",
                "break", "zip", "zipWith", "unzip", "concat",
                "concatMap", "and", "or", "any", "all",
                "sum", "product", "maximum", "minimum"
            },
            m_funcFmt);
        m_rules.append({
            QRegularExpression(R"(\b-?\d+\.?\d*([eE][+-]?\d+)?\b)"),
            m_numberFmt
        });
        m_rules.append(
            {QRegularExpression(R"(\b0[xX][0-9a-fA-F]+\b)"), m_numberFmt});
        m_rules.append({QRegularExpression(R"(\b0[oO][0-7]+\b)"), m_numberFmt});

        m_rules.append({QRegularExpression(R"("(?:[^"\\]|\\.)*")"), m_stringFmt});
        m_rules.append({QRegularExpression(R"('(?:[^'\\]|\\.)*')"), m_stringFmt});
        m_rules.append({QRegularExpression(R"(--.*)"), m_commentFmt});
        m_rules.append({QRegularExpression(R"(\{-)"), m_commentFmt});
        m_rules.append({QRegularExpression(R"(-\})"), m_commentFmt});
        m_mlCommentStart = QRegularExpression(R"(\{-)");
        m_mlCommentEnd = QRegularExpression(R"(-\})");
        m_rules.append({QRegularExpression(R"(\{-#.*?#-\})"), m_preprocFmt});
        m_rules.append(
            {QRegularExpression(R"(\b([a-z_][\w']*)\s*::)"), m_funcFmt, 1});
        break;
    }

    case Language::Assembly: {
        addKeywords(
            {
                // Data movement
                "mov", "movabs", "movsx", "movsxd", "movzx", "movnti", "cmove",
                "cmovne", "cmovg", "cmovge", "cmovl", "cmovle", "cmova", "cmovae",
                "cmovb", "cmovbe", "push", "pop", "pusha", "popa", "pushad", "popad",
                "pushf", "popf", "pushfd", "popfd", "pushfq", "popfq", "xchg",
                "xadd", "cmpxchg", "cmpxchg8b", "cmpxchg16b", "lea", "lds", "les",
                "lfs", "lgs", "lss", "lahf", "sahf", "movsb", "movsw", "movsd",
                "movsq", "stosb", "stosw", "stosd", "stosq", "lodsb", "lodsw",
                "lodsd", "lodsq", "scasb", "scasw", "scasd", "scasq", "cmpsb",
                "cmpsw", "cmpsd", "cmpsq",

                // Arithmetic
                "add", "adc", "sub", "sbb", "mul", "imul", "div", "idiv", "inc",
                "dec", "neg", "cmp", "test", "and", "or", "xor", "not", "shl", "shr",
                "sal", "sar", "rol", "ror", "rcl", "rcr", "shld", "shrd", "bt",
                "bts", "btr", "btc", "bsf", "bsr", "bswap", "bound", "enter",
                "leave",

                // Control flow
                "jmp", "je", "jz", "jne", "jnz", "jg", "jnle", "jge", "jnl", "jl",
                "jnge", "jle", "jng", "ja", "jnbe", "jae", "jnb", "jb", "jnae",
                "jbe", "jna", "jo", "jno", "js", "jns", "jp", "jpe", "jnp", "jpo",
                "jc", "jnc", "call", "ret", "retn", "retf", "iret", "iretd", "iretq",
                "int", "into", "int3", "loop", "loope", "loopne", "loopz", "loopnz",
                "jecxz", "jrcxz",

                // String/flag operations
                "clc", "stc", "cmc", "cld", "std", "cli", "sti", "clac", "clwb",
                "clflush", "clflushopt", "cpuid", "rdtsc", "rdtscp", "rdmsr",
                "wrmsr", "in", "ins", "insb", "insw", "insd", "out", "outs", "outsb",
                "outsw", "outsd",

                // Floating point (x87)
                "fld", "fst", "fstp", "fild", "fist", "fistp", "fisttp", "fadd",
                "faddp", "fiadd", "fsub", "fsubp", "fisub", "fsubr", "fsubrp",
                "fisubr", "fmul", "fmulp", "fimul", "fdiv", "fdivp", "fidiv",
                "fdivr", "fdivrp", "fidivr", "fprem", "fprem1", "fabs", "fchs",
                "frndint", "fscale", "fsqrt", "fxtract", "fcom", "fcomp", "fcompp",
                "fucom", "fucomp", "fucompp", "ficom", "ficomp", "ftst", "fxam",
                "fsin", "fcos", "fsincos", "fptan", "fpatan", "f2xm1", "fyl2x",
                "fyl2xp1", "fldz", "fld1", "fldpi", "fldl2e", "fldl2t", "fldlg2",
                "fldln2", "finit", "fninit", "fclex", "fnclex", "fstcw", "fnstcw",
                "fldcw", "fstenv", "fnstenv", "fldenv", "fsave", "fnsave", "frstor",
                "fstsw", "fnstsw", "wait", "fwait", "fnop",

                // SSE/AVX
                "movss", "movsd", "movaps", "movups", "movapd", "movupd", "movdqa",
                "movdqu", "movntps", "movntpd", "movntdq", "movnti", "addss",
                "addsd", "addps", "addpd", "subss", "subsd", "subps", "subpd",
                "mulss", "mulsd", "mulps", "mulpd", "divss", "divsd", "divps",
                "divpd", "sqrtss", "sqrtsd", "sqrtps", "sqrtpd", "rcpss", "rcpps",
                "rsqrtss", "rsqrtps", "minss", "minsd", "minps", "minpd", "maxss",
                "maxsd", "maxps", "maxpd", "andps", "andpd", "andnps", "andnpd",
                "orps", "orpd", "xorps", "xorpd", "cmpss", "cmpsd", "cmpps", "cmppd",
                "shufps", "shufpd", "unpckhps", "unpckhpd", "unpcklps", "unpcklpd",
                "cvtpi2ps", "cvtpi2pd", "cvtps2pi", "cvtpd2pi", "cvttps2pi",
                "cvttpd2pi", "cvtps2pd", "cvtpd2ps", "cvtss2sd", "cvtsd2ss",
                "cvtsi2ss", "cvtsi2sd", "cvtss2si", "cvtsd2si", "cvttss2si",
                "cvttsd2si", "cvtdq2ps", "cvtps2dq", "cvttps2dq", "cvtdq2pd",
                "cvtpd2dq", "cvttpd2dq", "ucomiss", "ucomisd", "comiss", "comisd",
                "ldmxcsr", "stmxcsr", "sfence", "lfence", "mfence", "maskmovq",
                "maskmovdqu", "movmskps", "movmskpd", "pmovmskb",

                // AVX
                "vmovss", "vmovsd", "vmovaps", "vmovups", "vmovapd", "vmovupd",
                "vaddss", "vaddsd", "vaddps", "vaddpd", "vsubss", "vsubsd", "vsubps",
                "vsubpd", "vmulss", "vmulsd", "vmulps", "vmulpd", "vdivss", "vdivsd",
                "vdivps", "vdivpd", "vsqrtss", "vsqrtsd", "vsqrtps", "vsqrtpd",
                "vandps", "vandpd", "vandnps", "vandnpd", "vorps", "vorpd", "vxorps",
                "vxorpd", "vblendps", "vblendpd", "vblendvps", "vblendvpd",
                "vshufps", "vshufpd", "vpermilps", "vpermilpd", "vperm2f128",
                "vperm2i128", "vinsertf128", "vextractf128", "vbroadcastss",
                "vbroadcastsd", "vbroadcastf128", "vzeroall", "vzeroupper",
                "vldmxcsr", "vstmxcsr",

                // System
                "syscall", "sysret", "sysenter", "sysexit", "swapgs", "rdmsr",
                "wrmsr", "rdpmc", "rdtsc", "rdtscp", "cpuid", "hlt", "invd",
                "wbinvd", "invlpg", "lgdt", "sgdt", "lidt", "sidt", "ltr", "str",
                "lldt", "sldt", "lmsw", "smsw", "clts", "arpl", "lar", "lsl", "verr",
                "verw", "invpcid", "invept", "invvpid", "vmcall", "vmclear",
                "vmlaunch", "vmresume", "vmptrld", "vmptrst", "vmread", "vmwrite",
                "vmxoff", "vmxon"
            },
            m_keywordFmt);

        // Directives
        addKeywords(
            {
                // NASM
                "db", "dw", "dd", "dq", "dt", "do", "dy", "dz", "resb", "resw",
                "resd", "resq", "rest", "reso", "resy", "resz", "incbin", "equ",
                "times", "align", "alignb", "sectalign", "section", "segment",
                "absolute", "extern", "global", "common", "cpu", "float", "default",
                "warning", "error", "pragma", "struc", "endstruc", "istruc", "at",
                "iend", "bits", "use16", "use32", "use64", "org", "start", "group",
                "uppercase", "import", "export", "safeseh",

                // GAS/AT&T
                ".text", ".data", ".bss", ".rodata", ".section", ".globl", ".global",
                ".extern", ".extern", ".ascii", ".asciz", ".string", ".byte",
                ".short", ".word", ".long", ".int", ".quad", ".octa", ".single",
                ".float", ".double", ".space", ".skip", ".zero", ".fill", ".org",
                ".align", ".balign", ".p2align", ".set", ".equ", ".equiv", ".eqv",
                ".macro", ".endm", ".endmacro", ".rept", ".endr", ".irp", ".irpc",
                ".if", ".else", ".elseif", ".endif", ".ifdef", ".ifndef", ".ifb",
                ".ifnb", ".ifc", ".ifeqs", ".err", ".warning", ".print", ".file",
                ".loc", ".size", ".type", ".ident", ".hidden", ".protected",
                ".internal", ".weak", ".set", ".def", ".endef", ".scl", ".tag",
                ".line", ".stabd", ".stabn", ".stabs", ".comm", ".lcomm", ".struct",
                ".union",

                // MASM
                ".386", ".486", ".586", ".686", ".mmx", ".xmm", ".model", "flat",
                "stdcall", "c", "syscall", "optlink", "casemap", "none", "proc",
                "endp", "proto", "invoke", "addr", "offset", "ptr", "byte", "word",
                "dword", "qword", "tbyte", "oword", "ymmword", "zmmword", "real4",
                "real8", "real10", "sbyte", "sword", "sdword", "sqword", "dup",
                "lengthof", "sizeof", "type", "this", "offset", "seg", "length",
                "mask", "width", "sizeof", "typeof", "opattr", "short", "near",
                "far", "near16", "near32", "far16", "far32", "public", "extern",
                "externdef", "comm", "comment", "include", "includelib", "assume",
                "option", "end", "group", "segment", "ends", "at", "byte", "word",
                "para", "page", "public", "stack", "common", "memory", "readonly",
                "execute", "shared", "alias", "align", "even", "org", "radix",
                "title", "subtitle", "page", "list", "cref", "nocref", "listall",
                "listif", "listmacro", "listmacroall", "nolist", "nolistif",
                "nolistmacro", "sfcond", "tfcond", "lfcond", "b", "break",
                "continue", "exit", "exitm", "for", "forc", "goto", "if", "else",
                "elseif", "elseifb", "elseifdef", "elseifdif", "elseifidn",
                "elseifnb", "elseifndef", "repeat", "until", "untilcxz", "while",
                "purge", "tbyte", "vararg", "near", "far", "h", "c", "syscall",
                "stdcall", "pascal", "fortran", "basic", "fastcall", "vectorcall",
                "language", "frame", "nodisplay", "uses", "readonly", "writeonly",
                "noscoped", "scoped", "public", "private", "export", "import",
                "dllimport", "dllexport", "noretval", "vararg", "initializer",
                "align", "unique", "nonunique", "at", "sum", "size", "length",
                "mask", "width", "sizeof", "typeof", "opattr", "imagebase",
                "sectionrel", "imagerel", "low", "lowword", "high", "highword",
                "low32", "high32", "low64", "high64", "sizeof", "lengthof", "short",
                "type", "this", "offset", "seg", "ptr", "offset", "seg", "length",
                "mask", "width", "sizeof", "typeof", "opattr"
            },
            m_preprocFmt);

        // Registers
        addKeywords(
            {
                // 8-bit
                "al", "ah", "bl", "bh", "cl", "ch", "dl", "dh", "sil", "dil", "bpl",
                "spl", "r8b", "r9b", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b",

                // 16-bit
                "ax", "bx", "cx", "dx", "si", "di", "bp", "sp", "r8w", "r9w", "r10w",
                "r11w", "r12w", "r13w", "r14w", "r15w", "ip",

                // 32-bit
                "eax", "ebx", "ecx", "edx", "esi", "edi", "ebp", "esp", "r8d", "r9d",
                "r10d", "r11d", "r12d", "r13d", "r14d", "r15d", "eip", "eflags",

                // 64-bit
                "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "rbp", "rsp", "r8", "r9",
                "r10", "r11", "r12", "r13", "r14", "r15", "rip", "rflags",

                // Segment
                "cs", "ds", "ss", "es", "fs", "gs",

                // FPU
                "st", "st0", "st1", "st2", "st3", "st4", "st5", "st6", "st7",

                // MMX
                "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7",

                // SSE
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
                "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
                "xmm16", "xmm17", "xmm18", "xmm19", "xmm20", "xmm21", "xmm22",
                "xmm23", "xmm24", "xmm25", "xmm26", "xmm27", "xmm28", "xmm29",
                "xmm30", "xmm31",

                // AVX
                "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7",
                "ymm8", "ymm9", "ymm10", "ymm11", "ymm12", "ymm13", "ymm14", "ymm15",
                "ymm16", "ymm17", "ymm18", "ymm19", "ymm20", "ymm21", "ymm22",
                "ymm23", "ymm24", "ymm25", "ymm26", "ymm27", "ymm28", "ymm29",
                "ymm30", "ymm31",

                // AVX-512
                "zmm0", "zmm1", "zmm2", "zmm3", "zmm4", "zmm5", "zmm6", "zmm7",
                "zmm8", "zmm9", "zmm10", "zmm11", "zmm12", "zmm13", "zmm14", "zmm15",
                "zmm16", "zmm17", "zmm18", "zmm19", "zmm20", "zmm21", "zmm22",
                "zmm23", "zmm24", "zmm25", "zmm26", "zmm27", "zmm28", "zmm29",
                "zmm30", "zmm31",

                // Control
                "cr0", "cr2", "cr3", "cr4", "cr8",

                // Debug
                "dr0", "dr1", "dr2", "dr3", "dr6", "dr7",

                // Test
                "tr3", "tr4", "tr5", "tr6", "tr7",

                // MSRs
                "msr", "tsc", "eax", "ebx", "ecx", "edx"
            },
            m_typeFmt);
        m_rules.append(
            {QRegularExpression(R"(\b\d+\b)"), m_numberFmt}); // Decimal
        m_rules.append({
            QRegularExpression(R"(\b0[xX][0-9a-fA-F]+\b)"),
            m_numberFmt
        }); // Hex
        m_rules.append(
            {QRegularExpression(R"(\b0[0-7]+\b)"), m_numberFmt}); // Octal
        m_rules.append(
            {QRegularExpression(R"(\b0[bB][01]+\b)"), m_numberFmt}); // Binary
        m_rules.append({
            QRegularExpression(R"(\b\d+[hH]\b)"),
            m_numberFmt
        }); // Hex with suffix
        m_rules.append({
            QRegularExpression(R"(\b[0-9a-fA-F]+[hH]\b)"),
            m_numberFmt
        }); // Hex with suffix (no 0x)
        m_rules.append({QRegularExpression(R"("(?:[^"\\]|\\.)*")"), m_stringFmt});
        m_rules.append({QRegularExpression(R"('(?:[^'\\]|\\.)*')"), m_stringFmt});
        m_rules.append({
            QRegularExpression(R"(\b([A-Za-z_][\w@\$]*)\s*:)"),
            m_preprocFmt, 1
        });
        m_rules.append(
            {QRegularExpression(R"(;.*)"), m_commentFmt}); // ; comment
        m_rules.append(
            {QRegularExpression(R"(#.*)"), m_commentFmt}); // # comment (GAS)
        m_rules.append(
            {QRegularExpression(R"(//.*)"), m_commentFmt}); // // comment
        m_rules.append({
            QRegularExpression(R"(@@.*)"),
            m_commentFmt
        }); // @@ comment (some assemblers)
        m_mlCommentStart = QRegularExpression(R"(/\*)");
        m_mlCommentEnd = QRegularExpression(R"(\*/)");
        // m_rules.append({QRegularExpression(R"(\[[^\]]+\])"), m_specialFmt});
        m_rules.append({QRegularExpression(R"(\$\d+)"), m_numberFmt}); // $number
        m_rules.append(
            {QRegularExpression(R"(\b[0-9a-fA-F]{2}\b)"), m_numberFmt});
        break;
    }

    case Language::ActionScript: {
        addKeywords({
                        "as", "break", "case", "catch", "class", "const", "continue",
                        "default", "delete", "do", "else", "extends", "false", "finally",
                        "for", "function", "get", "if", "implements", "import", "in",
                        "instanceof", "interface", "internal", "is", "native", "new",
                        "null", "package", "private", "protected", "public", "return",
                        "set", "static", "super", "switch", "this", "throw", "to",
                        "true", "try", "typeof", "use", "var", "void", "while", "with",
                        "each", "namespace", "include", "dynamic", "final", "native",
                        "override", "static", "abstract", "boolean", "byte", "cast",
                        "char", "debugger", "double", "enum", "export", "float", "goto",
                        "long", "prototype", "short", "synchronized", "throws", "transient",
                        "volatile", "arguments"
                    }, m_keywordFmt);
        addKeywords({
                        "int", "uint", "Number", "String", "Boolean", "Object", "Array",
                        "Date", "Error", "Function", "RegExp", "XML", "XMLList", "QName",
                        "Namespace", "Vector", "Boolean", "int", "uint", "Number", "String",
                        "void", "Null", "undefined"
                    }, m_typeFmt);
        m_rules.append({QRegularExpression(R"(\b(0[xX][0-9a-fA-F]+|\d+\.?\d*([eE][+-]?\d+)?)\b)"), m_numberFmt});
        m_rules.append({QRegularExpression(R"("(?:[^"\\]|\\.)*")"), m_stringFmt});
        m_rules.append({QRegularExpression(R"('(?:[^'\\]|\\.)*')"), m_stringFmt});
        m_rules.append({QRegularExpression(R"(//.*)"), m_commentFmt});
        m_rules.append({QRegularExpression(R"(/\*[\s\S]*?\*/)"), m_commentFmt});
        m_rules.append({QRegularExpression(R"(\b([A-Za-z_]\w*)(?=\s*\())"), m_funcFmt});
        m_mlCommentFmt = m_commentFmt;
        m_mlCommentStart = QRegularExpression(R"(/\*)");
        m_mlCommentEnd = QRegularExpression(R"(\*/)");
        break;
    }

    case Language::Clojure: {
        addKeywords({
                        "def", "defn", "defmacro", "defmulti", "defmethod", "defstruct",
                        "defrecord", "deftype", "defprotocol", "defonce", "defalways",
                        "let", "letfn", "binding", "loop", "recur", "fn", "if", "if-not",
                        "if-let", "if-some", "when", "when-not", "when-let", "when-some",
                        "when-first", "cond", "condp", "cond->", "cond->>", "case",
                        "for", "doseq", "dotimes", "while", "do", "doall", "dorun",
                        "dosync", "doto", "->", "->>", "as->", "some->", "some->>",
                        "and", "or", "not", "nil", "true", "false", "try", "catch",
                        "finally", "throw", "ns", "in-ns", "import", "require", "use",
                        "refer", "refer-clojure", "gen-class", "load", "load-file",
                        "quote", "var", "symbol", "keyword", "list", "vector", "set",
                        "hash-map", "sorted-map", "sorted-set", "atom", "ref", "agent",
                        "delay", "future", "promise", "with-meta", "meta", "alter-var-root",
                        "declare", "comment", "assert", "println", "prn", "print", "pr",
                        "str", "format", "printf", "newline", "flush", "read-line",
                        "slurp", "spit", "with-open", "with-local-vars", "with-bindings",
                        "with-precision", "with-redefs", "binding", "locking", "monitor-enter",
                        "monitor-exit", "new", "instance?", "isa?", "identical?", "nil?",
                        "some?", "true?", "false?", "zero?", "pos?", "neg?", "even?",
                        "odd?", "number?", "integer?", "float?", "rational?", "ratio?",
                        "decimal?", "string?", "keyword?", "symbol?", "fn?", "ifn?",
                        "coll?", "seq?", "vector?", "list?", "map?", "set?", "associative?",
                        "sequential?", "sorted?", "counted?", "reversible?", "contains?",
                        "distinct?", "empty?", "every?", "not-every?", "some", "not-any?",
                        "reduce", "reductions", "filter", "remove", "map", "mapcat",
                        "map-indexed", "mapv", "mapcat", "take", "take-while", "take-last",
                        "take-nth", "drop", "drop-while", "drop-last", "nth", "first",
                        "second", "last", "rest", "next", "seq", "cons", "conj", "concat",
                        "reverse", "sort", "sort-by", "partition", "partition-all",
                        "partition-by", "split-at", "split-with", "group-by", "frequencies",
                        "merge", "merge-with", "zipmap", "get", "get-in", "assoc",
                        "assoc-in", "dissoc", "update", "update-in", "keys", "vals",
                        "select-keys", "find", "contains?", "count", "inc", "dec",
                        "max", "min", "max-key", "min-key", "compare", "comparator",
                        "sort", "sort-by", "shuffle", "rand", "rand-int", "rand-nth",
                        "repeatedly", "iterate", "cycle", "repeat", "range", "into",
                        "vec", "set", "seq", "rseq", "subvec", "subseq", "rsubseq",
                        "clojure-version", "clojure-main", "clojure-core", "clojure-string",
                        "clojure-set", "clojure-io", "clojure-pprint", "clojure-repl",
                        "clojure-test", "clojure-edn", "clojure-walk", "clojure-xml",
                        "clojure-zip", "clojure-data", "clojure-instant", "clojure-csv",
                        "clojure-json", "clojure-math", "clojure-reflect", "clojure-java",
                        "clojure-inspector", "clojure-stacktrace", "clojure-template",
                        "clojure-lazy", "clojure-main", "clojure-repl", "clojure-sh",
                        "clojure-sql", "clojure-datalog", "clojure-logic", "clojure-core-async",
                        "clojure-core-logic", "clojure-core-typed", "clojure-spec",
                        "clojure-tools", "clojure-cli", "clojure-deps", "clojure-jar",
                        "clojure-uberjar", "clojure-native", "clojure-graal", "clojure-bean",
                        "clojure-proxy", "clojure-reify", "clojure-deftype", "clojure-defrecord",
                        "clojure-extend", "clojure-extend-type", "clojure-extend-protocol"
                    }, m_keywordFmt);
        addKeywords({
                        "Object", "String", "Number", "Boolean", "Array", "Function",
                        "Symbol", "Keyword", "PersistentVector", "PersistentList",
                        "PersistentHashMap", "PersistentTreeMap", "PersistentHashSet",
                        "PersistentTreeSet", "LazySeq", "Cons", "Range", "ChunkedCons",
                        "ArraySeq", "StringSeq", "IteratorSeq", "EnumerationSeq",
                        "Cycle", "Repeat", "Iterate", "Replicate", "LongRange",
                        "Atom", "Ref", "Agent", "Delay", "Future", "Promise",
                        "IDeref", "IRef", "IAtom", "IAgent", "IPending", "IPromise",
                        "IFn", "ISeq", "IPersistentCollection", "IPersistentVector",
                        "IPersistentList", "IPersistentMap", "IPersistentSet",
                        "IAssociative", "IIndexed", "IStack", "IReversible",
                        "ISorted", "ICounted", "ISequential", "INamed", "IMeta",
                        "IWithMeta", "IHashEq", "IEquiv", "IComparable", "IRecord",
                        "IType", "IProxy", "IExceptionInfo", "Throwable", "Exception",
                        "RuntimeException", "IllegalArgumentException", "IllegalStateException",
                        "IndexOutOfBoundsException", "NullPointerException", "ClassCastException",
                        "ArithmeticException", "NumberFormatException", "UnsupportedOperationException",
                        "ConcurrentModificationException", "NoSuchElementException",
                        "java.lang.Object", "java.lang.String", "java.lang.Number",
                        "java.lang.Integer", "java.lang.Long", "java.lang.Double",
                        "java.lang.Float", "java.math.BigInteger", "java.math.BigDecimal",
                        "java.util.Date", "java.util.UUID", "java.io.File", "byte",
                        "short", "int", "long", "float", "double", "char", "boolean",
                        "void", "bytes", "shorts", "ints", "longs", "floats", "doubles",
                        "chars", "booleans", "objects"
                    }, m_typeFmt);
        m_rules.append({QRegularExpression(R"(:[a-zA-Z_+\-*/<>=!?&][a-zA-Z0-9_+\-*/<>=!?&]*)"), m_decoratorFmt});
        m_rules.append({
            QRegularExpression(
                R"((?<![\w#])[-+]?(?:\d+\.?\d*([eE][+-]?\d+)?M?|\d+/\d+|0[xX][0-9a-fA-F]+|0\d+|0[bB][01]+)[lLNsSfFdDbB]?)"),
            m_numberFmt
        });
        m_rules.append({QRegularExpression(R"("(?:[^"\\]|\\.)*")"), m_stringFmt});
        m_rules.append({
            QRegularExpression(
                R"((?<!\\)\\(newline|space|tab|formfeed|backspace|return|u[0-9a-fA-F]{4}|o[0-3]?[0-7]{1,2}|.))"),
            m_stringFmt
        });
        m_rules.append({QRegularExpression(R"(;.*)"), m_commentFmt});
        m_rules.append({QRegularExpression(R"(\b([a-zA-Z_+\-*/<>=!?&][a-zA-Z0-9_+\-*/<>=!?&]*)(?=\s*\())"), m_funcFmt});
        m_rules.append({QRegularExpression(R"([#'@^`~])"), m_preprocFmt});
        break;
    }

    case Language::Dart: {
        addKeywords({
                        "abstract", "as", "assert", "async", "await", "break", "case",
                        "catch", "class", "const", "continue", "covariant", "default",
                        "deferred", "do", "dynamic", "else", "enum", "export", "extends",
                        "extension", "external", "factory", "false", "final", "finally",
                        "for", "function", "get", "hide", "if", "implements", "import",
                        "in", "interface", "is", "late", "library", "mixin", "new",
                        "null", "on", "operator", "part", "required", "rethrow", "return",
                        "set", "show", "static", "super", "switch", "sync", "this",
                        "throw", "true", "try", "typedef", "var", "void", "while", "with",
                        "yield", "covariant", "deferred", "extension", "external", "factory",
                        "get", "implements", "interface", "library", "mixin", "operator",
                        "part", "set", "show", "static", "typedef", "async", "await",
                        "hide", "of", "on", "show", "sync", "yield"
                    }, m_keywordFmt);
        addKeywords({
                        "bool", "double", "int", "num", "String", "dynamic", "void",
                        "Object", "Null", "Never", "Enum", "List", "Map", "Set",
                        "Iterable", "Iterator", "Stream", "Future", "FutureOr",
                        "RuneIterator", "RegExp", "DateTime", "Duration", "Uri",
                        "BigInt", "Comparable", "Comparator", "Function", "Invocation",
                        "Symbol", "Type", "Runes", "StringBuffer", "StringSink",
                        "Pattern", "Match", "Expando", "WeakReference", "Finalizer",
                        "FinalizerEntry", "Enum", "Record", "Never", "dynamic", "var"
                    }, m_typeFmt);
        m_rules.append({QRegularExpression(R"(\b(0[xX][0-9a-fA-F]+|\d+\.?\d*([eE][+-]?\d+)?)\b)"), m_numberFmt});
        m_rules.append({QRegularExpression(R"("(?:[^"\\]|\\.)*")"), m_stringFmt});
        m_rules.append({QRegularExpression(R"('(?:[^'\\]|\\.)*')"), m_stringFmt});
        m_rules.append({QRegularExpression(R"("""[\s\S]*?""")"), m_stringFmt}); // Multi-line strings
        m_rules.append({QRegularExpression(R"('''[\s\S]*?''')"), m_stringFmt});
        m_rules.append({QRegularExpression(R"(//.*)"), m_commentFmt});
        m_rules.append({QRegularExpression(R"(/\*[\s\S]*?\*/)"), m_commentFmt});
        m_rules.append({QRegularExpression(R"(@\w+)"), m_decoratorFmt}); // Metadata/annotations
        m_rules.append({QRegularExpression(R"(\b([A-Za-z_]\w*)(?=\s*[<\(])"), m_funcFmt});
        m_mlCommentFmt = m_commentFmt;
        m_mlCommentStart = QRegularExpression(R"(/\*)");
        m_mlCommentEnd = QRegularExpression(R"(\*/)");
        break;
    }

    case Language::Fortran: {
        addKeywords({
                        // Fortran 77/90/95/2003/2008/2018 keywords
                        "program", "end", "stop", "return", "call", "subroutine", "function",
                        "module", "use", "only", "implicit", "none", "parameter", "save",
                        "data", "common", "equivalence", "dimension", "allocatable",
                        "allocate", "deallocate", "intent", "in", "out", "inout", "optional",
                        "pointer", "target", "interface", "abstract", "contains", "sequence",
                        "public", "private", "protected", "extends", "import", "external",
                        "intrinsic", "recursive", "pure", "elemental", "impure", "non_recursive",
                        "asynchronous", "volatile", "value", "bind", "result", "entry",
                        "block", "associate", "critical", "do", "while", "concurrent",
                        "if", "then", "else", "elseif", "endif", "select", "case", "selectcase",
                        "where", "elsewhere", "forall", "go", "to", "continue", "cycle",
                        "exit", "pause", "format", "namelist", "open", "close", "read",
                        "write", "print", "rewind", "backspace", "endfile", "inquire",
                        "flush", "wait", "include", "define", "undef", "ifdef", "ifndef",
                        "if", "elif", "else", "endif", "error", "warning", "pragma",
                        "assign", "backspace", "blockdata", "byte", "character", "close",
                        "common", "complex", "continue", "data", "dimension", "do", "double",
                        "else", "else", "elseif", "end", "enddo", "endfile", "endif",
                        "endfunction", "endif", "endmodule", "endprogram", "endsubroutine",
                        "entry", "equivalence", "exit", "external", "format", "function",
                        "goto", "if", "implicit", "in", "inout", "integer", "intrinsic",
                        "kind", "len", "logical", "module", "namelist", "nullify", "only",
                        "open", "operator", "optional", "out", "parameter", "pause", "pointer",
                        "precision", "print", "private", "program", "public", "read",
                        "real", "recursive", "result", "return", "rewind", "save", "select",
                        "sequence", "stop", "subroutine", "target", "then", "type", "use",
                        "where", "while", "write"
                    }, m_keywordFmt);
        addKeywords({
                        "integer", "real", "doubleprecision", "complex", "doublecomplex",
                        "logical", "character", "type", "class", "enum", "enumerator",
                        "c_ptr", "c_funptr", "c_int", "c_short", "c_long", "c_long_long",
                        "c_signed_char", "c_size_t", "c_int8_t", "c_int16_t", "c_int32_t",
                        "c_int64_t", "c_int128_t", "c_intmax_t", "c_intptr_t", "c_float",
                        "c_double", "c_long_double", "c_float128", "c_float_complex",
                        "c_double_complex", "c_long_double_complex", "c_char", "c_bool",
                        "c_null_ptr", "c_null_funptr", "iso_c_binding", "iso_fortran_env",
                        "ieee_arithmetic", "ieee_exceptions", "ieee_features", "team_type",
                        "event_type", "lock_type", "atomic_int_kind", "atomic_logical_kind",
                        "memory_order", "notify_type", "image_status_type"
                    }, m_typeFmt);
        m_rules.append({
            QRegularExpression(R"(\b(\d+\.?\d*([eEdD][+-]?\d+)?|\.?\d+([eEdD][+-]?\d+)?)\b)"), m_numberFmt
        });
        m_rules.append({QRegularExpression(R"('(?:[^']|'')*')"), m_stringFmt});
        m_rules.append({QRegularExpression(R"("(?:[^"]|"")*")"), m_stringFmt}); // Double quoted strings (Fortran 2003+)
        m_rules.append({QRegularExpression(R"(!.*)"), m_commentFmt}); // Comments start with !
        m_rules.append({QRegularExpression(R"(\b\d+H)"), m_numberFmt});
        m_rules.append({QRegularExpression(R"(\b\d+_)"), m_numberFmt});
        m_rules.append({QRegularExpression(R"(^\s*\d+)"), m_preprocFmt});
        m_rules.append({QRegularExpression(R"(^\s*#\s*\w+)"), m_preprocFmt});
        break;
    }

    case Language::INI: {
        addKeywords({
                        "true", "false", "yes", "no", "on", "off", "null", "none"
                    }, m_keywordFmt);
        m_rules.append({QRegularExpression(R"(^\s*\[.*?\]\s*$)"), m_typeFmt});
        m_rules.append({QRegularExpression(R"(^\s*([a-zA-Z_][a-zA-Z0-9_]*)\s*[=:])"), m_attrFmt});
        m_rules.append({QRegularExpression(R"(^\s*[;#].*)"), m_commentFmt});
        m_rules.append({QRegularExpression(R"(\s+[;#].*)"), m_commentFmt});
        m_rules.append({QRegularExpression(R"(\b\d+\.?\d*\b)"), m_numberFmt});
        m_rules.append({QRegularExpression(R"("(?:[^"\\]|\\.)*")"), m_stringFmt});
        m_rules.append({QRegularExpression(R"('(?:[^'\\]|\\.)*')"), m_stringFmt});
        break;
    }

    case Language::Lisp: {
        addKeywords({
                        "defun", "defmacro", "defparameter", "defvar", "defconstant",
                        "defstruct", "defclass", "defmethod", "defgeneric", "defpackage",
                        "defsystem", "defmodule", "define-condition", "define-symbol-macro",
                        "define-modify-macro", "define-setf-expander", "define-method-combination",
                        "let", "let*", "flet", "labels", "macrolet", "symbol-macrolet",
                        "if", "when", "unless", "cond", "case", "ecase", "ccase",
                        "typecase", "etypecase", "ctypecase", "and", "or", "not",
                        "progn", "prog1", "prog2", "block", "return", "return-from",
                        "loop", "do", "do*", "dotimes", "dolist", "while", "until",
                        "catch", "throw", "unwind-protect", "handler-case", "handler-bind",
                        "ignore-errors", "restart-case", "restart-bind", "with-simple-restart",
                        "use-value", "store-value", "continue", "abort", "muffle-warning",
                        "invoke-debugger", "break", "error", "cerror", "warn", "signal",
                        "assert", "check-type", "declaim", "declare", "proclaim",
                        "the", "locally", "eval-when", "load-time-value", "quote",
                        "function", "lambda", "go", "tagbody", "multiple-value-bind",
                        "multiple-value-list", "multiple-value-call", "multiple-value-prog1",
                        "values", "values-list", "nth-value", "call-with-values",
                        "with-open-file", "with-open-stream", "with-input-from-string",
                        "with-output-to-string", "with-slots", "with-accessors",
                        "with-package-iterator", "with-hash-table-iterator",
                        "with-condition-restarts", "with-compilation-unit",
                        "trace", "untrace", "step", "time", "describe", "inspect",
                        "apropos", "documentation", "room", "gc", "exit", "quit",
                        "load", "compile", "compile-file", "disassemble",
                        "require", "provide", "in-package", "use-package",
                        "import", "export", "shadow", "shadowing-import",
                        "unuse-package", "unexport", "unintern", "delete-package",
                        "rename-package", "package-name", "package-nicknames",
                        "list-all-packages", "find-package", "find-symbol",
                        "intern", "gensym", "gentemp", "make-symbol", "copy-symbol",
                        "symbol-name", "symbol-package", "symbol-value", "symbol-function",
                        "symbol-plist", "boundp", "fboundp", "special-operator-p",
                        "macro-function", "compiler-macro-function",
                        "eq", "eql", "equal", "equalp", "string=", "string-equal",
                        "char=", "char-equal", "=", "/=", "<", ">", "<=", ">=",
                        "string<", "string>", "string<=", "string>=",
                        "char<", "char>", "char<=", "char>=",
                        "null", "atom", "consp", "listp", "numberp", "integerp",
                        "rationalp", "floatp", "realp", "complexp", "characterp",
                        "stringp", "symbolp", "keywordp", "packagep", "hash-table-p",
                        "arrayp", "vectorp", "simple-vector-p", "bit-vector-p",
                        "simple-bit-vector-p", "functionp", "compiled-function-p",
                        "car", "cdr", "caar", "cadr", "cdar", "cddr",
                        "caaar", "caadr", "cadar", "caddr", "cdaar", "cdadr", "cddar", "cdddr",
                        "first", "second", "third", "fourth", "fifth", "sixth", "seventh", "eighth", "ninth", "tenth",
                        "rest", "last", "butlast", "nth", "nthcdr", "cons", "list",
                        "list*", "append", "nconc", "reverse", "nreverse", "revappend",
                        "nreconc", "member", "member-if", "member-if-not",
                        "assoc", "rassoc", "assoc-if", "rassoc-if",
                        "mapcar", "mapc", "maplist", "mapl", "mapcan", "mapcon",
                        "map", "reduce", "remove", "remove-if", "remove-if-not",
                        "delete", "delete-if", "delete-if-not", "subst", "subst-if",
                        "nsubst", "nsubst-if", "find", "find-if", "find-if-not",
                        "position", "position-if", "position-if-not",
                        "count", "count-if", "count-if-not",
                        "mismatch", "search", "merge", "sort", "stable-sort",
                        "every", "some", "notevery", "notany",
                        "+", "-", "*", "/", "mod", "rem", "floor", "ceiling",
                        "truncate", "round", "abs", "gcd", "lcm", "exp", "expt",
                        "log", "sqrt", "isqrt", "sin", "cos", "tan", "asin",
                        "acos", "atan", "sinh", "cosh", "tanh", "asinh", "acosh",
                        "atanh", "conjugate", "phase", "realpart", "imagpart",
                        "numerator", "denominator", "rational", "rationalize",
                        "float", "coerce", "type-of", "typep", "subtypep",
                        "slot-value", "slot-boundp", "slot-exists-p", "slot-makunbound",
                        "make-instance", "make-instances-obsolete", "change-class",
                        "update-instance-for-different-class", "update-instance-for-redefined-class",
                        "shared-initialize", "initialize-instance", "reinitialize-instance",
                        "allocate-instance", "compute-applicable-methods",
                        "find-method", "add-method", "remove-method",
                        "call-next-method", "next-method-p", "no-applicable-method",
                        "no-next-method", "method-combination", "method-qualifiers",
                        "print-object", "print-unreadable-object", "pprint", "princ",
                        "prin1", "print", "write", "write-to-string", "prin1-to-string",
                        "princ-to-string", "format", "read", "read-preserving-whitespace",
                        "read-delimited-list", "read-from-string", "read-byte",
                        "read-char", "read-char-no-hang", "peek-char", "unread-char",
                        "read-line", "read-sequence", "write-byte", "write-char",
                        "write-string", "write-line", "terpri", "fresh-line",
                        "finish-output", "force-output", "clear-output", "y-or-n-p",
                        "yes-or-no-p", "make-string-input-stream", "make-string-output-stream",
                        "with-standard-io-syntax", "with-readtable", "copy-readtable",
                        "set-syntax-from-char", "set-macro-character", "get-macro-character",
                        "make-dispatch-macro-character", "set-dispatch-macro-character",
                        "get-dispatch-macro-character", "readtable-case", "set-case",
                        "get-case", "copy-pprint-dispatch", "set-pprint-dispatch",
                        "pprint-dispatch", "pprint-fill", "pprint-linear", "pprint-tabular",
                        "pprint-indent", "pprint-newline", "pprint-tab", "pprint-exit-if-list-exhausted",
                        "pprint-pop", "pprint-logical-block", "format", "formatter",
                        "compile", "eval", "load", "load-logical-pathname-translations",
                        "logical-pathname", "namestring", "parse-namestring", "merge-pathnames",
                        "truename", "probe-file", "ensure-directories-exist", "file-author",
                        "file-write-date", "rename-file", "delete-file", "file-namestring",
                        "directory-namestring", "host-namestring", "enough-namestring",
                        "wild-pathname-p", "pathname-match-p", "translate-pathname",
                        "directory", "make-pathname", "pathname", "pathnamep",
                        "user-homedir-pathname", "default-directory", "chdir", "getcwd",
                        "file-position", "file-length", "stream-element-type", "open",
                        "close", "with-open-file", "listen", "clear-input", "read-sequence",
                        "write-sequence", "file-string-length", "open-stream-p",
                        "input-stream-p", "output-stream-p", "interactive-stream-p",
                        "streamp", "stream-external-format", "with-open-stream",
                        "make-broadcast-stream", "make-concatenated-stream",
                        "make-echo-stream", "make-two-way-stream", "make-string-input-stream",
                        "make-string-output-stream", "get-output-stream-string",
                        "broadcast-stream-streams", "concatenated-stream-streams",
                        "echo-stream-input-stream", "echo-stream-output-stream",
                        "two-way-stream-input-stream", "two-way-stream-output-stream",
                        "synonym-stream-symbol", "make-synonym-stream",
                        "make-package", "in-package", "use-package", "unuse-package",
                        "export", "unexport", "import", "shadow", "shadowing-import",
                        "delete-package", "rename-package", "package-name",
                        "package-nicknames", "package-use-list", "package-used-by-list",
                        "package-shadowing-symbols", "list-all-packages", "find-package",
                        "find-all-symbols", "intern", "unintern", "find-symbol",
                        "gensym", "gentemp", "symbol-name", "symbol-package",
                        "symbol-value", "symbol-function", "symbol-plist",
                        "boundp", "makunbound", "fmakunbound", "fboundp",
                        "special-operator-p", "macro-function",
                        "compiler-macro-function", "proclaim", "declaim", "declare",
                        "the", "locally", "type", "ftype", "inline", "notinline",
                        "ignore", "ignorable", "dynamic-extent", "type", "optimize",
                        "debug", "safety", "space", "speed", "compilation-speed",
                        "eval-when", "load-time-value", "quote", "function",
                        "lambda", "apply", "funcall", "call-arguments-limit",
                        "lambda-parameters-limit", "multiple-values-limit",
                        "call-method", "make-method", "method-combination",
                        "define-method-combination", "method-qualifiers",
                        "compute-effective-method", "find-method", "add-method",
                        "remove-method", "initialize-instance", "reinitialize-instance",
                        "shared-initialize", "update-instance-for-different-class",
                        "update-instance-for-redefined-class", "change-class",
                        "slot-value", "slot-boundp", "slot-exists-p", "slot-makunbound",
                        "slot-missing", "slot-unbound", "method-combination-error",
                        "invalid-method-error", "make-instances-obsolete",
                        "allocate-instance", "class-name", "class-of", "find-class",
                        "classp", "subtypep", "typep", "type-of", "coerce",
                        "deftype", "defstruct", "defclass", "defmethod", "defgeneric",
                        "define-condition", "with-slots", "with-accessors",
                        "with-added-methods", "print-object", "print-unreadable-object",
                        "documentation", "ensure-generic-function",
                        "generic-function", "standard-generic-function",
                        "class", "standard-class", "structure-class", "built-in-class",
                        "standard-object", "structure-object", "method",
                        "standard-method", "standard-reader-method",
                        "standard-writer-method", "standard-accessor-method",
                        "method-combination", "standard-method-combination",
                        "slot-definition", "standard-slot-definition",
                        "direct-slot-definition", "effective-slot-definition",
                        "standard-direct-slot-definition", "standard-effective-slot-definition",
                        "specializer", "eql-specializer", "class", "forward-referenced-class",
                        "funcallable-standard-class", "funcallable-standard-object",
                        "generic-function-argument-precedence-order",
                        "generic-function-declarations", "generic-function-lambda-list",
                        "generic-function-method-class", "generic-function-method-combination",
                        "generic-function-methods", "generic-function-name",
                        "add-direct-subclass", "remove-direct-subclass",
                        "specializer-direct-generic-functions", "specializer-direct-methods",
                        "compute-applicable-methods", "compute-applicable-methods-using-classes",
                        "compute-class-precedence-list", "compute-default-initargs",
                        "compute-discriminating-function", "compute-effective-method",
                        "compute-effective-slot-definition", "compute-slots",
                        "ensure-class", "ensure-class-using-class", "ensure-generic-function",
                        "ensure-generic-function-using-class", "find-method-combination",
                        "make-method-lambda", "reader-method-class", "writer-method-class",
                        "class-default-initargs", "class-direct-default-initargs",
                        "class-direct-slots", "class-direct-subclasses",
                        "class-direct-superclasses", "class-finalized-p",
                        "class-precedence-list", "class-prototype", "class-slots",
                        "class-name", "class-of", "find-class", "setf", "find-class",
                        "setf", "class-name", "slot-boundp", "slot-exists-p",
                        "slot-makunbound", "slot-value", "method-function",
                        "method-generic-function", "method-lambda-list",
                        "method-specializers", "accessor-method-slot-definition",
                        "slot-definition-allocation", "slot-definition-initargs",
                        "slot-definition-initform", "slot-definition-initfunction",
                        "slot-definition-location", "slot-definition-name",
                        "slot-definition-readers", "slot-definition-writers",
                        "slot-definition-type", "eql-specializer-object",
                        "intern-eql-specializer", "set-funcallable-instance-function",
                        "funcallable-standard-instance-access",
                        "standard-instance-access", "slot-value-using-class",
                        "slot-boundp-using-class", "slot-makunbound-using-class",
                        "slot-exists-p-using-class", "compute-class-precedence-list",
                        "compute-default-initargs", "compute-effective-slot-definition",
                        "compute-slots", "compute-effective-method",
                        "find-method-combination", "make-method-lambda",
                        "reader-method-class", "writer-method-class",
                        "validate-superclass", "add-dependent", "remove-dependent",
                        "map-dependents", "update-dependent", "add-direct-method",
                        "remove-direct-method", "specializer-direct-generic-functions",
                        "specializer-direct-methods", "generic-function-argument-precedence-order",
                        "generic-function-declarations", "generic-function-lambda-list",
                        "generic-function-method-class", "generic-function-method-combination",
                        "generic-function-methods", "generic-function-name",
                        "set-funcallable-instance-function", "funcallable-standard-instance-access",
                        "standard-instance-access", "slot-value-using-class",
                        "slot-boundp-using-class", "slot-makunbound-using-class",
                        "slot-exists-p-using-class", "ensure-class", "ensure-class-using-class",
                        "ensure-generic-function", "ensure-generic-function-using-class",
                        "allocate-instance", "reinitialize-instance", "shared-initialize",
                        "update-instance-for-different-class", "update-instance-for-redefined-class",
                        "change-class", "slot-missing", "slot-unbound", "method-combination-error",
                        "invalid-method-error", "make-instances-obsolete",
                        "compute-applicable-methods", "compute-applicable-methods-using-classes",
                        "compute-class-precedence-list", "compute-default-initargs",
                        "compute-discriminating-function", "compute-effective-method",
                        "compute-effective-slot-definition", "compute-slots",
                        "ensure-class", "ensure-class-using-class", "ensure-generic-function",
                        "ensure-generic-function-using-class", "find-method-combination",
                        "make-method-lambda", "reader-method-class", "writer-method-class",
                        "class-default-initargs", "class-direct-default-initargs",
                        "class-direct-slots", "class-direct-subclasses",
                        "class-direct-superclasses", "class-finalized-p",
                        "class-precedence-list", "class-prototype", "class-slots",
                        "class-name", "class-of", "find-class", "setf", "find-class",
                        "setf", "class-name", "slot-boundp", "slot-exists-p",
                        "slot-makunbound", "slot-value", "method-function",
                        "method-generic-function", "method-lambda-list",
                        "method-specializers", "accessor-method-slot-definition",
                        "slot-definition-allocation", "slot-definition-initargs",
                        "slot-definition-initform", "slot-definition-initfunction",
                        "slot-definition-location", "slot-definition-name",
                        "slot-definition-readers", "slot-definition-writers",
                        "slot-definition-type", "eql-specializer-object",
                        "intern-eql-specializer", "set-funcallable-instance-function",
                        "funcallable-standard-instance-access",
                        "standard-instance-access", "slot-value-using-class",
                        "slot-boundp-using-class", "slot-makunbound-using-class",
                        "slot-exists-p-using-class", "compute-class-precedence-list",
                        "compute-default-initargs", "compute-effective-slot-definition",
                        "compute-slots", "compute-effective-method",
                        "find-method-combination", "make-method-lambda",
                        "reader-method-class", "writer-method-class",
                        "validate-superclass", "add-dependent", "remove-dependent",
                        "map-dependents", "update-dependent", "add-direct-method",
                        "remove-direct-method", "specializer-direct-generic-functions",
                        "specializer-direct-methods"
                    }, m_keywordFmt);
        addKeywords({
                        "t", "nil", "null", "boolean", "symbol", "keyword", "atom",
                        "cons", "list", "null", "sequence", "array", "vector",
                        "simple-vector", "bit-vector", "simple-bit-vector", "string",
                        "base-string", "simple-string", "simple-base-string", "character",
                        "base-char", "standard-char", "extended-char", "number",
                        "complex", "real", "float", "short-float", "single-float",
                        "double-float", "long-float", "rational", "ratio", "integer",
                        "fixnum", "bignum", "signed-byte", "unsigned-byte", "bit",
                        "random-state", "readtable", "package", "pathname", "logical-pathname",
                        "stream", "broadcast-stream", "concatenated-stream",
                        "echo-stream", "synonym-stream", "two-way-stream",
                        "string-stream", "file-stream", "generic-function",
                        "standard-generic-function", "method", "standard-method",
                        "standard-reader-method", "standard-writer-method",
                        "standard-accessor-method", "method-combination",
                        "standard-method-combination", "class", "standard-class",
                        "structure-class", "built-in-class", "standard-object",
                        "structure-object", "slot-definition", "standard-slot-definition",
                        "direct-slot-definition", "effective-slot-definition",
                        "standard-direct-slot-definition", "standard-effective-slot-definition",
                        "specializer", "eql-specializer", "class", "forward-referenced-class",
                        "funcallable-standard-class", "funcallable-standard-object",
                        "restart", "condition", "warning", "serious-condition",
                        "error", "simple-condition", "simple-warning", "simple-error",
                        "arithmetic-error", "division-by-zero", "floating-point-invalid-operation",
                        "floating-point-inexact", "floating-point-overflow",
                        "floating-point-underflow", "cell-error", "unbound-variable",
                        "undefined-function", "unbound-slot", "package-error",
                        "parse-error", "print-not-readable", "reader-error",
                        "stream-error", "end-of-file", "file-error", "storage-condition",
                        "control-error", "program-error", "undefined-function",
                        "simple-condition", "simple-warning", "simple-error",
                        "simple-type-error", "simple-style-warning", "style-warning",
                        "type-error", "simple-type-error", "case-failure",
                        "invalid-method-error", "method-combination-error",
                        "cell-error-name", "package-error-package", "print-not-readable-object",
                        "stream-error-stream", "file-error-pathname", "type-error-datum",
                        "type-error-expected-type", "arithmetic-error-operation",
                        "arithmetic-error-operands", "division-by-zero",
                        "floating-point-invalid-operation", "floating-point-inexact",
                        "floating-point-overflow", "floating-point-underflow",
                        "unbound-variable", "undefined-function", "unbound-slot",
                        "unbound-slot-instance", "restart-name", "compute-restarts",
                        "find-restart", "invoke-restart", "invoke-restart-interactively",
                        "restart-bind", "restart-case", "with-condition-restarts",
                        "with-simple-restart", "use-value", "store-value", "continue",
                        "abort", "muffle-warning", "invoke-debugger", "break",
                        "error", "cerror", "warn", "signal", "assert", "check-type",
                        "ecase", "ccase", "etypecase", "ctypecase"
                    }, m_typeFmt);
        m_rules.append({QRegularExpression(R"(\b[-+]?\d+(\.\d+)?([eEsSfFdDlL][-+]?\d+)?\b)"), m_numberFmt});
        m_rules.append({QRegularExpression(R"(\b[-+]?\d+/\d+\b)"), m_numberFmt}); // Ratios
        m_rules.append({QRegularExpression(R"(#\d+[rR][0-9a-zA-Z]+)"), m_numberFmt}); // Radix notation
        m_rules.append({QRegularExpression(R"(#\d*\.\d+([sSfFdDlL][-+]?\d+)?)"), m_numberFmt}); // Floats
        m_rules.append({QRegularExpression(R"(#c\([^)]+\))"), m_numberFmt}); // Complex numbers
        m_rules.append({QRegularExpression(R"("(?:[^"\\]|\\.)*")"), m_stringFmt});
        m_rules.append({QRegularExpression(R"(#\([^)]+\))"), m_stringFmt}); // Character names
        m_rules.append({
            QRegularExpression(R"(#\\(newline|space|tab|page|rubout|linefeed|return|backspace|.|[^#\s]))"), m_stringFmt
        });
        m_rules.append({QRegularExpression(R"(;.*)"), m_commentFmt});
        m_rules.append({QRegularExpression(R"(#\|[\s\S]*?\|#)"), m_commentFmt}); // Multi-line comments
        m_rules.append({QRegularExpression(R"(#[\'\.,@`^])"), m_preprocFmt});
        m_rules.append({QRegularExpression(R"(#\d*[aAbBcCoOxX])"), m_preprocFmt}); // #b, #o, #x, etc.
        m_rules.append({QRegularExpression(R"(#\()"), m_preprocFmt}); // Vector
        m_rules.append({QRegularExpression(R"(#\*)"), m_preprocFmt}); // Bit vector
        m_rules.append({QRegularExpression(R"(#\d*=)"), m_preprocFmt}); // Circular reference
        m_rules.append({QRegularExpression(R"(#\d+#)"), m_preprocFmt}); // Circular reference
        m_rules.append({QRegularExpression(R"(#\d+A)"), m_preprocFmt}); // Array
        m_rules.append({QRegularExpression(R"(#S)"), m_preprocFmt}); // Structure
        m_rules.append({QRegularExpression(R"(#P)"), m_preprocFmt}); // Pathname
        m_rules.append({QRegularExpression(R"(#\d*R)"), m_preprocFmt}); // Radix
        m_rules.append({QRegularExpression(R"(#\d+C)"), m_preprocFmt}); // Complex
        m_rules.append({
            QRegularExpression(R"(\b([a-zA-Z_+\-*/<>=!?&$%:@][a-zA-Z0-9_+\-*/<>=!?&$%:@]*)(?=\s*\())"), m_funcFmt
        });
        break;
    }

    case Language::Perl: {
        addKeywords({
                        "use", "no", "require", "do", "package", "sub", "my", "our",
                        "local", "state", "if", "unless", "elsif", "else", "while",
                        "until", "for", "foreach", "given", "when", "default", "continue",
                        "last", "next", "redo", "goto", "return", "eval", "exit",
                        "die", "warn", "print", "printf", "say", "open", "close",
                        "read", "write", "seek", "tell", "eof", "binmode", "truncate",
                        "fcntl", "ioctl", "flock", "pipe", "socket", "bind", "listen",
                        "accept", "connect", "shutdown", "getsockopt", "setsockopt",
                        "getsockname", "getpeername", "send", "recv", "sysread",
                        "syswrite", "sysseek", "sysopen", "sysclose", "umask",
                        "chown", "chmod", "chroot", "link", "symlink", "unlink",
                        "rename", "link", "readlink", "mkdir", "rmdir", "opendir",
                        "closedir", "readdir", "rewinddir", "seekdir", "telldir",
                        "fork", "wait", "waitpid", "system", "exec", "kill", "alarm",
                        "sleep", "times", "exit", "setpgrp", "setpriority", "getpriority",
                        "getpgrp", "getppid", "getpwnam", "getpwuid", "getgrnam",
                        "getgrgid", "getlogin", "getcwd", "chdir", "glob", "unlink",
                        "utime", "times", "localtime", "gmtime", "time", "mktime",
                        "strftime", "strptime", "defined", "undef", "delete", "exists",
                        "each", "keys", "values", "push", "pop", "shift", "unshift",
                        "splice", "reverse", "sort", "map", "grep", "join", "split",
                        "index", "rindex", "substr", "length", "pos", "quotemeta",
                        "lc", "uc", "lcfirst", "ucfirst", "chop", "chomp", "crypt",
                        "hex", "oct", "ord", "chr", "pack", "unpack", "vec", "study",
                        "scalar", "list", "array", "hash", "ref", "bless", "tie",
                        "tied", "untie", "dbmopen", "dbmclose", "srand", "rand",
                        "atan2", "sin", "cos", "exp", "log", "sqrt", "int", "hex",
                        "oct", "abs", "length", "substr", "index", "rindex", "sprintf",
                        "printf", "vec", "study", "pos", "quotemeta", "lc", "uc",
                        "lcfirst", "ucfirst", "fc", "chop", "chomp", "crypt",
                        "pack", "unpack", "join", "split", "reverse", "concat",
                        "eq", "ne", "lt", "gt", "le", "ge", "cmp", "eq", "ne",
                        "==", "!=", "<", ">", "<=", ">=", "<=>", "~~", "=~", "!~",
                        "and", "or", "xor", "not", "&&", "||", "!", "++", "--",
                        "**", "+", "-", "*", "/", "%", ".", "x", "<<", ">>",
                        "&", "|", "^", "~", "+=", "-=", "*=", "/=", "%=", ".=",
                        "x=", "&=", "|=", "^=", "<<=", ">>=", "**=", "&&=", "||=",
                        "//=", "=>", "->", "::", ":", "?", "..", "...", "and",
                        "or", "xor", "not", "eq", "ne", "cmp", "lt", "gt", "le",
                        "ge", "isa", "AUTOLOAD", "BEGIN", "CHECK", "END", "INIT",
                        "UNITCHECK", "DESTROY", "import", "unimport", "can",
                        "VERSION", "super", "method", "has", "extends", "with",
                        "around", "before", "after", "augment", "inner", "override",
                        "super", "bless", "ref", "tie", "tied", "untie", "weaken",
                        "isweak", "unweaken", "readonly", "open", "close", "read",
                        "write", "seek", "tell", "eof", "binmode", "truncate",
                        "fcntl", "ioctl", "flock", "pipe", "socket", "bind",
                        "listen", "accept", "connect", "shutdown", "getsockopt",
                        "setsockopt", "getsockname", "getpeername", "send", "recv",
                        "sysread", "syswrite", "sysseek", "sysopen", "sysclose",
                        "umask", "chown", "chmod", "chroot", "link", "symlink",
                        "unlink", "rename", "link", "readlink", "mkdir", "rmdir",
                        "opendir", "closedir", "readdir", "rewinddir", "seekdir",
                        "telldir", "fork", "wait", "waitpid", "system", "exec",
                        "kill", "alarm", "sleep", "times", "exit", "setpgrp",
                        "setpriority", "getpriority", "getpgrp", "getppid",
                        "getpwnam", "getpwuid", "getgrnam", "getgrgid", "getlogin",
                        "getcwd", "chdir", "glob", "unlink", "utime", "times",
                        "localtime", "gmtime", "time", "mktime", "strftime",
                        "strptime", "defined", "undef", "delete", "exists", "each",
                        "keys", "values", "push", "pop", "shift", "unshift",
                        "splice", "reverse", "sort", "map", "grep", "join", "split",
                        "index", "rindex", "substr", "length", "pos", "quotemeta",
                        "lc", "uc", "lcfirst", "ucfirst", "chop", "chomp", "crypt",
                        "hex", "oct", "ord", "chr", "pack", "unpack", "vec", "study",
                        "scalar", "list", "array", "hash", "ref", "bless", "tie",
                        "tied", "untie", "dbmopen", "dbmclose", "srand", "rand",
                        "atan2", "sin", "cos", "exp", "log", "sqrt", "int", "hex",
                        "oct", "abs", "length", "substr", "index", "rindex", "sprintf",
                        "printf", "vec", "study", "pos", "quotemeta", "lc", "uc",
                        "lcfirst", "ucfirst", "fc", "chop", "chomp", "crypt", "pack",
                        "unpack", "join", "split", "reverse", "concat", "eq", "ne",
                        "lt", "gt", "le", "ge", "cmp", "eq", "ne", "==", "!=", "<",
                        ">", "<=", ">=", "<=>", "~~", "=~", "!~", "and", "or", "xor",
                        "not", "&&", "||", "!", "++", "--", "**", "+", "-", "*", "/",
                        "%", ".", "x", "<<", ">>", "&", "|", "^", "~", "+=", "-=",
                        "*=", "/=", "%=", ".=", "x=", "&=", "|=", "^=", "<<=", ">>=",
                        "**=", "&&=", "||=", "//=", "=>", "->", "::", ":", "?", "..",
                        "...", "and", "or", "xor", "not", "eq", "ne", "cmp", "lt",
                        "gt", "le", "ge", "isa"
                    }, m_keywordFmt);
        addKeywords({
                        "int", "integer", "float", "double", "string", "array", "hash",
                        "object", "boolean", "true", "false", "null", "undef", "FileHandle",
                        "GLOB", "IO", "Socket", "ARRAY", "HASH", "CODE", "REF", "SCALAR",
                        "Regexp", "Format", "Lvalue", "VString", "Object", "Class",
                        "Method", "Role", "Moose", "Moo", "Mouse", "Type::Tiny",
                        "Moose::Role", "Moose::Util::TypeConstraints", "Moose::Meta::Class",
                        "Moose::Meta::Attribute", "Moose::Meta::Method", "Moose::Meta::Role",
                        "Moose::Meta::TypeConstraint", "Moose::Meta::TypeCoercion",
                        "Moose::Meta::TypeConstraint::Union", "Moose::Meta::TypeConstraint::Parameterized",
                        "Moose::Meta::TypeConstraint::Class", "Moose::Meta::TypeConstraint::Role",
                        "Moose::Meta::TypeConstraint::Enum", "Moose::Meta::TypeConstraint::DuckType",
                        "Moose::Meta::TypeConstraint::Registry", "Moose::Util", "Moose::Util::MetaRole",
                        "Moose::Util::TypeConstraints", "Moose::Exporter", "Moose::Conflicts",
                        "Moose::Deprecated", "Moose::Exception", "Moose::Exception::FeatureConflict",
                        "Moose::Exception::InvalidAttributeDefinition", "Moose::Exception::InvalidRoleApplication",
                        "Moose::Exception::InvalidTypeConstraint", "Moose::Exception::Legacy",
                        "Moose::Exception::MOP", "Moose::Exception::Superclass",
                        "Moose::Exception::TypeConstraint", "Moose::Exception::ValidationFailedForTypeConstraint",
                        "Moose::Meta::Instance", "Moose::Meta::Class::Immutable::Trait",
                        "Moose::Meta::Method::Accessor", "Moose::Meta::Method::Constructor",
                        "Moose::Meta::Method::Destructor", "Moose::Meta::Method::Meta",
                        "Moose::Meta::Method::Overridden", "Moose::Meta::Method::Augmented",
                        "Moose::Meta::Role::Application", "Moose::Meta::Role::Application::ToClass",
                        "Moose::Meta::Role::Application::ToRole", "Moose::Meta::Role::Application::ToInstance",
                        "Moose::Meta::Role::Composite", "Moose::Meta::Role::Method::Conflicting",
                        "Moose::Meta::Role::Method::Required", "Moose::Object", "Moose::Role",
                        "Moose::Util::TypeConstraints::Builtins", "MooseX::Types", "MooseX::Types::Moose",
                        "MooseX::Types::Common", "MooseX::Types::Common::Numeric",
                        "MooseX::Types::Common::String", "MooseX::Types::LoadableClass",
                        "MooseX::Types::Path::Class", "MooseX::Types::Structured",
                        "MooseX::Types::DateTime", "MooseX::Types::URI", "MooseX::Types::UUID",
                        "MooseX::Getopt", "MooseX::SimpleConfig", "MooseX::ConfigFromFile",
                        "MooseX::Daemonize", "MooseX::Declare", "MooseX::Method::Signatures",
                        "MooseX::Params::Validate", "MooseX::ClassAttribute", "MooseX::AttributeShortcuts",
                        "MooseX::LazyRequire", "MooseX::SetOnce", "MooseX::StrictConstructor",
                        "MooseX::Traits", "MooseX::Object::Pluggable", "MooseX::Singleton",
                        "MooseX::NonMoose", "MooseX::InsideOut", "MooseX::FollowPBP",
                        "MooseX::SemiAffordanceAccessor", "MooseX::HasDefaults::RO",
                        "MooseX::HasDefaults::RW", "MooseX::Aliases", "MooseX::ChainedAccessors",
                        "MooseX::CurriedDelegation", "MooseX::CascadeClearing", "MooseX::Storage",
                        "MooseX::Types::LoadableClass", "MooseX::Types::Path::Class",
                        "MooseX::Types::Structured", "MooseX::Types::DateTime", "MooseX::Types::URI",
                        "MooseX::Types::UUID", "MooseX::Getopt", "MooseX::SimpleConfig",
                        "MooseX::ConfigFromFile", "MooseX::Daemonize", "MooseX::Declare",
                        "MooseX::Method::Signatures", "MooseX::Params::Validate", "MooseX::ClassAttribute",
                        "MooseX::AttributeShortcuts", "MooseX::LazyRequire", "MooseX::SetOnce",
                        "MooseX::StrictConstructor", "MooseX::Traits", "MooseX::Object::Pluggable",
                        "MooseX::Singleton", "MooseX::NonMoose", "MooseX::InsideOut",
                        "MooseX::FollowPBP", "MooseX::SemiAffordanceAccessor", "MooseX::HasDefaults::RO",
                        "MooseX::HasDefaults::RW", "MooseX::Aliases", "MooseX::ChainedAccessors",
                        "MooseX::CurriedDelegation", "MooseX::CascadeClearing", "MooseX::Storage"
                    }, m_typeFmt);
        m_rules.append({QRegularExpression(R"(\b\d+(\.\d+)?([eE][+-]?\d+)?\b)"), m_numberFmt});
        m_rules.append({QRegularExpression(R"(\bv\d+(\.\d+)*\b)"), m_numberFmt});
        m_rules.append({QRegularExpression(R"("(?:[^"\\]|\\.)*")"), m_stringFmt});
        m_rules.append({QRegularExpression(R"('(?:[^'\\]|\\.)*')"), m_stringFmt});
        m_rules.append({QRegularExpression(R"(q[qrwx]?\s*\{(?:[^}]|\\.)*\})"), m_stringFmt});
        m_rules.append({QRegularExpression(R"(q[qrwx]?\s*\[(?:[^\]]|\\.)*\])"), m_stringFmt});
        m_rules.append({QRegularExpression(R"(q[qrwx]?\s*\((?:[^)]|\\.)*\))"), m_stringFmt});
        m_rules.append({QRegularExpression(R"(q[qrwx]?\s*<(?:[^>]|\\.)*>)"), m_stringFmt});
        m_rules.append({QRegularExpression(R"(q[qrwx]?\s*([^a-zA-Z0-9])(?:[^\1\\]|\\.)*\1)"), m_stringFmt});
        m_rules.append({QRegularExpression(R"(<<['"]?(\w+)['"]?;[\s\S]*?^\1$)"), m_stringFmt});
        m_rules.append({QRegularExpression(R"(#.*)"), m_commentFmt});
        m_rules.append({QRegularExpression(R"(^=[a-zA-Z][\s\S]*?=cut\s*$)"), m_commentFmt}); // POD
        m_rules.append({QRegularExpression(R"(m/[^/]*/[gimosx]*)"), m_decoratorFmt});
        m_rules.append({QRegularExpression(R"(s/[^/]*/[^/]*/[gimosxe]*)"), m_decoratorFmt});
        m_rules.append({QRegularExpression(R"(tr/[^/]*/[^/]*/[cds])"), m_decoratorFmt});
        m_rules.append({QRegularExpression(R"(y/[^/]*/[^/]*/[cds])"), m_decoratorFmt});
        m_rules.append({QRegularExpression(R"(/[^/]*/[gimosx]*)"), m_decoratorFmt});
        m_rules.append({QRegularExpression(R"([\$\@%&*][a-zA-Z_]\w*)"), m_preprocFmt});
        m_rules.append({QRegularExpression(R"(\$\#)"), m_preprocFmt});
        m_rules.append({QRegularExpression(R"(\bsub\s+([a-zA-Z_]\w*))"), m_funcFmt});
        m_rules.append({QRegularExpression(R"(\bpackage\s+([a-zA-Z_]\w*(::\w+)*))"), m_typeFmt});
        m_rules.append({QRegularExpression(R"(\b(use|no|require)\s+([a-zA-Z_]\w*(::\w+)*))"), m_preprocFmt});
        break;
    }

    case Language::Scala: {
        addKeywords({
                        "abstract", "case", "catch", "class", "def", "do", "else",
                        "extends", "false", "final", "finally", "for", "forSome",
                        "if", "implicit", "import", "lazy", "match", "new", "null",
                        "object", "override", "package", "private", "protected",
                        "return", "sealed", "super", "this", "throw", "trait",
                        "try", "true", "type", "val", "var", "while", "with",
                        "yield", "enum", "export", "given", "then", "extension",
                        "derives", "end", "opaque", "inline", "transparent",
                        "open", "infix", "throws", "as", "erased"
                    }, m_keywordFmt);
        addKeywords({
                        "Any", "AnyRef", "AnyVal", "Array", "Boolean", "Byte", "Char",
                        "Double", "Float", "Int", "Long", "Nothing", "Null", "Object",
                        "Short", "String", "Unit", "Seq", "List", "Vector", "Map", "Set",
                        "Iterator", "Option", "Some", "None", "Either", "Left", "Right",
                        "Try", "Success", "Failure", "Future", "Promise", "Await",
                        "Duration", "Thread", "Runnable", "ExecutionContext", "Stream",
                        "InputStream", "OutputStream", "Reader", "Writer", "File",
                        "Path", "URI", "URL", "ByteBuffer", "CharBuffer", "Charset",
                        "StringBuilder", "StringBuffer", "System", "Runtime", "Process"
                    }, m_typeFmt);
        m_rules.append({QRegularExpression(R"(\b(0[xX][0-9a-fA-F]+|\d+\.?\d*([eE][+-]?\d+)?[dDfF]?)\b)"), m_numberFmt});
        m_rules.append({QRegularExpression(R"("(?:[^"\\]|\\.)*")"), m_stringFmt});
        m_rules.append({QRegularExpression(R"("""[\s\S]*?""")"), m_stringFmt});
        m_rules.append({QRegularExpression(R"('(?:[^'\\]|\\.)')"), m_stringFmt});
        m_rules.append({QRegularExpression(R"('\w+)"), m_decoratorFmt});
        m_rules.append({QRegularExpression(R"(//.*)"), m_commentFmt});
        m_rules.append({QRegularExpression(R"(/\*\*[\s\S]*?\*/)"), m_commentFmt});
        m_rules.append({QRegularExpression(R"(/\*[\s\S]*?\*/)"), m_commentFmt});
        m_rules.append({QRegularExpression(R"(@[A-Za-z_]\w*)"), m_decoratorFmt});
        m_rules.append({QRegularExpression(R"(\bdef\s+([a-zA-Z_]\w*))"), m_funcFmt});
        m_rules.append({QRegularExpression(R"(\b(class|trait|object|enum)\s+([A-Z]\w*))"), m_typeFmt});
        m_rules.append({QRegularExpression(R"([sfraw]"(?:[^"\\$]|\$(?:\{[^}]*\}|\w+)|\\.)*")"), m_stringFmt});
        m_mlCommentFmt = m_commentFmt;
        m_mlCommentStart = QRegularExpression(R"(/\*)");
        m_mlCommentEnd = QRegularExpression(R"(\*/)");
        break;
    }

    case Language::SQL: {
        // DML / DDL / TCL keywords  (case-insensitive)
        m_rules.append(
            {
                QRegularExpression(
                    R"(\b(ADD|ALL|ALTER|AND|ANY|AS|ASC|AUTO_INCREMENT|BEGIN|BETWEEN|BY|CASE|)"
                    R"(CHECK|COLUMN|COMMIT|CONSTRAINT|CREATE|CROSS|DATABASE|DEFAULT|DELETE|DESC|)"
                    R"(DISTINCT|DROP|ELSE|END|EXCEPT|EXISTS|EXPLAIN|FALSE|FOREIGN|FROM|FULL|)"
                    R"(GROUP|HAVING|IF|IN|INDEX|INNER|INSERT|INTERSECT|INTO|IS|JOIN|KEY|LEFT|)"
                    R"(LIKE|LIMIT|NOT|NULL|OFFSET|ON|OR|ORDER|OUTER|PRIMARY|REFERENCES|REPLACE|)"
                    R"(RETURNING|RIGHT|ROLLBACK|SELECT|SET|TABLE|THEN|TOP|TRANSACTION|TRIGGER|)"
                    R"(TRUNCATE|TRUE|UNION|UNIQUE|UPDATE|USE|VALUES|VIEW|WHEN|WHERE|WITH)\b)",
                    QRegularExpression::CaseInsensitiveOption),
                m_keywordFmt
            });
        // Data types  (case-insensitive)
        m_rules.append(
            {
                QRegularExpression(
                    R"(\b(BIGINT|BINARY|BIT|BLOB|BOOLEAN|CHAR|CLOB|DATE|DATETIME|DECIMAL|DOUBLE|)"
                    R"(ENUM|FLOAT|INT|INTEGER|JSON|LONGBLOB|LONGTEXT|MEDIUMBLOB|MEDIUMINT|MEDIUMTEXT|)"
                    R"(NUMERIC|REAL|SERIAL|SMALLINT|TEXT|TIME|TIMESTAMP|TINYBLOB|TINYINT|TINYTEXT|)"
                    R"(VARBINARY|VARCHAR|YEAR)\b)",
                    QRegularExpression::CaseInsensitiveOption),
                m_typeFmt
            });
        // Built-in functions  (case-insensitive)
        m_rules.append(
            {
                QRegularExpression(
                    R"(\b(ABS|AVG|CAST|CEIL|CEILING|COALESCE|CONCAT|COUNT|CURDATE|CURTIME|)"
                    R"(DATEDIFF|DATE_FORMAT|DAY|DAYOFWEEK|FLOOR|FORMAT|GREATEST|GROUP_CONCAT|)"
                    R"(IFNULL|ISNULL|LAST_INSERT_ID|LCASE|LEAST|LENGTH|LOWER|LTRIM|MAX|MIN|)"
                    R"(MOD|MONTH|NOW|NULLIF|RAND|REPLACE|ROUND|RTRIM|SQRT|STR_TO_DATE|)"
                    R"(SUBSTRING|SUM|TRIM|UCASE|UPPER|WEEK|YEAR)\b)",
                    QRegularExpression::CaseInsensitiveOption),
                m_funcFmt
            });
        m_rules.append({
            QRegularExpression(R"(-?\b\d+\.?\d*([eE][+-]?\d+)?\b)"),
            m_numberFmt
        });
        m_rules.append({QRegularExpression(R"('(?:[^'\\]|\\.)*')"), m_stringFmt});
        m_rules.append({
            QRegularExpression(R"("(?:[^"\\]|\\.)*")"),
            m_stringFmt
        }); // identifier quoting (MySQL)
        m_rules.append({QRegularExpression(R"(--.*)"), m_commentFmt});
        m_mlCommentFmt = m_commentFmt;
        m_mlCommentStart = QRegularExpression(R"(/\*)");
        m_mlCommentEnd = QRegularExpression(R"(\*/)");
        break;
    }

    case Language::Dockerfile: {
        // Instructions  (case-insensitive, capture group 1 to skip leading
        // whitespace)
        QTextCharFormat instrFmt;
        instrFmt.setForeground(QColor("#569cd6"));
        instrFmt.setFontWeight(QFont::Bold);
        m_rules.append(
            {
                QRegularExpression(
                    R"(^\s*(ADD|ARG|CMD|COPY|ENTRYPOINT|ENV|EXPOSE|FROM|HEALTHCHECK|)"
                    R"(LABEL|MAINTAINER|ONBUILD|RUN|SHELL|STOPSIGNAL|USER|VOLUME|WORKDIR)\b)",
                    QRegularExpression::CaseInsensitiveOption),
                instrFmt, 1
            });
        // AS alias in FROM ... AS name
        m_rules.append(
            {
                QRegularExpression(R"(\bAS\b)",
                                   QRegularExpression::CaseInsensitiveOption),
                m_keywordFmt
            });
        // Variables  $VAR  ${VAR}
        m_rules.append(
            {QRegularExpression(R"(\$(?:\{[^}]+\}|[A-Za-z_]\w*))"), m_typeFmt});
        // Strings
        m_rules.append({QRegularExpression(R"("(?:[^"\\]|\\.)*")"), m_stringFmt});
        m_rules.append({QRegularExpression(R"('(?:[^'\\]|\\.)*')"), m_stringFmt});
        // Comments
        m_rules.append({QRegularExpression(R"(#.*)"), m_commentFmt});
        break;
    }

    case Language::Markdown: {
        QTextCharFormat h1Fmt;
        h1Fmt.setForeground(QColor("#569cd6"));
        h1Fmt.setFontWeight(QFont::Bold);
        QTextCharFormat hFmt;
        hFmt.setForeground(QColor("#569cd6"));
        hFmt.setFontWeight(QFont::Bold);
        m_rules.append({QRegularExpression(R"(^#\s.*)"), h1Fmt});
        m_rules.append({QRegularExpression(R"(^#{2,6}\s.*)"), hFmt});

        QTextCharFormat boldFmt;
        boldFmt.setFontWeight(QFont::Bold);
        boldFmt.setForeground(QColor("#d4d4d4"));
        m_rules.append(
            {QRegularExpression(R"(\*\*(?:[^*]|\*(?!\*))+\*\*)"), boldFmt});
        m_rules.append({QRegularExpression(R"(__(?:[^_]|_(?!_))+__)"), boldFmt});

        QTextCharFormat italicFmt;
        italicFmt.setFontItalic(true);
        italicFmt.setForeground(QColor("#d4d4d4"));
        m_rules.append({QRegularExpression(R"(\*[^*\n]+\*)"), italicFmt});
        m_rules.append({QRegularExpression(R"(_[^_\n]+_)"), italicFmt});

        QTextCharFormat strikeFmt;
        strikeFmt.setFontStrikeOut(true);
        strikeFmt.setForeground(QColor("#858585"));
        m_rules.append({QRegularExpression(R"(~~[^~\n]+~~)"), strikeFmt});

        QTextCharFormat codeFmt;
        codeFmt.setForeground(QColor("#ce9178"));
        m_rules.append({QRegularExpression(R"(`[^`\n]+`)"), codeFmt});

        // Fenced code block markers  ```lang  or  ~~~
        QTextCharFormat fenceFmt;
        fenceFmt.setForeground(QColor("#c586c0"));
        m_rules.append(
            {QRegularExpression(R"(^(?:```|~~~)[A-Za-z0-9_-]*)"), fenceFmt});
        m_rules.append({QRegularExpression(R"(^(?:```|~~~)\s*$)"), fenceFmt});

        QTextCharFormat linkFmt;
        linkFmt.setForeground(QColor("#4ec9b0"));
        // Images  ![alt](url)
        m_rules.append({QRegularExpression(R"(!\[[^\]]*\]\([^)]*\))"), linkFmt});
        // Links  [text](url)  or  [text][ref]
        m_rules.append({QRegularExpression(R"(\[[^\]]*\]\([^)]*\))"), linkFmt});
        m_rules.append({QRegularExpression(R"(\[[^\]]*\]\[[^\]]*\])"), linkFmt});
        // Bare URLs
        m_rules.append({QRegularExpression(R"(<https?://[^>]+>)"), linkFmt});

        QTextCharFormat quoteFmt;
        quoteFmt.setForeground(QColor("#6a9955"));
        quoteFmt.setFontItalic(true);
        m_rules.append({QRegularExpression(R"(^>\s.*)"), quoteFmt});

        // List bullets (unordered and ordered)
        m_rules.append({QRegularExpression(R"(^[ \t]*[-*+]\s)"), m_keywordFmt});
        m_rules.append({QRegularExpression(R"(^[ \t]*\d+\.\s)"), m_keywordFmt});

        // Task list  - [ ]  - [x]
        QTextCharFormat taskFmt;
        taskFmt.setForeground(QColor("#c586c0"));
        m_rules.append({QRegularExpression(R"(\[[ xX]\])"), taskFmt});

        // Table separators  |---|
        QTextCharFormat tableFmt;
        tableFmt.setForeground(QColor("#4ec9b0"));
        m_rules.append({QRegularExpression(R"(\|[-: ]+\|[-|: ]*)"), tableFmt});
        m_rules.append({QRegularExpression(R"(\|)"), tableFmt});

        QTextCharFormat hrFmt;
        hrFmt.setForeground(QColor("#555555"));
        m_rules.append({QRegularExpression(R"(^[-*_]{3,}\s*$)"), hrFmt});
        break;
    }

    default:
        break;
    }
}

void SyntaxHighlighter::highlightBlock(const QString& text) {
    // Apply all single-line rules in order (last rule wins for overlapping
    // ranges)
    for (const auto& rule : std::as_const(m_rules)) {
        auto it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            auto match = it.next();
            int start = (rule.captureGroup > 0)
                            ? match.capturedStart(rule.captureGroup)
                            : match.capturedStart();
            int len = (rule.captureGroup > 0)
                          ? match.capturedLength(rule.captureGroup)
                          : match.capturedLength();
            if (start >= 0 && len > 0) setFormat(start, len, rule.format);
        }
    }

    if (!m_mlStringDelim.pattern().isEmpty()) {
        setCurrentBlockState(0);
        int searchFrom = 0;

        if (previousBlockState() == 2) {
            auto endMatch = m_mlStringDelim.match(text, 0);
            if (!endMatch.hasMatch()) {
                setCurrentBlockState(2);
                setFormat(0, text.length(), m_mlStringFmt);
                return;
            }
            int endPos = endMatch.capturedStart() + 3;
            setFormat(0, endPos, m_mlStringFmt);
            searchFrom = endPos;
        }

        while (true) {
            auto startMatch = m_mlStringDelim.match(text, searchFrom);
            if (!startMatch.hasMatch()) break;

            int startPos = startMatch.capturedStart();
            auto endMatch = m_mlStringDelim.match(text, startPos + 3);
            if (!endMatch.hasMatch()) {
                setCurrentBlockState(2);
                setFormat(startPos, text.length() - startPos, m_mlStringFmt);
                return;
            }
            int endPos = endMatch.capturedStart() + 3;
            setFormat(startPos, endPos - startPos, m_mlStringFmt);
            searchFrom = endPos;
        }
        return; // Python doesn't use C-style block comments
    }

    // C-style / HTML / Lua block comments (block state 1)
    if (m_mlCommentStart.pattern().isEmpty()) return;

    setCurrentBlockState(0);
    int startIndex = 0;

    if (previousBlockState() != 1) {
        startIndex = text.indexOf(m_mlCommentStart);
    }

    while (startIndex >= 0) {
        auto endMatch = m_mlCommentEnd.match(text, startIndex);
        int endIndex = endMatch.capturedStart();
        int length;

        if (endIndex == -1) {
            setCurrentBlockState(1);
            length = text.length() - startIndex;
        }
        else {
            length = endIndex - startIndex + endMatch.capturedLength();
        }

        setFormat(startIndex, length, m_mlCommentFmt);
        startIndex = text.indexOf(m_mlCommentStart, startIndex + length);
    }
}
