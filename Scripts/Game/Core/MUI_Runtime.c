//------------------------------------------------------------------------------------------------
class MUI_Runtime
{
	protected Widget m_wHost;
	protected WorkspaceWidget m_Workspace;
	protected ref MUI_Node m_Root;
	protected ref MUI_RenderSurface m_RootSurface;
	protected ref array<ref MUI_RenderSurface> m_aClipSurfaces;
	protected ref array<ref MUI_Node> m_aOwned;
	protected ref MUI_InputRouter m_Input;
	protected ref MUI_EditBridge m_Edit;
	protected ref ScriptInvoker m_OnBack;
	protected Widget m_wPromptRoot;
	protected RichTextWidget m_wPromptSelect;
	protected RichTextWidget m_wPromptBack;
	protected TextWidget m_wMeasure;
	protected bool m_bLayoutDirty;
	protected bool m_bPaintDirty;
	protected bool m_bMounted;
	protected bool m_bInteractive;
	protected float m_fHostW;
	protected float m_fHostH;
	protected float m_fTime;

	//------------------------------------------------------------------------------------------------
	void MUI_Runtime()
	{
		m_aClipSurfaces = new array<ref MUI_RenderSurface>();
		m_aOwned = new array<ref MUI_Node>();
		m_Input = new MUI_InputRouter();
		m_Edit = new MUI_EditBridge();
		m_OnBack = new ScriptInvoker();
	}

	//------------------------------------------------------------------------------------------------
	bool Mount(notnull Widget host)
	{
		return MountInternal(host, true);
	}

	//------------------------------------------------------------------------------------------------
	//! HUD-safe mount: paint + layout only. No input router, edit bridge, or Select/Back prompts.
	bool MountPassive(notnull Widget host)
	{
		return MountInternal(host, false);
	}

