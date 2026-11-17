// BookmarkModel — exposes a book's bookmarks as a list model for QML.

#pragma once

#include <QAbstractListModel>
#include "book.h"

class BookmarkModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        LabelRole = Qt::UserRole + 1,
        ChapterRole,
        PositionRole,
        CreatedAtRole
    };

    explicit BookmarkModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const;

    Q_INVOKABLE void setBook(Book *book);
    Q_INVOKABLE void removeAt(int index);

signals:
    void countChanged();

private:
    void refresh();
    Book *m_book = nullptr;
};
