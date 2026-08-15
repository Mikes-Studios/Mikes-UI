//------------------------------------------------------------------------------------------------
//! Non-interactive 0..1 progress bar.
//!
//! Consumer:
//!   MUI_Progress p = runtime.CreateProgress("hp");
//!   p.SetValue(0.65);
//!
//! Layout:
//!   Fill width, Exact height 10.
//------------------------------------------------------------------------------------------------
class MUI_Progress : MUI_Node
{
	protected float m_fValue;

	//------------------------------------------------------------------------------------------------
	void MUI_Progress()
	{
		m_Style.m_WidthMode = MUI_SizeMode.Fill;
		m_Style.m_HeightMode = MUI_SizeMode.Exact;
		m_Style.m_fHeight = 10;
		m_Style.m_fMinHeight = 10;
		m_Style.m_fRadius = 4;
		m_Style.m_Fill = Color.FromInt(0);
		m_Style.m_bInteractive = false;
		m_fValue = 0;
	}

	//------------------------------------------------------------------------------------------------
	void SetValue(float value)
	{
		if (value < 0)
			value = 0;
		if (value > 1)
			value = 1;
		if (m_fValue == value)
			return;
		m_fValue = value;
		InvalidatePaint();
	}

	//------------------------------------------------------------------------------------------------
	float GetValue()
	{
		return m_fValue;
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

		MUI_ThemeData theme = GetTheme();
		float rad = m_Style.m_fRadius;
		surface.FillRect(x, y, w, h, MUI_ColorUtil.Fade(theme.Field, op), rad);
		float fillW = w * m_fValue;
		if (fillW > 0.75)
			surface.FillRect(x, y, fillW, h, MUI_ColorUtil.Fade(theme.Accent, op), rad);
	}
}
