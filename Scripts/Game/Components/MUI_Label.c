//------------------------------------------------------------------------------------------------
class MUI_Label : MUI_Node
{
	protected string m_sText;

	//------------------------------------------------------------------------------------------------
	void MUI_Label()
	{
		m_Style.m_WidthMode = MUI_SizeMode.Fill;
		m_Style.m_HeightMode = MUI_SizeMode.Hug;
		m_Style.m_iFontSize = MUI_Theme.FONT_BODY;
		m_Style.m_Text = MUI_Theme.Text;
		m_Style.m_bInteractive = false;
		m_Style.m_Fill = Color.FromInt(0);
	}

	//------------------------------------------------------------------------------------------------
	void SetText(string text)
	{
		m_sText = text;
		InvalidateLayout();
	}

	//------------------------------------------------------------------------------------------------
	string GetText()
	{
		return m_sText;
	}

	//------------------------------------------------------------------------------------------------
	void SetFontSize(int size)
	{
		m_Style.m_iFontSize = size;
		InvalidateLayout();
	}

	//------------------------------------------------------------------------------------------------
	void SetBold(bool bold)
	{
		m_Style.m_bBold = bold;
		InvalidatePaint();
	}

	//------------------------------------------------------------------------------------------------
	void SetMuted(bool muted)
	{
		if (muted)
			m_Style.m_Text = MUI_Theme.TextMuted;
		else
			m_Style.m_Text = MUI_Theme.Text;
		InvalidatePaint();
	}

	//------------------------------------------------------------------------------------------------
	override void MeasureIntrinsic(float availW, float availH)
	{
		float w;
		float h;
		if (m_Runtime)
			m_Runtime.MeasureText(m_sText, m_Style.m_iFontSize, m_Style.m_bBold, availW, w, h);
		else
		{
			w = 0;
			h = m_Style.m_iFontSize;
		}
		if (h < m_Style.m_iFontSize)
			h = m_Style.m_iFontSize;
		m_fDesiredW = w;
		m_fDesiredH = h + 4;
	}

	//------------------------------------------------------------------------------------------------
	override void PaintForeground(MUI_RenderSurface surface)
	{
		surface.DrawText(DrawX(), DrawY(), m_World.m_fW, m_World.m_fH, m_sText, m_Style.m_iFontSize, MUI_ColorUtil.Fade(m_Style.m_Text, GetDrawOpacity()), m_Style.m_bBold, false, true, true);
	}
}
