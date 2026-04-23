# Сборка проекта TestEngine

## Требования

- Windows 10+
- MSVC (полная Visual Studio или **Build Tools** с компонентом «Разработка классических приложений на C++»)
- CMake 3.20+

На Windows CMake по умолчанию выбирает генератор **Visual Studio** (конкретная годовая версия зависит от установленного ПО). Если конфигурация падает с ошибкой вроде «could not find any instance of Visual Studio», задайте генератор вручную (см. ниже) под вашу установленную версию. Список генераторов: `cmake --help` (раздел *Generators*).

## Команды сборки

Рабочая директория — корень репозитория (`TestEngine`, где лежит `CMakeLists.txt`).

**Не запускайте `cmake .` в корне исходников** — в репозитории появится `CMakeCache.txt`, а смена генератора или папки сборки начнёт конфликтовать с кэшем. Сборку держите только в отдельной папке (`build`, `out/build` и т.п.).

### 1. Конфигурация проекта

Удобный вариант (CMake сам создаст каталог сборки):

```bash
cd TestEngine
cmake -S . -B build -G "Visual Studio 18 2026" -A x64
```

Подставьте свой генератор, если у вас другая линейка IDE, например `-G "Visual Studio 17 2022" -A x64`.

Классический вариант:

```bash
cd TestEngine
mkdir build
cd build
cmake .. -G "Visual Studio 18 2026" -A x64
```

В **PowerShell** для пустой папки: `New-Item -ItemType Directory -Path build`.

**Параметры CMake:**

- Для генераторов **Visual Studio** / **Ninja Multi-Config** тип сборки задаётся при **сборке** (`--config Release` или `Debug`), а не только через `CMAKE_BUILD_TYPE`.
- `-DCMAKE_BUILD_TYPE=Release` или `Debug` — имеет смысл для одноконфигурационных генераторов (например, Ninja в одноконфигурационном режиме).

Пример Debug-конфигурации (мультиконфиг VS):

```bash
cmake --build build --config Debug
```

### 2. Сборка проекта

Из корня репозитория, если использовали `-B build`:

```bash
cmake --build build --config Release
```

Если вы уже находитесь в каталоге `build` после `cmake ..`:

```bash
cmake --build . --config Release
```

**Параметры:**

- `--config Release` — сборка Release
- `--config Debug` — сборка Debug
- `-j <число>` — параллельная сборка

Пример с параллельной сборкой:

```bash
cmake --build build --config Release -j 8
```

## Полный цикл сборки

```bash
cd TestEngine
cmake -S . -B build -G "Visual Studio 18 2026" -A x64
cmake --build build --config Release
```

## Результаты сборки

После успешной сборки исполняемый файл (при каталоге сборки `build`):

```
build/bin/Release/TestEngine.exe
```

Если вы указали другое имя в `-B`, замените префикс `build` на свой путь (например, `out/vs18/bin/Release/TestEngine.exe`).

## Запуск из Visual Studio (CMake)

Служебные цели CMake вроде **`ALL_BUILD`** или **`ZERO_CHECK`** — это не приложения. Если в списке **«Элемент запуска»** (рядом с зелёной стрелкой) выбран `ALL_BUILD`, Visual Studio пытается «запустить» его как программу и выдаёт ошибку вроде **«Не удается запустить программу … ALL_BUILD»** / **«Отказано в доступе»**.

Что сделать: откройте выпадающий список **«Элемент запуска»** и выберите **`TestEngine`** (исполняемая цель из `CMakeLists.txt`).

В корне репозитория лежит **`launch.vs.json`** (поддерживается Visual Studio **16.9+**): в нём по умолчанию задана цель **`TestEngine`**, чтобы отладка и запуск из IDE шли в исполняемый файл, а не в `ALL_BUILD`. Если VS уже создал свой `launch.vs.json` в папке `.vs`, при конфликте ориентируйтесь на настройки **«Элемент запуска»** вручную.

## Очистка сборки

```bash
cd TestEngine
cmake --build build --target clean
```

Полная очистка (удалить каталог сборки):

- Git Bash: `rm -rf build`
- PowerShell: `Remove-Item -Recurse -Force build`
- cmd: `rmdir /s /q build`
