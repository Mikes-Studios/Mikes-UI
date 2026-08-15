//------------------------------------------------------------------------------------------------
//! Static uplink palette tokens. Convenience for one-off SetColor(MUI_Theme.Danger).
//!
//! Consumer:
//!   Prefer runtime.SetTheme(MUI_ThemeData.CreateUplink()) for full restyle.
//!   Built-in controls paint via node.GetTheme(), not these statics.
//!   Author colors with Color.FromSRGBA(r,g,b,a). Never new Color(sRGB floats).
//------------------------------------------------------------------------------------------------
class MUI_Theme
{
	static const string FONT_REGULAR = "{3E7733BAC8C831F6}UI/Fonts/RobotoCondensed/RobotoCondensed_Regular.fnt";
	static const string FONT_BOLD = "{EABA4FE9D014CCEF}UI/Fonts/RobotoCondensed/RobotoCondensed_Bold.fnt";

	static const int FONT_TITLE = 32;
	static const int FONT_BODY = 20;
	static const int FONT_SMALL = 16;

	// Surfaces
	static const ref Color Overlay = Color.FromSRGBA(5, 8, 10, 184);
	static const ref Color Panel = Color.FromSRGBA(23, 31, 36, 247);
	static const ref Color PanelAlt = Color.FromSRGBA(31, 41, 46, 255);
	static const ref Color Deep = Color.FromSRGBA(8, 12, 14, 245);
	static const ref Color DeepFrost = Color.FromSRGBA(10, 15, 20, 220);
	static const ref Color Header = Color.FromSRGBA(18, 28, 33, 255);
	static const ref Color Field = Color.FromSRGBA(13, 18, 20, 255);
	static const ref Color FieldFocus = Color.FromSRGBA(20, 28, 26, 255);
	static const ref Color Border = Color.FromSRGBA(71, 92, 87, 255);

	// Accents
	static const ref Color Accent = Color.FromSRGBA(237, 158, 41, 255);
	static const ref Color AccentDark = Color.FromSRGBA(158, 97, 20, 255);
	static const ref Color Cyan = Color.FromSRGBA(89, 235, 224, 255);
	static const ref Color CyanDim = Color.FromSRGBA(46, 140, 133, 255);
	static const ref Color Live = Color.FromSRGBA(89, 242, 115, 255);
	static const ref Color Danger = Color.FromSRGBA(184, 56, 41, 255);
	static const ref Color DangerHover = Color.FromSRGBA(219, 77, 51, 255);

	// Text / controls
	static const ref Color Text = Color.FromSRGBA(237, 240, 230, 255);
	static const ref Color TextMuted = Color.FromSRGBA(158, 168, 163, 255);
	static const ref Color Button = Color.FromSRGBA(41, 56, 61, 255);
	static const ref Color ButtonHover = Color.FromSRGBA(56, 77, 71, 255);
	static const ref Color ButtonPress = Color.FromSRGBA(237, 158, 41, 255);
	static const ref Color ToggleOn = Color.FromSRGBA(237, 158, 41, 255);
	static const ref Color ToggleOff = Color.FromSRGBA(46, 56, 59, 255);

	// FX (keep alpha intentional)
	static const ref Color Glow = Color.FromSRGBA(237, 158, 41, 56);
	static const ref Color Grid = Color.FromSRGBA(51, 140, 128, 26);
	static const ref Color Mote = Color.FromSRGBA(237, 184, 71, 140);
	static const ref Color Scan = Color.FromSRGBA(89, 235, 224, 18);
	static const ref Color Sheen = Color.FromSRGBA(255, 255, 235, 46);
}
