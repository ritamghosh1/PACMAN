#pragma once

#include <QCoreApplication>
#include <QDir>
#include <QObject>
#include <QSoundEffect>
#include <QString>
#include <QUrl>

class SoundManager : public QObject {
    Q_OBJECT
public:
    explicit SoundManager(QObject* parent = nullptr);

    void playIntro();
    void playWaka();
    void playEatGhost();
    void playDeath();
    void playFruit();
    void playIntermission();
    void stopAll();

    void toggleMute();
    bool isMuted() const { return muted_; }
    void setMuted(bool m) { muted_ = m; if (muted_) stopAll(); }

private:
    void initEffect(QSoundEffect& effect, const QString& filename, float volume = 0.8f);
    QString findSoundFile(const QString& filename) const;

    QSoundEffect sndIntro_;
    QSoundEffect sndWaka1_;
    QSoundEffect sndWaka2_;
    QSoundEffect sndEatGhost_;
    QSoundEffect sndDeath_;
    QSoundEffect sndFruit_;
    QSoundEffect sndIntermission_;

    bool muted_ = false;
    bool wakaFlip_ = false;
    qint64 lastWakaMs_ = 0;
};
