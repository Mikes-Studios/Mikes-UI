# Conflict UI replacement plan

Handoff for the next agent. Read this whole file before writing code.

**Verdict:** Mike's UI can own Conflict's **2D chrome** (score, tasks, briefing, deploy shell, commander panels, toasts, pause). It **cannot** replace map rendering, nametags, inventory simulation, optics, loading, or the engine MenuManager. Do not attempt a 100% HUD wipe.

**Where work goes:** almost all of it in the **consuming addon**, not this repo. Mikes-UI is a library.

---

## 0. Who you are and what you must not do

This repo (`F:\Mikes-UI`, addon ID `MikesUI`, GUID `B3F91C6A4E275D08`) is a retained-node UI toolkit. It is **not** a game mode and **not** a Conflict HUD pack.

1. Read [README.md](README.md) and [AGENTS.md](AGENTS.md) first.
2. **Do not add Conflict / I&A feature screens here.** The only in-tree menu is `MUI_SampleMenu`.
3. **Do not edit `MUI_InputRouter`** to special-case a widget.
4. **Do not copy `MUI_BlankMenu.layout`** into the consumer. Point at this addon's GUID.
5. Only change Mikes-UI if a **library gap** blocks the plan (new primitive / host). Feature UI stays in the consumer.

Enforce (hard):

- No ternary (`?:`).
- Every `new` lives in a `ref` (member, local, or `ref array<ref T>`). Missing `ref` → GC → `NULL pointer ... Variable 'child'` in `AddChild`.
- Call `super` unless you are replacing the method.
- Colors: `Color.FromSRGBA(r,g,b,a)`. Never `new Color(0.93, …)` with sRGB floats.
- Paint with `DrawX()` / `DrawY()` / `GetDrawOpacity()`, not raw `m_World`.
- Every `Scripts/Game/**/*.c` file starts with a Consumer / Layout / Extend header. Trust it.

---

## 1. Workspaces

