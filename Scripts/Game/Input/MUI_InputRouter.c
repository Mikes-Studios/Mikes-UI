//------------------------------------------------------------------------------------------------
class MUI_InputRouter : ScriptedWidgetEventHandler
{
	protected MUI_Runtime m_Runtime;
	protected MUI_Node m_Hover;
	protected MUI_Node m_Pressed;
	protected MUI_Node m_Focused;
	protected bool m_bDown;

	//------------------------------------------------------------------------------------------------
	void Init(MUI_Runtime runtime)
	{
		m_Runtime = runtime;
	}

	//------------------------------------------------------------------------------------------------
	void Clear()
	{
		m_Hover = null;
		m_Pressed = null;
		m_Focused = null;
		m_bDown = false;
		m_Runtime = null;
	}

	//------------------------------------------------------------------------------------------------
	MUI_Node GetHover()
	{
		return m_Hover;
	}

	//------------------------------------------------------------------------------------------------
	MUI_Node GetFocused()
	{
		return m_Focused;
	}

	//------------------------------------------------------------------------------------------------
	void SetFocused(MUI_Node node)
	{
		if (m_Focused == node)
			return;
		if (m_Focused)
			m_Focused.SetFocused(false);
		m_Focused = node;
		if (m_Focused)
			m_Focused.SetFocused(true);
		if (m_Runtime)
			m_Runtime.OnFocusChanged(m_Focused);
	}

	//------------------------------------------------------------------------------------------------
	void UpdatePointer()
	{
		if (!m_Runtime)
			return;

		float lx;
		float ly;
		if (!m_Runtime.GetLocalPointer(lx, ly))
			return;

		MUI_Node hit = m_Runtime.HitTest(lx, ly);
		if (hit != m_Hover)
		{
			if (m_Hover)
				m_Hover.SetHover(false);
			m_Hover = hit;
			if (m_Hover)
				m_Hover.SetHover(true);
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		if (button != 0)
			return false;
		if (!m_Runtime)
			return false;

		float lx;
		float ly;
		m_Runtime.GetLocalPointer(lx, ly);
		MUI_Node hit = m_Runtime.HitTest(lx, ly);
		m_bDown = true;
		m_Pressed = hit;
		if (m_Pressed && m_Pressed.AcceptsClick())
			m_Pressed.SetPressed(true);

		if (!hit || !hit.AcceptsClick())
			SetFocused(null);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonUp(Widget w, int x, int y, int button)
	{
		if (button != 0)
			return false;
		if (!m_Runtime)
			return false;

		float lx;
		float ly;
		m_Runtime.GetLocalPointer(lx, ly);
		MUI_Node hit = m_Runtime.HitTest(lx, ly);

		if (m_Pressed)
			m_Pressed.SetPressed(false);

		if (m_bDown && m_Pressed && hit == m_Pressed && m_Pressed.AcceptsClick())
		{
			SetFocused(m_Pressed);
			m_Pressed.HandleActivate();
		}

		m_bDown = false;
		m_Pressed = null;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseWheel(Widget w, int x, int y, int wheel)
	{
		if (!m_Runtime)
			return false;
		MUI_Node node = m_Hover;
		while (node)
		{
			if (node.ClipsChildren())
			{
				node.OnMouseWheel(wheel);
				return true;
			}
			node = node.GetParent();
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnController(Widget w, ControlID control, int value)
	{
		if (control != ControlID.SELECT)
			return false;
		if (value <= 0)
			return false;
		if (m_Focused)
			return m_Focused.HandleActivate();
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		return false;
	}
}
