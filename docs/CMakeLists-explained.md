# Построчный разбор `CMakeLists.txt` проекта TestEngine

Ниже — файл из корня репозитория, строки совпадают с актуальной версией. Сигнатуры даны в упрощённом виде, как в [документации CMake](https://cmake.org/cmake/help/latest/) (полные формы с альтернативами см. по ссылкам в конце).

---

## Строка 1: минимальная версия CMake

```cmake
cmake_minimum_required(VERSION 3.16)
```

**Сигнатура (упрощённо):**

```cmake
cmake_minimum_required(VERSION <min>[...<max>] [FATAL_ERROR])
```

**Параметры:**

| Параметр | Здесь | Зачем |
|----------|--------|--------|
| `VERSION` | обязательный ключевое слово | Задаёт диапазон поддерживаемых версий CMake. |
| `<min>` | `3.16` | Если CMake старее — конфигурация прерывается (по умолчанию с `FATAL_ERROR`). |
| `...<max>` | не указан | Можно ограничить сверху, чтобы не подхватить несовместимое поведение новых CMake. |
| `FATAL_ERROR` | не указан | Явная ошибка вместо предупреждения на очень старых CMake. |

**Почему так:** без этой строки CMake не гарантирует, что доступны нужные команды и политики. `3.16` — разумный минимум для современного C++20 и многих идиом в подпроектах.

---

## Строки 3–7: политика CMP0091 (MSVC и формат отладочной информации)

```cmake
# MSVC: honor MSVC_DEBUG_INFORMATION_FORMAT (Embedded /Z7 vs ProgramDatabase) in generated flags.
if(POLICY CMP0091)
	cmake_policy(SET CMP0091 NEW)
endif()
```

**`if(POLICY CMP0091)`** — проверка: известна ли политика текущей версии CMake (на очень старых CMake идентификатор политики может отсутствовать).

**Сигнатура:**

```cmake
cmake_policy(SET CMP0091 NEW|OLD)
```

**CMP0091 (NEW):** свойства таргета `MSVC_RUNTIME_LIBRARY` и **`MSVC_DEBUG_INFORMATION_FORMAT`** начинают реально влиять на флаги компилятора MSVC. В **OLD** поведении эти свойства часто игнорировались.

**Неиспользуемых параметров нет** — `SET` принимает ровно имя политики и режим `NEW` или `OLD`.

**Почему так:** ниже используется `MSVC_DEBUG_INFORMATION_FORMAT` (строки 62–63). Без `CMP0091 NEW` настройка могла бы не попасть в командную строку `cl.exe`.

---

## Строка 8: объявление проекта

```cmake
project(TestEngine LANGUAGES CXX)
```

**Сигнатура (упрощённо):**

```cmake
project(
  <PROJECT-NAME>
  [VERSION <major>[.<minor>[.<patch>[.<tweak>]]]]
  [DESCRIPTION <...>]
  [HOMEPAGE_URL <...>]
  [LANGUAGES <lang>...]
)
```

**Параметры:**

| Параметр | Здесь | Зачем |
|----------|--------|--------|
| `<PROJECT-NAME>` | `TestEngine` | Имя проекта; задаёт переменные `PROJECT_NAME`, влияет на дефолты генераторов (например, имя solution в VS). |
| `VERSION` | не указан | Можно задать версию для пакетирования/CPack. |
| `LANGUAGES CXX` | указано | Включает язык C++ и проверяет наличие компилятора C++. Языки C/CXX и т.д. не добавляются лишние — быстрее конфигурация, меньше требований к окружению. |

**Почему только CXX:** проект на C++20, отдельный C не объявлен — не нужен.

---

## Строки 10–11: стандарт C++

```cmake
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

**Сигнатура:**

```cmake
set(<variable> <value>... [PARENT_SCOPE])
```

**Переменные (не параметры функции, а имена свойств сборки):**

| Имя | Значение | Смысл |
|-----|----------|--------|
| `CMAKE_CXX_STANDARD` | `20` | Запросить стандарт ISO C++20 для всех таргетов, где не переопределено. |
| `CMAKE_CXX_STANDARD_REQUIRED` | `ON` | Если компилятор не умеет C++20 — **ошибка конфигурации**, а не «тихий» откат на более старый стандарт. |

**Почему так:** единое требование к стандарту для всего дерева `src/` без дублирования `target_compile_features` на каждый таргет.

---

## Строка 13: куда класть `.exe`

```cmake
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
```

**Встроенные переменные:**

- `CMAKE_BINARY_DIR` — каталог **текущего** build (например `out/build/x64-Debug`).
- Итог: исполняемый файл окажется в `<build>/bin`, а не глубоко в `CMakeFiles/...`.

**Почему так:** предсказуемый путь к `TestEngine.exe` для запуска, скриптов и отладки.

---

## Строки 15–17: сбор списка исходников

```cmake
file(GLOB_RECURSE ENGINE_SOURCES
    "src/**.cpp"
)
```

**Сигнатура:**

```cmake
file(GLOB_RECURSE <out-var>
     [LIST_DIRECTORIES true|false]
     [RELATIVE <path>]
     [CONFIGURE_DEPENDS]
     <glob>...)
```

**Параметры:**

| Параметр | Здесь | Зачем |
|----------|--------|--------|
| `<out-var>` | `ENGINE_SOURCES` | Список путей к файлам попадёт в эту CMake-переменную. |
| `LIST_DIRECTORIES` | не указан | По умолчанию `false` — в список не попадают каталоги как «файлы». |
| `RELATIVE` | не указан | Пути обычно абсолютные или относительно текущего списка — зависит от версии/контекста; для `add_executable` обычно подходит. |
| `CONFIGURE_DEPENDS` | **не указан** | Если бы был: при изменении набора файлов под шаблоном CMake мог бы перезапустить конфигурацию. **Без него:** добавили новый `.cpp` в `src/` — иногда нужен ручной reconfigure. |
| `<glob>` | `"src/**.cpp"` | Рекурсивный обход: все `.cpp` под `src/`. (Вариант `src/**/*.cpp` часто пишут явно; поведение зависит от версии CMake и шаблона.) |

**Почему GLOB, а не явный список файлов:** быстро подключить всё дерево `src/`; минус — нет `CONFIGURE_DEPENDS` и возможные нюансы с кэшем при добавлении файлов.

---

## Строки 19–21: проверка, что исходники нашлись

```cmake
if(NOT ENGINE_SOURCES)
    message(FATAL_ERROR "No source files found in src/. Check project structure.")
endif()
```

**Сигнатура `message`:**

```cmake
message([STATUS|WARNING|AUTHOR_WARNING|SEND_ERROR|FATAL_ERROR|DEPRECATION] "текст")
```

**`FATAL_ERROR`** — остановить конфигурацию. Иначе `add_executable` получил бы пустой список и дал бы менее понятную ошибку.

---

## Строка 23: исполняемый файл

```cmake
add_executable(TestEngine WIN32 ${ENGINE_SOURCES})
```

**Сигнатура (упрощённо):**

```cmake
add_executable(<name>
  [WIN32] [MACOSX_BUNDLE] [EXCLUDE_FROM_ALL] ...
  <sources>...)
```

**Параметры:**

| Параметр | Здесь | Зачем |
|----------|--------|--------|
| `<name>` | `TestEngine` | Имя CMake-таргета и по умолчанию имя выходного файла (`TestEngine.exe`). |
| `WIN32` | указано | На Windows — **подсистема Windows** (`/SUBSYSTEM:WINDOWS`), без консольного окна по умолчанию; точка входа ожидается как `WinMain` у GUI-приложений (для SFML/окон это типично). |
| `MACOSX_BUNDLE` и др. | нет | Не нужны для текущей цели. |
| `<sources>` | `${ENGINE_SOURCES}` | Раскрытие списка файлов из `file(GLOB_RECURSE ...)`. |

---

## Строки 25–29: флаги MSVC

```cmake
if(MSVC)
	target_compile_options(TestEngine PRIVATE /FS)
	target_compile_options(TestEngine PRIVATE $<$<CONFIG:Debug>:/Z7>)
endif()
```

**Сигнатура:**

```cmake
target_compile_options(<target> <INTERFACE|PUBLIC|PRIVATE> [<options>...] ...)
```

**Параметры:**

| Параметр | Значение | Зачем |
|----------|----------|--------|
| `<target>` | `TestEngine` | Только ваш exe, не third-party. |
| `PRIVATE` | да | Флаги только при компиляции этого таргета; не «наследуются» потребителями (у exe потребителей нет). `INTERFACE`/`PUBLIC` здесь не нужны. |
| опции | `/FS` | Принудительная синхронизация доступа к PDB между параллельными `cl.exe` (снижает риск ошибки **C1041**). |
| опции | `$<$<CONFIG:Debug>:/Z7>` | **Генераторное выражение:** в конфигурации **Debug** добавить `/Z7` (отладочные символы в `.obj`), в Release — ничего не добавить. Уменьшает конфликты за один `.pdb` при параллельной сборке; комментарий в файле это поясняет. |

**`if(MSVC)`** — эти флаги имеют смысл только для Microsoft Visual C++.

---

## Строки 31–33: include для своего кода

```cmake
target_include_directories(TestEngine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
```

**Сигнатура:**

```cmake
target_include_directories(<target> [SYSTEM] [BEFORE]
  <INTERFACE|PUBLIC|PRIVATE> <items>...)
```

**Параметры:**

| Параметр | Здесь | Зачем |
|----------|--------|--------|
| `SYSTEM` | нет | Если бы был: компилятор мог бы трактировать заголовки как «системные» (меньше предупреждений). Для своего `src/` обычно не нужно. |
| `BEFORE` | нет | Порядок `-I` относительно других include. |
| `PRIVATE` | да | Пути include только при сборке `TestEngine`. |
| `<items>` | `${CMAKE_CURRENT_SOURCE_DIR}/src` | Корень исходников проекта: `#include "Engine/..."` и т.д. `CMAKE_CURRENT_SOURCE_DIR` — каталог **этого** `CMakeLists.txt` (корень репо). |

---

## Строки 35–37: библиотека fmt как подпроект

```cmake
# fmt submodule: third-party/fmt
add_subdirectory(third-party/fmt EXCLUDE_FROM_ALL)
target_link_libraries(TestEngine PRIVATE fmt)
```

**Сигнатура `add_subdirectory`:**

```cmake
add_subdirectory(<source_dir> [<binary_dir>] [EXCLUDE_FROM_ALL | SYSTEM])
```

| Параметр | Здесь | Зачем |
|----------|--------|--------|
| `<source_dir>` | `third-party/fmt` | Где лежит `CMakeLists.txt` fmt. |
| `<binary_dir>` | не указан | Binary dir по умолчанию: `<build>/third-party/fmt` (или аналогично относительно build root). |
| `EXCLUDE_FROM_ALL` | указано | Таргеты fmt **не** входят в дефолтную цель `all`/`ALL_BUILD` сверху — собираются только если от них зависит что-то (у вас — линковка `TestEngine`). Ускоряет сборку, не тащит тесты fmt в общий билд. |
| `SYSTEM` | нет | Помечает подпроект как системный для предупреждений — не используется. |

**Сигнатура `target_link_libraries`:**

```cmake
target_link_libraries(<target> <PRIVATE|PUBLIC|INTERFACE> <items>...)
```

| Параметр | Здесь | Зачем |
|----------|--------|--------|
| `<target>` | `TestEngine` | Кто линкуется. |
| `PRIVATE` | да | Зависимость не экспортируется наружу (опять же, у exe нет потребителей в CMake-смысле). |
| `<items>` | `fmt` | Имя **таргета**, объявленного внутри fmt — не путь к `.lib`. CMake сам добавит линковку и include при использовании современного CMake fmt. |

---

## Строки 39–44: отключение лишнего в SFML

```cmake
# SFML submodule: third-party/SFML
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "" FORCE)

add_subdirectory(third-party/SFML EXCLUDE_FROM_ALL)
```

**Сигнатура `set` для CACHE:**

```cmake
set(<var> <value>... CACHE <type> <docstring> [FORCE])
```

| Параметр | Здесь | Зачем |
|----------|--------|--------|
| `CACHE` | да | Значение попадает в кэш CMake (видно в `cmake-gui`, сохраняется между запусками). |
| `<type>` | `BOOL` | Логический переключатель. |
| `<docstring>` | `""` | Пустое описание в GUI — **не ошибка**, просто в интерфейсе не будет подсказки. |
| `FORCE` | да | Перезаписать значение в кэше даже если оно уже было (гарантирует OFF для этих опций SFML). |

**Почему до `add_subdirectory`:** при первом чтении `third-party/SFML/CMakeLists.txt` эти переменные уже должны быть выставлены, иначе SFML мог бы включить примеры/доки/тесты.

---

## Строки 46–52: линковка модулей SFML

```cmake
target_link_libraries(TestEngine PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
    sfml-network
)
```

Те же правила `target_link_libraries`, несколько **item** подряд: все как `PRIVATE` зависимости. Имена — таргеты SFML 3 (`sfml-*`). Порядок для статической/динамической линковки CMake обычно разрешит транзитивно (`graphics` тянет `window` → `system` и т.д.), но явный список оставляет зависимости читаемыми и гарантирует подключение нужных компонентов.

---

## Строка 55: стартовый проект в Visual Studio

```cmake
set_property(DIRECTORY "${CMAKE_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT TestEngine)
```

**Сигнатура:**

```cmake
set_property(DIRECTORY <dirs> ... PROPERTY <name> [value...])
```

| Параметр | Здесь | Зачем |
|----------|--------|--------|
| `DIRECTORY` | `"${CMAKE_SOURCE_DIR}"` | Свойство вешается на **корневой** каталог проекта в CMake. |
| `PROPERTY` | `VS_STARTUP_PROJECT` | Специфично для генератора **Visual Studio**: какой проект выбран по умолчанию при F5. |
| `value` | `TestEngine` | Имя таргета — ваш exe, а не `ALL_BUILD` или случайный таргет из third-party. |

---

## Строки 58–64: рабочая каталог отладчика и формат PDB

```cmake
set_target_properties(
	TestEngine
	PROPERTIES
		VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
		MSVC_DEBUG_INFORMATION_FORMAT
		"$<$<CONFIG:Debug>:Embedded>$<$<NOT:$<CONFIG:Debug>>:ProgramDatabase>"
)
```

**Сигнатура:**

```cmake
set_target_properties(<target1> [<target2> ...]
  PROPERTIES <prop1> <val1> [<prop2> <val2> ...])
```

| Свойство | Значение | Зачем |
|----------|----------|--------|
| `VS_DEBUGGER_WORKING_DIRECTORY` | `${CMAKE_SOURCE_DIR}` | При **Run/Debug из VS** (и частично из CMake Tools в VS Code) текущая директория процесса = корень репозитория — удобно для относительных путей к ассетам (`./data`, `fonts/` и т.д.). |
| `MSVC_DEBUG_INFORMATION_FORMAT` | генераторное выражение | **Debug:** `Embedded` → в духе `/Z7`, символы в объектах. **Не Debug:** `ProgramDatabase` → отдельный `.pdb` (типично для Release/RelWithDebInfo). Связано с политикой **CMP0091 NEW** в начале файла. |

**Разбор генератора:**

```text
$<$<CONFIG:Debug>:Embedded>$<$<NOT:$<CONFIG:Debug>>:ProgramDatabase>
```

- Первый блок: если конфигурация Debug — строка `Embedded`.
- Второй блок: если **не** Debug — строка `ProgramDatabase`.
- Склеивается в одно значение свойства в зависимости от конфигурации.

---

## Краткая схема потока

```text
cmake_minimum_required + policy
        → project + стандарты + пути вывода
        → GLOB исходники → add_executable
        → опции компиляции + include
        → add_subdirectory(fmt) → link fmt
        → кэш-опции SFML → add_subdirectory(SFML) → link sfml-*
        → свойства VS / MSVC для отладки и старта F5
```

---

## Ссылки на официальную справку

- [cmake_minimum_required](https://cmake.org/cmake/help/latest/command/cmake_minimum_required.html)
- [cmake_policy](https://cmake.org/cmake/help/latest/command/cmake_policy.html) — [CMP0091](https://cmake.org/cmake/help/latest/policy/CMP0091.html)
- [project](https://cmake.org/cmake/help/latest/command/project.html)
- [set](https://cmake.org/cmake/help/latest/command/set.html)
- [file(GLOB)](https://cmake.org/cmake/help/latest/command/file.html#glob)
- [message](https://cmake.org/cmake/help/latest/command/message.html)
- [add_executable](https://cmake.org/cmake/help/latest/command/add_executable.html)
- [target_compile_options](https://cmake.org/cmake/help/latest/command/target_compile_options.html)
- [target_include_directories](https://cmake.org/cmake/help/latest/command/target_include_directories.html)
- [add_subdirectory](https://cmake.org/cmake/help/latest/command/add_subdirectory.html)
- [target_link_libraries](https://cmake.org/cmake/help/latest/command/target_link_libraries.html)
- [set_property](https://cmake.org/cmake/help/latest/command/set_property.html)
- [set_target_properties](https://cmake.org/cmake/help/latest/command/set_target_properties.html)
- [Generator expressions](https://cmake.org/cmake/help/latest/manual/cmake-generator-expressions.7.html)
