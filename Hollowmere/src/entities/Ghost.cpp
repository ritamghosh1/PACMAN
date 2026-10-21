#include "Ghost.h"
#include <cmath>

namespace hm {

Ghost::Ghost(GhostType type, const Vec3& spawnPos)
    : m_type(type), m_position(spawnPos), m_targetPos(spawnPos) {
    switch (type) {
        case GhostType::Weeper:    m_visualType = GhostVisualType::Weeper; break;
        case GhostType::HollowMan: m_visualType = GhostVisualType::HollowMan; break;
        case GhostType::Choir:     m_visualType = GhostVisualType::Choir; break;
        case GhostType::Doll:      m_visualType = GhostVisualType::Doll; break;
        case GhostType::Warden:    m_visualType = GhostVisualType::Warden; break;
    }
}

void Ghost::stun(float durationSec) {
    m_state = GhostState::Stunned;
    m_stunTimer = durationSec;
    m_agitation = 0.8f;
}

void Ghost::repel(const Vec3& sourcePos) {
    m_state = GhostState::Fleeing;
    m_fleeTimer = 18.0f;
    Vec3 away = normalize(m_position - sourcePos);
    m_targetPos = m_position + away * 12.0f;
    m_agitation = 1.0f;
}

void Ghost::pacify() {
    m_state = GhostState::Pacified;
    m_agitation = 0.0f;
}

Mat4 Ghost::modelMatrix() const {
    Mat4 m = Mat4::translate(m_position);
    m = m * Mat4::rotateY(m_yaw);
    return m;
}

} // namespace hm
