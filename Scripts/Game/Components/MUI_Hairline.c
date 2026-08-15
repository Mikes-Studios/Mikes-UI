//------------------------------------------------------------------------------------------------
//! Accent / muted / cyan divider. Fill width, Exact height 12.
//! Consumer: runtime.CreateHairline("lineA"); parent.AddChild(line);
//------------------------------------------------------------------------------------------------
class MUI_Hairline : MUI_Node
{
	//------------------------------------------------------------------------------------------------
	void MUI_Hairline()
	{
		m_Style.m_WidthMode = MUI_SizeMode.Fill;
		m_Style.m_HeightMode = MUI_SizeMode.Exact;
		m_Style.m_fHeight = 12;
		m_Style.m_fMinHeight = 12;
		m_Style.m_Fill = Color.FromInt(0);
	}

	//------------------------------------------------------------------------------------------------
	override void PaintForeground(MUI_RenderSurface surface)
	{
		float x = DrawX();
		float y = DrawY() + m_World.m_fH * 0.5;
		float w = m_World.m_fW;
		float op = GetDrawOpacity();
		if (op < 0.01)
			return;
		MUI_ThemeData theme = GetTheme();
		surface.DrawLine(x, y, x + w * 0.08, y, MUI_ColorUtil.Fade(theme.Accent, op), 1.5);
		surface.DrawLine(x + w * 0.08, y, x + w * 0.92, y, MUI_ColorUtil.Fade(theme.Border, op * 0.7), 1);
		surface.DrawLine(x + w * 0.92, y, x + w, y, MUI_ColorUtil.Fade(theme.Cyan, op), 1.5);
	}
}
