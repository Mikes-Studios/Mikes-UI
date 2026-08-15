//------------------------------------------------------------------------------------------------
//! Labeled string field. Typing goes through the hidden EditBox (IME / console VK).
//!
//! Consumer:
//!   MUI_TextField f = runtime.CreateTextField("Civilian multiplier", "civ");
//!   f.SetText("1.0");
//!   f.GetOnChanged().Insert(OnCivChanged);
//!
//! Layout:
//!   Fill width, Exact height 74. Put in a ScrollView for long forms.
//------------------------------------------------------------------------------------------------
class MUI_TextField : MUI_Node
{
	protected string m_sLabel;
	protected string m_sValue;
	protected ref ScriptInvoker m_OnChanged;

	//------------------------------------------------------------------------------------------------
	void MUI_TextField()
	{
		m_OnChanged = new ScriptInvoker();
		m_Style.m_WidthMode = MUI_SizeMode.Fill;
		m_Style.m_HeightMode = MUI_SizeMode.Exact;
		m_Style.m_fHeight = 74;
		m_Style.m_fMinHeight = 74;
		m_Style.m_bInteractive = true;
		m_Style.m_Fill = Color.FromInt(0);
		m_Style.m_iFontSize = MUI_Theme.FONT_BODY;
	}

	//------------------------------------------------------------------------------------------------
	override void ApplyTheme(notnull MUI_ThemeData theme)
	{
		m_Style.m_iFontSize = theme.FONT_BODY;
	}

	//------------------------------------------------------------------------------------------------
	void SetLabel(string label)
	{
		m_sLabel = label;
		InvalidatePaint();
	}

	//------------------------------------------------------------------------------------------------
	void SetText(string text)
	{
		if (m_sValue == text)
			return;
		m_sValue = text;
		InvalidatePaint();
		if (m_OnChanged)
			m_OnChanged.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	void SetTextFromBridge(string text)
	{
		if (m_sValue == text)
			return;
		m_sValue = text;
		InvalidatePaint();
	}

	//------------------------------------------------------------------------------------------------
	string GetText()
	{
		return m_sValue;
	}

	//------------------------------------------------------------------------------------------------
	override bool WantsTextInput()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override string GetEditText()
	{
		return m_sValue;
	}

	//------------------------------------------------------------------------------------------------
	override void SetEditTextFromBridge(string text)
	{
		SetTextFromBridge(text);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnChanged()
	{
		return m_OnChanged;
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
		float focus = GetFocusT();
		float hover = GetHoverT();
		surface.DrawText(x + 4, y, w, 18, m_sLabel, theme.FONT_SMALL, MUI_ColorUtil.Fade(theme.TextMuted, op), false, false, true, false);

		float boxY = y + 22;
		float boxH = h - 24;
		if (boxH < 28)
			boxH = 28;

		if (focus > 0.01)
			surface.FillRect(x - 3, boxY - 3, w + 6, boxH + 6, MUI_ColorUtil.Fade(theme.Glow, op * focus * 0.45), 10);

		surface.FillRect(x, boxY, w, boxH, MUI_ColorUtil.Fade(theme.Field, op), 8);
		surface.FillRect(x, boxY, 3, boxH, MUI_ColorUtil.Fade(theme.Accent, op * (0.25 + focus * 0.75)), 0);

		Color stroke = theme.Border;
		if (focus > 0.5)
			stroke = theme.Accent;
		else if (hover > 0.5)
			stroke = theme.CyanDim;
		surface.StrokeRect(x, boxY, w, boxH, MUI_ColorUtil.Fade(stroke, op), 1.3 + focus * 0.6, 8);

		string shown = m_sValue;
		surface.DrawText(x + 14, boxY, w - 24, boxH, shown, m_Style.m_iFontSize, MUI_ColorUtil.Fade(theme.Text, op), false, false, true, false);

		if (focus > 0.5)
		{
			float blink = MUI_Ease.Pulse(GetTime(), 1.6);
			if (blink > 0.35)
			{
				float tw = 0;
				float th = 0;
				if (m_Runtime)
					m_Runtime.MeasureText(shown, m_Style.m_iFontSize, false, 0, tw, th);
				float cx = x + 14 + tw + 2;
				if (cx > x + w - 12)
					cx = x + w - 12;
				surface.FillRect(cx, boxY + 8, 2, boxH - 16, MUI_ColorUtil.Fade(theme.Accent, op * blink), 0);
			}
		}
	}
}
