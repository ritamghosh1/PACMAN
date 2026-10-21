#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

namespace hm {

void GameSettings::saveToFile(const QString& path) const {
    QJsonObject obj;
    obj["qualityLevel"] = qualityLevel;
    obj["resolutionScale"] = resolutionScale;
    obj["vsync"] = vsync;
    obj["masterVolume"] = masterVolume;
    obj["sfxVolume"] = sfxVolume;
    obj["musicVolume"] = musicVolume;
    obj["voiceVolume"] = voiceVolume;
    obj["mouseSensitivity"] = mouseSensitivity;
    obj["invertY"] = invertY;
    obj["fov"] = fov;
    obj["subtitles"] = subtitles;
    obj["lightsOnMode"] = lightsOnMode;

    QFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(obj).toJson());
    }
}

void GameSettings::loadFromFile(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return;

    QJsonObject obj = QJsonDocument::fromJson(file.readAll()).object();
    if (obj.contains("qualityLevel")) qualityLevel = obj["qualityLevel"].toInt();
    if (obj.contains("vsync")) vsync = obj["vsync"].toBool();
    if (obj.contains("masterVolume")) masterVolume = static_cast<float>(obj["masterVolume"].toDouble());
    if (obj.contains("sfxVolume")) sfxVolume = static_cast<float>(obj["sfxVolume"].toDouble());
    if (obj.contains("musicVolume")) musicVolume = static_cast<float>(obj["musicVolume"].toDouble());
    if (obj.contains("mouseSensitivity")) mouseSensitivity = static_cast<float>(obj["mouseSensitivity"].toDouble());
    if (obj.contains("fov")) fov = static_cast<float>(obj["fov"].toDouble());
    if (obj.contains("subtitles")) subtitles = obj["subtitles"].toBool();
    if (obj.contains("lightsOnMode")) lightsOnMode = obj["lightsOnMode"].toBool();
}

SettingsDialog::SettingsDialog(GameSettings& settings, QWidget* parent)
    : QDialog(parent), m_settings(settings) {
    setWindowTitle("Hollowmere — Settings");
    setModal(true);
    resize(480, 520);
    setStyleSheet("QDialog { background-color: #121218; color: #E0E0E0; font-family: 'Helvetica Neue', Arial; }"
                  "QLabel { color: #CCCCCC; font-size: 13px; }"
                  "QComboBox, QSlider { background-color: #1A1A24; color: #FFFFFF; }"
                  "QPushButton { background-color: #242436; color: #E0E0E0; border: 1px solid #404060; padding: 8px 16px; border-radius: 4px; }"
                  "QPushButton:hover { background-color: #383854; border-color: #707090; }");

    setupUi();
}

void SettingsDialog::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel("CONFIG & AUDIO/VIDEO SETTINGS", this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #E5C158; letter-spacing: 2px;");
    mainLayout->addWidget(titleLabel);

    auto* form = new QFormLayout();
    form->setSpacing(12);

    // Graphics Quality
    m_qualityCombo = new QComboBox(this);
    m_qualityCombo->addItem("Low (Integrated GPU)", 0);
    m_qualityCombo->addItem("Medium (Standard 1080p)", 1);
    m_qualityCombo->addItem("High (Full PCF + HDR + PostFx)", 2);
    m_qualityCombo->setCurrentIndex(m_settings.qualityLevel);
    form->addRow("Graphics Quality:", m_qualityCombo);

    // VSync
    m_vsyncCheck = new QCheckBox("Enable VSync (Lock 60 FPS)", this);
    m_vsyncCheck->setChecked(m_settings.vsync);
    form->addRow("Display:", m_vsyncCheck);

    // FOV Slider
    m_fovSlider = new QSlider(Qt::Horizontal, this);
    m_fovSlider->setRange(60, 105);
    m_fovSlider->setValue(static_cast<int>(m_settings.fov));
    form->addRow("Field of View (FOV):", m_fovSlider);

    // Master Volume
    m_masterVolSlider = new QSlider(Qt::Horizontal, this);
    m_masterVolSlider->setRange(0, 100);
    m_masterVolSlider->setValue(static_cast<int>(m_settings.masterVolume * 100.0f));
    form->addRow("Master Volume:", m_masterVolSlider);

    // SFX Volume
    m_sfxVolSlider = new QSlider(Qt::Horizontal, this);
    m_sfxVolSlider->setRange(0, 100);
    m_sfxVolSlider->setValue(static_cast<int>(m_settings.sfxVolume * 100.0f));
    form->addRow("SFX & Binaural:", m_sfxVolSlider);

    // Music Volume
    m_musicVolSlider = new QSlider(Qt::Horizontal, this);
    m_musicVolSlider->setRange(0, 100);
    m_musicVolSlider->setValue(static_cast<int>(m_settings.musicVolume * 100.0f));
    form->addRow("Generative Score:", m_musicVolSlider);

    // Mouse Sensitivity
    m_mouseSensSlider = new QSlider(Qt::Horizontal, this);
    m_mouseSensSlider->setRange(2, 35);
    m_mouseSensSlider->setValue(static_cast<int>(m_settings.mouseSensitivity * 100.0f));
    form->addRow("Mouse Sensitivity:", m_mouseSensSlider);

    // Subtitles
    m_subtitlesCheck = new QCheckBox("Enable Story & Ambient Subtitles", this);
    m_subtitlesCheck->setChecked(m_settings.subtitles);
    form->addRow("Accessibility:", m_subtitlesCheck);

    // Lights-On Mode
    m_lightsOnCheck = new QCheckBox("Lights-On Mode (Elevated Ambient)", this);
    m_lightsOnCheck->setChecked(m_settings.lightsOnMode);
    form->addRow("Game Mode:", m_lightsOnCheck);

    mainLayout->addLayout(form);
    mainLayout->addStretch();

    // Buttons
    auto* btnLayout = new QHBoxLayout();
    auto* saveBtn = new QPushButton("Apply & Save", this);
    auto* cancelBtn = new QPushButton("Cancel", this);

    btnLayout->addWidget(saveBtn);
    btnLayout->addWidget(cancelBtn);
    mainLayout->addLayout(btnLayout);

    connect(saveBtn, &QPushButton::clicked, this, [this]() {
        applyFromUi();
        m_settings.saveToFile();
        emit settingsApplied();
        accept();
    });

    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void SettingsDialog::applyFromUi() {
    m_settings.qualityLevel = m_qualityCombo->currentIndex();
    m_settings.vsync = m_vsyncCheck->isChecked();
    m_settings.fov = static_cast<float>(m_fovSlider->value());
    m_settings.masterVolume = static_cast<float>(m_masterVolSlider->value()) / 100.0f;
    m_settings.sfxVolume = static_cast<float>(m_sfxVolSlider->value()) / 100.0f;
    m_settings.musicVolume = static_cast<float>(m_musicVolSlider->value()) / 100.0f;
    m_settings.mouseSensitivity = static_cast<float>(m_mouseSensSlider->value()) / 100.0f;
    m_settings.subtitles = m_subtitlesCheck->isChecked();
    m_settings.lightsOnMode = m_lightsOnCheck->isChecked();
}

} // namespace hm
