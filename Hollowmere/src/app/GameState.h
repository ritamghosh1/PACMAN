#pragma once

namespace hm {

enum class GameState {
    Boot,
    Menu,
    Intro,
    Playing,
    Paused,
    Dead,
    Ending,
    Memory,     // animated flashback after a sigil
    Story       // menu: story overview and endings gallery
};

} // namespace hm
