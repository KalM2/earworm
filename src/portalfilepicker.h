// PortalFilePicker — xdg-desktop-portal folder chooser over D-Bus.

#pragma once

#include <QDBusObjectPath>
#include <QObject>
#include <QString>

class PortalFilePicker : public QObject {
    Q_OBJECT

public:
    explicit PortalFilePicker(QObject *parent = nullptr);

    Q_INVOKABLE void openFolder(const QString &title);

signals:
    void folderSelected(const QString &path);
    void cancelled();
    void failed(const QString &message);

private slots:
    void onResponse(uint response, const QVariantMap &results);

private:
    QDBusObjectPath m_requestPath;
};
