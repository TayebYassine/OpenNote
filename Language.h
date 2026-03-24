#ifndef OPENNOTE_LINUX_LANGUAGE_H
#define OPENNOTE_LINUX_LANGUAGE_H
#pragma once

#include <QFileInfo>
#include <QList>
#include <QMap>
#include <QString>

enum class Language {
    None,
    Cpp,
    C,
    CSharp,
    Java,
    Kotlin,
    Groovy,
    Python,
    Ruby,
    Lua,
    PHP,
    JavaScript,
    TypeScript,
    HTML,
    CSS,
    XML,
    JSON,
    YAML,
    TOML,
    Bash,
    Rust,
    Go,
    Swift,
    Assembly,
    Pascal,
    ActionScript,
    Clojure,
    Dart,
    Fortran,
    INI,
    Lisp,
    Perl,
    Scala,
    Zig,
    Haskell,
    SQL,
    Dockerfile,
    Markdown
};

inline Language languageFromFile(const QString& filePath) {
    const QFileInfo info(filePath);

    // Dockerfile has no extension, match by filename first
    const QString fname = info.fileName().toLower();
    if (fname == "dockerfile" || fname.startsWith("dockerfile."))
        return Language::Dockerfile;

    const QString ext = info.suffix().toLower();
    static const QMap<QString, Language> extMap = {
        {"cpp", Language::Cpp},
        {"cc", Language::Cpp},
        {"cxx", Language::Cpp},
        {"h", Language::Cpp},
        {"hpp", Language::Cpp},
        {"hxx", Language::Cpp},
        {"ino", Language::Cpp}, // .ino is Arduino
        {"c", Language::C},
        {"cs", Language::CSharp},
        {"csx", Language::CSharp},
        {"java", Language::Java},
        {"kt", Language::Kotlin},
        {"kts", Language::Kotlin},
        {"groovy", Language::Groovy},
        {"gvy", Language::Groovy},
        {"gy", Language::Groovy},
        {"gsh", Language::Groovy},
        {"py", Language::Python},
        {"pyw", Language::Python},
        {"pyi", Language::Python},
        {"js", Language::JavaScript},
        {"jsx", Language::JavaScript},
        {"mjs", Language::JavaScript},
        {"cjs", Language::JavaScript},
        {"ts", Language::TypeScript},
        {"tsx", Language::TypeScript},
        {"html", Language::HTML},
        {"htm", Language::HTML},
        {"css", Language::CSS},
        {"scss", Language::CSS},
        {"less", Language::CSS},
        {"xml", Language::XML},
        {"svg", Language::XML},
        {"xaml", Language::XML},
        {"json", Language::JSON},
        {"jsonc", Language::JSON},
        {"yaml", Language::YAML},
        {"yml", Language::YAML},
        {"toml", Language::TOML},
        {"sh", Language::Bash},
        {"bash", Language::Bash},
        {"zsh", Language::Bash},
        {"fish", Language::Bash},
        {"rs", Language::Rust},
        {"go", Language::Go},
        {"swift", Language::Swift},
        {"hs", Language::Haskell},
        {"lhs", Language::Haskell},
        {"zig", Language::Zig},
        {"pas", Language::Pascal},
        {"pp", Language::Pascal},
        {"inc", Language::Pascal},
        {"asm", Language::Assembly},
        {"s", Language::Assembly},
        {"S", Language::Assembly},
        {"nasm", Language::Assembly},
        {"as", Language::ActionScript},
        {"clj", Language::Clojure},
        {"cljs", Language::Clojure},
        {"cljc", Language::Clojure},

        {"dart", Language::Dart},

        // Fortran
        {"f", Language::Fortran},
        {"for", Language::Fortran},
        {"f90", Language::Fortran},
        {"f95", Language::Fortran},
        {"f03", Language::Fortran},
        {"f08", Language::Fortran},
        {"f18", Language::Fortran},
        {"F", Language::Fortran},
        {"F90", Language::Fortran},
        {"F95", Language::Fortran},
        {"F03", Language::Fortran},
        {"F08", Language::Fortran},
        {"F18", Language::Fortran},

        {"ini", Language::INI},
        {"cfg", Language::INI},
        {"conf", Language::INI},
        {"properties", Language::INI},
        {"prop", Language::INI},
        {"settings", Language::INI},
        {"desktop", Language::INI},
        {"service", Language::INI},
        {"socket", Language::INI},
        {"device", Language::INI},
        {"mount", Language::INI},
        {"automount", Language::INI},
        {"swap", Language::INI},
        {"path", Language::INI},
        {"timer", Language::INI},
        {"slice", Language::INI},
        {"scope", Language::INI},
        {"lisp", Language::Lisp},
        {"lsp", Language::Lisp},
        {"l", Language::Lisp},
        {"cl", Language::Lisp},
        {"el", Language::Lisp},
        {"elc", Language::Lisp},
        {"pl", Language::Perl},
        {"pm", Language::Perl},
        {"pod", Language::Perl},
        {"t", Language::Perl},
        {"cgi", Language::Perl},
        {"perl", Language::Perl},
        {"p6", Language::Perl},
        {"scala", Language::Scala},
        {"sc", Language::Scala},
        {"sbt", Language::Scala},
        {"php", Language::PHP},
        {"phtml", Language::PHP},
        {"php8", Language::PHP},
        {"rb", Language::Ruby},
        {"rake", Language::Ruby},
        {"gemspec", Language::Ruby},
        {"lua", Language::Lua},
        {"sql", Language::SQL},
        {"md", Language::Markdown},
        {"markdown", Language::Markdown},
    };
    return extMap.value(ext, Language::None);
}