| Path | Role |
|---|---|
| `F:\Mikes-UI` | Library. GUID `B3F91C6A4E275D08`. Blank layout `{A1C47E92B6D8305F}UI/Layouts/MUI_BlankMenu.layout` (`MUI_Resources.BLANK_MENU_LAYOUT`). |
| `F:\Mikes-Invade-and-Annex` | Current consumer. GUID `6556458885927F2F`. Depends on engine + MikesUI. **Not Conflict.** |
| `D:\ReforgerGameSources` | Unpacked vanilla. Conflict lives under `data\data007\`. |

Engine dependency GUID: `58D0FB3206B6F859`.

---

## 2. Decision you must make first

Vanilla Conflict is **Campaign**:

- Class: `SCR_GameModeCampaign`
- Prefab: `D:\ReforgerGameSources\data\data007\Prefabs\MP\Modes\Conflict\GameMode_Campaign.et`
- Player controller: `D:\ReforgerGameSources\data\data007\Prefabs\Characters\Core\DefaultPlayerControllerMP_Campaign.et` (`{D39FCAB93FC4FB02}`)
- HUD is **not** spawned by the GameMode class. Binding is data-driven.

I&A today:

- `F:\Mikes-Invade-and-Annex\Prefabs\GameMode_IA.et` inherits `GameMode_Base.et`, **not** Campaign.
- Conflict score HUD, Conflict task list, and faction commander chrome are **never mounted**.
- MUI is used for three screens only: `IA_AdminConfigMenu`, `IA_StatisticsMenu`, `IA_NotificationDisplay`.
- Vanilla deploy, pause, map, inventory, chat, nametags, building editor still run.

Pick **one** target. Do not mix them in the same prefab pass.

| Target | What to inherit / override | Meaning |
|---|---|---|
| **A. Vanilla Conflict look-swap** | GUID-override `GameMode_Campaign.et` + Campaign player controller in a Workshop addon that depends on MikesUI | Every Conflict mission using those prefabs gets the new HUD. Vanilla Conflict is no longer vanilla for those players. |
| **B. Custom Campaign scenario** | New GameMode prefab that inherits `GameMode_Campaign.et` | Cleaner. Vanilla Conflict stays vanilla. |
| **C. I&A only** | Stay on `GameMode_IA` / `GameMode_Base` | You are **not** replacing Conflict UI. You are adding I&A chrome. Skip Campaign InfoDisplays; they are not there. |

Recommended default: **B** (or C if the user only cares about I&A). Ask if unclear.

`EnableHUD(false)` is not a strategy. It hides InfoDisplays and leaves Map, Inventory, Pause, and Deploy open.

---

## 3. How vanilla Conflict UI is bound

Three injection points. Replace these; do not hunt for a `ConflictHUD` manager — there is none.

### 3.1 Player controller HUD (`DefaultPlayerControllerMP_Campaign.et`)

`SCR_HUDManagerComponent` InfoDisplays include:

| Display | Layout | Role |
|---|---|---|
| `SCR_TaskListDisplay` | `{3ED1A4A882FC4FD8}UI/layouts/Task/ConflictTaskList.layout` | Conflict task list (overrides generic TaskList) |
| `SCR_CampaignInfoDisplay` | `{47864BB47AB0B1F4}UI/layouts/HUD/CampaignMP/CampaignMainHUD.layout` | Flags / CPs / win timer |
| `SCR_CampaignMapInfoDisplay` | `{D74D24696C4F32F0}UI/layouts/HUD/CampaignMP/CampaignMenuScoreUI.layout` | Score + commander while map open (`ALWAYS_TOP`) |
| Popup InfoDisplay | `{8EF935F196AADE33}UI/layouts/Common/PopupUI.layout` | Toasts |

Also on that PC: `SCR_CampaignFactionCommanderPlayerComponent`, `SCR_CampaignFeedbackComponent`, `SCR_CampaignNetworkComponent`.

Shared MP HUD (weapon info, chat, VON, nametags, screen effects, hints, notifications log) comes from parent player-controller prefabs. Weapon info lives on the **character**, not the PC.

### 3.2 GameMode (`GameMode_Campaign.et`)

| Component | What it points at |
|---|---|
| `SCR_MapConfigComponent` | `{700E03AE8ED298ED}Configs/Map/MapFullscreenConflict.conf`, `{901F9ED2088BBCA4}Configs/Map/MapSpawnConflict.conf`, `{51AA01AB46AF6938}Configs/Map/MapEditorConflict.conf` |
| `SCR_TaskManagerUIComponent` | `{3EA8F170E2134F1B}UI/layouts/Tasks/ConflictCurrentTask.layout` |
| `SCR_WelcomeScreenComponent` / debrief | Welcome / debrief copy |
| `SCR_NametagConfigComponent` | `{09F7AAA14A642683}Configs/NameTags/NametagFriendlies.conf` |
| Journal | `{FFB1A12A7BFC4957}Configs/Journal/JournalConflict.conf` |
| `SCR_CampaignBuildingManagerComponent` | Free-roam building |
| `SCR_CampaignFactionCommanderHandlerComponent` | Commander + voting |

Map overlays are listed in those `.conf` files (`m_aUIComponents`), not in scripts.

### 3.3 MenuManager presets

Vanilla `D:\ReforgerGameSources\data\data007\Configs\System\chimeraMenus.conf`. Engine `MenuManager` is a **sealed proto**. You remap layout/class; you cannot delete Pause or Inventory.

| Preset | Class | Layout |
|---|---|---|
| `RespawnSuperMenu` | `SCR_DeployMenuMain` | `{1CFCE24DE17928EE}UI/layouts/Menus/DeployMenu/RespawnMenu.layout` |
| `WelcomeScreenMenu` | `SCR_WelcomeScreenMenu` | `{0B55E5823B2AF1B7}UI/layouts/Menus/DeployMenu/WelcomeScreenMenu.layout` |
| `DebriefingScreenMenu` | `SCR_DebriefingScreenMenu` | same welcome layout |
| `RoleSelectionDialog` | `SCR_RoleSelectionMenu` | `{74D65A35C9BA0671}UI/layouts/Menus/DeployMenu/RoleSelectionMenu.layout` |
| `PauseMenu` | `PauseMenuUI` | `{E81F33FD5F8C893B}UI/layouts/Menus/PauseMenu/pauseMenu.layout` |
| `MapMenu` | `SCR_MapMenuUI` | `{F87BB6B065B395D0}UI/layouts/Map/MapMenu.layout` |
| `CommandPostMapMenu` | `SCR_CommandPostMapMenuUI` | same MapMenu layout |
| `Inventory20Menu` | inventory UI | `{981ACF4F45EAECAE}UI/layouts/Menus/Inventory/InventoryMain.layout` |
| `EndgameScreen` | `GameOverScreenInput` | `{640F7941FB211DB9}UI/layouts/HUD/GameOver/EndScreen/EndScreenHolder.layout` |
| `PlayerListMenu` | `SCR_PlayerListMenu` | `{E3EB6A74D1CA3BBD}UI/layouts/Menus/PlayerList/PlayerListMenu.layout` |

Hardcoded opens (preset is immortal even if you restyle it):

- `game.c` → `OpenMenu(ChimeraMenuPreset.PauseMenu)`
- `SCR_RespawnSystemComponent` / `SCR_DeployMenuBase` → `RespawnSuperMenu`

Hardcoded layout **consts** (need `modded class`, not a prefab swap):

- `SCR_PopUpNotification.LAYOUT_NAME` = `{8EF935F196AADE33}UI/layouts/Common/PopupUI.layout`
- `SCR_SupplyInventorySlotUI.SLOT_LAYOUT_SUPPLY` = `{AF25F325D2730142}UI/layouts/Menus/Inventory/SupplyInventoryItemSlot.layout`

---

## 4. How Mike's UI injects (and how it does not)

MUI **overlays**. It does not globally hide vanilla UI.

| Host | Use | Open |
|---|---|---|
| `MUI_MenuBase` | Fullscreen / modal menus | Consumer `modded enum ChimeraMenuPreset` + `chimeraMenus.conf` pointing at blank layout. `OpenMenu(ChimeraMenuPreset.X)`. |
| `MUI_HudHost` | Always-on HUD | `SCR_InfoDisplayExtended` on the player controller. `Open(m_wRoot)`, `Tick` in `DisplayUpdate`, `Close` in `DisplayStopDraw`. |

HUD rules (breaking these steals world clicks):

- `MakePassThroughOverlay()` on the overlay root.
- `MUI_HudHost` already sets `IGNORE_CURSOR`.
- **Never** `ActivateContext("MenuWithEditorContext")` on HUD.
- **Never** `MUI_MenuHost` / `MenuWithEditorContext` on an `ALWAYS_TOP` HUD.

`HideLegacyChildren()` only hides siblings **inside that one host Frame**. Other InfoDisplays keep drawing. To suppress vanilla Conflict HUD you **remove or disable those InfoDisplays on the player-controller prefab**, then mount MUI replacements.

I&A HUD pattern to copy: `F:\Mikes-Invade-and-Annex\Scripts\Game\IA_NotificationDisplay.c` + entry on `Prefabs\Characters\Core\DefaultPlayerControllerMP.et`.

I&A menu chrome to copy (consumer-side, do not move into Mikes-UI unless asked): `F:\Mikes-Invade-and-Annex\Scripts\Game\UI\IA_MuiShell.c`.

I&A already-registered presets (`F:\Mikes-Invade-and-Annex\Configs\System\chimeraMenus.conf`): `IA_StatisticsMenu`, `IA_AdminConfigMenu`, both using the blank layout GUID.

---

## 5. Toolkit coverage vs Conflict

**Have (build these in the consumer):** panel, label, button, toggle, text/numeric field, scroll, row, spacer, surface, header, divider, card, fx backdrop, live header, hairline, image, progress, slider, tabs, dropdown. Custom `MUI_Node` via `runtime.Adopt`.

**Do not have (do not invent in Canvas):** MapWidget host, world-space nametags, inventory grid/drag, chat, analog gauges, radial menus, VON overlay. Hosted native widgets today: `BlurWidget`, `ImageWidget`, hidden `EditBox`.

Theme: `runtime.SetTheme` **before** `Create*` / `Adopt`. Hot-swap after `SetRoot` is not supported in 0.2.0. Skin factories (`CreateCard`, `CreateLiveHeader`, …) keep uplink chrome; primitives (`CreateSurface`, `CreateHeader`, `CreateDivider`) are the neutral path.

---

## 6. Surface inventory

Legend:

- **MUI 2D** — replace with `MUI_MenuBase` or `MUI_HudHost`. Keep vanilla simulation events.
- **Wrap** — keep the native subsystem; restyle chrome or host it beside MUI.
- **Locked** — engine. Skin/config only, or leave.

### 6.1 MUI 2D (do these)

| Surface | Vanilla | Spawn | Replacement |
|---|---|---|---|
| Match score / flags / win timer | `SCR_CampaignInfoDisplay` | Campaign PC HUD | MUI HUD bound to `SCR_GameModeCampaign` CPs / victory |
| Map score + commander name | `SCR_CampaignMapInfoDisplay` | HUD `ALWAYS_TOP` while map open | Same HUD, visibility gated on map open |
| Rank / XP bar | `SCR_XPInfoDisplay` | MP PC HUD | `CreateProgress` + labels |
| Conflict task list | `SCR_TaskListDisplay` + ConflictTaskList.layout | Campaign PC | MUI scroll + rows; keep task events |
| Current task hint | `SCR_ConflictCurrentTaskUIComponent` | GameMode `SCR_TaskManagerUIComponent` | MUI HUD strip (same idea as I&A toasts) |
| Welcome / briefing | `SCR_WelcomeScreenMenu` | `WelcomeScreenMenu` | `MUI_MenuBase`; remap preset |
| Debrief | `SCR_DebriefingScreenMenu` | `DebriefingScreenMenu` | `MUI_MenuBase` |
| Role selection | `SCR_RoleSelectionMenu` | `RoleSelectionDialog` | MUI cards / dropdown |
| Game over | `SCR_GameOverScreenManagerComponent` | `EndgameScreen` | `MUI_MenuBase`; keep victory logic |
| Seizing / radio / cohesion popups | `SCR_PopUpNotification` + `SCR_CampaignFeedbackComponent` | Hardcoded `LAYOUT_NAME` + Campaign PC | `modded class` const **or** disable vanilla popups and route copy into MUI toasts |
| Pause | `PauseMenuUI` | `game.c` always opens `PauseMenu` | Remap preset Class/Layout to `MUI_MenuBase`. Cannot delete the preset. |
| Player list | `SCR_PlayerListMenu` | Dialog | MUI menu. I&A leaderboard is **not** this list. |
| Group menu | `SCR_GroupMenu` | Dialog | `MUI_MenuBase` wrapping group manager API |
| Journal | `SCR_JournalConfig` | Map / deploy | MUI scroll of journal entries |
| Hints + notifications log | `SCR_HintInfoDisplay` / `SCR_NotificationsLogDisplay` | Shared MP HUD | Disable vanilla displays; route into MUI HUD |
| Faction / side picker chrome | `SCR_FactionRequestUIComponent` | Nested in RespawnMenu.layout | MUI buttons/dropdown |
| Respawn timer readout | Inside deploy | `SCR_MenuSpawnLogic` | MUI HUD label/progress |

### 6.2 Wrap vanilla (do not rewrite)

| Surface | Vanilla | What to keep | What MUI may do |
|---|---|---|---|
| Deploy / respawn | `SCR_DeployMenuMain` | Spawn `MapEntity` + spawn-point logic | MUI chrome, faction/loadout/group lists |
| Fullscreen map overlays | `SCR_MapCampaignUI` + `BaseElement.layout` | `MapEntity` terrain/tiles/descriptors | Thin `MapFullscreenConflict.conf`; optional MUI frame |
| Command post / logistics / ops maps | station map configs | Same MapEntity | MUI panels around the map |
| Faction commander radial | `SCR_MapFactionCommanderRadialMenu` + handlers under `scripts\Game\FactionCommander\MenuHandlers\` | Command simulation | Optional MUI command panel instead of radial |
| Free-roam building | `SCR_CampaignBuilding*` editor | 3D editor, outlines, placing | Optional MUI supply/budget strip |
| Supply slots in inventory | `SCR_SupplyInventorySlotUI` | Inventory grid | `modded class` layout const only |
| Chat + VON | `SCR_ChatHud` / `SCR_VonDisplay` | Native | Leave or light skin |
| Weapon / vehicle HUD | `SCR_WeaponInfo` + vehicle gauges | Character/vehicle prefabs | Leave. No analog-gauge primitive. |
| Inventory menu | `SCR_InventoryMenuUI` | Drag-drop / simulation | GUID skin at most |

### 6.3 Engine-locked (never rebuild in MUI)

- Map terrain / tiles / descriptors (`MapEntity`)
- World-space nametags (`SCR_NameTagDisplay`)
- `MenuManager` / `Widget` (sealed proto)
- Loading / splash
- 2D optics / PIP sights
- Inventory simulation / drag-drop
- Pause **open path** from `game.c` (you restyle PauseMenu; you cannot stop Escape opening it)

---

## 7. Implementation plan (consumer addon)

Do not start with pause or inventory. Order is chosen so each phase is playable.

### Phase 0 — Scaffold (do this first)

1. Confirm target A / B / C from section 2.
2. Consumer `addon.gproj` already has `"58D0FB3206B6F859" "B3F91C6A4E275D08"` (I&A does).
3. **Target A/B:** inherit `GameMode_Campaign.et` + `DefaultPlayerControllerMP_Campaign.et` (or GUID-override them). Do not start from I&A Base if you need Conflict simulation.
4. **Target C:** keep `GameMode_IA`; skip every Campaign-only row in §6.1.
5. Add consumer `modded enum ChimeraMenuPreset` entries **only for new presets**. Remapping an existing preset (`PauseMenu`, `RespawnSuperMenu`, …) is done in **your** `Configs/System/chimeraMenus.conf` by re-declaring that preset name with a new Class/Layout — do not invent a second Pause enum value.
6. Shared consumer chrome (optional): follow `IA_MuiShell` in the consumer. Do not add it to Mikes-UI.

### Phase 1 — HUD takeover (Conflict 2D)

On the Campaign (or I&A) player controller:

1. Remove or disable:
   - `SCR_CampaignInfoDisplay`
   - `SCR_CampaignMapInfoDisplay`
   - Conflict `SCR_TaskListDisplay` (the one with ConflictTaskList.layout)
   - Popup InfoDisplay using `PopupUI.layout` if you are replacing toasts
2. Add one or more `SCR_InfoDisplayExtended` subclasses that own `MUI_HudHost`.
3. Suggested first HUD widgets (one host, multiple nodes is fine):
   - Match score / CP / timer (Target A/B only)
   - Current task / AO strip
   - Respawn timer
   - Toasts (I&A already has `IA_NotificationDisplay` — extend it rather than adding a second toast host)
4. Bind to **existing** Campaign / I&A events. Do not reimplement capture, tasks, or respawn.
5. Tick: `DisplayUpdate` → `m_Hud.Tick(timeSlice)`. Close in `DisplayStopDraw`.
6. Layer: `ALWAYS_TOP` for toasts/score-on-map; `MEDIUM` for in-world HUD. Pass-through overlay only.

### Phase 2 — Menus that Conflict actually opens

Remap in consumer `chimeraMenus.conf`. Layout for every MUI menu:

```
Layout "{A1C47E92B6D8305F}UI/Layouts/MUI_BlankMenu.layout"
```

Order:

1. **Welcome / debrief / role select** — data already on GameMode welcome/debrief components. I&A already has welcome content.
2. **Pause** — remap `PauseMenu` Class to a `MUI_MenuBase`. Keep Settings / Field Manual / Leave as actions that open the **existing** vanilla presets (or leave those submenus vanilla). I&A currently injects two extra buttons into vanilla pause (`IA_PauseMenuStatsButtonComponent`, `IA_PauseMenuAdminButtonComponent`) — fold those into the MUI pause instead of stacking both.
3. **Player list** — optional; I&A stats menu is separate (`IA_StatisticsMenu`).
4. **Deploy shell** — remap `RespawnSuperMenu` Class to a consumer `MUI_MenuBase` **only after** you have a plan for the spawn map. The spawn map is `MapEntity` via `MapSpawnConflict.conf`. Options:
   - **Keep vanilla deploy** until Phase 3 (recommended).
   - Or MUI lists (faction / loadout / group) + still call `SCR_MapEntity.OpenMap` with the spawn config underneath / beside the overlay.

Do **not** remap `Inventory20Menu` or `MapMenu` in this phase.

### Phase 3 — Map chrome, not the map

1. Duplicate / inherit `MapFullscreenConflict.conf` (and spawn/editor/station configs) into the consumer.
2. Point GameMode `SCR_MapConfigComponent` at the copies.
3. Remove overlay widgets you are replacing with MUI (task wrapper, campaign score strip if HUD already shows it).
4. Keep `SCR_MapCampaignUI` / base icons / spawn points / logistics — those are MapEntity overlays, not Canvas.
5. Commander: keep vanilla radial **or** add a MUI command panel that calls the same handler APIs under `scripts\Game\FactionCommander\MenuHandlers\`. Do not rewrite commander simulation.

### Phase 4 — Only if still needed

- `modded class SCR_PopUpNotification` to no-op or retarget if Phase 1 did not fully kill vanilla popups.
- `modded class SCR_SupplyInventorySlotUI` only to change `SLOT_LAYOUT_SUPPLY`.
- Building: optional MUI supply/budget readout. Leave `EditorModeBuilding` and placing dialogs.
- Chat / VON / weapon / vehicle / nametags / optics / loading: **leave**.

---

## 8. Recipes (copy these)

### 8.1 New fullscreen menu (consumer)

```
modded enum ChimeraMenuPreset
{
	MyConflictMenu
}
```

`Configs/System/chimeraMenus.conf` (consumer):

```
MenuManager {
 MenuPresets {
  MenuPreset MyConflictMenu {
   Layout "{A1C47E92B6D8305F}UI/Layouts/MUI_BlankMenu.layout"
   Class "MyConflictMenu"
  }
 }
}
```

```
class MyConflictMenu : MUI_MenuBase
{
	protected ref MUI_Button m_Close;

