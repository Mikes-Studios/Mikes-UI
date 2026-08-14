//------------------------------------------------------------------------------------------------
class MUI_LiveHeader : MUI_Node
{
	protected string m_sKicker;
	protected string m_sTitle;

	//------------------------------------------------------------------------------------------------
	void MUI_LiveHeader()
	{
		m_Style.m_WidthMode = MUI_SizeMode.Fill;
		m_Style.m_HeightMode = MUI_SizeMode.Exact;
		m_Style.m_fHeight = 58;
		m_Style.m_fMinHeight = 58;
		m_Style.m_Fill = Color.FromInt(0);
		m_sKicker = "COMMAND UPLINK";
		m_sTitle = "ADMIN CONFIG";
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
		float h = m_World.m_fH;
		float op = GetDrawOpacity();
		if (op < 0.01)
			return;

		surface.DrawText(x, y, w * 0.62, 16, m_sKicker, MUI_Theme.FONT_SMALL, MUI_ColorUtil.Fade(MUI_Theme.Cyan, op), true, false, true, false);
		surface.DrawText(x, y + 16, w * 0.7, 36, m_sTitle, 30, MUI_ColorUtil.Fade(MUI_Theme.Text, op), true, false, true, false);

		float pulse = 0.45 + 0.55 * MUI_Ease.Pulse(GetTime(), 1.4);
		float pillW = 78;
		float pillH = 22;
		float px = x + w - pillW;
		float py = y + 6;
		surface.FillRect(px, py, pillW, pillH, MUI_ColorUtil.Fade(MUI_Theme.Live, op * 0.16), 11);
		surface.StrokeRect(px, py, pillW, pillH, MUI_ColorUtil.Fade(MUI_Theme.Live, op * pulse), 1.2, 11);
		surface.FillCircle(px + 12, py + pillH * 0.5, 4, MUI_ColorUtil.Fade(MUI_Theme.Live, op * pulse));
		surface.DrawText(px + 20, py, pillW - 24, pillH, "LIVE", MUI_Theme.FONT_SMALL, MUI_ColorUtil.Fade(MUI_Theme.Live, op), true, false, true, false);

		float tNow = GetTime();
		int secs = tNow;
		string clock = "T+" + secs.ToString();
		surface.DrawText(x + w - 90, y + 30, 90, 20, clock, MUI_Theme.FONT_SMALL, MUI_ColorUtil.Fade(MUI_Theme.TextMuted, op), false, false, true, false);
	}
}
