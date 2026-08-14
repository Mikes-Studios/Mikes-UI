//------------------------------------------------------------------------------------------------
class MUI_Rect
{
	float m_fX;
	float m_fY;
	float m_fW;
	float m_fH;

	//------------------------------------------------------------------------------------------------
	void MUI_Rect(float x = 0, float y = 0, float w = 0, float h = 0)
	{
		m_fX = x;
		m_fY = y;
		m_fW = w;
		m_fH = h;
	}

	//------------------------------------------------------------------------------------------------
	void Set(float x, float y, float w, float h)
	{
		m_fX = x;
		m_fY = y;
		m_fW = w;
		m_fH = h;
	}

	//------------------------------------------------------------------------------------------------
	bool Contains(float x, float y)
	{
		if (x < m_fX)
			return false;
		if (y < m_fY)
			return false;
		if (x > m_fX + m_fW)
			return false;
		if (y > m_fY + m_fH)
			return false;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool Intersects(MUI_Rect other)
	{
		if (!other)
			return false;
		if (m_fX + m_fW < other.m_fX)
			return false;
		if (other.m_fX + other.m_fW < m_fX)
			return false;
		if (m_fY + m_fH < other.m_fY)
			return false;
		if (other.m_fY + other.m_fH < m_fY)
			return false;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	float Right()
	{
		return m_fX + m_fW;
	}

	//------------------------------------------------------------------------------------------------
	float Bottom()
	{
		return m_fY + m_fH;
	}
}