	override string GetMUILogTag()
	{
		return "MyConflictMenu";
	}

	override void BuildUI(notnull MUI_Runtime runtime)
	{
		ref MUI_Panel overlay = runtime.CreatePanel("overlay");
		overlay.MakeOverlay();
		overlay.GetStyle().m_Fill = Color.FromInt(0);

		ref MUI_Card card = runtime.CreateCard("card");
		card.SetWidth(560);
		card.SetPadding(28);
		card.SetGap(12);
		card.SetAlign(0.5, 0.5);
		card.SetIntro(0.06, 0.55, 46);

		m_Close = runtime.CreateButton("Close", "close");
		m_Close.MakeAccent();
		m_Close.GetOnClicked().Insert(OnMUIBack);

		card.AddChild(m_Close);
		overlay.AddChild(card);
		runtime.SetRoot(overlay);
	}
}
```

Open: `GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.MyConflictMenu);`

### 8.2 Remap an existing vanilla preset (pause example)

In the **consumer** `chimeraMenus.conf`, re-declare the vanilla preset name:

```
MenuPreset PauseMenu {
 Layout "{A1C47E92B6D8305F}UI/Layouts/MUI_BlankMenu.layout"
 ActionContext "MenuWithEditorContext"
 Class "MyPauseMenu"
}
```

`class MyPauseMenu : MUI_MenuBase`. Escape still opens `ChimeraMenuPreset.PauseMenu`; it now instantiates your class.

### 8.3 HUD host (consumer InfoDisplay)

```
[BaseContainerProps()]
class MyConflictHud : SCR_InfoDisplayExtended
{
	protected ref MUI_HudHost m_Hud;

