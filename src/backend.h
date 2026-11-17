// Backend — wires the library, player, sleep timer, and bookmarks for QML.

#pragma once

#include <QObject>

#include "bookmarkmodel.h"
#include "library.h"
#include "player.h"
#include "portalfilepicker.h"
#include "sleeptimer.h"

class Backend : public QObject {
    Q_OBJECT
    Q_PROPERTY(Library *library READ library CONSTANT)
    Q_PROPERTY(Player *player READ player CONSTANT)
    Q_PROPERTY(SleepTimer *sleepTimer READ sleepTimer CONSTANT)
    Q_PROPERTY(BookmarkModel *bookmarkModel READ bookmarkModel CONSTANT)
    Q_PROPERTY(Book *currentBook READ currentBook NOTIFY currentBookChanged)
    Q_PROPERTY(QString view READ view WRITE setView NOTIFY viewChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)

public:
    explicit Backend(QObject *parent = nullptr);

    Library *library() { return &m_library; }
    Player *player() { return &m_player; }
    SleepTimer *sleepTimer() { return &m_sleepTimer; }
    BookmarkModel *bookmarkModel() { return &m_bookmarkModel; }
    Book *currentBook() const { return m_currentBook; }
    QString view() const { return m_view; }
    QString error() const { return m_error; }

    Q_INVOKABLE void setView(const QString &view);

    Q_INVOKABLE void chooseFolder();
    Q_INVOKABLE void openBook(int index);
    Q_INVOKABLE void backToLibrary();
    Q_INVOKABLE void addBookmark(const QString &label);
    Q_INVOKABLE void goToBookmark(int index);
    Q_INVOKABLE void removeBookmark(int index);
    Q_INVOKABLE void markBookFinished();
    Q_INVOKABLE void markBookNew();

    // Formatting helpers for QML.
    Q_INVOKABLE QString formatDuration(qint64 ms) const;
    Q_INVOKABLE QString formatSpeed(qreal speed) const;

signals:
    void currentBookChanged();
    void viewChanged();
    void errorChanged();

private:
    Library m_library;
    Player m_player;
    SleepTimer m_sleepTimer;
    BookmarkModel m_bookmarkModel;
    PortalFilePicker m_picker;

    Book *m_currentBook = nullptr;
    QString m_view = "library"; // "library", "player", "chapters", "bookmarks"
    QString m_error;
};
