#include "SaveGame.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace hm {

bool SaveGame::saveToFile(const std::string& filepath, const SaveData& data) {
    QJsonObject root;
    root["version"] = data.version;
    root["playTimeSec"] = data.playTimeSec;

    // Player position and look
    QJsonObject pObj;
    pObj["x"] = data.playerPos.x;
    pObj["y"] = data.playerPos.y;
    pObj["z"] = data.playerPos.z;
    pObj["yaw"] = data.playerYaw;
    pObj["pitch"] = data.playerPitch;
    root["player"] = pObj;

    // Status
    root["healthState"] = data.healthState;
    root["sanity"] = data.sanity;
    root["lanternOil"] = data.lanternOil;

    // Inventory
    QJsonArray invArr;
    for (int t : data.inventoryItemTypes) invArr.append(t);
    root["inventory"] = invArr;

    // Unlocked doors
    QJsonArray doorsArr;
    for (int d : data.unlockedDoorIds) doorsArr.append(d);
    root["unlockedDoors"] = doorsArr;

    // Solved puzzles
    QJsonArray puzzArr;
    for (int p : data.solvedPuzzleIds) puzzArr.append(p);
    root["solvedPuzzles"] = puzzArr;

    // Notes
    QJsonArray notesArr;
    for (int n : data.collectedNoteIds) notesArr.append(n);
    root["notes"] = notesArr;

    // Fragments
    QJsonArray fragArr;
    for (int f : data.discoveredFragmentIds) fragArr.append(f);
    root["fragments"] = fragArr;

    QJsonArray pickArr;
    for (int p : data.collectedPickupIds) pickArr.append(p);
    root["collectedPickups"] = pickArr;
    QJsonArray openArr;
    for (int d : data.openDoorIds) openArr.append(d);
    root["openDoors"] = openArr;
    root["keyFlags"] = data.keyFlags;
    root["sigils"] = data.sigilCount;
    root["visitedWings"] = data.visitedWingMask;
    root["wardenActive"] = data.wardenActive;
    root["explored"] = QString::fromStdString(data.exploredBits);

    // Choices
    QJsonObject cObj;
    cObj["sparedAda"] = data.choices.sparedAda;
    cObj["sparedEli"] = data.choices.sparedEli;
    cObj["sparedChoir"] = data.choices.sparedChoir;
    cObj["burnedFiles"] = data.choices.burnedFiles;
    cObj["confessedEarly"] = data.choices.confessedEarly;
    cObj["fragmentsFound"] = data.choices.fragmentsFound;
    cObj["maePhotosFound"] = data.choices.maePhotosFound;
    root["choices"] = cObj;

    QFile file(QString::fromStdString(filepath));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }

    QJsonDocument doc(root);
    file.write(doc.toJson());
    file.close();
    return true;
}

bool SaveGame::loadFromFile(const std::string& filepath, SaveData& outData) {
    QFile file(QString::fromStdString(filepath));
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray bytes = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(bytes);
    if (!doc.isObject()) return false;

    QJsonObject root = doc.object();
    outData.version = root["version"].toInt(1);
    outData.playTimeSec = static_cast<float>(root["playTimeSec"].toDouble(0.0));

    QJsonObject pObj = root["player"].toObject();
    outData.playerPos.x = static_cast<float>(pObj["x"].toDouble(0.0));
    outData.playerPos.y = static_cast<float>(pObj["y"].toDouble(0.0));
    outData.playerPos.z = static_cast<float>(pObj["z"].toDouble(0.0));
    outData.playerYaw = static_cast<float>(pObj["yaw"].toDouble(0.0));
    outData.playerPitch = static_cast<float>(pObj["pitch"].toDouble(0.0));

    outData.healthState = root["healthState"].toInt(0);
    outData.sanity = static_cast<float>(root["sanity"].toDouble(100.0));
    outData.lanternOil = static_cast<float>(root["lanternOil"].toDouble(100.0));

    outData.inventoryItemTypes.clear();
    for (auto val : root["inventory"].toArray()) {
        outData.inventoryItemTypes.push_back(val.toInt());
    }

    outData.unlockedDoorIds.clear();
    for (auto val : root["unlockedDoors"].toArray()) {
        outData.unlockedDoorIds.push_back(val.toInt());
    }

    outData.solvedPuzzleIds.clear();
    for (auto val : root["solvedPuzzles"].toArray()) {
        outData.solvedPuzzleIds.push_back(val.toInt());
    }

    outData.collectedNoteIds.clear();
    for (auto val : root["notes"].toArray()) {
        outData.collectedNoteIds.push_back(val.toInt());
    }

    outData.discoveredFragmentIds.clear();
    for (auto val : root["fragments"].toArray()) {
        outData.discoveredFragmentIds.push_back(val.toInt());
    }

    outData.collectedPickupIds.clear();
    for (auto val : root["collectedPickups"].toArray()) outData.collectedPickupIds.push_back(val.toInt());
    outData.openDoorIds.clear();
    for (auto val : root["openDoors"].toArray()) outData.openDoorIds.push_back(val.toInt());
    outData.keyFlags = root["keyFlags"].toInt(0);
    outData.sigilCount = root["sigils"].toInt(0);
    outData.visitedWingMask = root["visitedWings"].toInt(0);
    outData.wardenActive = root["wardenActive"].toBool(false);
    outData.exploredBits = root["explored"].toString().toStdString();

    QJsonObject cObj = root["choices"].toObject();
    outData.choices.sparedAda = cObj["sparedAda"].toBool(false);
    outData.choices.sparedEli = cObj["sparedEli"].toBool(false);
    outData.choices.sparedChoir = cObj["sparedChoir"].toBool(false);
    outData.choices.burnedFiles = cObj["burnedFiles"].toBool(false);
    outData.choices.confessedEarly = cObj["confessedEarly"].toBool(false);
    outData.choices.fragmentsFound = cObj["fragmentsFound"].toInt(0);
    outData.choices.maePhotosFound = cObj["maePhotosFound"].toInt(0);

    return true;
}

} // namespace hm
