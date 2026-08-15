//------------------------------------------------------------------------------------------------
//! Subclass this in your addon to get a code-defined menu. Do not edit Mikes-UI.
//!
//! Consumer:
//!   1. addon.gproj Dependencies include "B3F91C6A4E275D08"
//!   2. modded enum ChimeraMenuPreset { MyMenu }
//!   3. chimeraMenus.conf:
//!        MenuPreset MyMenu {
//!          Layout "{A1C47E92B6D8305F}UI/Layouts/MUI_BlankMenu.layout"
//!          Class "MyMenu"
//!        }
//!   4. class MyMenu : MUI_MenuBase
//!      {
//!        protected ref MUI_Button m_ok;
//!        override void BuildUI(notnull MUI_Runtime runtime) { ... runtime.SetRoot(overlay); }
//!      }
//!   5. OpenMenu(ChimeraMenuPreset.MyMenu)
//!
//! Lifecycle:
//!   OnMenuOpen mounts, calls BuildUI, binds Back to OnMUIBack (default Close).
//!   OnMenuUpdate ticks the compositor and MenuWithEditorContext.
//!   Override OnMUIMountFailed() if you still have a legacy layout fallback.
//!   Every node you keep after BuildUI must be `protected ref`.
//------------------------------------------------------------------------------------------------
class MUI_MenuBase : ChimeraMenuBase
{
	protected Widget m_wRoot;
	protected ref MUI_MenuHost m_Host;

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		m_wRoot = GetRootWidget();
		if (!m_wRoot)
			return;

		m_Host = new MUI_MenuHost();
		if (!m_Host.Open(m_wRoot, GetMUILogTag()))
		{
			m_Host = null;
			OnMUIMountFailed();
			return;
		}

		MUI_Runtime runtime = m_Host.GetRuntime();
		if (!runtime)
		{
			OnMUIMountFailed();
			return;
		}

		BuildUI(runtime);
		runtime.GetOnBack().Insert(OnMUIBack);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		if (m_Host)
			m_Host.Tick(tDelta);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusLost()
	{
		super.OnMenuFocusLost();
		if (m_Host)
			m_Host.Blur();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		if (m_Host)
		{
			m_Host.Close();
			m_Host = null;
		}
		super.OnMenuClose();
	}

	//------------------------------------------------------------------------------------------------
	//! Build the node tree and call runtime.SetRoot. Override in the consumer addon.
	void BuildUI(notnull MUI_Runtime runtime)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Called when Mount fails. Default: do nothing (empty layout). Override to open legacy UI.
	void OnMUIMountFailed()
	{
		Print(string.Format("[%1] Mike's UI mount failed", GetMUILogTag()), LogLevel.ERROR);
	}

	//------------------------------------------------------------------------------------------------
	//! Bound to GetOnBack(). Default closes the menu.
	void OnMUIBack()
	{
		Close();
	}

	//------------------------------------------------------------------------------------------------
	string GetMUILogTag()
	{
		return "MUI_Menu";
	}

	//------------------------------------------------------------------------------------------------
	bool IsMUIOpen()
	{
		if (!m_Host)
			return false;
		return m_Host.IsOpen();
	}

	//------------------------------------------------------------------------------------------------
	MUI_Runtime GetRuntime()
	{
		if (!m_Host)
			return null;
		return m_Host.GetRuntime();
	}

	//------------------------------------------------------------------------------------------------
	MUI_MenuHost GetMenuHost()
	{
		return m_Host;
	}
}
