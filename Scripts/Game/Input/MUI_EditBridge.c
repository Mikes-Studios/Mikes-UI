//------------------------------------------------------------------------------------------------
//! Hidden EditBoxWidget that feeds IME / console virtual keyboard into a MUI node.
//!
//! Consumer:
//!   Do not create this. MUI_Runtime.Mount() owns it. Focusing a WantsTextInput node
//!   on KBM, or pressing Select on gamepad, attaches the bridge.
//!
//! Extend:
//!   Custom fields: override WantsTextInput, GetEditText, SetEditTextFromBridge,
//!   CommitEdit on your MUI_Node subclass. No Mikes-UI edits required.
//!
//! One shared EditBox serves every field. It is IGNORE_CURSOR so write-mode cannot
//! steal hit-tests from the compositor. Switching fields commits in place (no hide)
//! because OnWriteModeLeave from the previous session would otherwise Detach the
//! newly attached node. SetFocused on an already-focused field must still BeginEditing.
//------------------------------------------------------------------------------------------------
class MUI_EditBridge : ScriptedWidgetEventHandler
{
	protected EditBoxWidget m_wEdit;
	protected Widget m_wHost;
	protected MUI_Node m_Node;
	protected WorkspaceWidget m_Workspace;
	protected string m_sAttachedText;
	protected bool m_bEmptyPlaceholder;
	protected bool m_bDetaching;
	protected bool m_bAttaching;
	protected int m_iSuppressLeave;

	//------------------------------------------------------------------------------------------------
	bool Create(notnull WorkspaceWidget workspace, notnull Widget parent)
	{
		m_Workspace = workspace;
		m_wHost = parent;
		int flags = WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR;
		Widget w = workspace.CreateWidget(WidgetType.EditBoxWidgetTypeID, flags, Color.FromInt(Color.WHITE), 20, parent);
		m_wEdit = EditBoxWidget.Cast(w);
		if (!m_wEdit)
		{
			MUI_Log.Error("Failed to create EditBoxWidget bridge");
			return false;
		}
		m_wEdit.AddHandler(this);
		m_wEdit.SetOpacity(0.02);
		m_wEdit.SetText(" ");
		m_wEdit.SetVisible(false);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void BindPointerHandler(MUI_InputRouter input)
	{
		if (!m_wEdit)
			return;
		if (!input)
			return;
		m_wEdit.AddHandler(input);
	}

	//------------------------------------------------------------------------------------------------
	void UnbindPointerHandler(MUI_InputRouter input)
	{
		if (!m_wEdit)
			return;
		if (!input)
			return;
		m_wEdit.RemoveHandler(input);
	}

	//------------------------------------------------------------------------------------------------
	void Tick()
	{
		if (m_iSuppressLeave <= 0)
			return;
		m_iSuppressLeave = m_iSuppressLeave - 1;
	}

	//------------------------------------------------------------------------------------------------
	void Attach(notnull MUI_Node node)
	{
		if (m_Node == node)
		{
			SyncLayout();
			EnsureWriteMode();
			return;
		}

		m_bAttaching = true;
		m_iSuppressLeave = 2;
		if (m_Node)
			CommitAttachedNode();

		m_Node = node;
		if (!m_wEdit)
		{
			m_bAttaching = false;
			return;
		}

		m_sAttachedText = node.GetEditText();
		m_bEmptyPlaceholder = m_sAttachedText.IsEmpty();
		SyncLayout();
		if (m_bEmptyPlaceholder)
			m_wEdit.SetText(" ");
		else
			m_wEdit.SetText(m_sAttachedText);
		EnsureWriteMode();
		m_bAttaching = false;
	}

	//------------------------------------------------------------------------------------------------
	protected void EnsureWriteMode()
	{
		if (!m_wEdit)
			return;
		m_iSuppressLeave = 2;
		m_wEdit.SetVisible(true);
		if (m_Workspace)
			m_Workspace.SetFocusedWidget(m_wEdit);
		m_wEdit.ActivateWriteMode();
	}

	//------------------------------------------------------------------------------------------------
	protected void CommitAttachedNode()
	{
		if (!m_Node)
			return;
		if (!m_wEdit)
			return;
		m_Node.SetEditTextFromBridge(ReadEditText());
		m_Node.CommitEdit();
	}

	//------------------------------------------------------------------------------------------------
	void SyncLayout()
	{
		if (!m_Node || !m_wEdit)
			return;
		MUI_Rect r = m_Node.GetWorldRect();
		FrameSlot.SetAnchorMin(m_wEdit, 0, 0);
		FrameSlot.SetAnchorMax(m_wEdit, 0, 0);
		FrameSlot.SetPos(m_wEdit, r.m_fX, r.m_fY + m_Node.GetSlideY());
		FrameSlot.SetSize(m_wEdit, r.m_fW, r.m_fH);
	}

	//------------------------------------------------------------------------------------------------
	protected string ReadEditText()
	{
		if (!m_wEdit)
			return "";
		string text = m_wEdit.GetText();
		if (m_bEmptyPlaceholder)
		{
			if (text.IsEmpty())
				return "";
			if (text == " ")
				return "";
			m_bEmptyPlaceholder = false;
		}
		return text;
	}

	//------------------------------------------------------------------------------------------------
	void Detach()
	{
		if (m_bDetaching)
			return;
		m_bDetaching = true;
		CommitAttachedNode();
		m_Node = null;
		m_bEmptyPlaceholder = false;
		m_sAttachedText = "";
		if (m_wEdit)
		{
			m_wEdit.SetVisible(false);
			m_wEdit.SetText(" ");
		}
		if (m_Workspace && m_wHost)
			m_Workspace.SetFocusedWidget(m_wHost);
		m_bDetaching = false;
	}

	//------------------------------------------------------------------------------------------------
	bool IsAttached()
	{
		return m_Node != null;
	}

	//------------------------------------------------------------------------------------------------
	MUI_Node GetNode()
	{
		return m_Node;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnChange(Widget w, bool finished)
	{
		if (!m_Node || !m_wEdit)
			return false;
		m_Node.SetEditTextFromBridge(ReadEditText());
		if (finished)
			m_Node.CommitEdit();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnWriteModeLeave(Widget w)
	{
		if (m_bDetaching)
			return false;
		if (m_bAttaching)
			return false;
		if (m_iSuppressLeave > 0)
			return false;
		if (m_wEdit && m_wEdit.IsInWriteMode())
			return false;
		Detach();
		return false;
	}

	//------------------------------------------------------------------------------------------------
	void Destroy()
	{
		Detach();
		if (m_wEdit)
		{
			m_wEdit.RemoveHandler(this);
			m_wEdit.RemoveFromHierarchy();
		}
		m_wEdit = null;
		m_wHost = null;
		m_Workspace = null;
	}
}
