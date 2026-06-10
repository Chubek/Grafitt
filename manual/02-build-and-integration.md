# Chapter 02 — Build & Integration

## Requirements
- C++20 compiler.
- Include `Grafitt.hpp` for core library.
- Optional: `Grafitt-Plugin.hpp`, `Grafitt-Module.h`, `Grafitt-ModuleHost.hpp`.

## Minimal Include
```cpp
#include "Grafitt.hpp"
```

## Optional Headers
- `Grafitt-Plugin.hpp`: in-process C++ pass framework.
- `Grafitt-Module.h`: strict C ABI plugin contract.
- `Grafitt-ModuleHost.hpp`: host lifecycle manager for C ABI modules.

## Notes
- Keep include paths pointed at repository root.
- No linker step is required for core header-only usage.
