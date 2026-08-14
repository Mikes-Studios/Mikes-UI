//------------------------------------------------------------------------------------------------
class MUI_Card : MUI_Panel
{
	//------------------------------------------------------------------------------------------------
	void MUI_Card()
	{
		m_Style.m_Layout = MUI_LayoutKind.StackVertical;
		m_Style.m_WidthMode = MUI_SizeMode.Exact;
		m_Style.m_HeightMode = MUI_SizeMode.Hug;
		m_Style.m_Fill = MUI_Theme.Deep;
		m_Style.m_fRadius = 18;
		m_Style.m_bBlockHit = true;
		m_Style.m_Stroke = Color.FromInt(0);
		m_Style.m_fBorder = 0;
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

		float glow = 0.12 + 0.08 * MUI_Ease.Pulse(GetTime(), 0.7);
		surface.FillRect(x - 10, y - 10, w + 20, h + 20, MUI_ColorUtil.Fade(MUI_Theme.Glow, op * glow), 24);
		surface.FillRect(x - 4, y - 4, w + 8, h + 8, MUI_ColorUtil.Fade(MUI_Theme.Cyan, op * 0.06), 20);

		surface.FillRect(x, y, w, h, MUI_ColorUtil.Fade(MUI_Theme.Deep, op), 18);
		surface.FillGradientV(x, y, w, 64, MUI_ColorUtil.Fade(MUI_Theme.Header, op), MUI_ColorUtil.Fade(MUI_Theme.Deep, op), 8);

		surface.FillRect(x, y, w, 3, MUI_ColorUtil.Fade(MUI_Theme.Accent, op), 0);
		surface.FillRect(x, y + 3, w, 1, MUI_ColorUtil.Fade(MUI_Theme.Cyan, op * 0.7), 0);

		float sweep = MUI_Ease.Fract(GetTime() * 0.22);
		float sx = x + sweep * (w + 120) - 80;
		surface.FillRect(sx, y, 70, 4, MUI_ColorUtil.Fade(MUI_Theme.Sheen, op), 0);

		float scanY = y + 8 + MUI_Ease.Fract(GetTime() * 0.18) * (h - 16);
		surface.FillRect(x + 10, scanY, w - 20, 10, MUI_ColorUtil.Fade(MUI_Theme.Scan, op), 0);

		surface.StrokeRect(x, y, w, h, MUI_ColorUtil.Fade(MUI_Theme.Border, op * 0.9), 1.4, 18);

		DrawBracket(surface, x + 10, y + 10, 18, 18, 1, 1, op);
		DrawBracket(surface, x + w - 10, y + 10, 18, 18, -1, 1, op);
		DrawBracket(surface, x + 10, y + h - 10, 18, 18, 1, -1, op);
		DrawBracket(surface, x + w - 10, y + h - 10, 18, 18, -1, -1, op);

		PaintForeground(surface);
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawBracket(MUI_RenderSurface surface, float x, float y, float arm, float thick, float dirX, float dirY, float op)
	{
		Color c = MUI_ColorUtil.Fade(MUI_Theme.Accent, op);
		surface.DrawLine(x, y, x + arm * dirX, y, c, 2.2);
		surface.DrawLine(x, y, x, y + arm * dirY, c, 2.2);
		surface.FillRect(x - 1, y - 1, 3, 3, MUI_ColorUtil.Fade(MUI_Theme.Cyan, op), 0);
	}
}
