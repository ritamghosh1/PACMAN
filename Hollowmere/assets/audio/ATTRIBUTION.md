# Audio attribution

All `.ogg` files in this folder are recordings and music from the open-source game
**SCP - Containment Breach** (https://github.com/Regalis11/scpcb), which is released under the
**Creative Commons Attribution-ShareAlike 3.0** licence (https://creativecommons.org/licenses/by-sa/3.0/).

Sound effects and music credited by that project to: Joonas Rikkonen ("Regalis"), Caleb Yu ("Omniary"),
Class401, FireFox, gmodfan11, Kevin MacLeod (incompetech.com), phlip45, revo794, Shaun Gaylard ("ENDSHN"),
Slagar, Tim Morrison, Vladislav Borisevich ("Vane Brain"), Walach.

The files were copied unmodified and only renamed. Because of the ShareAlike term, these audio files remain
under CC BY-SA 3.0 even though the rest of this project is separate. The Ogg Vorbis decoder used to read them
is `src/third_party/stb_vorbis.c` (Sean Barrett, public domain / MIT).

The game selects the files by name (see `src/audio/AssetAudio.cpp` and `AudioEngine.cpp`). If this folder is
missing, Hollowmere falls back to its built-in procedural sounds.

| File prefix | Source in SCP:CB | Used for |
|---|---|---|
| `music_*` | `SFX/Music/` | menu, per-wing background music, chase themes, intro, endings |
| `horror_*` | `SFX/Horror/` | jump-scare stingers |
| `amb1_*`, `amb2_*`, `amb3_*` | `SFX/Ambient/Zone1-3/` | random unsettling ambient cues |
| `room_*` | `SFX/Ambient/Room ambience/` | low drone, pulse, rumble, drip |
| `weeper_*` | `SFX/SCP/096/` | Ada: attack scream and trigger cry |
| `pell_*` | `SFX/SCP/106`, `SFX/SCP/372` | Mr. Pell: breathing, laugh, rustle |
| `lure_*` | `SFX/Character/LureSubject/` | sniffles and whimpers (Ada / Eli) |
| `pd_*` | `SFX/Room/PocketDimension/` | the Choir's whispering voices, screech, rumble |
| `warden_*` | `SFX/SCP/173`, `SFX/SCP/049` | stone drag, rattles, breathing |
| `wood_*`, `glass_break`, `body_fall`, `pick*` | `SFX/Door`, `SFX/General`, `SFX/Interact` | doors and pickups |
