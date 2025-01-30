#include "QmlClipboardAdapter.hpp"

#include <QApplication>

// QmlClipboardAdapter constructor
QmlClipboardAdapter::QmlClipboardAdapter(QObject *parent)
        : QObject(parent)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    clipboard = QGuiApplication::clipboard();
#else
    clipboard = QApplication::clipboard();
#endif
}

Q_INVOKABLE void QmlClipboardAdapter::setText(QString text) {
    clipboard->setText(text, QClipboard::Clipboard);
    clipboard->setText(text, QClipboard::Selection);
}
