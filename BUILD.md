# Сборка проекта TestEngine

## Требования

- Windows 10+
- MSVC компилятор (Visual Studio или Build Tools)
- CMake 3.20+

## Команды сборки

### 1. Конфигурация проекта

```bash
cd TestEngine
mkdir -p build
cd build
cmake ..
```

**Параметры CMake:**
- `-DCMAKE_BUILD_TYPE=Release` — сборка в режиме Release (по умолчанию)
- `-DCMAKE_BUILD_TYPE=Debug` — сборка с отладочной информацией

Пример:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

### 2. Сборка проекта

```bash
cmake --build . --config Release
```

**Параметры:**
- `--config Release` — сборка Release версии
- `--config Debug` — сборка Debug версии
- `-j <число>` — параллельная сборка (по числу процессоров)

Пример с параллельной сборкой:
```bash
cmake --build . --config Release -j 8
```

## Полный цикл сборки

```bash
cd TestEngine
mkdir -p build && cd build
cmake ..
cmake --build . --config Release
```

## Результаты сборки

После успешной сборки исполняемый файл находится в:
```
build/bin/Release/TestEngine.exe
```

## Очистка сборки

```bash
cd build
cmake --build . --target clean
```

Или полная очистка:
```bash
rm -rf build
```
