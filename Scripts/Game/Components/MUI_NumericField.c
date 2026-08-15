//------------------------------------------------------------------------------------------------
//! Numeric text field with clamp, step, and gamepad left/right.
//!
//! Consumer:
//!   MUI_NumericField n = runtime.CreateNumericField("AI scale", "ai");
//!   n.SetRange(0.1, 5); n.SetStep(0.1); n.SetDecimals(2); n.SetValue(1);
//!
//! Layout:
//!   Same as TextField (Fill width, Exact 74).
//!
//! Extend:
//!   Typing keeps partial text. Invalid / empty snaps to the last good value on CommitEdit
//!   (edit-box leave). Left/right steps without opening IME.
//------------------------------------------------------------------------------------------------
class MUI_NumericField : MUI_TextField
{
	protected float m_fMin;
	protected float m_fMax;
	protected float m_fValue;
	protected float m_fStep;
	protected int m_iDecimals;

	//------------------------------------------------------------------------------------------------
	void MUI_NumericField()
	{
		m_fMin = 0;
		m_fMax = 100000;
		m_fValue = 0;
		m_fStep = 1;
		m_iDecimals = 2;
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
	void SetDecimals(int decimals)
	{
		if (decimals < 0)
			decimals = 0;
		if (decimals > 6)
			decimals = 6;
		m_iDecimals = decimals;
		RefreshTextFromValue();
	}

	//------------------------------------------------------------------------------------------------
	void SetValue(float value)
	{
		if (value < m_fMin)
			value = m_fMin;
		if (value > m_fMax)
			value = m_fMax;
		m_fValue = value;
		RefreshTextFromValue();
	}

	//------------------------------------------------------------------------------------------------
	float GetValue()
	{
		return m_fValue;
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshTextFromValue()
	{
		string text = m_fValue.ToString();
		if (m_iDecimals == 0)
			text = Math.Round(m_fValue).ToString();
		super.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	override void SetText(string text)
	{
		super.SetText(text);
		ParseAndClamp(true);
	}

	//------------------------------------------------------------------------------------------------
	override void SetEditTextFromBridge(string text)
	{
		super.SetEditTextFromBridge(text);
		ParseAndClamp(false);
	}

	//------------------------------------------------------------------------------------------------
	override void CommitEdit()
	{
		ParseAndClamp(true);
	}

	//------------------------------------------------------------------------------------------------
	protected void ParseAndClamp(bool snapText)
	{
		float parsed;
		if (!TryParseFloat(m_sValue, parsed))
		{
			if (snapText)
				RefreshTextFromValue();
			return;
		}

		if (parsed < m_fMin)
			parsed = m_fMin;
		if (parsed > m_fMax)
			parsed = m_fMax;
		m_fValue = parsed;
		if (snapText)
			RefreshTextFromValue();
	}

	//------------------------------------------------------------------------------------------------
	//! Enforce ToFloat throws "Wrong parameter value" on "", "-", ".", "1.".
	//! string.Get returns a 1-char string, not an ASCII int.
	protected bool TryParseFloat(string text, out float value)
	{
		value = 0;
		if (text.IsEmpty())
			return false;

		int len = text.Length();
		int i = 0;
		string ch = text.Get(0);
		if (ch == "-" || ch == "+")
			i = 1;
		if (i >= len)
			return false;

		bool sawDigit = false;
		bool sawDot = false;
		while (i < len)
		{
			ch = text.Get(i);
			if ("0123456789".Contains(ch))
			{
				sawDigit = true;
				i = i + 1;
				continue;
			}
			if (ch == ".")
			{
				if (sawDot)
					return false;
				sawDot = true;
				i = i + 1;
				continue;
			}
			return false;
		}

		if (!sawDigit)
			return false;

		string parseSrc = text;
		while (parseSrc.Length() > 0)
		{
			ch = parseSrc.Get(parseSrc.Length() - 1);
			if (ch != ".")
				break;
			if (parseSrc.Length() == 1)
				return false;
			parseSrc = parseSrc.Substring(0, parseSrc.Length() - 1);
		}

		if (parseSrc.IsEmpty())
			return false;

		ch = parseSrc.Get(0);
		if (ch == "-" || ch == "+")
		{
			if (parseSrc.Length() < 2)
				return false;
		}

		value = parseSrc.ToFloat();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool HandleNavAxis(int dirX, int dirY)
	{
		if (dirX == 0)
			return false;
		float step = m_fStep;
		if (step < 0.0001)
			step = 1;
		SetValue(m_fValue + dirX * step);
		return true;
	}
}
