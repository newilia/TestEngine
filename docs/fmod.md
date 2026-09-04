# FMOD banks (TestEngine)

Runtime uses **FMOD Engine** (C++ Studio API). Author events in **FMOD Studio**, then export banks into this repo.

## Export from FMOD Studio

1. Create a Windows project (same **major** version as the installed FMOD Engine, e.g. both 2.02.x).
2. Build banks into `resources/audio/` (working directory of the game is the repo root).
3. You need at least:
   - `Master.bank` — audio data
   - `Master.strings.bank` — event path lookup (`event:/...`)

Without the strings bank, `PlayEvent("event:/SFX/CueHit")` cannot resolve the path.

## Code

```cpp
auto audio = Engine::MainContext::GetInstance().GetAudioManager();
audio->LoadBank("resources/audio/Master.bank"); // also loads Master.strings.bank if that file exists
audio->PlayEvent("event:/SFX/CueHit");
```

`AudioManager` is initialized with the rest of `MainContext` and updated each presented frame. Banks are not loaded automatically at startup.
