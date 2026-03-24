#ifndef OPENNOTE_LINUX_SYNTAXTHEME_H
#define OPENNOTE_LINUX_SYNTAXTHEME_H
#pragma once

#include <QApplication>
#include <QMap>
#include <QMetaType>
#include <QPalette>
#include <QString>
#include <QStringList>
#include <QVariant>

struct TokenStyle {
    QString color = "#d4d4d4";
    bool bold = false;
    bool italic = false;

    QVariantMap toVariantMap() const {
        QVariantMap m;
        m["color"] = color;
        m["bold"] = bold;
        m["italic"] = italic;
        return m;
    }

    static TokenStyle fromVariantMap(const QVariantMap& m) {
        TokenStyle ts;
        ts.color = m.value("color", "#d4d4d4").toString();
        ts.bold = m.value("bold", false).toBool();
        ts.italic = m.value("italic", false).toBool();
        return ts;
    }

    bool operator==(const TokenStyle& other) const {
        return color == other.color && bold == other.bold && italic == other.italic;
    }
};

using SyntaxTheme = QMap<QString, TokenStyle>;

inline QVariant themeToVariant(const SyntaxTheme& theme) {
    QVariantMap themeMap;
    for (auto it = theme.begin(); it != theme.end(); ++it) {
        themeMap[it.key()] = it.value().toVariantMap();
    }
    return themeMap;
}

inline SyntaxTheme variantToTheme(const QVariant& var) {
    SyntaxTheme theme;
    QVariantMap themeMap = var.toMap();
    for (auto it = themeMap.begin(); it != themeMap.end(); ++it) {
        theme[it.key()] = TokenStyle::fromVariantMap(it.value().toMap());
    }
    return theme;
}

inline QStringList tokenKeys() {
    return {
        "keyword", "type", "string", "comment", "number",
        "preprocessor", "function", "decorator", "tag", "attribute"
    };
}

inline QMap<QString, QString> tokenDisplayNames() {
    return {
        {"keyword", "Keyword"}, {"type", "Type / Class"},
        {"string", "String"}, {"comment", "Comment"},
        {"number", "Number"}, {"preprocessor", "Preprocessor / Macro"},
        {"function", "Function"}, {"decorator", "Decorator / Annotation"},
        {"tag", "Tag  (HTML / XML)"}, {"attribute", "Attribute"},
    };
}

inline bool isSystemDarkTheme() {
    QPalette palette = QApplication::palette();
    QColor bgColor = palette.color(QPalette::Window);

    return bgColor.lightness() < 128;
}

// OpenNote Original Themes, No attribution required
// OpenNote Default Dark, The default theme for dark mode
inline SyntaxTheme openNoteDefaultDarkTheme() {
    SyntaxTheme t;
    t["keyword"] = {"#5b9bd5", true, false}; // Medium blue, bold
    t["type"] = {"#4dbfad", false, false}; // Teal
    t["string"] = {"#d19a66", false, false}; // Warm orange
    t["comment"] = {"#7c9d7e", false, true}; // Muted green, italic
    t["number"] = {"#b4d4a8", false, false}; // Light green
    t["preprocessor"] = {"#c678dd", false, false}; // Lavender
    t["function"] = {"#e5c07b", false, false}; // Gold
    t["decorator"] = {"#c678dd", false, false}; // Lavender
    t["tag"] = {"#4dbfad", false, false}; // Teal
    t["attribute"] = {"#61afef", false, false}; // Bright blue
    return t;
}

// OpenNote Default Light - The default theme for light mode
inline SyntaxTheme openNoteDefaultLightTheme() {
    SyntaxTheme t;
    t["keyword"] = {"#0066cc", true, false}; // Darker blue, bold
    t["type"] = {"#267f99", false, false}; // Darker teal
    t["string"] = {"#b5651d", false, false}; // Darker orange
    t["comment"] = {"#5a7d5c", false, true}; // Darker green, italic
    t["number"] = {"#2d8659", false, false}; // Darker green
    t["preprocessor"] = {"#9b4dca", false, false}; // Darker purple
    t["function"] = {"#aa8328", false, false}; // Darker gold
    t["decorator"] = {"#9b4dca", false, false}; // Darker purple
    t["tag"] = {"#1d7a6e", false, false}; // Darker teal
    t["attribute"] = {"#2563eb", false, false}; // Darker blue
    return t;
}

// OpenNote Default - Auto detect system theme (dark or light mode)
inline SyntaxTheme openNoteDefaultTheme() {
    return isSystemDarkTheme() ? openNoteDefaultDarkTheme() : openNoteDefaultLightTheme();
}

