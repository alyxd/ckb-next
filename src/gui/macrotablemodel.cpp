#include "macrotablemodel.h"
#include <QRegularExpression>
#include <QStringBuilder>

const QSet<QString> MacroTableModel::validMacroKeys =
        QSet<QString>{"esc",
                    "f1",
                    "f2",
                    "f3",
                    "f4",
                    "f5",
                    "f6",
                    "f7",
                    "f8",
                    "f9",
                    "f10",
                    "f11",
                    "grave",
                    "1",
                    "2",
                    "3",
                    "4",
                    "5",
                    "6",
                    "7",
                    "8",
                    "9",
                    "0",
                    "minus",
                    "tab",
                    "q",
                    "w",
                    "e",
                    "r",
                    "t",
                    "y",
                    "u",
                    "i",
                    "o",
                    "p",
                    "lbrace",
                    "caps",
                    "a",
                    "s",
                    "d",
                    "f",
                    "g",
                    "h",
                    "j",
                    "k",
                    "l",
                    "colon",
                    "quote",
                    "lshift",
                    "bslash_iso",
                    "z",
                    "x",
                    "c",
                    "v",
                    "b",
                    "n",
                    "m",
                    "comma",
                    "dot",
                    "slash",
                    "lctrl",
                    "lwin",
                    "lalt",
                    "hanja",
                    "space",
                    "hangul",
                    "katahira",
                    "ralt",
                    "rwin",
                    "rmenu",
                    "f12",
                    "prtscn",
                    "scroll",
                    "pause",
                    "ins",
                    "home",
                    "pgup",
                    "rbrace",
                    "bslash",
                    "hash",
                    "enter",
                    "ro",
                    "equal",
                    "yen",
                    "bspace",
                    "del",
                    "end",
                    "pgdn",
                    "rshift",
                    "rctrl",
                    "up",
                    "left",
                    "down",
                    "right",
                    "mute",
                    "stop",
                    "prev",
                    "play",
                    "next",
                    "numlock",
                    "numslash",
                    "numstar",
                    "numminus",
                    "numplus",
                    "numenter",
                    "num7",
                    "num8",
                    "num9",
                    "num4",
                    "num5",
                    "num6",
                    "num1",
                    "num2",
                    "num3",
                    "num0",
                    "numdot",
                    "volup",
                    "voldn",
                    "muhenkan",
                    "henkan",
                    "fn",
                    "lightup",
                    "lightdn",
                    "eject",
                    "power",
                    "f13",
                    "f14",
                    "f15",
                    "f16",
                    "f17",
                    "f18",
                    "f19",
                    "f20",
                    "f21",
                    "f22",
                    "f23",
                    "f24",
                    "mouse1",
                    "mouse2",
                    "mouse3",
                    "mouse4",
                    "mouse5",
                    "wheelup",
                    "wheeldn",
                    "mouse6",
                    "mouse7",
                    "mouse8"
    };


int MacroTableModel::rowCount(const QModelIndex& parent) const{
    return macroLines.length();
}

int MacroTableModel::columnCount(const QModelIndex& parent) const{
    return 3;
}

QVariant MacroTableModel::data(const QModelIndex& index, int role) const{
    if (role == Qt::DisplayRole || role == Qt::EditRole){
        const macroLine ml = macroLines.at(index.row());
        switch(index.column()){
            case 0:
                return QString((ml.keyDown ? "↓" : "↑"));
            case 1:
                return QString(ml.key);
            case 2:
                if(index.row() == 0)
                    return QString(tr("None"));
                else if(defaultDelay || ml.usTime == MACRO_DELAY_DEFAULT)
                    return QString(tr("Default"));
                else {
                    if(ml.usTimeMax == MACRO_DELAY_DEFAULT)
                        return QString::number(ml.usTime);
                    else
                        return QString("%1_%2").arg(ml.usTime).arg(ml.usTimeMax);
                }
            default:
                return tr("Unknown");
        }
    }
    return QVariant();
}

QVariant MacroTableModel::headerData(int section, Qt::Orientation orientation, int role) const{
    if(orientation != Qt::Horizontal)
        return QVariant();

    if(role == Qt::DisplayRole) {
        switch(section){
            case 0:
                return QString("↓");
            case 1:
                return QString(tr("Key"));
            case 2:
                return QString(tr("Delay"));
            default:
                return QVariant();
        }
    }
    return QVariant();

}

