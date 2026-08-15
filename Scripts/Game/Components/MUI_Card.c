//------------------------------------------------------------------------------------------------
//! Command-uplink frosted card (brackets, accent bar, sweep). Colors from GetTheme().
//! Paint ignores SetFill for the skin chrome. For a plain panel use CreateSurface.
//!
//! Consumer:
//!   MUI_Card card = runtime.CreateCard("card");
//!   card.SetWidth(600); card.SetPadding(28); card.SetAlign(0.5, 0.5);
//------------------------------------------------------------------------------------------------
class MUI_Card : MUI_Surface
{
	//------------------------------------------------------------------------------------------------
	void MUI_Card()
	{
		m_Style.m_Fill = MUI_Theme.DeepFrost;
		m_Style.m_fRadius = 18;
		m_bBlurEnabled = true;
		m_fBlurIntensity = 0.72;
	}

	//------------------------------------------------------------------------------------------------
	override void ApplyTheme(notnull MUI_ThemeData theme)
	{
		m_Style.m_Fill = theme.DeepFrost;
	}

	//------------------------------------------------------------------------------------------------
	override void Paint(MUI_RenderSurface surface)
	{
		float x = DrawX();
		float y = DrawY();
		float w = m_World.m_fW;
		float h = m_World.m_fH;
		float op = GetDrawOpacity();
		if (op < 0.01)
			return;

		SyncHostWidgets();
		MUI_ThemeData theme = GetTheme();

		float glow = 0.12 + 0.08 * MUI_Ease.Pulse(GetTime(), 0.7);
		surface.FillRect(x - 10, y - 10, w + 20, h + 20, MUI_ColorUtil.Fade(theme.Glow, op * glow), 24);
		surface.FillRect(x - 4, y - 4, w + 8, h + 8, MUI_ColorUtil.Fade(theme.Cyan, op * 0.06), 20);

		Color fill = theme.DeepFrost;
		if (!m_bBlurEnabled)
			fill = theme.Deep;
		surface.FillRect(x, y, w, h, MUI_ColorUtil.Fade(fill, op), 18);
		surface.FillGradientV(x, y, w, 64, MUI_ColorUtil.Fade(theme.Header, op * 0.85), MUI_ColorUtil.Fade(fill, op), 8);

		surface.FillRect(x, y, w, 3, MUI_ColorUtil.Fade(theme.Accent, op), 0);
		surface.FillRect(x, y + 3, w, 1, MUI_ColorUtil.Fade(theme.Cyan, op * 0.7), 0);

		float sweep = MUI_Ease.Fract(GetTime() * 0.22);
		float sx = x + sweep * (w + 120) - 80;
		surface.FillRect(sx, y, 70, 4, MUI_ColorUtil.Fade(theme.Sheen, op), 0);

		surface.StrokeRect(x, y, w, h, MUI_ColorUtil.Fade(theme.Border, op * 0.9), 1.4, 18);

		DrawBracket(surface, x + 10, y + 10, 18, 18, 1, 1, op);
		DrawBracket(surface, x + w - 10, y + 10, 18, 18, -1, 1, op);
		DrawBracket(surface, x + 10, y + h - 10, 18, 18, 1, -1, op);
		DrawBracket(surface, x + w - 10, y + h - 10, 18, 18, -1, -1, op);

		PaintForeground(surface);
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawBracket(MUI_RenderSurface surface, float x, float y, float arm, float thick, float dirX, float dirY, float op)
	{
		MUI_ThemeData theme = GetTheme();
		Color c = MUI_ColorUtil.Fade(theme.Accent, op);
		surface.DrawLine(x, y, x + arm * dirX, y, c, 2.2);
		surface.DrawLine(x, y, x, y + arm * dirY, c, 2.2);
		surface.FillRect(x - 1, y - 1, 3, 3, MUI_ColorUtil.Fade(theme.Cyan, op), 0);
	}
}
