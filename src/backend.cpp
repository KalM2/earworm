// Backend — orchestrates library, player, timer, and bookmarks.
// Follows Omacut (MIT, David Heinemeier Hansson).

#include "backend.h"

Backend::Backend(QObject *parent)
    : QObject(parent), m_sleepTimer(&m_player) {

    connect(&m_picker, &PortalFilePicker::folderSelected, this, [this](const QString &path) {
        m_library.setRootFolder(path);
        m_error.clear();
        emit errorChanged();
    });
    connect(&m_picker, &PortalFilePicker::failed, this, [this](const QString &message) {
        m_error = message;
        emit errorChanged();
    });

    connect(&m_sleepTimer, &SleepTimer::expired, this, [this] {
        // Sleep timer ran out — playback is already paused by the timer.
    });
}

void Backend::setView(const QString &view) {
    if (m_view == view) return;
    m_view = view;
    emit viewChanged();
}

void Backend::chooseFolder() {
    m_picker.openFolder(tr("Choose Audiobook Folder"));
}

void Backend::openBook(int index) {
    Book *book = m_library.bookAt(index);
    if (!book) return;

    m_currentBook = book;
    m_bookmarkModel.setBook(book);
    m_player.loadBook(book);
    m_view = "player";
    emit currentBookChanged();
    emit viewChanged();
}

void Backend::backToLibrary() {
    m_player.pause();
    m_view = "library";
    emit viewChanged();
}

void Backend::addBookmark(const QString &label) {
    if (m_currentBook) {
        m_currentBook->setProgress(m_player.currentChapter(), m_player.position());
        m_currentBook->addBookmark(label);
    }
}

void Backend::goToBookmark(int index) {
    if (!m_currentBook) return;
    const auto &bookmarks = m_currentBook->bookmarks();
    if (index < 0 || index >= bookmarks.size()) return;

    const auto &bm = bookmarks[index];
    m_player.goToChapter(bm.chapterIndex);
    m_player.seekTo(bm.positionMs);
    m_view = "player";
    emit viewChanged();
}

void Backend::removeBookmark(int index) {
    if (m_currentBook)
        m_currentBook->removeBookmark(index);
}

void Backend::markBookFinished() {
    if (m_currentBook) {
        m_currentBook->markFinished();
        m_player.pause();
    }
}

void Backend::markBookNew() {
    if (m_currentBook) {
        m_currentBook->markNew();
    }
}

QString Backend::formatDuration(qint64 ms) const {
    if (ms < 0) ms = 0;
    int totalSeconds = static_cast<int>(ms / 1000);
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;

    if (hours > 0)
        return QString("%1:%2:%3")
            .arg(hours)
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'));
    else
        return QString("%1:%2")
            .arg(minutes)
            .arg(seconds, 2, 10, QChar('0'));
}

QString Backend::formatSpeed(qreal speed) const {
    return QString::number(speed, 'f', 1) + "x";
}