// Ocean Dark - Cool blues and teals
inline SyntaxTheme oceanDarkTheme() {
    SyntaxTheme t;
    t["keyword"] = {"#4fc3f7", true, false}; // Bright cyan, bold
    t["type"] = {"#26c6da", false, false}; // Teal
    t["string"] = {"#80deea", false, false}; // Light cyan
    t["comment"] = {"#607d8b", false, true}; // Blue-gray, italic
    t["number"] = {"#64b5f6", false, false}; // Light blue
    t["preprocessor"] = {"#7986cb", false, false}; // Indigo
    t["function"] = {"#81c784", false, false}; // Light green accent
    t["decorator"] = {"#7986cb", false, false}; // Indigo
    t["tag"] = {"#26c6da", false, false}; // Teal
    t["attribute"] = {"#4fc3f7", false, false}; // Bright cyan
    return t;
}

// Ocean Light - Darker ocean tones for light mode
inline SyntaxTheme oceanLightTheme() {
    SyntaxTheme t;
    t["keyword"] = {"#0277bd", true, false}; // Deep blue, bold
    t["type"] = {"#00838f", false, false}; // Dark teal
    t["string"] = {"#00695c", false, false}; // Dark cyan-green
    t["comment"] = {"#546e7a", false, true}; // Gray-blue, italic
    t["number"] = {"#1565c0", false, false}; // Dark blue
    t["preprocessor"] = {"#4527a0", false, false}; // Deep indigo
    t["function"] = {"#2e7d32", false, false}; // Dark green
    t["decorator"] = {"#4527a0", false, false}; // Deep indigo
    t["tag"] = {"#00838f", false, false}; // Dark teal
    t["attribute"] = {"#0277bd", false, false}; // Deep blue
    return t;
}

// Forest Dark - Greens and earth tones
inline SyntaxTheme forestDarkTheme() {
    SyntaxTheme t;
    t["keyword"] = {"#81c784", true, false}; // Light green, bold
    t["type"] = {"#aed581", false, false}; // Yellow-green
    t["string"] = {"#ffb74d", false, false}; // Orange accent
    t["comment"] = {"#8d6e63", false, true}; // Brown, italic
    t["number"] = {"#c5e1a5", false, false}; // Pale green
    t["preprocessor"] = {"#ba68c8", false, false}; // Purple accent
    t["function"] = {"#fff176", false, false}; // Yellow
    t["decorator"] = {"#ba68c8", false, false}; // Purple
    t["tag"] = {"#aed581", false, false}; // Yellow-green
    t["attribute"] = {"#ffab91", false, false}; // Peach
    return t;
}

// Forest Light - Darker forest tones for light mode
inline SyntaxTheme forestLightTheme() {
    SyntaxTheme t;
    t["keyword"] = {"#2e7d32", true, false}; // Dark green, bold
    t["type"] = {"#558b2f", false, false}; // Olive green
    t["string"] = {"#e65100", false, false}; // Dark orange
    t["comment"] = {"#5d4037", false, true}; // Dark brown, italic
    t["number"] = {"#689f38", false, false}; // Medium green
    t["preprocessor"] = {"#7b1fa2", false, false}; // Dark purple
    t["function"] = {"#f9a825", false, false}; // Dark yellow
    t["decorator"] = {"#7b1fa2", false, false}; // Dark purple
    t["tag"] = {"#558b2f", false, false}; // Olive green
    t["attribute"] = {"#d84315", false, false}; // Dark red-orange
    return t;
}

// Sunset Dark - Warm oranges and purples
inline SyntaxTheme sunsetDarkTheme() {
    SyntaxTheme t;
    t["keyword"] = {"#ff8a65", true, false}; // Coral, bold
    t["type"] = {"#ce93d8", false, false}; // Light purple
    t["string"] = {"#ffcc80", false, false}; // Peach
    t["comment"] = {"#90a4ae", false, true}; // Blue-gray, italic
    t["number"] = {"#f48fb1", false, false}; // Pink
    t["preprocessor"] = {"#ba68c8", false, false}; // Purple
    t["function"] = {"#ffd54f", false, false}; // Golden yellow
    t["decorator"] = {"#ba68c8", false, false}; // Purple
    t["tag"] = {"#ce93d8", false, false}; // Light purple
    t["attribute"] = {"#ffab40", false, false}; // Orange
    return t;
}

// Sunset Light - Darker sunset tones for light mode
inline SyntaxTheme sunsetLightTheme() {
    SyntaxTheme t;
    t["keyword"] = {"#d84315", true, false}; // Dark orange, bold
    t["type"] = {"#6a1b9a", false, false}; // Dark purple
    t["string"] = {"#ef6c00", false, false}; // Dark peach
    t["comment"] = {"#546e7a", false, true}; // Dark blue-gray, italic
    t["number"] = {"#c2185b", false, false}; // Dark pink
    t["preprocessor"] = {"#7b1fa2", false, false}; // Dark purple
    t["function"] = {"#f57f17", false, false}; // Dark golden
    t["decorator"] = {"#7b1fa2", false, false}; // Dark purple
    t["tag"] = {"#6a1b9a", false, false}; // Dark purple
    t["attribute"] = {"#e65100", false, false}; // Dark orange
    return t;
}

