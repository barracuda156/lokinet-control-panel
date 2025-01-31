#include "PlatformDetails.hpp"

#include <QPoint>
#include <QCursor>
#include <QScriptEngine>
#include <QDebug>

#include "process/LokinetProcessManager.hpp"

// PlatformDetails::isWindows
Q_INVOKABLE bool PlatformDetails::isWindows() {
#if defined(Q_OS_WINDOWS)
    return true;
#else
    return false;
#endif
}

// PlatformDetails::isLinux
Q_INVOKABLE bool PlatformDetails::isLinux() {
#if defined(Q_OS_LINUX)
    return true;
#else
    return false;
#endif
}

// PlatformDetails::hasSysTray
Q_INVOKABLE bool PlatformDetails::hasSysTray() {
  if(isWindows())
    return true;
  if(isMacOS())
    return true;
  if(isGayland())
  {
    // pretty much every sane environment has a systray but not gayland (GNOME WAYLAND) because $reasons.
    return false;
  }
  if(isLinux())
    return true;
  // we don't know what we are runnning on let's assume we dont have a tray
  return false;
}

// PlatformDetails::isGayland
Q_INVOKABLE bool PlatformDetails::isGayland() {
#if defined(Q_OS_LINUX)
  if(const auto wayland = ::getenv("WAYLAND_DISPLAY"); wayland)
  {
    if(const auto desktop = ::getenv("XDG_CURRENT_DESKTOP"); desktop)
    {
      return std::string{desktop} == "GNOME";
    }
  }
    return false;
#else
    return false;
#endif
}

// PlatformDetails::isMacOS
Q_INVOKABLE bool PlatformDetails::isMacOS() {
#if defined(Q_OS_MACOS)
    return true;
#else
    return false;
#endif
}

// PlatformDetails::isDebug
Q_INVOKABLE bool PlatformDetails::isDebug() {
#if defined(QT_DEBUG)
    return true;
#else
    return false;
#endif
}

Q_INVOKABLE bool PlatformDetails::startLokinetProcess() {
    return LokinetProcessManager::instance()->startLokinetProcess();
}

Q_INVOKABLE bool PlatformDetails::stopLokinetProcess() {
	return LokinetProcessManager::instance()->stopLokinetProcess();
}

Q_INVOKABLE bool PlatformDetails::forciblyStopLokinetProcess() {
    return LokinetProcessManager::instance()->forciblyStopLokinetProcess();
}

Q_INVOKABLE bool PlatformDetails::managedStopLokinetProcess() {
    return LokinetProcessManager::instance()->managedStopLokinetProcess();
}

Q_INVOKABLE bool PlatformDetails::stopLokinetIfWeStartedIt() {
    return LokinetProcessManager::instance()->stopLokinetIfWeStartedIt();
}

Q_INVOKABLE bool PlatformDetails::isLokinetRunning() {
    auto status = LokinetProcessManager::instance()->queryProcessStatus();
    return (status == LokinetProcessManager::ProcessStatus::Running);
}

class PlatformDetails : public QObject {
    Q_OBJECT

public:
    Q_INVOKABLE void downloadBootstrapFile(const QString& url, const QScriptValue& callback);

signals:
    void bootstrapFileDownloaded(int error, const QString& message);

private slots:
    void handleBootstrapFileDownloaded(int error, const QString& message, QScriptValue callback);

private:
    LokinetProcessManager* g_processManager = LokinetProcessManager::instance();
};

void PlatformDetails::downloadBootstrapFile(const QString& url, const QScriptValue& callback) {
    if (callback.isValid() && ! callback.isFunction()) {
        qDebug() << "callback should be a function";
        return;
    }

    connect(this, SIGNAL(bootstrapFileDownloaded(int, QString)), this, SLOT(handleBootstrapFileDownloaded(int, QString, callback)), Qt::UniqueConnection);

    manager->downloadBootstrapFile(url.toStdString(), [=](int error, const std::string& msg) {
        emit bootstrapFileDownloaded(error, QString::fromStdString(msg));
    });
}

void PlatformDetails::handleBootstrapFileDownloaded(int error, const QString& message, QScriptValue callback) {
    QScriptEngine* engine = callback.engine();

    QScriptValue callbackCopy = callback;
    QScriptValue result = callbackCopy.call(QScriptValue(), QScriptValueList() << engine->toScriptValue(error) << engine->toScriptValue(message));

    if (result.isError()) {
        qDebug() << "Error attempting callback";
    }

    disconnect(this, SIGNAL(bootstrapFileDownloaded(int, QString)), this, SLOT(handleBootstrapFileDownloaded(int, QString, callback)));
}

Q_INVOKABLE QPoint PlatformDetails::getAbsoluteCursorPosition() {
    return QCursor::pos();
}
