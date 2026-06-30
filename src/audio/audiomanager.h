#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QObject>
#include <QHash>
#include <QUrl>

class QMediaPlayer;
class QAudioOutput;
class QSoundEffect;

class AudioManager : public QObject {
    Q_OBJECT

    Q_PROPERTY(int bgmVolume READ bgmVolume WRITE setBgmVolume NOTIFY audioChanged)
    Q_PROPERTY(int sfxVolume READ sfxVolume WRITE setSfxVolume NOTIFY audioChanged)
    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY audioChanged)
    Q_PROPERTY(bool bgmPlaying READ bgmPlaying NOTIFY audioChanged)
    Q_PROPERTY(QString bgmTrack READ bgmTrack WRITE setBgmTrack NOTIFY audioChanged)

public:
    explicit AudioManager(QObject *parent = nullptr);
    ~AudioManager();

    int bgmVolume() const { return bgmVolume_; }
    int sfxVolume() const { return sfxVolume_; }
    bool muted() const { return muted_; }
    bool bgmPlaying() const { return bgmPlaying_; }
    QString bgmTrack() const { return bgmTrack_; }

    void setBgmVolume(int volume);
    void setSfxVolume(int volume);
    void setMuted(bool muted);
    void setBgmTrack(const QString &track);

    Q_INVOKABLE void playSfx(const QString &name);
    Q_INVOKABLE void startBgm();
    Q_INVOKABLE void stopBgm();
    Q_INVOKABLE void applyReducedMotion(bool enabled);

signals:
    void audioChanged();

private:
    void loadPreferences();
    void savePreferences() const;
    void applyVolumes();
    void applyBgmSource();
    QSoundEffect *sfx(const QString &name);

    QMediaPlayer *bgmPlayer_ = nullptr;
    QAudioOutput *bgmOutput_ = nullptr;
    QHash<QString, QSoundEffect *> sfxCache_;

    int bgmVolume_ = 60;
    int sfxVolume_ = 80;
    bool muted_ = false;
    bool bgmPlaying_ = false;
    QString bgmTrack_ = QStringLiteral("town");
};

#endif // AUDIOMANAGER_H
