// BookmarkModel — list model adapter for a Book's bookmarks.
// Follows Omacut (MIT, David Heinemeier Hansson).

#include "bookmarkmodel.h"

BookmarkModel::BookmarkModel(QObject *parent)
    : QAbstractListModel(parent) {}

int BookmarkModel::rowCount(const QModelIndex &) const {
    return m_book ? m_book->bookmarks().size() : 0;
}

QVariant BookmarkModel::data(const QModelIndex &index, int role) const {
    if (!m_book || !index.isValid()) return {};
    const auto &bookmarks = m_book->bookmarks();
    if (index.row() < 0 || index.row() >= bookmarks.size()) return {};

    const auto &bm = bookmarks[index.row()];
    switch (role) {
    case LabelRole: return bm.label;
    case ChapterRole: return bm.chapterIndex;
    case PositionRole: return bm.positionMs;
    case CreatedAtRole: return bm.createdAt;
    default: return {};
    }
}

QHash<int, QByteArray> BookmarkModel::roleNames() const {
    return {
        {LabelRole, "label"},
        {ChapterRole, "chapter"},
        {PositionRole, "position"},
        {CreatedAtRole, "createdAt"}
    };
}

int BookmarkModel::count() const {
    return m_book ? m_book->bookmarks().size() : 0;
}

void BookmarkModel::setBook(Book *book) {
    if (m_book) {
        disconnect(m_book, &Book::bookmarksChanged, this, &BookmarkModel::refresh);
    }

    m_book = book;

    if (m_book) {
        connect(m_book, &Book::bookmarksChanged, this, &BookmarkModel::refresh);
    }

    refresh();
}

void BookmarkModel::removeAt(int index) {
    if (m_book) {
        m_book->removeBookmark(index);
    }
}

void BookmarkModel::refresh() {
    beginResetModel();
    endResetModel();
    emit countChanged();
}
