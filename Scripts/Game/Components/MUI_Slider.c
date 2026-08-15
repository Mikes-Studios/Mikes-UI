//------------------------------------------------------------------------------------------------
//! Horizontal value slider. Mouse drag + gamepad left/right.
//!
//! Consumer:
//!   MUI_Slider s = runtime.CreateSlider("vol");
//!   s.SetRange(0, 1); s.SetValue(0.5); s.SetStep(0.05);
//!   s.GetOnChanged().Insert(OnVol);
//!
//! Layout:
//!   Fill width, Exact height 28.
//------------------------------------------------------------------------------------------------
class MUI_Slider : MUI_Node
{
	protected float m_fMin;
	protected float m_fMax;
	protected float m_fValue;
	protected float m_fStep;
	protected ref ScriptInvoker m_OnChanged;

	//------------------------------------------------------------------------------------------------
	void MUI_Slider()
	{
		m_OnChanged = new ScriptInvoker();
		m_fMin = 0;
		m_fMax = 1;
		m_fValue = 0;
		m_fStep = 0.05;
		m_Style.m_WidthMode = MUI_SizeMode.Fill;
		m_Style.m_HeightMode = MUI_SizeMode.Exact;
		m_Style.m_fHeight = 28;
		m_Style.m_fMinHeight = 28;
		m_Style.m_bInteractive = true;
		m_Style.m_Fill = Color.FromInt(0);
	}

	//------------------------------------------------------------------------------------------------
	void SetRange(float minV, float maxV)
	{
		m_fMin = minV;
		m_fMax = maxV;
		if (m_fMax < m_fMin)
		{
			float t = m_fMin;
			m_fMin = m_fMax;
			m_fMax = t;
		}
		SetValue(m_fValue);
	}

	//------------------------------------------------------------------------------------------------
	void SetStep(float step)
	{
		if (step < 0)
			step = 0;
		m_fStep = step;
	}

	//------------------------------------------------------------------------------------------------
	void SetValue(float value)
	{
		if (value < m_fMin)
			value = m_fMin;
		if (value > m_fMax)
			value = m_fMax;
		if (m_fValue == value)
			return;
		m_fValue = value;
		InvalidatePaint();
		if (m_OnChanged)
			m_OnChanged.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	float GetValue()
	{
		return m_fValue;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnChanged()
	{
		return m_OnChanged;
	}

	//------------------------------------------------------------------------------------------------
	override void OnDrag(float x, float y)
	{
		ApplyPointer(x);
	}

	//------------------------------------------------------------------------------------------------
	override void OnClicked()
	{
		float lx;
		float ly;
		if (m_Runtime && m_Runtime.GetLocalPointer(lx, ly))
			ApplyPointer(lx);
	}

	//------------------------------------------------------------------------------------------------
	protected void ApplyPointer(float x)
	{
		float trackX = m_World.m_fX + 8;
		float trackW = m_World.m_fW - 16;
		if (trackW < 1)
			return;
		float t = (x - trackX) / trackW;
		if (t < 0)
			t = 0;
		if (t > 1)
			t = 1;
		float span = m_fMax - m_fMin;
		float next = m_fMin + t * span;
		if (m_fStep > 0.0001)
		{
			float steps = Math.Round((next - m_fMin) / m_fStep);
			next = m_fMin + steps * m_fStep;
		}
		SetValue(next);
	}

	//------------------------------------------------------------------------------------------------
	override bool HandleNavAxis(int dirX, int dirY)
	{
		if (dirX == 0)
			return false;
		float step = m_fStep;
		if (step < 0.0001)
			step = (m_fMax - m_fMin) * 0.05;
		SetValue(m_fValue + dirX * step);
		return true;
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
		float trackH = 6;
		float trackX = x + 8;
		float trackW = w - 16;
		float trackY = y + (h - trackH) * 0.5;
		surface.FillRect(trackX, trackY, trackW, trackH, MUI_ColorUtil.Fade(theme.Field, op), 3);

		float span = m_fMax - m_fMin;
		float t = 0;
		if (span > 0.0001)
			t = (m_fValue - m_fMin) / span;
		float fillW = trackW * t;
		if (fillW > 0.75)
			surface.FillRect(trackX, trackY, fillW, trackH, MUI_ColorUtil.Fade(theme.Accent, op), 3);

		float thumb = 16;
		float kx = trackX + fillW - thumb * 0.5;
		float ky = y + (h - thumb) * 0.5;
		surface.FillRect(kx, ky, thumb, thumb, MUI_ColorUtil.Fade(theme.Text, op), 6);
		surface.StrokeRect(kx, ky, thumb, thumb, MUI_ColorUtil.Fade(theme.Border, op), 1.2, 6);
		PaintFocusRing(surface, x, y, w, h);
	}
}
