# AGENTS.md

This repository is a **UI library** for Arma Reforger, not a game mode.

1. Read [README.md](README.md) before writing UI.
2. Replacing vanilla Conflict UI: read [CONFLICT-UI-REPLACEMENT.md](CONFLICT-UI-REPLACEMENT.md) first. Feature screens still belong in the consuming addon.
3. Every `Scripts/Game/**/*.c` file starts with a **Consumer / Layout / Extend** header. Trust that header over rediscovering the compositor.
4. **Do not add feature screens here.** New menus belong in the consuming addon (`class MyMenu : MUI_MenuBase`). The in-tree demo is `MUI_SampleMenu`.
5. **Do not edit `MUI_InputRouter` to special-case a widget.** Use `WantsTextInput`, `HandleNavAxis`, `HandleActivate`, `HandleDismiss`, `OnDrag` on the node.
6. Custom nodes: `ref T n = new T(); runtime.Adopt(n); parent.AddChild(n);` plus `protected ref T m_n;` if you keep it.
7. Enforce: no ternary; every `new` in a `ref`; colors via `Color.FromSRGBA`; paint via `DrawX/Y` + `GetDrawOpacity()`.
8. Menus: `MUI_MenuBase`. HUD: `MUI_HudHost` + `MakePassThroughOverlay`. Blank layout GUID: `{A1C47E92B6D8305F}UI/Layouts/MUI_BlankMenu.layout`.
9. Addon GUID to depend on: `B3F91C6A4E275D08`.
10. Theme is per `MUI_Runtime`: `SetTheme` before `Create*` / `Adopt`. Paint via `GetTheme()`. Skin factories (`CreateCard`, `CreateLiveHeader`, …) keep uplink chrome; primitives (`CreateSurface`, `CreateHeader`, `CreateDivider`) sit beside them.
