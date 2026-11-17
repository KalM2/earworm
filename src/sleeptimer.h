// SleepTimer — counts down and pauses playback, like Smart AudioBook Player's sleep timer.

#pragma once

#include <QObject>
#include <QTimer>

class Player;

class SleepTimer : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool active READ active NOTIFY changed)
    Q_PROPERTY(int remainingSeconds READ remainingSeconds NOTIFY changed)
    Q_PROPERTY(int totalSeconds READ totalSeconds NOTIFY changed)

public:
    explicit SleepTimer(Player *player, QObject *parent = nullptr);

    bool active() const { return m_active; }
    int remainingSeconds() const { return m_remaining; }
    int totalSeconds() const { return m_total; }

    Q_INVOKABLE void start(int minutes);
    Q_INVOKABLE void stop();
    Q_INVOKABLE void addMinutes(int minutes);

signals:
    void changed();
    void expired();

private:
    Player *m_player;
    QTimer m_timer;
    bool m_active = false;
    int m_remaining = 0;
    int m_total = 0;
};
