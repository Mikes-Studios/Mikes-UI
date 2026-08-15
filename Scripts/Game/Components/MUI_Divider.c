//------------------------------------------------------------------------------------------------
//! 1px theme Border divider. For the 3-segment uplink line use CreateHairline.
//!
//! Consumer:
//!   parent.AddChild(runtime.CreateDivider("sep"));
//------------------------------------------------------------------------------------------------
class MUI_Divider : MUI_Node
{
	//------------------------------------------------------------------------------------------------
	void MUI_Divider()
	{
		m_Style.m_WidthMode = MUI_SizeMode.Fill;
		m_Style.m_HeightMode = MUI_SizeMode.Exact;
		m_Style.m_fHeight = 8;
		m_Style.m_fMinHeight = 8;
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
		surface.DrawLine(x, y, x + w, y, MUI_ColorUtil.Fade(GetTheme().Border, op), 1);
	}
}
