#pragma once

#include <QFont>
#include <QString>
#include <map>
#include <tuple>

namespace hm {

// QFont objects are expensive to create every frame (family alias resolution, engine lookup,
// especially on macOS). Keep one instance per distinct style and hand out references.
inline const QFont& cachedFont(const char* family, int px, bool bold = false, bool italic = false, double spacing = 0.0) {
    static std::map<std::tuple<QString, int, bool, bool, int>, QFont> cache;
    auto key = std::make_tuple(QString::fromLatin1(family), px, bold, italic, static_cast<int>(spacing * 10));
    auto it = cache.find(key);
    if (it == cache.end()) {
        QFont f(QString::fromLatin1(family));
        f.setPixelSize(px);
        f.setBold(bold);
        f.setItalic(italic);
        if (spacing != 0.0) f.setLetterSpacing(QFont::AbsoluteSpacing, spacing);
        it = cache.emplace(key, f).first;
    }
    return it->second;
}

} // namespace hm
