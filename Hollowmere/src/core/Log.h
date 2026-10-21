#pragma once

#include <QDebug>

#define HM_LOG_INFO(fmt, ...) qInfo("[HM:INFO] " fmt, ##__VA_ARGS__)
#define HM_LOG_WARN(fmt, ...) qWarning("[HM:WARN] " fmt, ##__VA_ARGS__)
#define HM_LOG_ERROR(fmt, ...) qCritical("[HM:ERROR] " fmt, ##__VA_ARGS__)
