# Mike's UI

Code-defined UI for **Arma Reforger**. Add this addon as a Workshop dependency and build menus in **your** addon. Do not fork or edit this project to add screens.

Enfusion has no WebView. Mikes UI is a retained node graph. Enfusion widgets are only the compositor (Canvas + pooled text + a hidden EditBox for typing).

| | |
|---|---|
| Addon ID | `MikesUI` |
| GUID | `B3F91C6A4E275D08` |
| Version | 0.2.0 |
| Engine dependency | Arma Reforger `58D0FB3206B6F859` |
| License | GPL-2.0 |

---

## For AI assistants

Read this section before writing any UI.

**Never modify Mikes-UI to add a feature screen.** Create the menu class in the consuming addon. The only in-tree demo is `MUI_SampleMenu`.

**Every `Scripts/Game/**/*.c` file starts with a header** (`Consumer` / `Layout` / `Extend`). Read that header before calling into the class.

**Hard rules (Enforce):**

- No ternary (`?:`).
- Every `new` lives in a `ref` — member (`protected ref MUI_Button m_ok;`), local (`ref MUI_Panel overlay = ...`), or `ref array<ref T>`. Missing `ref` → GC → `NULL pointer ... Variable 'child'` in `AddChild`.
- Call `super` unless you are replacing the method.
- Colors: `Color.FromSRGBA(r,g,b,a)` (sRGB bytes). Never `new Color(0.93, …)` with sRGB floats.
- Paint with `DrawX()` / `DrawY()` / `GetDrawOpacity()`, not raw `m_World`.

**Default menu recipe:** subclass `MUI_MenuBase`, override `BuildUI`, `runtime.SetRoot(overlay)`.

**Default HUD recipe:** `MUI_HudHost.Open` + `MakePassThroughOverlay`. Never `MenuWithEditorContext` on HUD.

**Custom widget recipe:** `class MyThing : MUI_Node`, `ref MyThing t = new MyThing(); runtime.Adopt(t); parent.AddChild(t);`. Input: override `WantsTextInput` / `HandleNavAxis` / `HandleActivate` — do not edit `MUI_InputRouter`.

---

## Add as a dependency

In the consuming addon's `addon.gproj`:

```
GameProject {
 ID "MyMod"
 GUID "..."
 TITLE "My Mod"
 Dependencies {
  "58D0FB3206B6F859" "B3F91C6A4E275D08"
 }
}
```

Register a menu preset in **your** `Configs/System/chimeraMenus.conf`. Point `Layout` at this addon's blank frame — do not copy the layout file:

```
MenuManager {
 MenuPresets {
  MenuPreset MyMenu {
   Layout "{A1C47E92B6D8305F}UI/Layouts/MUI_BlankMenu.layout"
   Class "MyMenu"
  }
 }
}
```

And in **your** scripts:

```
modded enum ChimeraMenuPreset
{
	MyMenu
}
```

Open it with `GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.MyMenu)`.

The layout GUID is also `MUI_Resources.BLANK_MENU_LAYOUT`.

---

## Your first menu

```
class MyMenu : MUI_MenuBase
{
	protected ref MUI_Button m_Close;
	protected ref MUI_TextField m_Name;

	override string GetMUILogTag()
	{
		return "MyMenu";
	}

	override void BuildUI(notnull MUI_Runtime runtime)
	{
		ref MUI_Panel overlay = runtime.CreatePanel("overlay");
		overlay.MakeOverlay();
		overlay.GetStyle().m_Fill = Color.FromInt(0);

		ref MUI_FxBackdrop fx = runtime.CreateFxBackdrop("fx");

		ref MUI_Card card = runtime.CreateCard("card");
		card.SetWidth(560);
		card.SetPadding(28);
		card.SetGap(12);
		card.SetAlign(0.5, 0.5);
		card.SetIntro(0.06, 0.55, 46);

		ref MUI_LiveHeader header = runtime.CreateLiveHeader("MY MENU", "header");
		header.SetKicker("MY MOD  //  UPLINK");

		m_Name = runtime.CreateTextField("Callsign", "name");

		ref MUI_Row buttons = runtime.CreateRow("buttons");
		buttons.SetGap(12);

		m_Close = runtime.CreateButton("Close", "close");
		m_Close.MakeAccent();
		m_Close.GetOnClicked().Insert(OnMUIBack);
		buttons.AddChild(m_Close);

		card.AddChild(header);
		card.AddChild(m_Name);
		card.AddChild(buttons);

		overlay.AddChild(fx);
		overlay.AddChild(card);
		runtime.SetRoot(overlay);
	}
}
```

`MUI_MenuBase` already:

