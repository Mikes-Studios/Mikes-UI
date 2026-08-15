//------------------------------------------------------------------------------------------------
//! Mouse + gamepad routing for one mounted MUI_Runtime. Created by Mount(), skipped by
//! MountPassive() (HUD).
//!
//! Consumer:
//!   Do not create this. Activate MenuWithEditorContext each tick (MUI_MenuHost does).
//!   Back: StopEditing first, else GetOnBack(). Select: WantsTextInput -> BeginEditing,
//!   else HandleActivate. Left/right: HandleNavAxis, else move focus in-row.
//!
//! Extend:
//!   Custom widgets hook input via MUI_Node.WantsTextInput / HandleNavAxis / HandleActivate.
//!   Do not edit this router to special-case your class.
//------------------------------------------------------------------------------------------------
class MUI_InputRouter : ScriptedWidgetEventHandler
{
	protected MUI_Runtime m_Runtime;
	protected MUI_Node m_Hover;
	protected MUI_Node m_Pressed;
	protected MUI_Node m_Focused;
	protected bool m_bDown;
	protected bool m_bBound;
	protected ref array<MUI_Node> m_aFocusables;

	//------------------------------------------------------------------------------------------------
	void Init(MUI_Runtime runtime)
	{
		m_Runtime = runtime;
		m_aFocusables = new array<MUI_Node>();
		BindActions();
	}

	//------------------------------------------------------------------------------------------------
	void Clear()
	{
		UnbindActions();
		m_Hover = null;
		m_Pressed = null;
		m_Focused = null;
		m_bDown = false;
		m_Runtime = null;
		if (m_aFocusables)
			m_aFocusables.Clear();
	}

