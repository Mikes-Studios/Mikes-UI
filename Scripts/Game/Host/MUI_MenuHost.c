//------------------------------------------------------------------------------------------------
//! Shared bootstrap for Chimera menus that host Mike's UI on a fullscreen Frame.
//! Prefer MUI_MenuBase (subclass it) over using this helper directly.
//!
//! Consumer:
//!   HUD toasts use MUI_HudHost, not this.
//!   Direct use: m_Host = new MUI_MenuHost(); m_Host.Open(root, "MyMenu");
//!   each frame m_Host.Tick(tDelta); on close m_Host.Close().
//------------------------------------------------------------------------------------------------
class MUI_MenuHost
{
	protected Widget m_wRoot;
	protected Widget m_wHost;
	protected ref MUI_Runtime m_Runtime;
	protected string m_sLogTag;

	//------------------------------------------------------------------------------------------------
	//! Creates the host Frame, mounts MUI, optionally hides leftover layout children.
	bool Open(notnull Widget root, string logTag, bool hideLegacy = true)
	{
		Close();

		m_wRoot = root;
		m_sLogTag = logTag;

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
		{
			Print(string.Format("[%1] No workspace for Mike's UI", m_sLogTag), LogLevel.ERROR);
			return false;
		}

		m_wHost = workspace.CreateWidget(WidgetType.FrameWidgetTypeID, WidgetFlags.VISIBLE, Color.FromInt(Color.WHITE), 50, m_wRoot);
		if (!m_wHost)
		{
			Print(string.Format("[%1] Failed to create Mike's UI host", m_sLogTag), LogLevel.ERROR);
			return false;
		}

		FrameSlot.SetAnchorMin(m_wHost, 0, 0);
		FrameSlot.SetAnchorMax(m_wHost, 1, 1);
		FrameSlot.SetOffsets(m_wHost, 0, 0, 0, 0);

		m_Runtime = new MUI_Runtime();
		if (!m_Runtime.Mount(m_wHost))
		{
			Print(string.Format("[%1] Mike's UI mount failed", m_sLogTag), LogLevel.ERROR);
			m_Runtime.Unmount();
			m_Runtime = null;
			m_wHost.RemoveFromHierarchy();
			m_wHost = null;
			return false;
		}

		if (hideLegacy)
			HideLegacyChildren();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float tDelta)
	{
		InputManager im = GetGame().GetInputManager();
		if (im)
			im.ActivateContext("MenuWithEditorContext");
		if (m_Runtime)
			m_Runtime.Tick(tDelta);
	}

	//------------------------------------------------------------------------------------------------
	void Blur()
	{
		if (m_Runtime)
			m_Runtime.Blur();
	}

	//------------------------------------------------------------------------------------------------
	void Close()
	{
		if (m_Runtime)
		{
			m_Runtime.Unmount();
			m_Runtime = null;
		}
		if (m_wHost)
		{
			m_wHost.RemoveFromHierarchy();
			m_wHost = null;
		}
		m_wRoot = null;
	}

	//------------------------------------------------------------------------------------------------
	bool IsOpen()
	{
		if (!m_Runtime)
			return false;
		return m_Runtime.IsMounted();
	}

	//------------------------------------------------------------------------------------------------
	MUI_Runtime GetRuntime()
	{
		return m_Runtime;
	}

	//------------------------------------------------------------------------------------------------
	Widget GetHostWidget()
	{
		return m_wHost;
	}

	//------------------------------------------------------------------------------------------------
	protected void HideLegacyChildren()
	{
		if (!m_wRoot)
			return;

		Widget child = m_wRoot.GetChildren();
		while (child)
		{
			Widget next = child.GetSibling();
			if (child != m_wHost)
				child.SetVisible(false);
			child = next;
		}
	}
}
