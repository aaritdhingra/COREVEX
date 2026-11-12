// ============================================================================
// SECTION 1 (impl) — ASSET PATH RESOLUTION (resource-first)
// ============================================================================
#include "Assets.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QUrl>

namespace corevex {

static QString resourcePath(const QString& relativePath) {
    return QStringLiteral(":/assets/") + relativePath;
}

static QByteArray readAssetBytes(const QString& relativePath) {
    QFile res(resourcePath(relativePath));
    if (res.open(QIODevice::ReadOnly)) return res.readAll();
    QFile file(assetPath(relativePath));
    if (file.open(QIODevice::ReadOnly)) return file.readAll();
    return QByteArray();
}

QString assetPath(const QString& relativePath) {
    if (QFile::exists(resourcePath(relativePath))) return resourcePath(relativePath);
    const QStringList candidates = {
        QCoreApplication::applicationDirPath() + "/assets/" + relativePath,
        QCoreApplication::applicationDirPath() + "/../assets/" + relativePath,
        QDir::currentPath() + "/assets/" + relativePath,
        QDir::currentPath() + "/browser/assets/" + relativePath,
    };
    for (const QString& path : candidates) {
        if (QFile::exists(path)) return QDir::cleanPath(path);
    }
    return resourcePath(relativePath);
}

QString assetDataUri(const QString& relativePath) {
    const QByteArray bytes = readAssetBytes(relativePath);
    if (bytes.isEmpty()) return QString();
    // bg.png ke liye mime PNG hona chahiye, warna browser image nahi dikhata.
    QString mime = QStringLiteral("image/svg+xml");
    if (relativePath.endsWith(QStringLiteral(".png"), Qt::CaseInsensitive)) mime = QStringLiteral("image/png");
    else if (relativePath.endsWith(QStringLiteral(".jpg"), Qt::CaseInsensitive) || relativePath.endsWith(QStringLiteral(".jpeg"), Qt::CaseInsensitive)) mime = QStringLiteral("image/jpeg");
    return QStringLiteral("data:") + mime + QStringLiteral(";base64,") + QString::fromLatin1(bytes.toBase64());
}

QString assetUrl(const QString& relativePath) {
    return assetDataUri(relativePath);
}

void loadApplicationStyleSheet() {
    QByteArray qss;
    QFile res(QStringLiteral(":/assets/styles/corevex.qss"));
    if (res.open(QIODevice::ReadOnly)) qss = res.readAll();
    else {
        QFile file(assetPath(QStringLiteral("styles/corevex.qss")));
        if (file.open(QIODevice::ReadOnly)) qss = file.readAll();
    }
    if (!qss.isEmpty() && qApp) qApp->setStyleSheet(QString::fromUtf8(qss));
}

}  // namespace corevex
