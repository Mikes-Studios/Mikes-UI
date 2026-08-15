//------------------------------------------------------------------------------------------------
//! HUD-safe bootstrap. MountPassive + IGNORE_CURSOR so canvases never steal world clicks.
//!
//! Consumer (inside SCR_InfoDisplayExtended or similar):
//!   m_Hud = new MUI_HudHost();
//!   if (!m_Hud.Open(m_wRoot, "MyHud"))
//!     return;
//!   BuildTree(m_Hud.GetRuntime());
//!   DisplayUpdate: m_Hud.Tick(timeSlice);
//!   DisplayStopDraw: m_Hud.Close();
//!
//! Do not ActivateContext(MenuWithEditorContext) on HUD. Do not use MUI_MenuHost here.
//------------------------------------------------------------------------------------------------
class MUI_HudHost
{
	protected Widget m_wRoot;
	protected Widget m_wHost;
	protected ref MUI_Runtime m_Runtime;
	protected string m_sLogTag;

	//------------------------------------------------------------------------------------------------
	bool Open(notnull Widget root, string logTag, bool hideLegacy = true)
	{
		Close();

		m_wRoot = root;
		m_sLogTag = logTag;

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
		{
			Print(string.Format("[%1] No workspace for Mike's UI HUD", m_sLogTag), LogLevel.ERROR);
			return false;
		}

		m_wHost = workspace.CreateWidget(WidgetType.FrameWidgetTypeID, WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR, Color.FromInt(Color.WHITE), 50, m_wRoot);
		if (!m_wHost)
		{
			Print(string.Format("[%1] Failed to create Mike's UI HUD host", m_sLogTag), LogLevel.ERROR);
			return false;
		}

		FrameSlot.SetAnchorMin(m_wHost, 0, 0);
		FrameSlot.SetAnchorMax(m_wHost, 1, 1);
		FrameSlot.SetOffsets(m_wHost, 0, 0, 0, 0);
		m_wHost.SetFlags(WidgetFlags.IGNORE_CURSOR);
		if (m_wRoot)
			m_wRoot.SetFlags(WidgetFlags.IGNORE_CURSOR);

		m_Runtime = new MUI_Runtime();
		if (!m_Runtime.MountPassive(m_wHost))
		{
			Print(string.Format("[%1] Mike's UI HUD mount failed", m_sLogTag), LogLevel.ERROR);
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
		if (m_Runtime)
			m_Runtime.Tick(tDelta);
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
