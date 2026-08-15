//------------------------------------------------------------------------------------------------
//! Plain title + optional kicker. No LIVE pill / T+ clock (use CreateLiveHeader for that).
//!
//! Consumer:
//!   MUI_Header h = runtime.CreateHeader("SETTINGS", "header");
//!   h.SetKicker("MY MOD");
//!
//! Layout:
//!   Fill width, Exact height 52.
//------------------------------------------------------------------------------------------------
class MUI_Header : MUI_Node
{
	protected string m_sKicker;
	protected string m_sTitle;

	//------------------------------------------------------------------------------------------------
	void MUI_Header()
	{
		m_Style.m_WidthMode = MUI_SizeMode.Fill;
		m_Style.m_HeightMode = MUI_SizeMode.Exact;
		m_Style.m_fHeight = 52;
		m_Style.m_fMinHeight = 52;
		m_Style.m_Fill = Color.FromInt(0);
	}

	//------------------------------------------------------------------------------------------------
	void SetKicker(string text)
	{
		m_sKicker = text;
		InvalidatePaint();
	}

	//------------------------------------------------------------------------------------------------
	void SetTitle(string text)
	{
		m_sTitle = text;
		InvalidateLayout();
	}

	//------------------------------------------------------------------------------------------------
	override void PaintForeground(MUI_RenderSurface surface)
	{
		float x = DrawX();
		float y = DrawY();
		float w = m_World.m_fW;
		float op = GetDrawOpacity();
		if (op < 0.01)
			return;

		MUI_ThemeData theme = GetTheme();
		if (m_sKicker != "")
			surface.DrawText(x, y, w, 16, m_sKicker, theme.FONT_SMALL, MUI_ColorUtil.Fade(theme.TextMuted, op), true, false, true, false);
		float titleY = y;
		if (m_sKicker != "")
			titleY = y + 16;
		surface.DrawText(x, titleY, w, 36, m_sTitle, theme.FONT_TITLE, MUI_ColorUtil.Fade(theme.Text, op), true, false, true, false);
	}
}
