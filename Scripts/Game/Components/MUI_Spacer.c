//------------------------------------------------------------------------------------------------
//! Invisible layout gap. Prefer this over empty panels when stacking controls.
//!
//! Consumer:
//!   MUI_Spacer s = runtime.CreateSpacer(12);
//!   parent.AddChild(s);
//!   Horizontal gap in a row: spacer.SetWidth(16); spacer.SetHugWidth(); spacer.SetFillHeight();
//!
//! Layout:
//!   Default Fill width, Exact height (the CreateSpacer argument). Not interactive.
//------------------------------------------------------------------------------------------------
class MUI_Spacer : MUI_Node
{
	//------------------------------------------------------------------------------------------------
	void MUI_Spacer()
	{
		m_Style.m_WidthMode = MUI_SizeMode.Fill;
		m_Style.m_HeightMode = MUI_SizeMode.Exact;
		m_Style.m_fHeight = 8;
		m_Style.m_fMinHeight = 0;
		m_Style.m_Fill = Color.FromInt(0);
		m_Style.m_bInteractive = false;
		m_Style.m_bBlockHit = false;
	}
}
