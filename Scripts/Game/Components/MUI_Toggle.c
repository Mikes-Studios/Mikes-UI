//------------------------------------------------------------------------------------------------
//! Labeled on/off switch. Gamepad left/right flips it (HandleNavAxis).
//!
//! Consumer:
//!   MUI_Toggle t = runtime.CreateToggle("Disable HQ helipads", "heli");
//!   t.SetChecked(true);
//!   t.GetOnChanged().Insert(OnHeliChanged);
//!
//! Layout:
//!   Fill width, Exact height 42.
//------------------------------------------------------------------------------------------------
class MUI_Toggle : MUI_Node
{
	protected string m_sText;
	protected bool m_bChecked;
	protected float m_fCheckT;
	protected ref ScriptInvoker m_OnChanged;
	protected ref Color m_Mix;

	//------------------------------------------------------------------------------------------------
	void MUI_Toggle()
	{
		m_OnChanged = new ScriptInvoker();
		m_Mix = new Color(1, 1, 1, 1);
		m_Style.m_WidthMode = MUI_SizeMode.Fill;
		m_Style.m_HeightMode = MUI_SizeMode.Exact;
		m_Style.m_fHeight = 42;
		m_Style.m_fMinHeight = 42;
		m_Style.m_bInteractive = true;
		m_Style.m_Fill = Color.FromInt(0);
		m_Style.m_Text = MUI_Theme.Text;
		m_Style.m_iFontSize = MUI_Theme.FONT_BODY;
	}

	//------------------------------------------------------------------------------------------------
	override void ApplyTheme(notnull MUI_ThemeData theme)
	{
		m_Style.m_Text = theme.Text;
		m_Style.m_iFontSize = theme.FONT_BODY;
	}

	//------------------------------------------------------------------------------------------------
	void SetText(string text)
	{
		m_sText = text;
		InvalidatePaint();
	}

	//------------------------------------------------------------------------------------------------
	void SetChecked(bool checked)
	{
		if (m_bChecked == checked)
			return;
		m_bChecked = checked;
		InvalidatePaint();
		if (m_OnChanged)
			m_OnChanged.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	bool IsChecked()
	{
		return m_bChecked;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnChanged()
	{
		return m_OnChanged;
	}

	//------------------------------------------------------------------------------------------------
	override void OnClicked()
	{
		SetChecked(!m_bChecked);
	}

	//------------------------------------------------------------------------------------------------
	override bool HandleNavAxis(int dirX, int dirY)
	{
		if (dirX == 0)
			return false;
		HandleActivate();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnTick(float dt)
	{
		float target = 0;
		if (m_bChecked)
			target = 1;
		m_fCheckT = MUI_Ease.Approach(m_fCheckT, target, dt, 16);
	}

	//------------------------------------------------------------------------------------------------
	override void PaintForeground(MUI_RenderSurface surface)
	{
		float x = DrawX();
		float y = DrawY();
		float h = m_World.m_fH;
		float op = GetDrawOpacity();
		if (op < 0.01)
			return;

		MUI_ThemeData theme = GetTheme();
		float trackW = 48;
		float trackH = 24;
		float tx = x;
		float ty = y + (h - trackH) * 0.5;
		float t = m_fCheckT;
		MUI_ColorUtil.Mix(theme.ToggleOff, theme.ToggleOn, t, m_Mix);
		surface.FillRect(tx, ty, trackW, trackH, MUI_ColorUtil.Fade(m_Mix, op), 10);
		surface.StrokeRect(tx, ty, trackW, trackH, MUI_ColorUtil.Fade(theme.Border, op * (0.5 + t * 0.5)), 1.3, 12);

		float knob = 18;
		float kx = tx + 3 + t * (trackW - knob - 6);
		float ky = ty + (trackH - knob) * 0.5;
		surface.FillCircle(kx + knob * 0.5, ky + knob * 0.5, 11 + GetHoverT() * 1.5, MUI_ColorUtil.Fade(theme.Glow, op * t * 0.65));
		surface.FillRect(kx, ky, knob, knob, MUI_ColorUtil.Fade(theme.Text, op), 7);

		float lx = tx + trackW + 14;
		float lw = m_World.m_fW - trackW - 14;
		surface.DrawText(lx, y, lw, h, m_sText, m_Style.m_iFontSize, MUI_ColorUtil.Fade(m_Style.m_Text, op), false, false, true, false);
	}
}
