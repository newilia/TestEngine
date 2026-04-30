# Сборка TestEngine

**Требования:** Windows, MSVC (Visual Studio или Build Tools с C++), **Python 3** (интерпретатор нужен CMake на этапе `find_package`), CMake 3.28+ (см. `cmake_minimum_required` в `CMakeLists.txt`).

Вся работа из **корня репозитория** (рядом с `CMakeLists.txt`). Каталог **сборки — `build`**, **не** запускайте `cmake` прямо в исходниках (иначе появится `CMakeCache.txt` в корне).

## Скрипты (рекомендуется)

| Скрипт | Действие |
|--------|----------|
| `__gen_solution.cmd` | `cmake -S . -B build -A x64`, при успехе пытается открыть `build\TestEngine.slnx` (если есть; иначе в `build` смотрите `TestEngine.slnx` — откройте вручную) |
| `_build_release.cmd` / `_build_debug.cmd` | `cmake --build build` с `Release` или `Debug` |
| `_run_release.cmd` / `_run_debug.cmd` | Запуск `TestEngine.exe` с **рабочим каталогом = корень репо** |
| `__clean.cmd` | Полностью **удаляет** каталог `build` (если есть) |

Параллельная сборка, например: `_build_release.cmd -j 8` (аргументы передаются в `cmake --build`).

## Вручную (CMake в PATH)

```bash
cmake -S . -B build -A x64
cmake --build build --config Release
```

Первый шаг: генератор (часто Visual Studio). Если `cmake` не находит Visual Studio, укажите `-G` сами, см. `cmake --help` (Generators). Для Visual Studio / Ninja **Multi-Config** тип (Release/Debug) задаётся **при сборке** (`--config`), а не одним `CMAKE_BUILD_TYPE` в чистом виде.

**Где exe:** `build/bin/Release/TestEngine.exe` и `build/bin/Debug/TestEngine.exe`.

## Codegen (дерево свойств)

Скрипт [`tools/property_codegen.py`](tools/property_codegen.py) генерирует заголовки `src/Codegen/<ИмяИсходника>_gen.hpp` из тегов `META_CLASS()` и `/// @property` в `src/**/*.h`. Каталог `src/Codegen/` в `.gitignore`; при сборке `TestEngine` цель **`Codegen`** запускается автоматически первой (`add_dependencies`).

| CMake-цель | Назначение |
|------------|------------|
| `Codegen` | Инкрементально: кеш `src/Codegen/.codegen_cache.json`, пропуск неизменённых `.h` |
| `Codegen_Force` | Полная перегенерация (`--force`, кеш перезаписывается) |

Вручную: `cmake --build build --config Debug --target Codegen` или `Codegen_Force`.

Полный лог по каждому заголовку: `python tools/property_codegen.py --root . --verbose` (в CMake по умолчанию без `--verbose`, в конце — сводка).

Список `src/**/*.cpp` и заголовков подхватывается при следующей сборке или конфигурации: в `CMakeLists.txt` для них задан `CONFIGURE_DEPENDS` (перезапуск configure при изменении дерева файлов).

## Visual Studio

- В корневом `CMakeLists.txt` задано **`VS_STARTUP_PROJECT` = `TestEngine`** — при открытии сгенерированного solution **элементом запуска** по умолчанию должна быть эта цель, а не `ALL_BUILD`. При необходимости выберите `TestEngine` вручную в списке запуска. Рабочая папка при отладке из VS — **корень репозитория** (`VS_DEBUGGER_WORKING_DIRECTORY`).

## Очистка

- Полный сброс: `__clean.cmd` (или вручную удалить `build`, затем снова `__gen_solution.cmd`). Если `build` занят (часто VS) — сначала закройте IDE.
- Без удаления каталога, только `clean` от CMake: `cmake --build build --config Release --target clean` (нужен уже сгенерированный `build`).
