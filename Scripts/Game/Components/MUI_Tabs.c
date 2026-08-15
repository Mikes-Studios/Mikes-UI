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
//!   Fill width, Exact height 44. Hug height from buttons.
//------------------------------------------------------------------------------------------------
class MUI_Tabs : MUI_Node
{
	protected ref array<ref MUI_Button> m_aTabs;
	protected ref ScriptInvoker m_OnChanged;
	protected int m_iIndex;

	//------------------------------------------------------------------------------------------------
	void MUI_Tabs()
	{
		m_aTabs = new array<ref MUI_Button>();
		m_OnChanged = new ScriptInvoker();
		m_iIndex = -1;
		m_Style.m_Layout = MUI_LayoutKind.StackHorizontal;
		m_Style.m_WidthMode = MUI_SizeMode.Fill;
		m_Style.m_HeightMode = MUI_SizeMode.Hug;
		m_Style.m_fGap = 8;
		m_Style.m_Fill = Color.FromInt(0);
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
		ref MUI_Button btn = new MUI_Button();
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
		// Enfusion ScriptInvoker cannot easily capture index; use name tag.
		btn.SetName("tab_" + index.ToString());
		btn.GetOnClicked().Insert(OnTabClicked);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTabClicked()
	{
		int i;
		for (i = 0; i < m_aTabs.Count(); i++)
		{
			if (m_aTabs[i] && m_aTabs[i].IsFocused())
			{
				SetIndex(i);
				return;
			}
		}
		for (i = 0; i < m_aTabs.Count(); i++)
		{
			if (m_aTabs[i] && m_aTabs[i].IsHover())
			{
				SetIndex(i);
				return;
			}
		}
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
}