- Creates the host Frame, `Mount`s, hides leftover layout children
- Ticks the compositor and activates `MenuWithEditorContext`
- Binds Back to `OnMUIBack()` (default `Close()`)
- Blurs on focus lost, Unmounts on close

Override `OnMUIMountFailed()` only if you still have a vanilla layout fallback.

In-game demo (this addon loaded):

```
GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.MUI_SampleMenu);
```

---

## HUD overlay

```
protected ref MUI_HudHost m_Hud;

void Start()
{
	m_Hud = new MUI_HudHost();
	if (!m_Hud.Open(m_wRoot, "MyHud"))
		return;

	MUI_Runtime runtime = m_Hud.GetRuntime();
	ref MUI_Panel overlay = runtime.CreatePanel("overlay");
	overlay.MakePassThroughOverlay();
	// build toast / indicator tree
	runtime.SetRoot(overlay);
}

void Tick(float dt)
{
	if (m_Hud)
		m_Hud.Tick(dt);
}

void Stop()
{
	if (m_Hud)
		m_Hud.Close();
	m_Hud = null;
}
```

`MountPassive` paints without input, edit bridge, or Select/Back prompts. Host canvases use `IGNORE_CURSOR`. A canvas **without** that flag on an always-on-top HUD steals every mouse click from the game.

---

## Factories

All `Create*` methods `Retain` the node (GC root until `Unmount`).

| Factory | Typical use |
|---|---|
| `CreatePanel` | Root overlay (`MakeOverlay` / `MakePassThroughOverlay`) or generic stack |
| `CreateLabel` | Text. `SetFontSize` `SetBold` `SetMuted` `SetColor` |
| `CreateButton` | Click. `MakeAccent` `MakeDanger` `MakeDefault`. `GetOnClicked()` |
| `CreateToggle` | On/off. Gamepad left/right flips. `GetOnChanged()` |
| `CreateTextField` | String + IME. `GetOnChanged()` fires on `SetText`, not every IME tick |
| `CreateNumericField` | Float field. `SetRange` `SetStep` `SetDecimals` `SetValue` / `GetValue`. Left/right steps |
| `CreateScrollView` | Clipped list. **`SetViewportHeight(h)`** for stable parent size |
| `CreateRow` | Horizontal stack |
| `CreateSpacer` | Gap. `CreateSpacer(12)` |
| `CreateSurface` | Primitive panel. Honors `SetFill` / `SetRadius` / blur |
| `CreateHeader` | Title + optional kicker (no LIVE pill) |
| `CreateDivider` | 1px theme border line |
| `CreateCard` | Frosted command-uplink panel (blur + brackets). Paint ignores `SetFill` |
| `CreateFxBackdrop` | Radar / motes behind the card |
| `CreateLiveHeader` | Title + kicker + LIVE pill |
| `CreateHairline` | 3-segment uplink divider |
| `CreateImage` | Hosted `ImageWidget`. `SetImage("{GUID}path.edds")`. Clip-aware host |
| `CreateProgress` | Non-interactive 0..1 bar |
| `CreateSlider` | Drag + gamepad left/right. `SetRange` `SetValue` `GetOnChanged()` |
| `CreateTabs` | Segmented tab bar. `AddTab` `SetIndex` `GetOnChanged()` |
| `CreateDropdown` | In-flow list + overlay catcher. `AddItem` `SetIndex` `GetText` |
| `Adopt` | Your `MUI_Node` subclass |

---

## Layout cheat sheet

`MUI_SizeMode`: Fill, Hug, Exact. `MUI_LayoutKind`: Overlay, StackVertical, StackHorizontal.

- Main-axis Fill is **measured as Hug**, then leftover is split by `m_fGrow` (Fill with grow 0 counts as flex 1).
- Cross-axis Fill stretches.
- Overlay: Fill child sizes to the inner box; `SetAlign(ax, ay)` 0..1 positions it (`0.5, 0.5` = center).
- **Buttons in a row:** keep Hug width + Grow 1 (the default). Do **not** `SetFillWidth()` on those buttons — each would measure as 100% of the row.
- **Lists:** `scroll.SetViewportHeight(420)` so the card does not grow with content. `SetMaxViewportHeight` hugs until the cap, then scrolls.
- `SetPadding` / `SetPaddingTRBL` / `SetGap` / `SetGrow` / `SetMinWidth` / `SetMaxWidth`.

Layout space is unscaled pixels (`DPIUnscale`). Canvas vertices are screen pixels (`DPIScale`). You always draw in layout space; the surface scales.

---

## Custom widgets

