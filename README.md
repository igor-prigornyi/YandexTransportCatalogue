# YandexTransportCatalogue
Финальный проект в Яндекс.Практикуме: транспортный справочник

## Сборка проекта

Сборка проекта осуществяется из папки `build`.

**1.** Запустить генерацию сценария сборки при помощи `CMake` с генератором `MinGW Makefiles` командой
```bash
cmake .. -G "MinGW Makefiles"
```

**2.** Запустить сборку проекта командой
```bash
cmake --build .
```

**3.** Запустить собранный исполняемый файл командой

```bash
./transport_catalogue
```