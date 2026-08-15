//------------------------------------------------------------------------------------------------
//! Wrapped text. Hug height from MeasureText.
//!
//! Consumer:
//!   MUI_Label l = runtime.CreateLabel("Hello", "hello");
//!   l.SetFontSize(MUI_Theme.FONT_SMALL);
//!   l.SetBold(true); l.SetMuted(true); l.SetColor(MUI_Theme.Danger);
//!
//! Layout:
//!   Fill width, Hug height. In a row with fixed columns, SetWidth(n) then optionally
//!   SetFillWidth + SetGrow(1) on the flexible column only.
//------------------------------------------------------------------------------------------------
class MUI_Label : MUI_Node
{
	protected string m_sText;
	protected bool m_bMuted;

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
	override void ApplyTheme(notnull MUI_ThemeData theme)
	{
		m_Style.m_iFontSize = theme.FONT_BODY;
		if (m_bMuted)
			m_Style.m_Text = theme.TextMuted;
		else
			m_Style.m_Text = theme.Text;
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
		InvalidateLayout();
	}

	//------------------------------------------------------------------------------------------------
	void SetMuted(bool muted)
	{
		m_bMuted = muted;
		MUI_ThemeData theme = GetTheme();
		if (muted)
			m_Style.m_Text = theme.TextMuted;
		else
			m_Style.m_Text = theme.Text;
		InvalidatePaint();
	}

	//------------------------------------------------------------------------------------------------
	void SetColor(Color color)
	{
		m_bMuted = false;
		m_Style.m_Text = color;
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
		m_fDesiredH = h + 8;
	}

	//------------------------------------------------------------------------------------------------
	override void PaintForeground(MUI_RenderSurface surface)
	{
		surface.DrawText(DrawX(), DrawY(), m_World.m_fW, m_World.m_fH, m_sText, m_Style.m_iFontSize, MUI_ColorUtil.Fade(m_Style.m_Text, GetDrawOpacity()), m_Style.m_bBold, false, false, true);
	}
}
