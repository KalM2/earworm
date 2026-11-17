// Library — folder-based audiobook scanner and collection model.
// Follows Omacut (MIT, David Heinemeier Hansson).

#include "library.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <algorithm>

Library::Library(QObject *parent)
    : QAbstractListModel(parent), m_filter("all") {
    m_debounce.setSingleShot(true);
    m_debounce.setInterval(500);
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, [this] { m_debounce.start(); });
    connect(&m_debounce, &QTimer::timeout, this, &Library::rescan);

    loadRootFolder();
}

int Library::rowCount(const QModelIndex &) const {
    return m_filtered.size();
}

QVariant Library::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_filtered.size())
        return {};

    Book *book = m_filtered[index.row()];
    switch (role) {
    case TitleRole: return book->title();
    case AuthorRole: return book->author();
    case CoverPathRole: return book->coverPath();
    case StatusRole: return book->status();
    case ProgressRole: return book->progressPercent();
    case FolderRole: return book->folder();
    case ChapterCountRole: return book->chapterCount();
    case TotalDurationRole: return book->totalDurationMs();
    case BookObjectRole: return QVariant::fromValue(book);
    default: return {};
    }
}

QHash<int, QByteArray> Library::roleNames() const {
    return {
        {TitleRole, "title"},
        {AuthorRole, "author"},
        {CoverPathRole, "coverPath"},
        {StatusRole, "status"},
        {ProgressRole, "progress"},
        {FolderRole, "folder"},
        {ChapterCountRole, "chapterCount"},
        {TotalDurationRole, "totalDuration"},
        {BookObjectRole, "bookObject"}
    };
}

void Library::setRootFolder(const QString &folder) {
    if (m_rootFolder == folder) return;
    m_rootFolder = folder;
    saveRootFolder();
    emit rootFolderChanged();
    scan();
}

void Library::setFilter(const QString &filter) {
    if (m_filter == filter) return;
    m_filter = filter;
    emit filterChanged();
    applyFilter();
}

Book *Library::bookAt(int index) const {
    if (index >= 0 && index < m_filtered.size())
        return m_filtered[index];
    return nullptr;
}

void Library::rescan() {
    scan();
}

void Library::scan() {
    if (m_rootFolder.isEmpty()) return;

    // Remove old watches.
    auto watched = m_watcher.directories();
    if (!watched.isEmpty())
        m_watcher.removePaths(watched);

    m_watcher.addPath(m_rootFolder);

    // Find all subfolders that contain audio files.
    QStringList audioExts = {"mp3", "m4a", "m4b", "ogg", "opus", "flac", "wma", "wav", "aac"};
    QStringList audioFilters;
    for (const auto &ext : audioExts)
        audioFilters << ("*." + ext);

    QDir rootDir(m_rootFolder);
    QStringList bookFolders;

    // Each immediate subfolder with audio files is a book.
    for (const auto &entry : rootDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QString subPath = rootDir.absoluteFilePath(entry);
        QDir subDir(subPath);
        if (!subDir.entryList(audioFilters, QDir::Files).isEmpty())
            bookFolders << subPath;
    }

    // Also check if the root folder itself contains audio files (single-book mode).
    if (!rootDir.entryList(audioFilters, QDir::Files).isEmpty() && bookFolders.isEmpty())
        bookFolders << m_rootFolder;

    // Build a set of existing folders to detect new and removed books.
    QHash<QString, Book *> existing;
    for (auto *book : m_books)
        existing[book->folder()] = book;

    beginResetModel();

    QList<Book *> newBooks;
    for (const auto &folder : bookFolders) {
        m_watcher.addPath(folder);

        if (existing.contains(folder)) {
            newBooks.append(existing.take(folder));
        } else {
            auto *book = new Book(folder, this);
            book->scanFiles();
            book->loadState();
            // Save progress when it changes.
            connect(book, &Book::progressChanged, this, [book] { book->saveState(); });
            connect(book, &Book::bookmarksChanged, this, [book] { book->saveState(); });
            newBooks.append(book);
        }
    }

    // Delete removed books.
    for (auto *book : existing)
        book->deleteLater();

    // Sort by title.
    std::sort(newBooks.begin(), newBooks.end(), [](const Book *a, const Book *b) {
        return a->title().toLower() < b->title().toLower();
    });

    m_books = newBooks;
    applyFilter();

    endResetModel();
    emit countChanged();
}

void Library::applyFilter() {
    beginResetModel();

    if (m_filter == "all") {
        m_filtered = m_books;
    } else {
        m_filtered.clear();
        for (auto *book : m_books) {
            if (book->status() == m_filter)
                m_filtered.append(book);
        }
    }

    endResetModel();
    emit countChanged();
}

void Library::saveAll() const {
    for (const auto *book : m_books)
        book->saveState();
}

void Library::loadRootFolder() {
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QFile file(configDir + "/config.json");
    if (!file.open(QIODevice::ReadOnly)) return;

    auto doc = QJsonDocument::fromJson(file.readAll());
    QString folder = doc.object().value("rootFolder").toString();
    if (!folder.isEmpty() && QDir(folder).exists()) {
        m_rootFolder = folder;
        emit const_cast<Library *>(this)->rootFolderChanged();
        const_cast<Library *>(this)->scan();
    }
}

void Library::saveRootFolder() const {
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(configDir);

    QJsonObject obj;
    obj["rootFolder"] = m_rootFolder;

    QString path = configDir + "/config.json";
    QString tmpPath = path + ".tmp";
    QFile file(tmpPath);
    if (!file.open(QIODevice::WriteOnly)) return;
    file.write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    file.close();
    QFile::remove(path);
    QFile::rename(tmpPath, path);
}
