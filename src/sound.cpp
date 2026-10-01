#include "sound.h"

#include <QDateTime>
#include <QFileInfo>

SoundManager::SoundManager(QObject* parent) : QObject(parent) {
    initEffect(sndIntro_, QStringLiteral("pacman_intro.wav"), 0.85f);
    initEffect(sndWaka1_, QStringLiteral("pacman_waka1.wav"), 0.6f);
    initEffect(sndWaka2_, QStringLiteral("pacman_waka2.wav"), 0.6f);
    initEffect(sndEatGhost_, QStringLiteral("pacman_eat_ghost.wav"), 0.8f);
    initEffect(sndDeath_, QStringLiteral("pacman_death.wav"), 0.85f);
    initEffect(sndFruit_, QStringLiteral("pacman_fruit.wav"), 0.8f);
}

QString SoundManager::findSoundFile(const QString& filename) const {
    const QStringList searchDirs = {
        QDir::current().filePath(QStringLiteral("sounds")),
        QCoreApplication::applicationDirPath() + QStringLiteral("/sounds"),
        QCoreApplication::applicationDirPath() + QStringLiteral("/../sounds"),
        QStringLiteral("/Users/ritamghosh/Desktop/CG/sounds")
    };

    for (const QString& dir : searchDirs) {
        QString fullPath = dir + QStringLiteral("/") + filename;
        if (QFileInfo::exists(fullPath)) {
            return QFileInfo(fullPath).absoluteFilePath();
        }
    }
    return QString();
}

void SoundManager::initEffect(QSoundEffect& effect, const QString& filename, float volume) {
    QString path = findSoundFile(filename);
    if (!path.isEmpty()) {
        effect.setSource(QUrl::fromLocalFile(path));
        effect.setVolume(volume);
    }
}

void SoundManager::playIntro() {
    if (muted_) return;
    sndIntro_.stop();
    sndIntro_.play();
}

void SoundManager::playWaka() {
    if (muted_) return;
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now - lastWakaMs_ < 100) return; // Rate limit chomp rate to match mouth rhythm
    lastWakaMs_ = now;

    if (wakaFlip_) {
        sndWaka1_.play();
    } else {
        sndWaka2_.play();
    }
    wakaFlip_ = !wakaFlip_;
}

void SoundManager::playEatGhost() {
    if (muted_) return;
    sndEatGhost_.stop();
    sndEatGhost_.play();
}

void SoundManager::playDeath() {
    if (muted_) return;
    stopAll();
    sndDeath_.play();
}

void SoundManager::playFruit() {
    if (muted_) return;
    sndFruit_.stop();
    sndFruit_.play();
}

void SoundManager::stopAll() {
    sndIntro_.stop();
    sndWaka1_.stop();
    sndWaka2_.stop();
    sndEatGhost_.stop();
    sndDeath_.stop();
    sndFruit_.stop();
}

void SoundManager::toggleMute() {
    setMuted(!muted_);
}