```
class MyGauge : MUI_Node
{
	void MyGauge()
	{
		m_Style.m_WidthMode = MUI_SizeMode.Exact;
		m_Style.m_HeightMode = MUI_SizeMode.Exact;
		m_Style.m_fWidth = 64;
		m_Style.m_fHeight = 64;
		m_Style.m_bInteractive = true; // click + gamepad focus
	}

	override void PaintForeground(MUI_RenderSurface surface)
	{
		float op = GetDrawOpacity();
		surface.FillCircle(DrawX() + 32, DrawY() + 32, 24, MUI_ColorUtil.Fade(MUI_Theme.Accent, op));
	}

	override void OnClicked() { /* ... */ }

	override bool HandleNavAxis(int dirX, int dirY)
	{
		if (dirX == 0)
			return false;
		// consume left/right (same idea as MUI_Toggle)
		return true;
	}
}

// in BuildUI:
ref MyGauge g = new MyGauge();
runtime.Adopt(g);
parent.AddChild(g);
```

| Hook | When |
|---|---|
| `MeasureIntrinsic` | Hug size (labels, buttons) |
| `Paint` / `PaintForeground` | Draw. Use `DrawX/Y`, `GetDrawOpacity`, `PaintFocusRing` |
| `OnClicked` / `HandleActivate` | Click and gamepad Select |
| `WantsTextInput` + `GetEditText` + `SetEditTextFromBridge` | IME / console keyboard |
| `HandleNavAxis` | Consume d-pad instead of moving focus |
| `SyncHostWidgets` / `DestroyHostWidgets` | Extra Enfusion widgets (see `MUI_Card` blur) |
| `PaintsOnBackdropLayer` | Draw under card blur (`MUI_FxBackdrop`) |

Composite widgets (several stock controls, no new paint) do not need a `MUI_Node` subclass. Keep `protected ref` children and `AddChild` the row/panel. See Invade & Annex `IA_LeaderboardRow`.

---

## Input

Focusables = visible, enabled, `m_Style.m_bInteractive`. `m_bBlockHit` eats hits without being a focus target (overlays, cards).

- Mouse: hit-test, click, wheel on the nearest clip/scroll ancestor.
- Gamepad: `MenuUp/Down/Left/Right/Select/Back`. Up/down by Y; left/right by X in the same row.
- KBM: focusing a text field attaches the hidden EditBox immediately. Gamepad: Select attaches. The EditBox is `IGNORE_CURSOR` so focusing one field does not trap mouse hits; clicking another field switches the bridge.
- Back: stop editing first, else `GetOnBack()` (`OnMUIBack` on `MUI_MenuBase`).
- Prompts: `runtime.SetPromptText(selectRichText, backRichText)`.

---

## Look and color

Theme is **per `MUI_Runtime`**. Call `SetTheme` **before** `Create*` / `Adopt` (typically at the start of `BuildUI`). Hot-swap after `SetRoot` is not supported in 0.2.0.

```
ref MUI_ThemeData theme = MUI_ThemeData.CreateUplink();
theme.Accent = Color.FromSRGBA(80, 160, 90, 255);
runtime.SetTheme(theme);
```

`MUI_Theme` statics remain the uplink default palette and are fine for one-off `SetColor(MUI_Theme.Danger)`. Controls paint through `node.GetTheme()`, not `MUI_Theme.X`.

**Skin vs primitives:** `CreateCard` / `CreateLiveHeader` / `CreateFxBackdrop` / `CreateHairline` keep the command-uplink chrome (I&A-safe). New consumers can use `CreateSurface` / `CreateHeader` / `CreateDivider` for neutral panels.

`MUI_Card.Paint` ignores `SetFill` and draws frost, accent bar, brackets, and sweep from the runtime theme. `SetBlurEnabled(false)` / `SetBlurIntensity(0..1)` still work on Card/Surface.

Animation: `SetIntro(delay, duration, fromY)`. `fromY > 24` uses BackOut, else CubicOut. Runtime always paints while mounted (FX).

---

## File map

```
Scripts/Game/
  Host/       MUI_MenuBase, MUI_MenuHost, MUI_HudHost   ← start here
  Core/       Runtime, Node, Style, Theme, ThemeData, Resources
  Components/ Panel, Label, Button, Toggle, TextField, NumericField,
              ScrollView, Row, Spacer, Surface, Header, Divider,
              Card, FxBackdrop, LiveHeader, Hairline,
              Image, Progress, Slider, Tabs, Dropdown
  Layout/     Overlay / stacks / Fill-Hug-Exact
  Render/     Canvas + text pool
  Input/      Mouse, gamepad, EditBox bridge, drag
  Sample/     MUI_SampleMenu (catalog demo only)
UI/Layouts/MUI_BlankMenu.layout
```

---

## License

GNU GPL v2. If you ship a Workshop addon that includes or depends on this code, you must respect GPL-2. Talk to the author before treating this as an LGPL-style library.
