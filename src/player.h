// Player — audio playback with variable speed, chapter navigation, and position tracking.

#pragma once

#include <QMediaPlayer>
#include <QAudioOutput>
#include <QObject>
#include <QTimer>

#include "book.h"

class Player : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool playing READ playing NOTIFY stateChanged)
    Q_PROPERTY(qreal speed READ speed WRITE setSpeed NOTIFY speedChanged)
    Q_PROPERTY(qint64 position READ position NOTIFY positionChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(int currentChapter READ currentChapter NOTIFY chapterChanged)
    Q_PROPERTY(QString currentChapterTitle READ currentChapterTitle NOTIFY chapterChanged)
    Q_PROPERTY(qreal volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(Book *book READ book NOTIFY bookChanged)

public:
    explicit Player(QObject *parent = nullptr);
    ~Player() override;

    bool playing() const;
    qreal speed() const { return m_speed; }
    qint64 position() const;
    qint64 duration() const;
    int currentChapter() const { return m_chapterIndex; }
    QString currentChapterTitle() const;
    qreal volume() const { return m_volume; }
    Book *book() const { return m_book; }

    void setSpeed(qreal speed);
    void setVolume(qreal volume);

    Q_INVOKABLE void loadBook(Book *book);
    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void togglePlayPause();
    Q_INVOKABLE void seekTo(qint64 ms);
    Q_INVOKABLE void skipForward(int seconds = 30);
    Q_INVOKABLE void skipBackward(int seconds = 10);
    Q_INVOKABLE void nextChapter();
    Q_INVOKABLE void previousChapter();
    Q_INVOKABLE void goToChapter(int index);
    Q_INVOKABLE void speedUp();
    Q_INVOKABLE void speedDown();

signals:
    void stateChanged();
    void speedChanged();
    void positionChanged();
    void durationChanged();
    void chapterChanged();
    void volumeChanged();
    void bookChanged();
    void chapterFinished();

private slots:
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onPositionTick();

private:
    void loadChapter(int index);
    void saveProgress();

    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;
    Book *m_book = nullptr;
    int m_chapterIndex = 0;
    qreal m_speed = 1.0;
    qreal m_volume = 1.0;
    QTimer m_progressTimer;
    bool m_autoAdvancing = false;
};
