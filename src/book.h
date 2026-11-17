// Book — represents a single audiobook: its metadata, chapters, progress, and bookmarks.

#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QStringList>

struct Chapter {
    QString filePath;
    QString title;
    qint64 durationMs = 0;
};

struct Bookmark {
    int chapterIndex = 0;
    qint64 positionMs = 0;
    QString label;
    qint64 createdAt = 0; // epoch ms
};

class Book : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString title READ title NOTIFY metaChanged)
    Q_PROPERTY(QString author READ author NOTIFY metaChanged)
    Q_PROPERTY(QString coverPath READ coverPath NOTIFY metaChanged)
    Q_PROPERTY(QString folder READ folder CONSTANT)
    Q_PROPERTY(QString status READ status NOTIFY progressChanged)
    Q_PROPERTY(int chapterCount READ chapterCount NOTIFY metaChanged)
    Q_PROPERTY(int currentChapter READ currentChapter NOTIFY progressChanged)
    Q_PROPERTY(qint64 currentPositionMs READ currentPositionMs NOTIFY progressChanged)
    Q_PROPERTY(qint64 totalDurationMs READ totalDurationMs NOTIFY metaChanged)
    Q_PROPERTY(qint64 elapsedMs READ elapsedMs NOTIFY progressChanged)
    Q_PROPERTY(double progressPercent READ progressPercent NOTIFY progressChanged)

public:
    explicit Book(const QString &folder, QObject *parent = nullptr);

    QString title() const { return m_title; }
    QString author() const { return m_author; }
    QString coverPath() const { return m_coverPath; }
    QString folder() const { return m_folder; }
    QString status() const { return m_status; }
    int chapterCount() const { return m_chapters.size(); }
    int currentChapter() const { return m_currentChapter; }
    qint64 currentPositionMs() const { return m_currentPositionMs; }
    qint64 totalDurationMs() const;
    qint64 elapsedMs() const;
    double progressPercent() const;

    const QList<Chapter> &chapters() const { return m_chapters; }
    const QList<Bookmark> &bookmarks() const { return m_bookmarks; }

    void setTitle(const QString &title);
    void setAuthor(const QString &author);
    void setCoverPath(const QString &path);

    void setProgress(int chapterIndex, qint64 positionMs);
    void markFinished();
    void markNew();

    void addBookmark(const QString &label);
    void removeBookmark(int index);

    void scanFiles();
    void loadState();
    void saveState() const;

    QJsonObject toJson() const;
    void fromJson(const QJsonObject &obj);

    // The file path for the current chapter.
    QString currentFilePath() const;
    // File path for a specific chapter.
    QString chapterFilePath(int index) const;
    Q_INVOKABLE QString chapterTitle(int index) const;
    Q_INVOKABLE qint64 chapterDurationMs(int index) const;

signals:
    void metaChanged();
    void progressChanged();
    void bookmarksChanged();

private:
    void probeFile(const QString &path);
    static QStringList audioExtensions();

    QString m_folder;
    QString m_title;
    QString m_author;
    QString m_coverPath;
    QString m_status = "new"; // "new", "started", "finished"

    QList<Chapter> m_chapters;
    int m_currentChapter = 0;
    qint64 m_currentPositionMs = 0;

    QList<Bookmark> m_bookmarks;
};
