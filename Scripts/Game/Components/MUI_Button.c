//------------------------------------------------------------------------------------------------
//! Clickable text control. GetOnClicked().Insert(handler).
//!
//! Consumer:
//!   MUI_Button b = runtime.CreateButton("Save", "save");
//!   b.MakeAccent(); // or MakeDanger() / MakeDefault()
//!   b.GetOnClicked().Insert(OnSave);
//!   row.AddChild(b);
//!
//! Layout:
//!   Hug width, Exact height 44, Grow 1 (equal share in a row).
//!   Do NOT SetFillWidth() in a row — each button would measure as 100% of the row.
//------------------------------------------------------------------------------------------------
class MUI_Button : MUI_Node
{
	protected string m_sText;
	protected ref ScriptInvoker m_OnClicked;
	protected bool m_bAccent;
	protected bool m_bDanger;

	//------------------------------------------------------------------------------------------------
	void MUI_Button()
	{
		m_OnClicked = new ScriptInvoker();
		m_Style.m_WidthMode = MUI_SizeMode.Hug;
		m_Style.m_HeightMode = MUI_SizeMode.Exact;
		m_Style.m_fHeight = 44;
		m_Style.m_fMinHeight = 44;
		m_Style.m_fMinWidth = 88;
		m_Style.m_fRadius = 10;
		m_Style.m_fGrow = 1;
		m_Style.m_bInteractive = true;
		m_Style.m_Fill = MUI_Theme.Button;
		m_Style.m_FillHover = MUI_Theme.ButtonHover;
		m_Style.m_FillPress = MUI_Theme.ButtonPress;
		m_Style.m_Text = MUI_Theme.Text;
		m_Style.m_iFontSize = MUI_Theme.FONT_BODY;
		m_Style.m_bBold = true;
	}

	//------------------------------------------------------------------------------------------------
	override void ApplyTheme(notnull MUI_ThemeData theme)
	{
		m_Style.m_iFontSize = theme.FONT_BODY;
		m_Style.m_Text = theme.Text;
		if (m_bDanger)
		{
			m_Style.m_Fill = theme.Danger;
			m_Style.m_FillHover = theme.DangerHover;
			m_Style.m_FillPress = theme.Accent;
		}
		else if (m_bAccent)
		{
			m_Style.m_Fill = theme.AccentDark;
			m_Style.m_FillHover = theme.Accent;
			m_Style.m_FillPress = theme.Accent;
		}
		else
		{
			m_Style.m_Fill = theme.Button;
			m_Style.m_FillHover = theme.ButtonHover;
			m_Style.m_FillPress = theme.ButtonPress;
		}
	}

	//------------------------------------------------------------------------------------------------
	void SetText(string text)
	{
		m_sText = text;
		InvalidatePaint();
	}

	//------------------------------------------------------------------------------------------------
	string GetText()
	{
		return m_sText;
	}

	//------------------------------------------------------------------------------------------------
	void MakeDanger()
	{
		m_bDanger = true;
		m_bAccent = false;
		ApplyTheme(GetTheme());
		InvalidatePaint();
	}

	//------------------------------------------------------------------------------------------------
	void MakeAccent()
	{
		m_bAccent = true;
		m_bDanger = false;
		ApplyTheme(GetTheme());
		InvalidatePaint();
	}

	//------------------------------------------------------------------------------------------------
	void MakeDefault()
	{
		m_bAccent = false;
		m_bDanger = false;
		ApplyTheme(GetTheme());
		InvalidatePaint();
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnClicked()
	{
		return m_OnClicked;
	}

	//------------------------------------------------------------------------------------------------
	override void MeasureIntrinsic(float availW, float availH)
	{
		float w;
		float h;
		if (m_Runtime)
			m_Runtime.MeasureText(m_sText, m_Style.m_iFontSize, true, 0, w, h);
		else
		{
			w = 64;
			h = m_Style.m_iFontSize;
		}
		m_fDesiredW = w + 28;
		m_fDesiredH = m_Style.m_fHeight;
	}

	//------------------------------------------------------------------------------------------------
	override void OnClicked()
	{
		if (m_OnClicked)
			m_OnClicked.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	override void Paint(MUI_RenderSurface surface)
	{
		float hover = GetHoverT();
		float press = GetPressT();
		float lift = hover * 3.0 - press * 2.0;
		float x = DrawX();
		float y = DrawY() - lift;
		float w = m_World.m_fW;
		float h = m_World.m_fH;
		float op = GetDrawOpacity();
		if (op < 0.01)
			return;

		float rad = m_Style.m_fRadius;
		Color fill = ResolveFill();
		MUI_ThemeData theme = GetTheme();
		float glow = hover * 0.28 + 0.08 * MUI_Ease.Pulse(GetTime(), 1.1);
		if (m_bAccent || m_bDanger)
			surface.FillRect(x - 3, y - 3, w + 6, h + 6, MUI_ColorUtil.Fade(fill, op * glow), rad + 2);

		surface.FillRect(x, y, w, h, MUI_ColorUtil.Fade(fill, op), rad);
		surface.StrokeRect(x, y, w, h, MUI_ColorUtil.Fade(theme.Sheen, op * (0.15 + hover * 0.4)), 1.2, rad);
		PaintFocusRing(surface, x, y, w, h);

		float ripple = GetRipple();
		if (ripple > 0 && ripple < 1)
		{
			float rr = (w * 0.15) + ripple * w * 0.55;
			surface.StrokeCircle(x + w * 0.5, y + h * 0.5, rr, MUI_ColorUtil.Fade(theme.Sheen, op * (1.0 - ripple) * 0.55), 2);
		}

		Color textCol = m_Style.m_Text;
		if (press > 0.5)
			textCol = Color.Black;
		surface.DrawText(x, y, w, h, m_sText, m_Style.m_iFontSize, MUI_ColorUtil.Fade(textCol, op), true, true, true, false);
	}
}
