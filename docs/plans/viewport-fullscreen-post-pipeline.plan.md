---
name: Fullscreen post pipeline
overview: Полноэкранный захват сцены в RT и цепочка из нескольких полноэкранных постэффектов за один кадр (ping-pong RT). Эффект гравитационной линзы — первый реализованный проход; архитектура допускает более сложные шейдеры и дополнительные типы проходов позже.
todos:
  - id: pipeline-api
    content: "Ввести абстракцию полноэкранного прохода (интерфейс + контекст окна/размер/view) и раннер цепочки с ping-pong RenderTexture"
    status: pending
  - id: lens-behaviour
    content: "GravitationalLensBehaviour (META_CLASS: амплитуда, falloff, offset, enabled) + сбор источников со сцены + .generated.hpp"
    status: pending
  - id: lens-pass
    content: "Класс GravitationalLensFullscreenPass: шейдер + uniforms; подключить как один элемент цепочки"
    status: pending
  - id: mainloop-wire
    content: "MainLoop: сцена → RT → цепочка эффектов → окно; если цепочка пуста — прямой window.draw(scene)"
    status: pending
  - id: ballgame-demo
    content: "BallGame1: GravitationalLensBehaviour на шаре в CreateBallNode; при необходимости регистрация/порядок эффектов"
    status: pending
  - id: build
    content: Собрать Debug и поправить регрессии
    status: pending
isProject: false
---

# План: полноэкранная линза + конвейер полноэкранных эффектов

## Цель

1. **Демо-эффект** — простая **полноэкранная** гравитационная линза: весь вывод `window->draw(*scene)` заменяется на захват в `sf::RenderTexture` (тот же `sf::View`, что у окна) и постобработку.
2. **Архитектура** — поддержка **нескольких полноэкранных эффектов в одном кадре** в **заданном порядке** (например: линза → размытие → цветокор).
3. **Расширяемость** — более сложные эффекты (доп. uniform, несколько текстур, позже — depth/MRT) подключаются как **новые типы проходов** в тот же конвейер, без переписывания линзы.

Региональные эффекты (поддерево в отдельный RT, маски) **не входят в v1**, но не конфликтуют с моделью: это будет отдельный вид прохода или дополнительный вход в шейдер позже.

## Конвейер полноэкранных проходов

- **Вход кадра**: цвет сцены в первом RT (`sceneRT`), размер = [`window.getSize()`](../src/Engine/Core/MainLoop.cpp), view скопирован с окна.
- **Цепочка**: для каждого активного эффекта `i`: читать текстуру из RT `A`, писать результат в RT `B`, затем поменять роли **ping-pong** (два `sf::RenderTexture` достаточно для любой длины цепочки).
- **Выход**: после последнего прохода нарисовать финальную текстуру в окно с **дефолтным view** (пиксельные координаты), как привычный fullscreen quad / `sf::RectangleShape`.
- **Нет активных эффектов**: без аллокаций лишних RT на кадр — прямой `window.draw(*scene)` (как сейчас).

Интерфейс прохода (имена условные): например `IViewportFullscreenEffect` с методом вида «применить: входная текстура / целевой `RenderTarget`» или «обновить uniforms из сцены» + статический/фабричный тип для регистрации. Внутри — `sf::Shader`, опционально вторичные текстуры.

**Порядок эффектов**: явный список (регистрация при старте окружения или из `MainContext`), порядок в списке = порядок в кадре. Несколько экземпляров одного типа (две линзы с разными параметрами) — либо два прохода с одним шейдером и разными uniform, либо два behaviour на сцене — деталь v1: достаточно **одного** lens-pass и при необходимости дублирования узла в списке «активных эффектов».

## Эффект линзы (первый проход)

- **`GravitationalLensBehaviour`** на ноде-источнике: параметры + опциональное смещение от [`Utils::GetWorldPos`](../src/Engine/Core/Utils.h); сбор активных линз обходом сцены; до **N** источников в шейдере (массивы uniform).
- Проход **`GravitationalLensFullscreenPass`**: fragment shader (UV-смещение, учёт aspect через `resolution`), vertex passthrough совместимый с SFML 3.

Если цепочка содержит только этот эффект — один ping-pong шаг: sceneRT → lens → окно.

## Точка входа

[`MainLoop::PresentFrame`](../src/Engine/Core/MainLoop.cpp): заменить единичный `window->draw(*scene)` на вызов **`PresentSceneWithFullscreenEffects(window, *scene)`** (или сервис на [`MainContext`](../src/Engine/Core/MainContext.h)), который строит список активных эффектов и выполняет конвейер. Редактор и ImGui — без изменения порядка относительно сцены (как сейчас).

При изменении размера окна — пересоздать/ресайз RT (уже есть [`OnMainWindowResized`](../src/Engine/Core/MainContext.cpp); при желании инвалидировать RT там же или при первом `Present` после ресайза).

## Демонстрация BallGame1

[`GameControllerBehaviour::CreateBallNode`](../src/Environments/BallGame1/GameControllerBehaviour.cpp): `RequireBehaviour<GravitationalLensBehaviour>()` на шаре; осмысленные дефолты параметров.

Регистрация lens-pass в конвейере: либо глобально для приложения, либо при поднятии сцены BallGame1 — выбрать минимально инвазивный вариант (например регистрация эффекта при первом `OnInit` окружения или статический список «всегда lens если есть behaviour на сцене» — предпочтительно **раннер читает сцену и включает lens-pass только если найден активный `GravitationalLensBehaviour`**, чтобы пустая сцена не платить за RT).

## Заметки по расширению (не v1)

- Эффекты с **depth**: добавить второй attachment / depth texture в проход захвата сцены — интерфейс прохода может принять расширенный `PresentContext`.
- **Не fullscreen**: отдельный тип прохода «subtree capture» — поверх того же раннера или параллельная ветка.

## Сборка

После изменений в `src/` — Debug: `cmake --build build --config Debug`.
