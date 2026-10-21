#pragma once

#include <vector>
#include <memory>
#include <QOpenGLFunctions_3_3_Core>
#include "MapGen.h"
#include "entities/Player.h"
#include "entities/Ghost.h"
#include "audio/AudioEngine.h"

namespace hm {

class World {
public:
    World();
    ~World();

    bool init(QOpenGLFunctions_3_3_Core* gl);
    void update(float dt);

    Player& player() { return m_player; }
    const Player& player() const { return m_player; }

    const MapData& map() const { return m_mapData; }
    const std::vector<Ghost>& ghosts() const { return m_ghosts; }

    AudioEngine& audio() { return m_audio; }

    float fearLevel() const { return m_fearLevel; }

private:
    MapData m_mapData;
    Player m_player;
    std::vector<Ghost> m_ghosts;
    AudioEngine m_audio;

    float m_fearLevel = 0.0f;
};

} // namespace hm
