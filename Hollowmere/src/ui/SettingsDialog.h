#pragma once

#include <QDialog>
#include <QSlider>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>

namespace hm {

struct GameSettings {
    int qualityLevel = 2; // 0=Low, 1=Med, 2=High
    float resolutionScale = 1.0f;
    bool vsync = true;
    float masterVolume = 0.85f;
    float sfxVolume = 0.90f;
    float musicVolume = 0.75f;
    float mouseSensitivity = 0.12f;
    bool invertY = false;
    float fov = 75.0f;
    bool subtitles = true;
    bool lightsOnMode = false;

    void saveToFile(const QString& path = "settings.json") const;
    void loadFromFile(const QString& path = "settings.json");
};

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(GameSettings& settings, QWidget* parent = nullptr);

signals:
    void settingsApplied();

private:
    GameSettings& m_settings;

    QComboBox* m_qualityCombo = nullptr;
    QCheckBox* m_vsyncCheck = nullptr;
    QSlider* m_masterVolSlider = nullptr;
    QSlider* m_sfxVolSlider = nullptr;
    QSlider* m_musicVolSlider = nullptr;
    QSlider* m_mouseSensSlider = nullptr;
    QSlider* m_fovSlider = nullptr;
    QCheckBox* m_subtitlesCheck = nullptr;
    QCheckBox* m_lightsOnCheck = nullptr;

    void setupUi();
    void applyFromUi();
};

} // namespace hm
