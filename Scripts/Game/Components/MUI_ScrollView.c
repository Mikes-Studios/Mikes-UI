//------------------------------------------------------------------------------------------------
//! Clipped vertical stack with mouse-wheel scroll and a paint-time scrollbar.
//!
//! Consumer:
//!   MUI_ScrollView s = runtime.CreateScrollView("list");
//!   s.SetViewportHeight(420);      // exact, stable parent size — prefer for lists
//!   s.SetMaxViewportHeight(420);   // hug until max, then scroll
//!
//! Layout:
//!   Default Hug height + clip. Prefer SetViewportHeight for long data.
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
	void SetViewportHeight(float height)
	{
		if (height < 1)
			height = 1;
		m_Style.m_HeightMode = MUI_SizeMode.Exact;
		m_Style.m_fHeight = height;
		m_Style.m_fMinHeight = height;
		m_Style.m_fMaxHeight = height;
		InvalidateLayout();
	}

	//------------------------------------------------------------------------------------------------
	void SetMaxViewportHeight(float maxHeight)
	{
		if (maxHeight < 1)
			maxHeight = 1;
		m_Style.m_HeightMode = MUI_SizeMode.Hug;
		m_Style.m_fMaxHeight = maxHeight;
		InvalidateLayout();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMouseWheel(int wheel)
	{
		float next = m_fScrollY - wheel * 28;
		SetScrollY(next);
	}

	//------------------------------------------------------------------------------------------------
	override void Paint(MUI_RenderSurface surface)
	{
		float x = DrawX();
		float y = DrawY();
		float op = GetDrawOpacity();
		if (op < 0.01)
			return;

		MUI_ThemeData theme = GetTheme();
		surface.FillRect(x, y, m_World.m_fW, m_World.m_fH, MUI_ColorUtil.Fade(theme.Field, op * 0.55), 8);
		surface.StrokeRect(x, y, m_World.m_fW, m_World.m_fH, MUI_ColorUtil.Fade(theme.Border, op * 0.7), 1.0, 8);
		PaintForeground(surface);
	}

	//------------------------------------------------------------------------------------------------
	override void PaintForeground(MUI_RenderSurface surface)
	{
		float innerH = m_World.m_fH - m_Style.m_fPadT - m_Style.m_fPadB;
		if (m_fContentH <= innerH + 1)
			return;

		MUI_ThemeData theme = GetTheme();
		float op = GetDrawOpacity();
		float trackW = 4;
		float trackX = DrawX() + m_World.m_fW - trackW - 2;
		float trackY = DrawY() + 4;
		float trackH = m_World.m_fH - 8;
		surface.FillRect(trackX, trackY, trackW, trackH, MUI_ColorUtil.Fade(theme.Field, op), 0);

		float thumbH = trackH * (innerH / m_fContentH);
		if (thumbH < 18)
			thumbH = 18;
		float maxScroll = m_fContentH - innerH;
		float t = 0;
		if (maxScroll > 0)
			t = m_fScrollY / maxScroll;
		float thumbY = trackY + (trackH - thumbH) * t;
		surface.FillRect(trackX, thumbY, trackW, thumbH, MUI_ColorUtil.Fade(theme.Accent, op), 0);
	}
}
