//------------------------------------------------------------------------------------------------
class MUI_Style
{
	MUI_LayoutKind m_Layout = MUI_LayoutKind.StackVertical;
	MUI_SizeMode m_WidthMode = MUI_SizeMode.Fill;
	MUI_SizeMode m_HeightMode = MUI_SizeMode.Hug;

	float m_fWidth;
	float m_fHeight;
	float m_fMinWidth;
	float m_fMinHeight;
	float m_fMaxWidth;
	float m_fMaxHeight;

	float m_fPadL;
	float m_fPadT;
	float m_fPadR;
	float m_fPadB;
	float m_fGap;
	float m_fRadius;
	float m_fBorder;
	float m_fAlignX;
	float m_fAlignY;
	float m_fGrow;

	ref Color m_Fill;
	ref Color m_FillHover;
	ref Color m_FillPress;
	ref Color m_FillDisabled;
	ref Color m_Stroke;
	ref Color m_Text;
	ref Color m_TextMuted;

	int m_iFontSize;
	bool m_bBold;
	bool m_bClipChildren;
	bool m_bInteractive;
	bool m_bBlockHit;

	//------------------------------------------------------------------------------------------------
	void MUI_Style()
	{
		m_fMaxWidth = 100000;
		m_fMaxHeight = 100000;
		m_iFontSize = MUI_Theme.FONT_BODY;
		m_Fill = Color.FromInt(0);
		m_FillHover = Color.FromInt(0);
		m_FillPress = Color.FromInt(0);
		m_FillDisabled = Color.FromInt(0);
		m_Stroke = Color.FromInt(0);
		m_Text = new Color(MUI_Theme.Text.R(), MUI_Theme.Text.G(), MUI_Theme.Text.B(), MUI_Theme.Text.A());
		m_TextMuted = new Color(MUI_Theme.TextMuted.R(), MUI_Theme.TextMuted.G(), MUI_Theme.TextMuted.B(), MUI_Theme.TextMuted.A());
	}

	//------------------------------------------------------------------------------------------------
	void SetPadding(float value)
	{
		m_fPadL = value;
		m_fPadT = value;
		m_fPadR = value;
		m_fPadB = value;
	}

	//------------------------------------------------------------------------------------------------
	void SetPaddingTRBL(float top, float right, float bottom, float left)
	{
		m_fPadT = top;
		m_fPadR = right;
		m_fPadB = bottom;
		m_fPadL = left;
	}

	//------------------------------------------------------------------------------------------------
	bool HasFill()
	{
		if (!m_Fill)
			return false;
		return m_Fill.A() > 0.001;
	}
}
