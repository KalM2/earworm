// Player — Qt Multimedia-based audiobook playback engine.
// Follows Omacut (MIT, David Heinemeier Hansson).

#include "player.h"

#include <QUrl>
#include <algorithm>

Player::Player(QObject *parent)
    : QObject(parent),
      m_player(new QMediaPlayer(this)),
      m_audioOutput(new QAudioOutput(this)) {

    m_player->setAudioOutput(m_audioOutput);
    m_audioOutput->setVolume(m_volume);

    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, &Player::onMediaStatusChanged);
    connect(m_player, &QMediaPlayer::playbackStateChanged, this, [this] { emit stateChanged(); });
    connect(m_player, &QMediaPlayer::durationChanged, this, [this] { emit durationChanged(); });

    // Save progress every 3 seconds during playback.
    m_progressTimer.setInterval(3000);
    connect(&m_progressTimer, &QTimer::timeout, this, &Player::onPositionTick);
}

Player::~Player() {
    saveProgress();
}

bool Player::playing() const {
    return m_player->playbackState() == QMediaPlayer::PlayingState;
}

qint64 Player::position() const {
    return m_player->position();
}

qint64 Player::duration() const {
    return m_player->duration();
}

QString Player::currentChapterTitle() const {
    if (m_book)
        return m_book->chapterTitle(m_chapterIndex);
    return {};
}

void Player::setSpeed(qreal speed) {
    speed = std::clamp(speed, 0.25, 4.0);
    if (qFuzzyCompare(m_speed, speed)) return;
    m_speed = speed;
    m_player->setPlaybackRate(m_speed);
    emit speedChanged();
}

void Player::setVolume(qreal volume) {
    volume = std::clamp(volume, 0.0, 1.0);
    if (qFuzzyCompare(m_volume, volume)) return;
    m_volume = volume;
    m_audioOutput->setVolume(m_volume);
    emit volumeChanged();
}

void Player::loadBook(Book *book) {
    if (m_book == book) return;

    saveProgress();
    m_book = book;
    if (!m_book) {
        m_player->stop();
        m_player->setSource(QUrl());
        m_progressTimer.stop();
        emit bookChanged();
        return;
    }

    m_chapterIndex = book->currentChapter();
    loadChapter(m_chapterIndex);

    // Seek to the saved position after the media loads.
    // The seek happens in onMediaStatusChanged when LoadedMedia fires.

    emit bookChanged();
    emit chapterChanged();
}

void Player::play() {
    if (!m_book || m_book->chapterCount() == 0) return;
    m_player->play();
    m_progressTimer.start();
}

void Player::pause() {
    m_player->pause();
    m_progressTimer.stop();
    saveProgress();
}

void Player::togglePlayPause() {
    if (playing())
        pause();
    else
        play();
}

void Player::seekTo(qint64 ms) {
    m_player->setPosition(std::max(qint64(0), ms));
    emit positionChanged();
}

void Player::skipForward(int seconds) {
    qint64 target = m_player->position() + seconds * 1000;
    if (target >= m_player->duration()) {
        // Skip to next chapter if beyond current.
        nextChapter();
    } else {
        seekTo(target);
    }
}

void Player::skipBackward(int seconds) {
    qint64 target = m_player->position() - seconds * 1000;
    if (target < 0) {
        // Go to previous chapter if at the start.
        if (m_chapterIndex > 0) {
            previousChapter();
        } else {
            seekTo(0);
        }
    } else {
        seekTo(target);
    }
}

void Player::nextChapter() {
    if (!m_book) return;
    if (m_chapterIndex + 1 < m_book->chapterCount()) {
        goToChapter(m_chapterIndex + 1);
    } else {
        // Book finished.
        pause();
        m_book->markFinished();
    }
}

void Player::previousChapter() {
    if (!m_book) return;
    // If more than 3 seconds in, restart current chapter.
    if (m_player->position() > 3000) {
        seekTo(0);
    } else if (m_chapterIndex > 0) {
        goToChapter(m_chapterIndex - 1);
    }
}

void Player::goToChapter(int index) {
    if (!m_book || index < 0 || index >= m_book->chapterCount()) return;
    bool wasPlaying = playing();
    m_chapterIndex = index;
    loadChapter(index);
    if (wasPlaying) play();
    emit chapterChanged();
}

void Player::speedUp() {
    // Step up by 0.1x.
    setSpeed(qRound((m_speed + 0.1) * 10.0) / 10.0);
}

void Player::speedDown() {
    setSpeed(qRound((m_speed - 0.1) * 10.0) / 10.0);
}

void Player::loadChapter(int index) {
    if (!m_book) return;
    QString path = m_book->chapterFilePath(index);
    if (path.isEmpty()) return;
    m_player->setSource(QUrl::fromLocalFile(path));
    m_player->setPlaybackRate(m_speed);
}

void Player::saveProgress() {
    if (!m_book) return;
    m_book->setProgress(m_chapterIndex, m_player->position());
}

void Player::onMediaStatusChanged(QMediaPlayer::MediaStatus status) {
    if (status == QMediaPlayer::LoadedMedia) {
        // If we just loaded a book, seek to the saved position.
        if (m_book && !m_autoAdvancing) {
            qint64 savedPos = m_book->currentPositionMs();
            if (savedPos > 0 && m_chapterIndex == m_book->currentChapter())
                seekTo(savedPos);
        }
        m_autoAdvancing = false;
    } else if (status == QMediaPlayer::EndOfMedia) {
        // Auto-advance to next chapter.
        if (m_book && m_chapterIndex + 1 < m_book->chapterCount()) {
            m_autoAdvancing = true;
            goToChapter(m_chapterIndex + 1);
            play();
        } else if (m_book) {
            m_book->markFinished();
            emit chapterFinished();
        }
    }
}

void Player::onPositionTick() {
    emit positionChanged();
    saveProgress();
}