inline QString languageName(Language lang) {
    switch (lang) {
    case Language::Cpp:
        return "C++";
    case Language::C:
        return "C";
    case Language::Java:
        return "Java";
    case Language::Kotlin:
        return "Kotlin";
    case Language::Python:
        return "Python";
    case Language::Ruby:
        return "Ruby";
    case Language::Lua:
        return "Lua";
    case Language::PHP:
        return "PHP";
    case Language::JavaScript:
        return "JavaScript";
    case Language::TypeScript:
        return "TypeScript";
    case Language::HTML:
        return "HTML";
    case Language::CSS:
        return "CSS";
    case Language::XML:
        return "XML";
    case Language::JSON:
        return "JSON";
    case Language::YAML:
        return "YAML Ain't Markup Language";
    case Language::TOML:
        return "TOML";
    case Language::Bash:
        return "Bash / Shell";
    case Language::Rust:
        return "Rust";
    case Language::Go:
        return "Go";
    case Language::Swift:
        return "Swift";
    case Language::SQL:
        return "SQL";
    case Language::Dockerfile:
        return "Dockerfile";
    case Language::Markdown:
        return "Markdown";
    case Language::Haskell:
        return "Haskell";
    case Language::Groovy:
        return "Groovy";
    case Language::Zig:
        return "Zig";
    case Language::CSharp:
        return "C#";
    case Language::Pascal:
        return "Pascal";
    case Language::Assembly:
        return "Assembly";
    case Language::ActionScript:
        return "ActionScript";
    case Language::Clojure:
        return "Clojure";
    case Language::Dart:
        return "Dart";
    case Language::Fortran:
        return "Fortran";
    case Language::INI:
        return "INI";
    case Language::Lisp:
        return "Lisp";
    case Language::Perl:
        return "Perl";
    case Language::Scala:
        return "Scala";
    default:
        return "Plain Text";
    }
}

inline QList<Language> allLanguages() {
    return {
        Language::Cpp, Language::C, Language::Java,
        Language::Kotlin, Language::Python, Language::Ruby,
        Language::Lua, Language::PHP, Language::JavaScript,
        Language::TypeScript, Language::HTML, Language::CSS,
        Language::XML, Language::JSON, Language::YAML,
        Language::TOML, Language::Bash, Language::Rust,
        Language::Go, Language::Swift, Language::SQL,
        Language::Dockerfile, Language::Markdown, Language::None,
        Language::Haskell, Language::Groovy, Language::Zig,
        Language::CSharp, Language::Pascal, Language::Assembly,
        Language::ActionScript, Language::Clojure, Language::Dart,
        Language::Fortran, Language::INI, Language::Lisp,
        Language::Perl, Language::Scala
    };
}

#endif  // OPENNOTE_LINUX_LANGUAGE_H
