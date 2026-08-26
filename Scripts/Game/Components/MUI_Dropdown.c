//------------------------------------------------------------------------------------------------
//! In-flow dropdown. List expands below; optional root overlay catcher closes on outside click.
//!
//! Consumer:
//!   MUI_Dropdown d = runtime.CreateDropdown("faction");
//!   d.AddItem("Blufor"); d.AddItem("Opfor");
//!   d.SetIndex(0);
//!   d.GetOnChanged().Insert(OnFaction);
//!
//! Layout:
//!   Fill width, Hug height. Closed header ~44. No left/right item cycling.
//!
//! Extend:
//!   Floating popup / focus trap is out of scope for v1.
//------------------------------------------------------------------------------------------------
class MUI_DropdownCatcher : MUI_Node
{
	protected ref ScriptInvoker m_OnClicked;

	//------------------------------------------------------------------------------------------------
	void MUI_DropdownCatcher()
	{
		m_OnClicked = new ScriptInvoker();
		m_Style.m_Layout = MUI_LayoutKind.Overlay;
		m_Style.m_WidthMode = MUI_SizeMode.Fill;
		m_Style.m_HeightMode = MUI_SizeMode.Fill;
		m_Style.m_Fill = Color.FromInt(0);
		m_Style.m_fRadius = 0;
		m_Style.m_bBlockHit = true;
		m_Style.m_bInteractive = true;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnClicked()
	{
		return m_OnClicked;
	}

	//------------------------------------------------------------------------------------------------
	override void OnClicked()
	{
		if (m_OnClicked)
			m_OnClicked.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	override bool WantsFocus()
	{
		return false;
	}
}

//------------------------------------------------------------------------------------------------
class MUI_DropdownItemBind
{
	protected MUI_Dropdown m_Dropdown;
	protected int m_iIndex;

	//------------------------------------------------------------------------------------------------
	void Init(MUI_Dropdown dropdown, int index)
	{
		m_Dropdown = dropdown;
		m_iIndex = index;
	}

	//------------------------------------------------------------------------------------------------
	void OnClicked()
	{
		if (!m_Dropdown)
			return;
		m_Dropdown.ChooseItem(m_iIndex);
	}
}

//------------------------------------------------------------------------------------------------
class MUI_Dropdown : MUI_Node
{
	protected static const int SCROLL_AFTER = 6;
	protected static const float ROW_H = 40;

	protected ref array<string> m_aItems;
	protected ref array<ref MUI_Button> m_aRows;
	protected ref array<ref MUI_DropdownItemBind> m_aItemBinds;
	protected ref ScriptInvoker m_OnChanged;
	protected ref MUI_Button m_Header;
	protected ref MUI_Node m_ListHost;
	protected ref MUI_DropdownCatcher m_Catcher;
	protected int m_iIndex;
	protected bool m_bOpen;
	protected bool m_bHeaderOwned;

	//------------------------------------------------------------------------------------------------
	void MUI_Dropdown()
	{
		m_aItems = new array<string>();
		m_aRows = new array<ref MUI_Button>();
		m_aItemBinds = new array<ref MUI_DropdownItemBind>();
		m_OnChanged = new ScriptInvoker();
		m_iIndex = -1;
		m_bOpen = false;
		m_bHeaderOwned = false;
		m_Style.m_Layout = MUI_LayoutKind.StackVertical;
		m_Style.m_WidthMode = MUI_SizeMode.Fill;
		m_Style.m_HeightMode = MUI_SizeMode.Hug;
		m_Style.m_fGap = 4;
		m_Style.m_Fill = Color.FromInt(0);

		m_Header = new MUI_Button();
		m_Header.SetText("-");
		m_Header.SetName("dd_header");
		m_Header.GetStyle().m_fGrow = 0;
		m_Header.GetOnClicked().Insert(OnHeaderClicked);
		AddChild(m_Header);
	}

	//------------------------------------------------------------------------------------------------
	override void ApplyTheme(notnull MUI_ThemeData theme)
	{
		if (m_Header)
			m_Header.ApplyTheme(theme);
		RefreshHeaderChrome();
	}

	//------------------------------------------------------------------------------------------------
	override void SetRuntime(MUI_Runtime runtime)
	{
		super.SetRuntime(runtime);
		EnsureHeaderRetained();
		RefreshHeaderChrome();
	}

	//------------------------------------------------------------------------------------------------
	protected void EnsureHeaderRetained()
	{
		if (!m_Header || !m_Runtime || m_bHeaderOwned)
			return;
		m_Runtime.Adopt(m_Header);
		m_bHeaderOwned = true;
	}

	//------------------------------------------------------------------------------------------------
	int AddItem(string label)
	{
		m_aItems.Insert(label);
		int index = m_aItems.Count() - 1;
		if (m_iIndex < 0)
			SetIndex(0);
		else
			RefreshHeaderChrome();
		if (m_bOpen)
			RebuildList();
		return index;
	}

	//------------------------------------------------------------------------------------------------
	void SetIndex(int index)
	{
		if (index < 0)
			return;
		if (index >= m_aItems.Count())
			return;
		bool changed = m_iIndex != index;
		m_iIndex = index;
		RefreshHeaderChrome();
		if (changed && m_OnChanged)
			m_OnChanged.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	int GetIndex()
	{
		return m_iIndex;
	}

	//------------------------------------------------------------------------------------------------
	void ChooseItem(int index)
	{
		SetIndex(index);
		SetOpen(false);
	}

	//------------------------------------------------------------------------------------------------
	string GetText()
	{
		if (m_iIndex < 0)
			return "";
		if (m_iIndex >= m_aItems.Count())
			return "";
		return m_aItems[m_iIndex];
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnChanged()
	{
		return m_OnChanged;
	}

	//------------------------------------------------------------------------------------------------
	bool IsOpen()
	{
		return m_bOpen;
	}

	//------------------------------------------------------------------------------------------------
	void SetOpen(bool open)
	{
		if (m_bOpen == open)
			return;
		m_bOpen = open;
		if (m_bOpen)
		{
			RebuildList();
			EnsureCatcher();
		}
		else
		{
			TearDownList();
			TearDownCatcher();
		}
		InvalidateLayout();
	}

	//------------------------------------------------------------------------------------------------
	override bool HandleActivate()
	{
		SetOpen(!m_bOpen);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool HandleNavAxis(int dirX, int dirY)
	{
		// Left/right must not cycle items (avoids fighting tab-like nav).
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnHeaderClicked()
	{
		SetOpen(!m_bOpen);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCatcherClicked()
	{
		SetOpen(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshHeaderChrome()
	{
		if (!m_Header)
			return;
		string label = "-";
		if (m_iIndex >= 0 && m_iIndex < m_aItems.Count())
			label = m_aItems[m_iIndex];
		m_Header.SetText(label);
		m_Header.MakeDefault();
	}

	//------------------------------------------------------------------------------------------------
	protected void RebuildList()
	{
		TearDownList();
		if (!m_bOpen)
			return;

		int count = m_aItems.Count();
		if (count <= 0)
			return;

		ref MUI_Node host;
		if (count > SCROLL_AFTER)
		{
			ref MUI_ScrollView scroll = new MUI_ScrollView();
			if (m_Runtime)
				m_Runtime.Adopt(scroll);
			else
				scroll.ApplyTheme(GetTheme());
			scroll.SetGap(4);
			float vh = SCROLL_AFTER * ROW_H;
			scroll.SetViewportHeight(vh);
			host = scroll;
		}
		else
		{
			ref MUI_Panel panel = new MUI_Panel();
			if (m_Runtime)
				m_Runtime.Adopt(panel);
			else
				panel.ApplyTheme(GetTheme());
			panel.GetStyle().m_Fill = Color.FromInt(0);
			panel.GetStyle().m_fRadius = 0;
			panel.GetStyle().m_fGap = 4;
			host = panel;
		}

		m_ListHost = host;
		AddChild(host);

		int i;
		for (i = 0; i < count; i++)
		{
			ref MUI_Button row = new MUI_Button();
			row.SetText(m_aItems[i]);
			row.SetName("dd_item_" + i.ToString());
			row.GetStyle().m_fGrow = 0;
			row.GetStyle().m_fHeight = ROW_H;
			row.GetStyle().m_fMinHeight = ROW_H;
			if (m_Runtime)
				m_Runtime.Adopt(row);
			else
				row.ApplyTheme(GetTheme());
			if (i == m_iIndex)
				row.MakeAccent();
			else
				row.MakeDefault();
			ref MUI_DropdownItemBind bind = new MUI_DropdownItemBind();
			bind.Init(this, i);
			m_aItemBinds.Insert(bind);
			row.GetOnClicked().Insert(bind.OnClicked);
			m_aRows.Insert(row);
			host.AddChild(row);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void TearDownList()
	{
		if (m_ListHost)
		{
			RemoveChild(m_ListHost);
			m_ListHost = null;
		}
		m_aRows.Clear();
		if (m_aItemBinds)
			m_aItemBinds.Clear();
	}

	//------------------------------------------------------------------------------------------------
	protected void EnsureCatcher()
	{
		if (m_Catcher)
			return;
		if (!m_Runtime)
			return;
		MUI_Node root = m_Runtime.GetRoot();
		if (!root)
			return;

		ref MUI_DropdownCatcher catcher = new MUI_DropdownCatcher();
		catcher.SetName("dd_catcher");
		catcher.GetOnClicked().Insert(OnCatcherClicked);
		m_Runtime.Adopt(catcher);
		m_Catcher = catcher;
		root.AddChildFirst(catcher);
	}

	//------------------------------------------------------------------------------------------------
	protected void TearDownCatcher()
	{
		if (!m_Catcher)
			return;
		m_Catcher.GetOnClicked().Remove(OnCatcherClicked);
		MUI_Node parent = m_Catcher.GetParent();
		if (parent)
			parent.RemoveChild(m_Catcher);
		m_Catcher = null;
	}
}
