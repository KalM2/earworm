// SleepTimer — ticks down and pauses playback when time runs out.
// Follows Omacut (MIT, David Heinemeier Hansson).

#include "sleeptimer.h"
#include "player.h"

SleepTimer::SleepTimer(Player *player, QObject *parent)
    : QObject(parent), m_player(player) {
    m_timer.setInterval(1000);
    connect(&m_timer, &QTimer::timeout, this, [this] {
        if (m_remaining <= 0) {
            stop();
            if (m_player) m_player->pause();
            emit expired();
            return;
        }
        --m_remaining;
        emit changed();
    });
}

void SleepTimer::start(int minutes) {
    m_total = minutes * 60;
    m_remaining = m_total;
    m_active = true;
    m_timer.start();
    emit changed();
}

void SleepTimer::stop() {
    m_active = false;
    m_remaining = 0;
    m_timer.stop();
    emit changed();
}

void SleepTimer::addMinutes(int minutes) {
    if (!m_active) {
        start(minutes);
        return;
    }
    m_remaining += minutes * 60;
    m_total += minutes * 60;
    emit changed();
}