	//------------------------------------------------------------------------------------------------
	protected bool MountInternal(notnull Widget host, bool interactive)
	{
		if (m_bMounted)
			Unmount();

		m_wHost = host;
		m_Workspace = GetGame().GetWorkspace();
		if (!m_Workspace)
		{
			MUI_Log.Error("No workspace");
			return false;
		}

		m_RootSurface = new MUI_RenderSurface();
		if (!m_RootSurface.Create(m_Workspace, m_wHost, 0, !interactive))
			return false;
		m_RootSurface.FillHost();

		m_bInteractive = interactive;
		if (m_bInteractive)
		{
			m_Input.Init(this);
			m_wHost.AddHandler(m_Input);

			if (!m_Edit.Create(m_Workspace, m_wHost))
				return false;

			CreatePrompts();
		}

		Widget measureW = m_Workspace.CreateWidget(WidgetType.TextWidgetTypeID, WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR, Color.FromInt(Color.WHITE), 0, m_wHost);
		m_wMeasure = TextWidget.Cast(measureW);
		if (m_wMeasure)
		{
			m_wMeasure.SetFont(MUI_Theme.FONT_REGULAR);
			m_wMeasure.SetExactFontSize(MUI_Theme.FONT_BODY);
			FrameSlot.SetPos(m_wMeasure, -4000, -4000);
			FrameSlot.SetSize(m_wMeasure, 8, 8);
		}

		m_bMounted = true;
		m_bLayoutDirty = true;
		m_bPaintDirty = true;
		if (m_bInteractive)
			MUI_Log.Info("Mounted");
		else
			MUI_Log.Info("Mounted (passive)");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void Unmount()
	{
		if (m_bInteractive && m_Input && m_wHost)
			m_wHost.RemoveHandler(m_Input);
		if (m_Input)
			m_Input.Clear();
		if (m_bInteractive && m_Edit)
			m_Edit.Destroy();
		if (m_bInteractive)
			DestroyPrompts();

		int i;
		for (i = 0; i < m_aClipSurfaces.Count(); i++)
		{
			if (m_aClipSurfaces[i])
				m_aClipSurfaces[i].Destroy();
		}
		m_aClipSurfaces.Clear();
		if (m_aOwned)
			m_aOwned.Clear();

		if (m_RootSurface)
			m_RootSurface.Destroy();
		m_RootSurface = null;

		if (m_wMeasure)
			m_wMeasure.RemoveFromHierarchy();
		m_wMeasure = null;

		m_Root = null;
		m_wHost = null;
		m_Workspace = null;
		m_bMounted = false;
		m_bInteractive = false;
		MUI_Log.Info("Unmounted");
	}

	//------------------------------------------------------------------------------------------------
	bool IsMounted()
	{
		return m_bMounted;
	}

	//------------------------------------------------------------------------------------------------
	void SetRoot(MUI_Node root)
	{
		m_Root = root;
		if (m_Root)
			m_Root.SetRuntime(this);
		MarkLayoutDirty();
	}

	//------------------------------------------------------------------------------------------------
	void Blur()
	{
		if (m_Input)
			m_Input.SetFocused(null);
	}

	//------------------------------------------------------------------------------------------------
	MUI_Node GetRoot()
	{
		return m_Root;
	}

	//------------------------------------------------------------------------------------------------
	void MarkLayoutDirty()
	{
		m_bLayoutDirty = true;
		m_bPaintDirty = true;
	}

	//------------------------------------------------------------------------------------------------
	void MarkPaintDirty()
	{
		m_bPaintDirty = true;
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float dt)
	{
		if (!m_bMounted)
			return;
		m_fTime = m_fTime + dt;
		if (m_Root)
			m_Root.Tick(dt);
		RefreshHostSize();
		if (m_fHostW < 1 || m_fHostH < 1)
			return;
		if (m_bInteractive && m_Input)
			m_Input.UpdatePointer();
		if (m_bLayoutDirty)
			Layout();
		Paint();
	}

	//------------------------------------------------------------------------------------------------
	float GetTime()
	{
		return m_fTime;
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshHostSize()
	{
		if (!m_wHost || !m_Workspace)
			return;
		float sw;
		float sh;
		m_wHost.GetScreenSize(sw, sh);
		float w = m_Workspace.DPIUnscale(sw);
		float h = m_Workspace.DPIUnscale(sh);
		if (w < 1 || h < 1)
		{
			w = m_Workspace.DPIUnscale(m_Workspace.GetWidth());
			h = m_Workspace.DPIUnscale(m_Workspace.GetHeight());
		}
		if (w != m_fHostW || h != m_fHostH)
		{
			m_fHostW = w;
			m_fHostH = h;
			m_bLayoutDirty = true;
			m_bPaintDirty = true;
			if (m_bInteractive)
				LayoutPrompts();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Layout()
	{
		m_bLayoutDirty = false;
		if (!m_Root)
			return;
		MUI_LayoutEngine.LayoutTree(m_Root, 0, 0, m_fHostW, m_fHostH);
		m_bPaintDirty = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void Paint()
	{
		m_bPaintDirty = false;
		if (!m_Root || !m_RootSurface)
			return;
		m_RootSurface.Begin();
		PaintNode(m_Root, m_RootSurface, 0, 0, 1);
		m_RootSurface.Submit();
	}

	//------------------------------------------------------------------------------------------------
	protected void PaintNode(MUI_Node node, notnull MUI_RenderSurface surface, float ox, float oy, float opacity)
	{
		if (!node || !node.IsVisible())
			return;

		node.BeginDraw(ox, oy + node.GetSlideY(), opacity);
		node.Paint(surface);

		float childOy = oy + node.GetSlideY();
		float childOp = opacity * node.GetIntro();

		if (node.ClipsChildren())
		{
			MUI_RenderSurface clip = GetOrCreateClipSurface(node);
			if (clip)
			{
				clip.Begin();
				int count = node.GetChildCount();
				int i;
				for (i = 0; i < count; i++)
					PaintNode(node.GetChild(i), clip, ox, childOy, childOp);
				clip.Submit();
			}
			return;
		}

		int count = node.GetChildCount();
		int i;
		for (i = 0; i < count; i++)
			PaintNode(node.GetChild(i), surface, ox, childOy, childOp);
	}

	//------------------------------------------------------------------------------------------------
	protected MUI_RenderSurface GetOrCreateClipSurface(notnull MUI_Node node)
	{
		int i;
		for (i = 0; i < m_aClipSurfaces.Count(); i++)
		{
			if (m_aClipSurfaces[i].GetClipNode() == node)
				return m_aClipSurfaces[i];
		}

		if (!m_Workspace || !m_wHost)
			return null;

		ref MUI_RenderSurface surface = new MUI_RenderSurface();
		int z = 2 + m_aClipSurfaces.Count();
		if (!surface.Create(m_Workspace, m_wHost, z, !m_bInteractive))
			return null;
		surface.SetClipNode(node);
		surface.SyncFrameToNode();
		m_aClipSurfaces.Insert(surface);
		return surface;
	}

	//------------------------------------------------------------------------------------------------
	bool GetLocalPointer(out float x, out float y)
	{
		x = 0;
		y = 0;
		if (!m_wHost || !m_Workspace)
			return false;

		int mx;
		int my;
		WidgetManager.GetMousePos(mx, my);
		float sx;
		float sy;
		m_wHost.GetScreenPos(sx, sy);
		x = m_Workspace.DPIUnscale(mx) - m_Workspace.DPIUnscale(sx);
		y = m_Workspace.DPIUnscale(my) - m_Workspace.DPIUnscale(sy);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	MUI_Node HitTest(float x, float y)
	{
		if (!m_Root)
			return null;
		return HitTestNode(m_Root, x, y);
	}

	//------------------------------------------------------------------------------------------------
	protected MUI_Node HitTestNode(MUI_Node node, float x, float y)
	{
		if (!node || !node.IsVisible())
			return null;
		if (!node.GetWorldRect().Contains(x, y))
			return null;

		int i;
		for (i = node.GetChildCount() - 1; i >= 0; i--)
		{
			MUI_Node childHit = HitTestNode(node.GetChild(i), x, y);
			if (childHit)
				return childHit;
		}

		if (node.IsInteractive())
			return node;
		return null;
	}

	//------------------------------------------------------------------------------------------------
	void OnFocusChanged(MUI_Node node)
	{
		MUI_TextField field = MUI_TextField.Cast(node);
		InputManager im = GetGame().GetInputManager();
		bool kbm = true;
		if (im)
			kbm = im.IsUsingMouseAndKeyboard();

		if (field && kbm)
		{
			m_Edit.Attach(field);
			return;
		}
		if (m_Edit)
			m_Edit.Detach();
	}

	//------------------------------------------------------------------------------------------------
	void BeginEditing(notnull MUI_TextField field)
	{
		if (m_Edit)
			m_Edit.Attach(field);
	}

	//------------------------------------------------------------------------------------------------
	void StopEditing()
	{
		if (m_Edit)
			m_Edit.Detach();
	}

	//------------------------------------------------------------------------------------------------
	bool IsEditing()
	{
		if (!m_Edit)
			return false;
		return m_Edit.IsAttached();
	}

	//------------------------------------------------------------------------------------------------
	void RequestBack()
	{
		if (m_OnBack)
			m_OnBack.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnBack()
	{
		return m_OnBack;
	}

	//------------------------------------------------------------------------------------------------
	void CollectFocusables(notnull array<MUI_Node> list)
	{
		CollectFocusablesNode(m_Root, list);
	}

	//------------------------------------------------------------------------------------------------
	protected void CollectFocusablesNode(MUI_Node node, notnull array<MUI_Node> list)
	{
		if (!node || !node.IsVisible())
			return;
		if (node.AcceptsClick())
			list.Insert(node);
		int count = node.GetChildCount();
		int i;
		for (i = 0; i < count; i++)
			CollectFocusablesNode(node.GetChild(i), list);
	}

	//------------------------------------------------------------------------------------------------
	protected void CreatePrompts()
	{
		if (!m_Workspace || !m_wHost)
			return;

		m_wPromptRoot = m_Workspace.CreateWidget(WidgetType.FrameWidgetTypeID, WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR, Color.FromInt(Color.WHITE), 80, m_wHost);
		if (!m_wPromptRoot)
			return;
		FrameSlot.SetAnchorMin(m_wPromptRoot, 0.5, 1);
		FrameSlot.SetAnchorMax(m_wPromptRoot, 0.5, 1);
		FrameSlot.SetAlignment(m_wPromptRoot, 0.5, 1);
		FrameSlot.SetPos(m_wPromptRoot, 0, -18);
		FrameSlot.SetSize(m_wPromptRoot, 420, 40);

		Widget selW = m_Workspace.CreateWidget(WidgetType.RichTextWidgetTypeID, WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR | WidgetFlags.CENTER | WidgetFlags.VCENTER, Color.FromInt(Color.WHITE), 0, m_wPromptRoot);
		m_wPromptSelect = RichTextWidget.Cast(selW);
		Widget backW = m_Workspace.CreateWidget(WidgetType.RichTextWidgetTypeID, WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR | WidgetFlags.CENTER | WidgetFlags.VCENTER, Color.FromInt(Color.WHITE), 0, m_wPromptRoot);
		m_wPromptBack = RichTextWidget.Cast(backW);

		StylePrompt(m_wPromptSelect, "<action name='MenuSelect' scale='1.35'/>  Select");
		StylePrompt(m_wPromptBack, "<action name='MenuBack' scale='1.35'/>  Back");
		LayoutPrompts();
	}

	//------------------------------------------------------------------------------------------------
	protected void StylePrompt(RichTextWidget tw, string text)
	{
		if (!tw)
			return;
		tw.SetFont(MUI_Theme.FONT_BOLD);
		tw.SetExactFontSize(MUI_Theme.FONT_BODY);
		tw.SetDesiredFontSize(MUI_Theme.FONT_BODY);
		tw.SetSharpness(0.35);
		tw.SetOutline(1, 0xB0141410);
		tw.SetShadow(2, 0xA0000000, 1, 0, 1);
		tw.SetColor(MUI_Theme.Text);
		tw.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	protected void LayoutPrompts()
	{
		if (!m_wPromptSelect || !m_wPromptBack)
			return;
		FrameSlot.SetPos(m_wPromptSelect, 20, 4);
		FrameSlot.SetSize(m_wPromptSelect, 180, 32);
		FrameSlot.SetPos(m_wPromptBack, 220, 4);
		FrameSlot.SetSize(m_wPromptBack, 180, 32);
	}

	//------------------------------------------------------------------------------------------------
	protected void DestroyPrompts()
	{
		if (m_wPromptRoot)
			m_wPromptRoot.RemoveFromHierarchy();
		m_wPromptRoot = null;
		m_wPromptSelect = null;
		m_wPromptBack = null;
	}

	//------------------------------------------------------------------------------------------------
	void MeasureText(string text, int fontSize, bool bold, float wrapWidth, out float w, out float h)
	{
		w = 0;
		h = fontSize;
		if (!m_wMeasure)
			return;
		if (bold)
			m_wMeasure.SetFont(MUI_Theme.FONT_BOLD);
		else
			m_wMeasure.SetFont(MUI_Theme.FONT_REGULAR);
		m_wMeasure.SetExactFontSize(fontSize);
		m_wMeasure.SetDesiredFontSize(fontSize);
		m_wMeasure.SetMinFontSize(fontSize);
		m_wMeasure.SetSharpness(0.35);
		m_wMeasure.SetText(text);
		if (wrapWidth > 0)
		{
			m_wMeasure.SetTextWrapping(true);
			FrameSlot.SetSize(m_wMeasure, wrapWidth, 2000);
		}
		else
		{
			m_wMeasure.SetTextWrapping(false);
		}
		m_wMeasure.GetTextSize(w, h);
	}

	//------------------------------------------------------------------------------------------------
	protected void Retain(notnull MUI_Node node)
	{
		if (!m_aOwned)
			m_aOwned = new array<ref MUI_Node>();
		m_aOwned.Insert(node);
	}

	//------------------------------------------------------------------------------------------------
	MUI_Panel CreatePanel(string name = "")
	{
		ref MUI_Panel panel = new MUI_Panel();
		Retain(panel);
		panel.SetName(name);
		return panel;
	}

	//------------------------------------------------------------------------------------------------
	MUI_Label CreateLabel(string text, string name = "")
	{
		ref MUI_Label label = new MUI_Label();
		Retain(label);
		label.SetName(name);
		label.SetText(text);
		return label;
	}

	//------------------------------------------------------------------------------------------------
	MUI_Button CreateButton(string text, string name = "")
	{
		ref MUI_Button button = new MUI_Button();
		Retain(button);
		button.SetName(name);
		button.SetText(text);
		return button;
	}

	//------------------------------------------------------------------------------------------------
	MUI_Toggle CreateToggle(string text, string name = "")
	{
		ref MUI_Toggle toggle = new MUI_Toggle();
		Retain(toggle);
		toggle.SetName(name);
		toggle.SetText(text);
		return toggle;
	}

	//------------------------------------------------------------------------------------------------
	MUI_TextField CreateTextField(string label, string name = "")
	{
		ref MUI_TextField field = new MUI_TextField();
		Retain(field);
		field.SetName(name);
		field.SetLabel(label);
		return field;
	}

	//------------------------------------------------------------------------------------------------
	MUI_ScrollView CreateScrollView(string name = "")
	{
		ref MUI_ScrollView scroll = new MUI_ScrollView();
		Retain(scroll);
		scroll.SetName(name);
		return scroll;
	}

	//------------------------------------------------------------------------------------------------
	MUI_Row CreateRow(string name = "")
	{
		ref MUI_Row row = new MUI_Row();
		Retain(row);
		row.SetName(name);
		return row;
	}

	//------------------------------------------------------------------------------------------------
	MUI_FxBackdrop CreateFxBackdrop(string name = "")
	{
		ref MUI_FxBackdrop fx = new MUI_FxBackdrop();
		Retain(fx);
		fx.SetName(name);
		return fx;
	}

	//------------------------------------------------------------------------------------------------
	MUI_Card CreateCard(string name = "")
	{
		ref MUI_Card card = new MUI_Card();
		Retain(card);
		card.SetName(name);
		return card;
	}

	//------------------------------------------------------------------------------------------------
	MUI_LiveHeader CreateLiveHeader(string title, string name = "")
	{
		ref MUI_LiveHeader header = new MUI_LiveHeader();
		Retain(header);
		header.SetName(name);
		header.SetTitle(title);
		return header;
	}

	//------------------------------------------------------------------------------------------------
	MUI_Hairline CreateHairline(string name = "")
	{
		ref MUI_Hairline line = new MUI_Hairline();
		Retain(line);
		line.SetName(name);
		return line;
	}
}
