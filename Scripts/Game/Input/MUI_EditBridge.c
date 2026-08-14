//------------------------------------------------------------------------------------------------
class MUI_EditBridge : ScriptedWidgetEventHandler
{
	protected EditBoxWidget m_wEdit;
	protected MUI_TextField m_Field;
	protected WorkspaceWidget m_Workspace;

	//------------------------------------------------------------------------------------------------
	bool Create(notnull WorkspaceWidget workspace, notnull Widget parent)
	{
		m_Workspace = workspace;
		Widget w = workspace.CreateWidget(WidgetType.EditBoxWidgetTypeID, WidgetFlags.VISIBLE, Color.FromInt(Color.WHITE), 20, parent);
		m_wEdit = EditBoxWidget.Cast(w);
		if (!m_wEdit)
		{
			MUI_Log.Error("Failed to create EditBoxWidget bridge");
			return false;
		}
		m_wEdit.AddHandler(this);
		m_wEdit.SetOpacity(0.02);
		m_wEdit.SetVisible(false);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void Attach(notnull MUI_TextField field)
	{
		m_Field = field;
		if (!m_wEdit)
			return;

		MUI_Rect r = field.GetWorldRect();
		m_wEdit.SetVisible(true);
		FrameSlot.SetAnchorMin(m_wEdit, 0, 0);
		FrameSlot.SetAnchorMax(m_wEdit, 0, 0);
		FrameSlot.SetPos(m_wEdit, r.m_fX, r.m_fY);
		FrameSlot.SetSize(m_wEdit, r.m_fW, r.m_fH);
		m_wEdit.SetText(field.GetText());
		if (m_Workspace)
			m_Workspace.SetFocusedWidget(m_wEdit);
		m_wEdit.ActivateWriteMode();
	}

	//------------------------------------------------------------------------------------------------
	void Detach()
	{
		if (m_Field && m_wEdit)
			m_Field.SetTextFromBridge(m_wEdit.GetText());
		m_Field = null;
		if (m_wEdit)
		{
			m_wEdit.SetVisible(false);
			m_wEdit.SetText("");
		}
	}

	//------------------------------------------------------------------------------------------------
	bool IsAttached()
	{
		return m_Field != null;
	}

	//------------------------------------------------------------------------------------------------
	MUI_TextField GetField()
	{
		return m_Field;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnChange(Widget w, bool finished)
	{
		if (!m_Field || !m_wEdit)
			return false;
		m_Field.SetTextFromBridge(m_wEdit.GetText());
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnWriteModeLeave(Widget w)
	{
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
		m_Workspace = null;
	}
}
