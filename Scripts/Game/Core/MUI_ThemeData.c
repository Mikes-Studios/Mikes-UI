//------------------------------------------------------------------------------------------------
//! Mutable per-runtime palette. Defaults match MUI_Theme (command-uplink).
//!
//! Consumer:
//!   ref MUI_ThemeData theme = MUI_ThemeData.CreateUplink();
//!   theme.Accent = Color.FromSRGBA(80, 160, 90, 255);
//!   runtime.SetTheme(theme);  // before BuildUI / Create*
//!   Colors: Color.FromSRGBA. Paint via node.GetTheme(), not MUI_Theme statics.
//!
//! Layout / look:
//!   SetTheme before creating widgets. Hot-swap after SetRoot is not supported in 0.2.0.
//------------------------------------------------------------------------------------------------
class MUI_ThemeData
{
	string FONT_REGULAR;
	string FONT_BOLD;
	int FONT_TITLE;
	int FONT_BODY;
	int FONT_SMALL;

	ref Color Overlay;
	ref Color Panel;
	ref Color PanelAlt;
	ref Color Deep;
	ref Color DeepFrost;
	ref Color Header;
	ref Color Field;
	ref Color FieldFocus;
	ref Color Border;

	ref Color Accent;
	ref Color AccentDark;
	ref Color Cyan;
	ref Color CyanDim;
	ref Color Live;
	ref Color Danger;
	ref Color DangerHover;

	ref Color Text;
	ref Color TextMuted;
	ref Color Button;
	ref Color ButtonHover;
	ref Color ButtonPress;
	ref Color ToggleOn;
	ref Color ToggleOff;

	ref Color Glow;
	ref Color Grid;
	ref Color Mote;
	ref Color Scan;
	ref Color Sheen;

	//------------------------------------------------------------------------------------------------
	void MUI_ThemeData()
	{
		FONT_REGULAR = MUI_Theme.FONT_REGULAR;
		FONT_BOLD = MUI_Theme.FONT_BOLD;
		FONT_TITLE = MUI_Theme.FONT_TITLE;
		FONT_BODY = MUI_Theme.FONT_BODY;
		FONT_SMALL = MUI_Theme.FONT_SMALL;

		Overlay = Color.FromSRGBA(5, 8, 10, 184);
		Panel = Color.FromSRGBA(23, 31, 36, 247);
		PanelAlt = Color.FromSRGBA(31, 41, 46, 255);
		Deep = Color.FromSRGBA(8, 12, 14, 245);
		DeepFrost = Color.FromSRGBA(10, 15, 20, 220);
		Header = Color.FromSRGBA(18, 28, 33, 255);
		Field = Color.FromSRGBA(13, 18, 20, 255);
		FieldFocus = Color.FromSRGBA(20, 28, 26, 255);
		Border = Color.FromSRGBA(71, 92, 87, 255);

		Accent = Color.FromSRGBA(237, 158, 41, 255);
		AccentDark = Color.FromSRGBA(158, 97, 20, 255);
		Cyan = Color.FromSRGBA(89, 235, 224, 255);
		CyanDim = Color.FromSRGBA(46, 140, 133, 255);
		Live = Color.FromSRGBA(89, 242, 115, 255);
		Danger = Color.FromSRGBA(184, 56, 41, 255);
		DangerHover = Color.FromSRGBA(219, 77, 51, 255);

		Text = Color.FromSRGBA(237, 240, 230, 255);
		TextMuted = Color.FromSRGBA(158, 168, 163, 255);
		Button = Color.FromSRGBA(41, 56, 61, 255);
		ButtonHover = Color.FromSRGBA(56, 77, 71, 255);
		ButtonPress = Color.FromSRGBA(237, 158, 41, 255);
		ToggleOn = Color.FromSRGBA(237, 158, 41, 255);
		ToggleOff = Color.FromSRGBA(46, 56, 59, 255);

		Glow = Color.FromSRGBA(237, 158, 41, 56);
		Grid = Color.FromSRGBA(51, 140, 128, 26);
		Mote = Color.FromSRGBA(237, 184, 71, 140);
		Scan = Color.FromSRGBA(89, 235, 224, 18);
		Sheen = Color.FromSRGBA(255, 255, 235, 46);
	}

	//------------------------------------------------------------------------------------------------
	static MUI_ThemeData CreateUplink()
	{
		ref MUI_ThemeData theme = new MUI_ThemeData();
		return theme;
	}
}
