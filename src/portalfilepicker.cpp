// PortalFilePicker — xdg-desktop-portal folder chooser over D-Bus.
// Based on Omacut's portal file picker (MIT, David Heinemeier Hansson).

#include "portalfilepicker.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusPendingReply>
#include <QGuiApplication>
#include <QRandomGenerator>
#include <QUrl>

PortalFilePicker::PortalFilePicker(QObject *parent) : QObject(parent) {}

void PortalFilePicker::openFolder(const QString &title) {
    auto bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        emit failed(tr("The desktop session bus is unavailable."));
        return;
    }

    // Build a unique token so the portal can send the reply to this instance.
    QString token = "earworm_" + QString::number(QRandomGenerator::global()->generate());

    // Build the sender name for the Request path.
    QString sender = bus.baseService().mid(1).replace('.', '_');
    m_requestPath = QDBusObjectPath("/org/freedesktop/portal/desktop/request/" + sender + "/" + token);

    // Listen for the portal's response before sending the request.
    if (!bus.connect(
        "org.freedesktop.portal.Desktop",
        m_requestPath.path(),
        "org.freedesktop.portal.Request",
        "Response",
        this,
        SLOT(onResponse(uint, QVariantMap))
    )) {
        m_requestPath = QDBusObjectPath();
        emit failed(tr("Could not listen for the folder chooser response."));
        return;
    }

    QVariantMap options;
    options["handle_token"] = token;
    options["directory"] = true;

    QDBusInterface portal(
        "org.freedesktop.portal.Desktop",
        "/org/freedesktop/portal/desktop",
        "org.freedesktop.portal.FileChooser",
        bus
    );
    if (!portal.isValid()) {
        bus.disconnect("org.freedesktop.portal.Desktop", m_requestPath.path(),
                       "org.freedesktop.portal.Request", "Response",
                       this, SLOT(onResponse(uint, QVariantMap)));
        m_requestPath = QDBusObjectPath();
        emit failed(tr("The desktop folder chooser is unavailable."));
        return;
    }

    // parent_window for Wayland is empty; the portal maps it via xdg-foreign.
    const QDBusMessage reply = portal.call("OpenFile", QString(), title, options);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        bus.disconnect("org.freedesktop.portal.Desktop", m_requestPath.path(),
                       "org.freedesktop.portal.Request", "Response",
                       this, SLOT(onResponse(uint, QVariantMap)));
        m_requestPath = QDBusObjectPath();
        emit failed(reply.errorMessage());
        return;
    }

    if (reply.arguments().isEmpty()) {
        bus.disconnect("org.freedesktop.portal.Desktop", m_requestPath.path(),
                       "org.freedesktop.portal.Request", "Response",
                       this, SLOT(onResponse(uint, QVariantMap)));
        m_requestPath = QDBusObjectPath();
        emit failed(tr("The folder chooser returned no request handle."));
        return;
    }

    const auto handle = qvariant_cast<QDBusObjectPath>(reply.arguments().first());
    if (handle.path().isEmpty()) {
        bus.disconnect("org.freedesktop.portal.Desktop", m_requestPath.path(),
                       "org.freedesktop.portal.Request", "Response",
                       this, SLOT(onResponse(uint, QVariantMap)));
        m_requestPath = QDBusObjectPath();
        emit failed(tr("The folder chooser returned an invalid request handle."));
        return;
    }

    // Older portals may ignore handle_token and choose their own request path.
    if (handle.path() != m_requestPath.path()) {
        bus.disconnect("org.freedesktop.portal.Desktop", m_requestPath.path(),
                       "org.freedesktop.portal.Request", "Response",
                       this, SLOT(onResponse(uint, QVariantMap)));
        m_requestPath = handle;
        if (!bus.connect("org.freedesktop.portal.Desktop", m_requestPath.path(),
                         "org.freedesktop.portal.Request", "Response",
                         this, SLOT(onResponse(uint, QVariantMap)))) {
            m_requestPath = QDBusObjectPath();
            emit failed(tr("Could not listen for the folder chooser response."));
        }
    }
}

void PortalFilePicker::onResponse(uint response, const QVariantMap &results) {
    // Disconnect so stale signals don't fire.
    auto bus = QDBusConnection::sessionBus();
    if (!m_requestPath.path().isEmpty()) {
        bus.disconnect(
        "org.freedesktop.portal.Desktop",
        m_requestPath.path(),
        "org.freedesktop.portal.Request",
        "Response",
        this,
        SLOT(onResponse(uint, QVariantMap))
        );
    }
    m_requestPath = QDBusObjectPath();

    if (response == 1) {
        emit cancelled();
        return;
    }
    if (response != 0) {
        const QString message = results.value("error").toString();
        emit failed(message.isEmpty() ? tr("The folder chooser failed.") : message);
        return;
    }

    QStringList uris = results.value("uris").toStringList();
    if (!uris.isEmpty()) {
        QUrl url(uris.first());
        const QString path = url.toLocalFile();
        if (!path.isEmpty()) {
            emit folderSelected(path);
            return;
        }
        emit failed(tr("The selected folder is not a local path."));
    } else {
        emit failed(tr("The folder chooser returned no folder."));
    }
}
