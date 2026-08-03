# Сборка TestEngine

**Требования:** Windows, MSVC (Visual Studio или Build Tools с C++), **Python 3** (интерпретатор нужен CMake на этапе `find_package`), CMake 3.28+ (см. `cmake_minimum_required` в `CMakeLists.txt`).

Вся работа из **корня репозитория** (рядом с `CMakeLists.txt`). Основной каталог сборки — **`build`** (генератор **Visual Studio**, MSBuild). **Не** запускайте `cmake` прямо в исходниках.

## Скрипты (рекомендуется)

| Скрипт | Действие |
|--------|----------|
| `_cmake.cmd` | `cmake -S . -B build -A x64` |
| `_cmake_gen_and_open.cmd` | то же + открыть `build\TestEngine.slnx` |
| `_cmake_ninja.cmd` | `build-ninja/` + Ninja (только `compile_commands.json` для clangd) |
| `_build_release.cmd` / `_build_debug.cmd` | `cmake --build build` с `Release` или `Debug` |
| `_run_release.cmd` / `_run_debug.cmd` | Запуск exe; **рабочий каталог = корень репо** |
| `_run_billiards_server.cmd` | Python BilliardSession-сервер (`127.0.0.1:7777`) |
| `_gen_proto.cmd` | Генерация `BilliardSession_pb2.py` (Python) и `BilliardSession.pb.*` (C++) из [`proto/BilliardSession.proto`](proto/BilliardSession.proto) |
| `__clean.cmd` | Удаляет `build`, `build-ninja`, `.vs`, `src/Codegen`, `src/proto_generated` |

Параллельная сборка: `_build_release.cmd -j 8`.

## Вручную (CMake в PATH)

```bash
cmake -S . -B build -A x64
cmake --build build --config Release
```

**Где exe:** `build/bin/Release/TestEngine.exe` и `build/bin/Debug/TestEngine.exe`.

Если в `build` остался генератор Ninja — `__clean.cmd`, затем `_cmake.cmd`.

## Профили запуска (`--env=`)

- по умолчанию (без аргумента) — [`EditorProfile`](src/LaunchProfiles/Editor/EditorProfile.h)
- `--env=editor` — то же
- `--env=test` — [`TestLaunchProfile`](src/LaunchProfiles/Test/TestLaunchProfile.h)
- `--env=pong` — [`PongLaunchProfile`](src/LaunchProfiles/Pong/PongLaunchProfile.h)

Пример:

```text
build\bin\Debug\TestEngine.exe --env=pong
```

В Visual Studio: откройте **`build\TestEngine.slnx`** (через `_cmake_gen_and_open.cmd`) или solution из VS; **TestEngine** → Debugging → Command Arguments. Рабочая папка при отладке: **`VS_DEBUGGER_WORKING_DIRECTORY`** = корень репо (`CMakeLists.txt`).

## clangd и `compile_commands.json`

Visual Studio **не** пишет `compile_commands.json` в `build/`. Для Cursor/clangd — отдельный каталог **`build-ninja`**:

```bash
_cmake_ninja.cmd
```

Перезапуск языкового сервера: **clangd: Restart language server**. В [`.vscode/settings.json`](.vscode/settings.json): `--compile-commands-dir=${workspaceFolder}/build-ninja`.

После крупных изменений CMake/новых `.cpp` — снова `_cmake_ninja.cmd` (полная сборка Ninja не обязательна, достаточно configure).

## Codegen (дерево свойств)

Скрипт [`tools/property_codegen.py`](tools/property_codegen.py) генерирует заголовки `src/Codegen/<ИмяИсходника>.generated.hpp` из тегов `META_CLASS()` и `/// @property` в `src/**/*.h`. Каталог `src/Codegen/` в `.gitignore`; при сборке `TestEngine` цель **`Codegen`** запускается автоматически первой (`add_dependencies`).

**Пользовательская справка** (типы, теги, мета-ключи и ограничения): [`docs/PROPERTY_CODEGEN.md`](docs/PROPERTY_CODEGEN.md).

| CMake-цель | Назначение |
|------------|------------|
| `Codegen` | Инкрементально: кеш `src/Codegen/.codegen_cache.json`, пропуск неизменённых `.h` |
| `Codegen_Force` | Полная перегенерация (`--force`, кеш перезаписывается) |

Вручную: `cmake --build build --config Debug --target Codegen` или `Codegen_Force`.

Полный лог по каждому заголовку: `python tools/property_codegen.py --root . --verbose` (в CMake по умолчанию без `--verbose`, в конце — сводка).

Список `src/**/*.cpp` подхватывается при следующей сборке/configure: в `CMakeLists.txt` задан `CONFIGURE_DEPENDS` на glob исходников.

При сборке с макросом `_CONSOLE` флаги `--env=…` передаются через `main(argc, argv)`.

## Visual Studio

- **`_cmake_gen_and_open.cmd`** — solution в `build/`, F5 на **TestEngine**.
- **Open Folder** с CMake без solution: по умолчанию снова **`-A x64`** в `build/`; не подмешивайте Ninja в `build`, если нужен `.slnx`.

## Очистка

- Полный сброс: `__clean.cmd`, затем `_cmake.cmd` и при необходимости `_cmake_ninja.cmd`.
- Только clean: `cmake --build build --config Release --target clean`.

## BilliardSession-сервер (MVP)

**Зависимости:** git submodule [`third-party/protobuf`](third-party/protobuf) (сборка вместе с `TestEngine`), Python 3 + `pip install -r server/requirements.txt` (`protobuf>=7.35.1`).

**Первый раз после клона:**

```text
git submodule update --init --recursive
_cmake.cmd
_build_release.cmd
_gen_proto.cmd
```

`_gen_proto.cmd` для Python вызывает `grpcio-tools.protoc` (ставится автоматически); для C++ — `protoc.exe` из `build/bin/Release/`. CMake также генерирует C++ при build в [`src/proto_generated/`](src/proto_generated/) (каталог в `.gitignore`).

**Запуск:**

```text
_run_billiards_server.cmd
```

Логирует `CreateSession` / `JoinSession`. Два клиента на одной машине: два окна `TestEngine`, на сцене бильярда — [`BilliardServerBehaviour`](src/LaunchProfiles/Billiard/BilliardServerBehaviour.h) (ПКМ в inspector → **Create Session** / **Join Session**, `/// @method`). Ответы — в консоль (`fmt::print`).