Qt::ItemFlags MacroTableModel::flags(const QModelIndex &idx) const{
    if (!idx.isValid())
        return Qt::ItemIsEnabled;
    else if(idx == index(0, 2) || (idx.column() == 2 && defaultDelay))
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

bool MacroTableModel::setData(const QModelIndex& index, const QVariant& value, int role){
    if (!index.isValid() || role != Qt::EditRole)
        return false;
    macroLine& ml = macroLines[index.row()];
    switch(index.column()){
    case 0:
        ml.keyDown = !value.toInt(); // toInt() because the variant has an index int in it, which will get converted to bool
        break;
    case 1:
    {
        QString key = value.toString();
        if(!validMacroKeys.contains(key))
            return false;
        ml.key = key;
    }
        break;
    case 2:
    {
        QString valstr = value.toString();
        QVector<QStringRef> split = valstr.splitRef(QChar('_'));
        const int len = split.length();
        if(len != 1 && len != 2)
            return false;
        bool ok;
        qint64 delay = split.at(0).toLongLong(&ok);
        if(!ok || delay < 0)
            return false;
        ml.usTime = delay;
        if(len == 2){
            qint64 maxDelay = split.at(1).toLongLong(&ok);
            // We must also make sure that the max delay is less than the min
            if(!ok || maxDelay <= delay)
                return false;
            ml.usTimeMax = maxDelay;
        }
    }
        break;
    default:
        qDebug() << "Unknown column in setData";
    }
    emit dataChanged(index, index, {role});
    return true;
}

QString MacroTableModel::toString(bool rawData){
    QString l;
    const int mlength = macroLines.length();
    for(int i = 0; i < mlength; i++){
        const macroLine& ml = macroLines.at(i);
        l.append((ml.keyDown ? '+' : '-'));
        l.append(ml.key);

        if(i + 1 < mlength){
            const macroLine& nextMl = macroLines.at(i+1);
            const qint64 usTime = nextMl.usTime;
            if(usTime != MACRO_DELAY_DEFAULT && (!defaultDelay || rawData)){
                l.append("=" + QString::number(usTime));
                if(nextMl.usTimeMax != MACRO_DELAY_DEFAULT)
                    l.append("_" + QString::number(nextMl.usTimeMax));
            }
            l.append(",");
        }
    }

    return l;
}

#define MACRO_ERROR(end, start)  return QString::number(end) % ":<br>" % str.leftRef(end) \
                                        % "<b><span style=\"color: #ff0000;\">" % str.midRef(end, start - end) % "</span></b>" \
                                        % str.midRef(start)
#define MACRO_ERROR_RET()       MACRO_ERROR(previousEnd, currentStart)
#define MACRO_ERROR_RET_SUFFIX(end, start, suffix) MACRO_ERROR(end, start) % "<br><br>" % suffix
QString MacroTableModel::fromString(const QString& input, const bool stopOnError){
    // Replace all "whitespace" characters with ' ', and then remove that as well
    QString str = input.simplified();
    str.replace(QChar(' '), QString(""));
    QVector<macroLine> newMacroLines;
    QRegularExpression re("(\\+|-)([a-z0-9]+)(=(\\d+)(_(\\d+))?)?(,|$)");
    QRegularExpressionMatchIterator i = re.globalMatch(str);
    qint64 prevDelay = MACRO_DELAY_DEFAULT, prevMaxDelay = MACRO_DELAY_DEFAULT;
    int previousEnd = 0;
    while (i.hasNext()) {
        QRegularExpressionMatch m = i.next();
        // Make sure the match started from the previous end. If it didn't, we have a parse error
        const int currentStart = m.capturedStart();
        if(previousEnd != currentStart && stopOnError){
            MACRO_ERROR_RET();
        }
        QStringRef act = m.capturedRef(1);
        QString key = m.captured(2);
        QStringRef delaystr = m.capturedRef(4);
        QStringRef maxDelayStr = m.capturedRef(6);

        qint64 delay = MACRO_DELAY_DEFAULT, maxDelay = MACRO_DELAY_DEFAULT;
        if(!delaystr.isNull()){
            bool ok;
            delay = delaystr.toLongLong(&ok);
            if(!ok){
                if(stopOnError)
                    MACRO_ERROR_RET_SUFFIX(m.capturedStart(4), m.capturedEnd(4), tr("Delay is too large"));
                delay = MACRO_DELAY_DEFAULT;
            }
            if(!maxDelayStr.isNull()){
                maxDelay = maxDelayStr.toLongLong(&ok);
                if(!ok){
                    if(stopOnError)
                        MACRO_ERROR_RET_SUFFIX(m.capturedStart(6), m.capturedEnd(6), tr("Max random delay is too large"));
                    maxDelay = MACRO_DELAY_DEFAULT;
                } else if(maxDelay <= delay) {
                    if(stopOnError)
                        MACRO_ERROR_RET_SUFFIX(m.capturedStart(6), m.capturedEnd(6), tr("Max random delay is less or equal to the minimum"));
                    maxDelay = MACRO_DELAY_DEFAULT;
                }
            }
        }
        // Act and key will always be valid strings
        const bool down = (act == "+");

        // Make sure the key actually exists
        if(!validMacroKeys.contains(key)){
            if(stopOnError)
                MACRO_ERROR_RET_SUFFIX(m.capturedStart(2), m.capturedEnd(2), tr("Invalid key ") % key);
            else
                continue;
        }

        newMacroLines.append(macroLine(key, prevDelay, prevMaxDelay, down, (key.length() == 1)));
        prevDelay = delay;
        prevMaxDelay = maxDelay;
        previousEnd = m.capturedEnd();
    }
    // If we get down here, check that we reached the end
    const int len = input.length();
    if(previousEnd != len && stopOnError)
        MACRO_ERROR(previousEnd, len);
    emit beginResetModel();
    macroLines = newMacroLines;
    emit endResetModel();
    return QString();
}

void MacroTableModel::removeLastMouseLeftClick(){
    // Go through the vector in reverse, find the last two mouse events and remove them
    bool foundKeyUp = false;
    for(int i = macroLines.length() - 1; i >= 0; i--){
        const macroLine& ml = macroLines.at(i);
        // Once the Key Up was found, start searching for a keydown
        if(foundKeyUp && ml.keyDown && ml.key == "mouse1"){
            emit beginRemoveRows(QModelIndex(), i, i);
            macroLines.removeAt(i);
            emit endRemoveRows();
            return;
        } else if(!ml.keyDown && ml.key == "mouse1") {
            emit beginRemoveRows(QModelIndex(), i, i);
            macroLines.removeAt(i);
            emit endRemoveRows();
            foundKeyUp = true;
        }
    }
}

void MacroDropdownDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
                                const QModelIndex& index) const
{
    MacroDropdown* e = qobject_cast<MacroDropdown*>(editor);
    model->setData(index, e->currentIndex());
}

void MacroDropdownDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    MacroDropdown* e = qobject_cast<MacroDropdown*>(editor);
    e->setCurrentIndex(index.data().toString() == "↑");
}

// Check if all the key events are matched
// For example, +key,-key returns -1
// +key returns 0, which is the position of +key in the vector
int MacroTableModel::isMacroMatched(){
    QSet<int> visitedIndices;
    const int mLength = macroLines.length();
    // While we could check if the number of items is odd, and if so, return,
    // we won't be able to know where the issue is, so we might as well parse it anyway.
    if(mLength == 1)
        return 0;
    for(int i = 0; i < mLength; i++){
        // If we already visited this key, ignore it
        if(visitedIndices.contains(i))
            continue;
        const macroLine& ml_i = macroLines.at(i);
        // Outer loop checks for keydowns, inner loop checks for keyups
        // If we get a keyup on the outer loop that wasn't skipped, then there's an error
        if(!ml_i.keyDown)
            return i;
        // No need to add indices visited with i, as neither loop goes back
        int j;
        for(j = i + 1; j < mLength; j++){
            if(visitedIndices.contains(j))
                continue;
            const macroLine& ml_j = macroLines.at(j);
            if(ml_j.keyDown)
                continue;

            if(ml_i.key == ml_j.key){
                visitedIndices.insert(j);
                break;
            }
        }
        // No match
        if(j == mLength)
            return i;
    }
    // If we get out here, everything matched
    return -1;
}
