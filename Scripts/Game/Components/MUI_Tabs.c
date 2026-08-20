//------------------------------------------------------------------------------------------------
//! Horizontal segmented tab bar.
//!
//! Consumer:
//!   MUI_Tabs tabs = runtime.CreateTabs("tabs");
//!   tabs.AddTab("Server"); tabs.AddTab("Global");
//!   tabs.SetIndex(0);
//!   tabs.GetOnChanged().Insert(OnTab);
//!
//! Layout:
//!   Fill width, Hug height from buttons.
//!
//! Gamepad:
//!   The bar is one focus target. Left/right and MenuTabLeft/Right (LB/RB) change
//!   the index immediately. Tab buttons still take mouse clicks; they do not steal
//!   d-pad focus. Click handlers bind an index — they must not guess via IsFocused.
//------------------------------------------------------------------------------------------------
class MUI_TabButton : MUI_Button
{
	//------------------------------------------------------------------------------------------------
	override bool WantsFocus()
	{
		return false;
	}
}

//------------------------------------------------------------------------------------------------
class MUI_TabClickBind
{
	protected MUI_Tabs m_Tabs;
	protected int m_iIndex;

	//------------------------------------------------------------------------------------------------
	void Init(MUI_Tabs tabs, int index)
	{
		m_Tabs = tabs;
		m_iIndex = index;
	}

	//------------------------------------------------------------------------------------------------
	void OnClicked()
	{
		if (!m_Tabs)
			return;
		m_Tabs.SetIndex(m_iIndex);
	}
}

//------------------------------------------------------------------------------------------------
class MUI_Tabs : MUI_Node
{
	protected ref array<ref MUI_Button> m_aTabs;
	protected ref array<ref MUI_TabClickBind> m_aBinds;
	protected ref ScriptInvoker m_OnChanged;
	protected int m_iIndex;
	protected bool m_bTabActionsBound;

	//------------------------------------------------------------------------------------------------
	void MUI_Tabs()
	{
		m_aTabs = new array<ref MUI_Button>();
		m_aBinds = new array<ref MUI_TabClickBind>();
		m_OnChanged = new ScriptInvoker();
		m_iIndex = -1;
		m_Style.m_Layout = MUI_LayoutKind.StackHorizontal;
		m_Style.m_WidthMode = MUI_SizeMode.Fill;
		m_Style.m_HeightMode = MUI_SizeMode.Hug;
		m_Style.m_fGap = 8;
		m_Style.m_Fill = Color.FromInt(0);
		m_Style.m_bInteractive = true;
	}

	//------------------------------------------------------------------------------------------------
	void ~MUI_Tabs()
	{
		UnbindTabActions();
	}

	//------------------------------------------------------------------------------------------------
	override void SetRuntime(MUI_Runtime runtime)
	{
		UnbindTabActions();
		super.SetRuntime(runtime);
		if (m_Runtime)
			BindTabActions();
	}

	//------------------------------------------------------------------------------------------------
	override void DestroyHostWidgets()
	{
		UnbindTabActions();
		super.DestroyHostWidgets();
	}

	//------------------------------------------------------------------------------------------------
	override void ApplyTheme(notnull MUI_ThemeData theme)
	{
		int i;
		for (i = 0; i < m_aTabs.Count(); i++)
		{
			if (m_aTabs[i])
				m_aTabs[i].ApplyTheme(theme);
		}
		RefreshStyles();
	}

	//------------------------------------------------------------------------------------------------
	int AddTab(string label)
	{
		ref MUI_TabButton btn = new MUI_TabButton();
		btn.SetText(label);
		if (m_Runtime)
			m_Runtime.Adopt(btn);
		else
			btn.ApplyTheme(GetTheme());

		int index = m_aTabs.Count();
		m_aTabs.Insert(btn);
		AddChild(btn);
		WireButton(btn, index);
		if (m_iIndex < 0)
			SetIndex(0);
		else
			RefreshStyles();
		return index;
	}

	//------------------------------------------------------------------------------------------------
	protected void WireButton(notnull MUI_Button btn, int index)
	{
		btn.SetName("tab_" + index.ToString());
		ref MUI_TabClickBind bind = new MUI_TabClickBind();
		bind.Init(this, index);
		m_aBinds.Insert(bind);
		btn.GetOnClicked().Insert(bind.OnClicked);
	}

	//------------------------------------------------------------------------------------------------
	void SetIndex(int index)
	{
		if (index < 0)
			return;
		if (index >= m_aTabs.Count())
			return;
		if (m_iIndex == index)
		{
			RefreshStyles();
			return;
		}
		m_iIndex = index;
		RefreshStyles();
		FocusSelfOnGamepad();
		if (m_OnChanged)
			m_OnChanged.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	int GetIndex()
	{
		return m_iIndex;
	}

	//------------------------------------------------------------------------------------------------
	string GetTabLabel(int index)
	{
		if (index < 0)
			return "";
		if (index >= m_aTabs.Count())
			return "";
		if (!m_aTabs[index])
			return "";
		return m_aTabs[index].GetText();
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnChanged()
	{
		return m_OnChanged;
	}

	//------------------------------------------------------------------------------------------------
	override bool HandleActivate()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool HandleNavAxis(int dirX, int dirY)
	{
		if (dirX == 0)
			return false;
		int next = m_iIndex + dirX;
		if (next < 0)
			next = 0;
		if (next >= m_aTabs.Count())
			next = m_aTabs.Count() - 1;
		SetIndex(next);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Paint(MUI_RenderSurface surface)
	{
		if (m_iIndex < 0)
			return;
		if (m_iIndex >= m_aTabs.Count())
			return;
		MUI_Button tab = m_aTabs[m_iIndex];
		if (!tab)
			return;
		MUI_Rect wr = tab.GetWorldRect();
		PaintFocusRing(surface, tab.DrawX(), tab.DrawY(), wr.m_fW, wr.m_fH);
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshStyles()
	{
		int i;
		for (i = 0; i < m_aTabs.Count(); i++)
		{
			if (!m_aTabs[i])
				continue;
			if (i == m_iIndex)
				m_aTabs[i].MakeAccent();
			else
				m_aTabs[i].MakeDefault();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void FocusSelfOnGamepad()
	{
		if (!m_Runtime)
			return;
		InputManager im = GetGame().GetInputManager();
		if (!im)
			return;
		if (im.IsUsingMouseAndKeyboard())
			return;
		m_Runtime.FocusNode(this);
	}

	//------------------------------------------------------------------------------------------------
	protected void BindTabActions()
	{
		if (m_bTabActionsBound)
			return;
		InputManager im = GetGame().GetInputManager();
		if (!im)
			return;
		im.AddActionListener("MenuTabLeft", EActionTrigger.DOWN, OnMenuTabLeft);
		im.AddActionListener("MenuTabRight", EActionTrigger.DOWN, OnMenuTabRight);
		m_bTabActionsBound = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void UnbindTabActions()
	{
		if (!m_bTabActionsBound)
			return;
		InputManager im = GetGame().GetInputManager();
		if (im)
		{
			im.RemoveActionListener("MenuTabLeft", EActionTrigger.DOWN, OnMenuTabLeft);
			im.RemoveActionListener("MenuTabRight", EActionTrigger.DOWN, OnMenuTabRight);
		}
		m_bTabActionsBound = false;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuTabLeft()
	{
		HandleNavAxis(-1, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuTabRight()
	{
		HandleNavAxis(1, 0);
	}
}
