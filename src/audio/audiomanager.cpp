#include "audiomanager.h"

#include <QMediaPlayer>
#include <QAudioOutput>
#include <QSoundEffect>
#include <QSettings>
#include <QUrl>
#include <QCoreApplication>

namespace {
constexpr int kMinVolume = 0;
constexpr int kMaxVolume = 100;

int clampVolume(int v) {
    return qBound(kMinVolume, v, kMaxVolume);
}

// Convert a 0..100 user volume to a perceptual-ish linear amplitude.
// QAudioSink expects 0..1 linear; we use a square-law curve so small slider
// values remain audible (matching how the SettingsPage slider feels).
qreal toLinearGain(int userVolume) {
    const qreal t = clampVolume(userVolume) / qreal(kMaxVolume);
    return t * t;
}
} // namespace

AudioManager::AudioManager(QObject *parent) : QObject(parent) {
    loadPreferences();

    bgmPlayer_ = new QMediaPlayer(this);
    bgmPlayer_->setLoops(QMediaPlayer::Infinite);

    bgmOutput_ = new QAudioOutput(this);
    bgmOutput_->setVolume(toLinearGain(bgmVolume_));

    bgmPlayer_->setAudioOutput(bgmOutput_);

    applyBgmSource();
    applyVolumes();
}

AudioManager::~AudioManager() = default;

void AudioManager::loadPreferences() {
    QSettings settings;
    bgmVolume_ = clampVolume(settings.value(QStringLiteral("audio/bgmVolume"), 60).toInt());
    sfxVolume_ = clampVolume(settings.value(QStringLiteral("audio/sfxVolume"), 80).toInt());
    muted_ = settings.value(QStringLiteral("audio/muted"), false).toBool();
    bgmTrack_ = settings.value(QStringLiteral("audio/bgmTrack"), QStringLiteral("town")).toString();
    if (bgmTrack_ != QStringLiteral("town") && bgmTrack_ != QStringLiteral("forest")) {
        bgmTrack_ = QStringLiteral("town");
    }
}

void AudioManager::savePreferences() const {
    QSettings settings;
    settings.setValue(QStringLiteral("audio/bgmVolume"), bgmVolume_);
    settings.setValue(QStringLiteral("audio/sfxVolume"), sfxVolume_);
    settings.setValue(QStringLiteral("audio/muted"), muted_);
    settings.setValue(QStringLiteral("audio/bgmTrack"), bgmTrack_);
}

void AudioManager::applyBgmSource() {
    if (!bgmPlayer_) {
        return;
    }
    QString path;
    if (bgmTrack_ == QStringLiteral("forest")) {
        path = QStringLiteral("qrc:/CodeFarm/resources/audio/bgm_forest.ogg");
    } else {
        path = QStringLiteral("qrc:/CodeFarm/resources/audio/bgm_town.ogg");
    }
    bgmPlayer_->setSource(QUrl(path));
}

void AudioManager::applyVolumes() {
    if (bgmOutput_) {
        bgmOutput_->setVolume(muted_ ? 0.0 : toLinearGain(bgmVolume_));
    }
    const qreal sfxGain = muted_ ? 0.0 : toLinearGain(sfxVolume_);
    for (QSoundEffect *sfx : sfxCache_) {
        if (sfx) {
            sfx->setVolume(sfxGain);
        }
    }
}

void AudioManager::setBgmVolume(int volume) {
    const int next = clampVolume(volume);
    if (bgmVolume_ == next) {
        return;
    }
    bgmVolume_ = next;
    applyVolumes();
    savePreferences();
    emit audioChanged();
}

void AudioManager::setSfxVolume(int volume) {
    const int next = clampVolume(volume);
    if (sfxVolume_ == next) {
        return;
    }
    sfxVolume_ = next;
    applyVolumes();
    savePreferences();
    emit audioChanged();
}

void AudioManager::setMuted(bool muted) {
    if (muted_ == muted) {
        return;
    }
    muted_ = muted;
    if (muted_) {
        if (bgmPlayer_ && bgmPlayer_->playbackState() == QMediaPlayer::PlayingState) {
            bgmPlayer_->pause();
        }
    } else if (bgmPlaying_) {
        if (bgmPlayer_ && bgmPlayer_->playbackState() != QMediaPlayer::PlayingState) {
            bgmPlayer_->play();
        }
    }
    applyVolumes();
    savePreferences();
    emit audioChanged();
}

void AudioManager::setBgmTrack(const QString &track) {
    if (bgmTrack_ == track) {
        return;
    }
    bgmTrack_ = track;
    const bool wasPlaying = bgmPlaying_ && !muted_;
    if (wasPlaying && bgmPlayer_) {
        bgmPlayer_->stop();
    }
    applyBgmSource();
    if (wasPlaying && bgmPlayer_) {
        bgmPlayer_->play();
    }
    savePreferences();
    emit audioChanged();
}

QSoundEffect *AudioManager::sfx(const QString &name) {
    const auto it = sfxCache_.constFind(name);
    if (it != sfxCache_.cend() && it.value()) {
        return it.value();
    }

    QString path;
    if (name == QStringLiteral("click")) {
        path = QStringLiteral("qrc:/CodeFarm/resources/audio/sfx_click.wav");
    } else if (name == QStringLiteral("clear")) {
        path = QStringLiteral("qrc:/CodeFarm/resources/audio/sfx_clear.wav");
    } else if (name == QStringLiteral("fail")) {
        path = QStringLiteral("qrc:/CodeFarm/resources/audio/sfx_fail.wav");
    } else {
        return nullptr;
    }

    auto *effect = new QSoundEffect(this);
    effect->setSource(QUrl(path));
    effect->setVolume(muted_ ? 0.0 : toLinearGain(sfxVolume_));
    sfxCache_.insert(name, effect);
    return effect;
}

void AudioManager::playSfx(const QString &name) {
    if (muted_) {
        return;
    }
    QSoundEffect *effect = sfx(name);
    if (effect) {
        effect->play();
    }
}

void AudioManager::startBgm() {
    if (muted_ || !bgmPlayer_) {
        return;
    }
    if (bgmPlayer_->playbackState() != QMediaPlayer::PlayingState) {
        bgmPlayer_->play();
    }
    if (!bgmPlaying_) {
        bgmPlaying_ = true;
        emit audioChanged();
    }
}

void AudioManager::stopBgm() {
    if (bgmPlayer_ && bgmPlayer_->playbackState() == QMediaPlayer::PlayingState) {
        bgmPlayer_->stop();
    }
    if (bgmPlaying_) {
        bgmPlaying_ = false;
        emit audioChanged();
    }
}

void AudioManager::applyReducedMotion(bool enabled) {
    Q_UNUSED(enabled)
    // Reserved for future visual-side coupling; audio has no motion to reduce.
}
