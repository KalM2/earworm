// Book — audiobook data model and persistence.
// Follows Omacut (MIT, David Heinemeier Hansson).

#include "book.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QProcess>
#include <QStandardPaths>
#include <algorithm>

Book::Book(const QString &folder, QObject *parent)
    : QObject(parent), m_folder(folder) {
    // Derive default title from the folder name.
    m_title = QFileInfo(folder).fileName();
}

QStringList Book::audioExtensions() {
    return {"mp3", "m4a", "m4b", "ogg", "opus", "flac", "wma", "wav", "aac"};
}

void Book::scanFiles() {
    m_chapters.clear();

    QDir dir(m_folder);
    QStringList filters;
    for (const auto &ext : audioExtensions())
        filters << ("*." + ext);

    QStringList files;
    QDirIterator it(m_folder, filters, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
        files << it.next();

    // Sort naturally so "Chapter 2" comes before "Chapter 10".
    std::sort(files.begin(), files.end(), [](const QString &a, const QString &b) {
        return QFileInfo(a).fileName().toLower() < QFileInfo(b).fileName().toLower();
    });

    for (const auto &file : files)
        probeFile(file);

    // Try to find a cover image.
    QStringList imageFilters = {"cover.*", "folder.*", "front.*", "*.jpg", "*.jpeg", "*.png"};
    for (const auto &filter : imageFilters) {
        QStringList covers = dir.entryList({filter}, QDir::Files);
        if (!covers.isEmpty()) {
            m_coverPath = dir.absoluteFilePath(covers.first());
            break;
        }
    }

    emit metaChanged();
}

void Book::probeFile(const QString &path) {
    // Use ffprobe to get duration and metadata.
    QProcess probe;
    probe.start("ffprobe", {
        "-v", "quiet",
        "-print_format", "json",
        "-show_format",
        "-show_entries", "format=duration",
        path
    });
    probe.waitForFinished(5000);

    Chapter ch;
    ch.filePath = path;
    ch.title = QFileInfo(path).completeBaseName();

    if (probe.exitCode() == 0) {
        auto doc = QJsonDocument::fromJson(probe.readAllStandardOutput());
        auto format = doc.object().value("format").toObject();
        double duration = format.value("duration").toString().toDouble();
        ch.durationMs = static_cast<qint64>(duration * 1000.0);

        // Extract title/author from tags if available.
        auto tags = format.value("tags").toObject();
        if (tags.contains("title") && !tags.value("title").toString().isEmpty())
            ch.title = tags.value("title").toString();
        if (tags.contains("album") && m_title == QFileInfo(m_folder).fileName())
            m_title = tags.value("album").toString();
        if (tags.contains("artist") && m_author.isEmpty())
            m_author = tags.value("artist").toString();
    }

    m_chapters.append(ch);
}

qint64 Book::totalDurationMs() const {
    qint64 total = 0;
    for (const auto &ch : m_chapters)
        total += ch.durationMs;
    return total;
}

qint64 Book::elapsedMs() const {
    qint64 elapsed = 0;
    for (int i = 0; i < m_currentChapter && i < m_chapters.size(); ++i)
        elapsed += m_chapters[i].durationMs;
    elapsed += m_currentPositionMs;
    return elapsed;
}

double Book::progressPercent() const {
    qint64 total = totalDurationMs();
    if (total <= 0) return 0.0;
    return static_cast<double>(elapsedMs()) / total * 100.0;
}

void Book::setTitle(const QString &title) {
    if (m_title != title) {
        m_title = title;
        emit metaChanged();
    }
}

void Book::setAuthor(const QString &author) {
    if (m_author != author) {
        m_author = author;
        emit metaChanged();
    }
}

void Book::setCoverPath(const QString &path) {
    if (m_coverPath != path) {
        m_coverPath = path;
        emit metaChanged();
    }
}

void Book::setProgress(int chapterIndex, qint64 positionMs) {
    if (chapterIndex < 0 || chapterIndex >= m_chapters.size())
        return;
    m_currentChapter = chapterIndex;
    m_currentPositionMs = positionMs;
    if (m_status == "new")
        m_status = "started";
    emit progressChanged();
}

void Book::markFinished() {
    m_status = "finished";
    emit progressChanged();
}

void Book::markNew() {
    m_status = "new";
    m_currentChapter = 0;
    m_currentPositionMs = 0;
    emit progressChanged();
}

void Book::addBookmark(const QString &label) {
    Bookmark bm;
    bm.chapterIndex = m_currentChapter;
    bm.positionMs = m_currentPositionMs;
    bm.label = label;
    bm.createdAt = QDateTime::currentMSecsSinceEpoch();
    m_bookmarks.append(bm);
    emit bookmarksChanged();
}

void Book::removeBookmark(int index) {
    if (index >= 0 && index < m_bookmarks.size()) {
        m_bookmarks.removeAt(index);
        emit bookmarksChanged();
    }
}

QString Book::currentFilePath() const {
    if (m_currentChapter >= 0 && m_currentChapter < m_chapters.size())
        return m_chapters[m_currentChapter].filePath;
    return {};
}

QString Book::chapterFilePath(int index) const {
    if (index >= 0 && index < m_chapters.size())
        return m_chapters[index].filePath;
    return {};
}

QString Book::chapterTitle(int index) const {
    if (index >= 0 && index < m_chapters.size())
        return m_chapters[index].title;
    return {};
}

qint64 Book::chapterDurationMs(int index) const {
    if (index >= 0 && index < m_chapters.size())
        return m_chapters[index].durationMs;
    return 0;
}

QJsonObject Book::toJson() const {
    QJsonObject obj;
    obj["folder"] = m_folder;
    obj["title"] = m_title;
    obj["author"] = m_author;
    obj["coverPath"] = m_coverPath;
    obj["status"] = m_status;
    obj["currentChapter"] = m_currentChapter;
    obj["currentPositionMs"] = m_currentPositionMs;

    QJsonArray bookmarkArray;
    for (const auto &bm : m_bookmarks) {
        QJsonObject bmObj;
        bmObj["chapterIndex"] = bm.chapterIndex;
        bmObj["positionMs"] = bm.positionMs;
        bmObj["label"] = bm.label;
        bmObj["createdAt"] = bm.createdAt;
        bookmarkArray.append(bmObj);
    }
    obj["bookmarks"] = bookmarkArray;

    return obj;
}

void Book::fromJson(const QJsonObject &obj) {
    m_title = obj.value("title").toString(m_title);
    m_author = obj.value("author").toString();
    m_coverPath = obj.value("coverPath").toString();
    m_status = obj.value("status").toString("new");
    m_currentChapter = obj.value("currentChapter").toInt(0);
    m_currentPositionMs = static_cast<qint64>(obj.value("currentPositionMs").toDouble(0));

    m_bookmarks.clear();
    auto bookmarkArray = obj.value("bookmarks").toArray();
    for (const auto &val : bookmarkArray) {
        auto bmObj = val.toObject();
        Bookmark bm;
        bm.chapterIndex = bmObj.value("chapterIndex").toInt(0);
        bm.positionMs = static_cast<qint64>(bmObj.value("positionMs").toDouble(0));
        bm.label = bmObj.value("label").toString();
        bm.createdAt = static_cast<qint64>(bmObj.value("createdAt").toDouble(0));
        m_bookmarks.append(bm);
    }

    emit metaChanged();
    emit progressChanged();
    emit bookmarksChanged();
}

void Book::loadState() {
    QString stateDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/books";
    // Use a hash of the folder path as the filename.
    QString hash = QString::number(qHash(m_folder), 16);
    QString path = stateDir + "/" + hash + ".json";

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return;

    auto doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isObject())
        fromJson(doc.object());
}

void Book::saveState() const {
    QString stateDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/books";
    QDir().mkpath(stateDir);

    QString hash = QString::number(qHash(m_folder), 16);
    QString path = stateDir + "/" + hash + ".json";
    QString tmpPath = path + ".tmp";

    QFile file(tmpPath);
    if (!file.open(QIODevice::WriteOnly))
        return;

    file.write(QJsonDocument(toJson()).toJson(QJsonDocument::Compact));
    file.close();

    // Atomic write: rename over the target.
    QFile::remove(path);
    QFile::rename(tmpPath, path);
}