// Lavender Dark - Purples and pinks
inline SyntaxTheme lavenderDarkTheme() {
    SyntaxTheme t;
    t["keyword"] = {"#ce93d8", true, false}; // Light purple, bold
    t["type"] = {"#ba68c8", false, false}; // Medium purple
    t["string"] = {"#f48fb1", false, false}; // Pink
    t["comment"] = {"#9575cd", false, true}; // Purple-gray, italic
    t["number"] = {"#e1bee7", false, false}; // Pale purple
    t["preprocessor"] = {"#ab47bc", false, false}; // Dark purple
    t["function"] = {"#80cbc4", false, false}; // Teal accent
    t["decorator"] = {"#ab47bc", false, false}; // Dark purple
    t["tag"] = {"#ba68c8", false, false}; // Medium purple
    t["attribute"] = {"#f06292", false, false}; // Bright pink
    return t;
}

// Lavender Light - Darker lavender tones for light mode
inline SyntaxTheme lavenderLightTheme() {
    SyntaxTheme t;
    t["keyword"] = {"#6a1b9a", true, false}; // Dark purple, bold
    t["type"] = {"#7b1fa2", false, false}; // Deep purple
    t["string"] = {"#ad1457", false, false}; // Dark pink
    t["comment"] = {"#5e35b1", false, true}; // Dark purple-blue, italic
    t["number"] = {"#8e24aa", false, false}; // Dark purple
    t["preprocessor"] = {"#6a1b9a", false, false}; // Dark purple
    t["function"] = {"#00838f", false, false}; // Dark teal
    t["decorator"] = {"#6a1b9a", false, false}; // Dark purple
    t["tag"] = {"#7b1fa2", false, false}; // Deep purple
    t["attribute"] = {"#c2185b", false, false}; // Dark pink
    return t;
}

// Midnight Dark - Deep blues and cool grays
inline SyntaxTheme midnightDarkTheme() {
    SyntaxTheme t;
    t["keyword"] = {"#82b1ff", true, false}; // Bright blue, bold
    t["type"] = {"#b39ddb", false, false}; // Light purple
    t["string"] = {"#80deea", false, false}; // Cyan
    t["comment"] = {"#78909c", false, true}; // Blue-gray, italic
    t["number"] = {"#90caf9", false, false}; // Light blue
    t["preprocessor"] = {"#9fa8da", false, false}; // Indigo
    t["function"] = {"#a5d6a7", false, false}; // Light green
    t["decorator"] = {"#9fa8da", false, false}; // Indigo
    t["tag"] = {"#b39ddb", false, false}; // Light purple
    t["attribute"] = {"#64b5f6", false, false}; // Medium blue
    return t;
}

// Midnight Light - Darker midnight tones for light mode
inline SyntaxTheme midnightLightTheme() {
    SyntaxTheme t;
    t["keyword"] = {"#1565c0", true, false}; // Deep blue, bold
    t["type"] = {"#4527a0", false, false}; // Dark purple
    t["string"] = {"#00695c", false, false}; // Dark cyan
    t["comment"] = {"#455a64", false, true}; // Dark blue-gray, italic
    t["number"] = {"#0d47a1", false, false}; // Very dark blue
    t["preprocessor"] = {"#311b92", false, false}; // Deep indigo
    t["function"] = {"#1b5e20", false, false}; // Dark green
    t["decorator"] = {"#311b92", false, false}; // Deep indigo
    t["tag"] = {"#4527a0", false, false}; // Dark purple
    t["attribute"] = {"#0d47a1", false, false}; // Very dark blue
    return t;
}

struct ThemeInfo {
    QString name;
    QString id;
    SyntaxTheme theme;
};

inline QList<ThemeInfo> predefinedThemes() {
    return {
        // Default theme (dark mode first)
        {
            "OpenNote Default Dark", "opennote-default-dark",
            openNoteDefaultDarkTheme()
        },
        {
            "OpenNote Default Light", "opennote-default-light",
            openNoteDefaultLightTheme()
        },

        // Ocean theme
        {"Ocean Dark", "ocean-dark", oceanDarkTheme()},
        {"Ocean Light", "ocean-light", oceanLightTheme()},

        // Forest theme
        {"Forest Dark", "forest-dark", forestDarkTheme()},
        {"Forest Light", "forest-light", forestLightTheme()},

        // Sunset theme
        {"Sunset Dark", "sunset-dark", sunsetDarkTheme()},
        {"Sunset Light", "sunset-light", sunsetLightTheme()},

        // Lavender theme
        {"Lavender Dark", "lavender-dark", lavenderDarkTheme()},
        {"Lavender Light", "lavender-light", lavenderLightTheme()},

        // Midnight theme
        {"Midnight Dark", "midnight-dark", midnightDarkTheme()},
        {"Midnight Light", "midnight-light", midnightLightTheme()}
    };
}

#endif  // OPENNOTE_LINUX_SYNTAXTHEME_H