	//------------------------------------------------------------------------------------------------
	protected void BindActions()
	{
		if (m_bBound)
			return;
		InputManager im = GetGame().GetInputManager();
		if (!im)
			return;
		im.AddActionListener(UIConstants.MENU_ACTION_UP, EActionTrigger.DOWN, OnNavUp);
		im.AddActionListener(UIConstants.MENU_ACTION_DOWN, EActionTrigger.DOWN, OnNavDown);
		im.AddActionListener(UIConstants.MENU_ACTION_LEFT, EActionTrigger.DOWN, OnNavLeft);
		im.AddActionListener(UIConstants.MENU_ACTION_RIGHT, EActionTrigger.DOWN, OnNavRight);
		im.AddActionListener(UIConstants.MENU_ACTION_SELECT, EActionTrigger.DOWN, OnNavSelect);
		im.AddActionListener(UIConstants.MENU_ACTION_BACK, EActionTrigger.DOWN, OnNavBack);
		im.AddActionListener(UIConstants.MENU_ACTION_BACK_WB, EActionTrigger.DOWN, OnNavBack);
		m_bBound = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void UnbindActions()
	{
		if (!m_bBound)
			return;
		InputManager im = GetGame().GetInputManager();
		if (im)
		{
			im.RemoveActionListener(UIConstants.MENU_ACTION_UP, EActionTrigger.DOWN, OnNavUp);
			im.RemoveActionListener(UIConstants.MENU_ACTION_DOWN, EActionTrigger.DOWN, OnNavDown);
			im.RemoveActionListener(UIConstants.MENU_ACTION_LEFT, EActionTrigger.DOWN, OnNavLeft);
			im.RemoveActionListener(UIConstants.MENU_ACTION_RIGHT, EActionTrigger.DOWN, OnNavRight);
			im.RemoveActionListener(UIConstants.MENU_ACTION_SELECT, EActionTrigger.DOWN, OnNavSelect);
			im.RemoveActionListener(UIConstants.MENU_ACTION_BACK, EActionTrigger.DOWN, OnNavBack);
			im.RemoveActionListener(UIConstants.MENU_ACTION_BACK_WB, EActionTrigger.DOWN, OnNavBack);
		}
		m_bBound = false;
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
		{
			m_Runtime.OnFocusChanged(m_Focused);
			if (m_Focused)
				m_Runtime.EnsureNodeVisible(m_Focused);
		}
	}

	//------------------------------------------------------------------------------------------------
	void UpdatePointer()
	{
		if (!m_Runtime)
			return;

		InputManager im = GetGame().GetInputManager();
		if (im && !im.IsUsingMouseAndKeyboard())
		{
			if (!m_Focused)
				EnsureFocus();
			if (m_Hover && m_Hover != m_Focused)
				m_Hover.SetHover(false);
			m_Hover = m_Focused;
			if (m_Hover)
				m_Hover.SetHover(true);
			return;
		}

		float lx;
		float ly;
		if (!m_Runtime.GetLocalPointer(lx, ly))
			return;

		if (m_bDown && m_Pressed)
			m_Pressed.OnDrag(lx, ly);

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
		else
			SetFocused(hit);

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
		{
			m_Pressed.OnDragEnd(lx, ly);
			m_Pressed.SetPressed(false);
		}

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
		if (!node)
			node = m_Focused;
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
	void OnNavUp()
	{
		if (TryNavAxis(0, -1))
			return;
		MoveFocus(0, -1);
	}

	//------------------------------------------------------------------------------------------------
	void OnNavDown()
	{
		if (TryNavAxis(0, 1))
			return;
		MoveFocus(0, 1);
	}

	//------------------------------------------------------------------------------------------------
	void OnNavLeft()
	{
		if (TryNavAxis(-1, 0))
			return;
		MoveFocus(-1, 0);
	}

	//------------------------------------------------------------------------------------------------
	void OnNavRight()
	{
		if (TryNavAxis(1, 0))
			return;
		MoveFocus(1, 0);
	}

	//------------------------------------------------------------------------------------------------
	void OnNavSelect()
	{
		if (!EnsureFocus())
			return;
		if (m_Focused && m_Focused.WantsTextInput())
		{
			if (m_Runtime)
				m_Runtime.BeginEditing(m_Focused);
			return;
		}
		if (m_Focused)
			m_Focused.HandleActivate();
	}

	//------------------------------------------------------------------------------------------------
	void OnNavBack()
	{
		if (m_Runtime && m_Runtime.IsEditing())
		{
			m_Runtime.StopEditing();
			return;
		}
		if (m_Runtime)
			m_Runtime.RequestBack();
	}

	//------------------------------------------------------------------------------------------------
	protected bool TryNavAxis(int dirX, int dirY)
	{
		if (!m_Focused)
			return false;
		return m_Focused.HandleNavAxis(dirX, dirY);
	}

	//------------------------------------------------------------------------------------------------
	protected bool EnsureFocus()
	{
		RefreshFocusables();
		if (m_Focused && m_aFocusables.Find(m_Focused) >= 0)
			return true;
		if (m_aFocusables.Count() < 1)
			return false;
		SetFocused(m_aFocusables[0]);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshFocusables()
	{
		if (!m_aFocusables)
			m_aFocusables = new array<MUI_Node>();
		m_aFocusables.Clear();
		if (m_Runtime)
			m_Runtime.CollectFocusables(m_aFocusables);
	}

	//------------------------------------------------------------------------------------------------
	protected void MoveFocus(int dirX, int dirY)
	{
		if (!EnsureFocus())
			return;
		if (m_aFocusables.Count() < 2)
			return;

		MUI_Rect cur = m_Focused.GetWorldRect();
		float cx = cur.m_fX + cur.m_fW * 0.5;
		float cy = cur.m_fY + cur.m_fH * 0.5;

		MUI_Node best = null;
		float bestScore = 1000000;
		int i;
		for (i = 0; i < m_aFocusables.Count(); i++)
		{
			MUI_Node node = m_aFocusables[i];
			if (node == m_Focused)
				continue;
			MUI_Rect r = node.GetWorldRect();
			float nx = r.m_fX + r.m_fW * 0.5;
			float ny = r.m_fY + r.m_fH * 0.5;
			float dx = nx - cx;
			float dy = ny - cy;

			if (dirX != 0)
			{
				if (dx * dirX <= 4)
					continue;
				if (Math.AbsFloat(dy) > 36)
					continue;
				float score = Math.AbsFloat(dx) + Math.AbsFloat(dy) * 2;
				if (score < bestScore)
				{
					bestScore = score;
					best = node;
				}
			}
			else
			{
				if (dy * dirY <= 4)
					continue;
				float score = Math.AbsFloat(dy) + Math.AbsFloat(dx) * 0.25;
				if (score < bestScore)
				{
					bestScore = score;
					best = node;
				}
			}
		}

		if (best)
			SetFocused(best);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnController(Widget w, ControlID control, int value)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		return false;
	}
}
