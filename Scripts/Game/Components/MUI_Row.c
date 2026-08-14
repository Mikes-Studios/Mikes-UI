//------------------------------------------------------------------------------------------------
class MUI_Row : MUI_Node
{
	//------------------------------------------------------------------------------------------------
	void MUI_Row()
	{
		m_Style.m_Layout = MUI_LayoutKind.StackHorizontal;
		m_Style.m_WidthMode = MUI_SizeMode.Fill;
		m_Style.m_HeightMode = MUI_SizeMode.Hug;
		m_Style.m_fGap = 10;
		m_Style.m_Fill = Color.FromInt(0);
	}
}
