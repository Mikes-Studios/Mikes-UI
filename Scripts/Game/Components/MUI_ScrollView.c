//------------------------------------------------------------------------------------------------
class MUI_ScrollView : MUI_Node
{
	//------------------------------------------------------------------------------------------------
	void MUI_ScrollView()
	{
		m_Style.m_Layout = MUI_LayoutKind.StackVertical;
		m_Style.m_WidthMode = MUI_SizeMode.Fill;
		m_Style.m_HeightMode = MUI_SizeMode.Hug;
		m_Style.m_bClipChildren = true;
		m_Style.m_bBlockHit = true;
		m_Style.m_Fill = Color.FromInt(0);
		m_Style.m_fGap = 10;
	}

	//------------------------------------------------------------------------------------------------
	override void OnMouseWheel(int wheel)
	{
		float next = m_fScrollY - wheel * 28;
		SetScrollY(next);
	}

	//------------------------------------------------------------------------------------------------
	override void PaintForeground(MUI_RenderSurface surface)
	{
		float innerH = m_World.m_fH - m_Style.m_fPadT - m_Style.m_fPadB;
		if (m_fContentH <= innerH + 1)
			return;

		float op = GetDrawOpacity();
		float trackW = 4;
		float trackX = DrawX() + m_World.m_fW - trackW - 2;
		float trackY = DrawY() + 4;
		float trackH = m_World.m_fH - 8;
		surface.FillRect(trackX, trackY, trackW, trackH, MUI_ColorUtil.Fade(MUI_Theme.Field, op), 0);

		float thumbH = trackH * (innerH / m_fContentH);
		if (thumbH < 18)
			thumbH = 18;
		float maxScroll = m_fContentH - innerH;
		float t = 0;
		if (maxScroll > 0)
			t = m_fScrollY / maxScroll;
		float thumbY = trackY + (trackH - thumbH) * t;
		surface.FillRect(trackX, thumbY, trackW, thumbH, MUI_ColorUtil.Fade(MUI_Theme.Accent, op), 0);
	}
}
