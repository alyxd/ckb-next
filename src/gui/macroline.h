#ifndef MACROLINE_H
#define MACROLINE_H
#include <QString>

#define MACRO_DELAY_DEFAULT -1
class macroLine {
public:
    macroLine() = default;
    macroLine(const QString& _key, const qint64& _usTime, const qint64& _usTimeMax, const bool _keyDown, const bool _printable) :
        key(_key), usTime(_usTime), usTimeMax(_usTimeMax), keyDown(_keyDown), printable(_printable) {}
    QString key;
    qint64 usTime;
    qint64 usTimeMax; // If this is set, then this is a random delay
    bool keyDown, printable;
};

#endif // MACROLINE_H
