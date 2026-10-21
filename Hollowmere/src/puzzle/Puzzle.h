#pragma once

#include <string>

namespace hm {

class IPuzzle {
public:
    virtual ~IPuzzle() = default;

    virtual const std::string& name() const = 0;
    virtual bool isSolved() const = 0;
    virtual void reset() = 0;
    virtual void update(float dt) = 0;
    virtual int targetUnlockDoorId() const = 0;
};

} // namespace hm
