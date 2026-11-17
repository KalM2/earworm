// Library — scans a root folder for audiobook subfolders, manages the collection.

#pragma once

#include <QAbstractListModel>
#include <QFileSystemWatcher>
#include <QList>
#include <QObject>
#include <QTimer>

#include "book.h"

class Library : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(QString rootFolder READ rootFolder WRITE setRootFolder NOTIFY rootFolderChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)

public:
    enum Roles {
        TitleRole = Qt::UserRole + 1,
        AuthorRole,
        CoverPathRole,
        StatusRole,
        ProgressRole,
        FolderRole,
        ChapterCountRole,
        TotalDurationRole,
        BookObjectRole
    };

    explicit Library(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString rootFolder() const { return m_rootFolder; }
    void setRootFolder(const QString &folder);

    int count() const { return m_filtered.size(); }

    QString filter() const { return m_filter; }
    Q_INVOKABLE void setFilter(const QString &filter);

    Q_INVOKABLE Book *bookAt(int index) const;
    Q_INVOKABLE void rescan();
    Q_INVOKABLE void saveAll() const;

signals:
    void rootFolderChanged();
    void countChanged();
    void filterChanged();

private:
    void scan();
    void applyFilter();
    void loadRootFolder();
    void saveRootFolder() const;

    QString m_rootFolder;
    QString m_filter; // "all", "new", "started", "finished"
    QList<Book *> m_books;
    QList<Book *> m_filtered;
    QFileSystemWatcher m_watcher;
    QTimer m_debounce;
};
