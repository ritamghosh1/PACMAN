# HORROR_CONTENT.md — visceral content spec
Gore and disturbing imagery are the CORE of the game, not an option: you live Nora's life. NO toggles, NO tiers, NO content gates, NO rating labels anywhere in code/UI/docs.

## ALWAYS PRESENT
- Gore: wounds, blood pools/trails/spray decals, corpses, mutilation, medical-torture props, burnt bodies, dissections in Crane's lab, injuries to the player (bleeding, limp).
- Body horror: fused throats (Choir), sewn-eye mask (Eli), pruned/faceless figure (Hollow Man), flesh-in-walls.
- Psychological: guilt, abuse by authority figures (told via records, drawings, audio, aftermath), child death as backstory, grief.
- Language: restrained; Crane's logs are clinical and cold.

## AUTHORING RULES (tone discipline, not user options)
- Child harm is told through aftermath, silhouettes, audio, drawings and records, never shown graphically.
- No sexual content. Any bodies shown (patients, corpses, statues) are clinical/horrific, never sexualised.
- Gore serves story: each gruesome set piece reveals lore or builds dread; avoid gore for its own sake.
- Subtitles + sound captions available (accessibility, in settings with other basics).

## IMPLEMENTATION NOTES
- Gore system: blood decals (pooled per surface, spread over time), wound overlays on corpse props (3 variants for variety, not tiers), blood-on-lens damage vignette, limb/gib props from MeshBuilder kit, wet shader (specular up), flies particles, drip audio.
- Corpse/gore props live in one place: `src/world/PropsGore.cpp`.