	override void DisplayStartDraw(IEntity owner)
	{
		super.DisplayStartDraw(owner);
		m_Hud = new MUI_HudHost();
		if (!m_Hud.Open(m_wRoot, "MyConflictHud"))
			return;

		MUI_Runtime runtime = m_Hud.GetRuntime();
		ref MUI_Panel overlay = runtime.CreatePanel("overlay");
		overlay.MakePassThroughOverlay();
		// build tree…
		runtime.SetRoot(overlay);
		m_bIsEnabled = true;
	}

	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		super.DisplayUpdate(owner, timeSlice);
		if (m_Hud)
			m_Hud.Tick(timeSlice);
	}

	override void DisplayStopDraw(IEntity owner)
	{
		if (m_Hud)
			m_Hud.Close();
		m_Hud = null;
		super.DisplayStopDraw(owner);
	}
}
```

Add that class as an InfoDisplay on the player-controller prefab (`m_eLayer`, `m_LayoutPath` can be a tiny empty/IA layout used only as the host root — I&A uses `{BEDCCD2C6F460736}UI/layouts/HUD/IA/IA_NotificationLayout.layout`).

### 8.4 Custom widget

```
class MyGauge : MUI_Node
{
	void MyGauge()
	{
		m_Style.m_WidthMode = MUI_SizeMode.Exact;
		m_Style.m_HeightMode = MUI_SizeMode.Exact;
		m_Style.m_fWidth = 64;
		m_Style.m_fHeight = 64;
	}

