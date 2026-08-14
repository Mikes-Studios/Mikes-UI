//------------------------------------------------------------------------------------------------
class MUI_Panel : MUI_Node
{
	//------------------------------------------------------------------------------------------------
	void MUI_Panel()
	{
		m_Style.m_Layout = MUI_LayoutKind.StackVertical;
		m_Style.m_WidthMode = MUI_SizeMode.Fill;
		m_Style.m_HeightMode = MUI_SizeMode.Hug;
		m_Style.m_Fill = MUI_Theme.Panel;
		m_Style.m_fRadius = 10;
		m_Style.m_bBlockHit = true;
	}

	//------------------------------------------------------------------------------------------------
	void MakeOverlay()
	{
		m_Style.m_Layout = MUI_LayoutKind.Overlay;
		m_Style.m_WidthMode = MUI_SizeMode.Fill;
		m_Style.m_HeightMode = MUI_SizeMode.Fill;
		m_Style.m_Fill = MUI_Theme.Overlay;
		m_Style.m_fRadius = 0;
		m_Style.m_bBlockHit = true;
		InvalidateLayout();
	}
}