	override void PaintForeground(MUI_RenderSurface surface)
	{
		float op = GetDrawOpacity();
		surface.FillCircle(DrawX() + 32, DrawY() + 32, 24, MUI_ColorUtil.Fade(GetTheme().Accent, op));
	}
}

// in BuildUI:
ref MyGauge g = new MyGauge();
runtime.Adopt(g);
parent.AddChild(g);
```

Keep `protected ref MyGauge m_Gauge;` if you hold it after `BuildUI`.

---

## 9. Files you will touch

### Never (unless a library gap is proven)

Anything under `F:\Mikes-UI\Scripts` except a documented missing primitive. Do not add `MUI_ConflictScoreHud`.

### Consumer (I&A or new Conflict overlay)

| File | Why |
|---|---|
| `addon.gproj` | MikesUI already in I&A deps |
| `Configs/System/chimeraMenus.conf` | New presets + remaps |
| `Scripts/Game/*ChimeraMenuPresets*.c` | `modded enum` for **new** presets only |
| Player controller `.et` | Strip Campaign InfoDisplays; add MUI HUD displays |
| GameMode `.et` | Map config paths, task hint layout, welcome/debrief |
| New `*_Menu.c` / `*_Hud.c` | All feature UI |
| Optional map `.conf` copies | Phase 3 |
| Optional `modded class` of `SCR_PopUpNotification` / `SCR_SupplyInventorySlotUI` | Phase 4 only |

### Vanilla reference (read-only)

- `D:\ReforgerGameSources\data\data007\scripts\Game\GameMode\SCR_GameModeCampaign.c`
- `D:\ReforgerGameSources\data\data007\scripts\Game\UI\HUD\` (campaign displays, popup)
- `D:\ReforgerGameSources\data\data007\scripts\Game\UI\Menu\DeployMenu\`
- `D:\ReforgerGameSources\data\data007\scripts\Game\UI\Conflict\`
- `D:\ReforgerGameSources\data\data007\scripts\Game\FactionCommander\MenuHandlers\`
- Prefabs listed in §3

---

## 10. What success looks like

A player in Conflict (target A/B) or I&A (target C) sees Mike's uplink chrome for score/tasks/briefing/pause/toasts. They still use:

- Native map for terrain and base icons
- Native inventory
- Native nametags
- Native weapon/vehicle HUD
- Native building editor (if FRB is on)
- Native chat / VON

Vanilla Campaign InfoDisplays are gone from the PC prefab so you do not get double scoreboards.

---

## 11. Verification checklist

- [ ] Mikes-UI still has no feature screens beyond `MUI_SampleMenu`.
- [ ] Every consumer menu uses the blank layout GUID, not a copied `.layout`.
- [ ] HUD canvases use `IGNORE_CURSOR` / `MakePassThroughOverlay` — mouse clicks still hit the world.
- [ ] HUD does not activate `MenuWithEditorContext`.
- [ ] Every `new` in a `ref`; no ternary.
- [ ] `SetTheme` (if any) runs before `Create*` / `Adopt`.
- [ ] Campaign / I&A simulation still runs with MUI HUD disabled (menus closed, HUD host failed) — UI is not the game mode.
- [ ] Pause still opens on Escape after remap.
- [ ] Deploy still selects a spawn point (MapEntity) after any deploy chrome work.
- [ ] Inventory, nametags, optics, chat still work.
- [ ] No second toast HUD stacked on `IA_NotificationDisplay` without removing the old one.

---

## 12. Library gaps (only then edit Mikes-UI)

Open a Mikes-UI change **only** if Phase 2–3 is blocked by a missing primitive. Candidates, not commitments:

- Hosting a native `MapWidget` under `SyncHostWidgets` (no example exists today).
- A pass-through “world click vs HUD hit” helper if `IGNORE_CURSOR` is too blunt for an interactive map-adjacent panel.

Do not add score/task/commander widgets to the library. Those are consumer screens.

---

## 13. Prior research

This plan was written from a read of Campaign prefabs/scripts, Mikes-UI 0.2.0 hosts, and I&A's three MUI screens. If vanilla GUIDs disagree with the unpacked dump, trust the `.et` / `.conf` on disk over this file and update this document.
