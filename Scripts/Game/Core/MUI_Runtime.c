//------------------------------------------------------------------------------------------------
//! Compositor instance. One per menu or HUD overlay. Owns surfaces, input, edit bridge,
//! and factory-created nodes (Retain / Adopt).
//!
//! Consumer (prefer MUI_MenuBase / MUI_HudHost instead of calling this directly):
//!   m_Runtime = new MUI_Runtime();
//!   m_Runtime.Mount(host);           // interactive menu
//!   m_Runtime.MountPassive(host);    // HUD: no input, no prompts, IGNORE_CURSOR canvases
//!   // Create* factories; every kept node is `protected ref`
//!   m_Runtime.SetRoot(overlay);
//!   m_Runtime.GetOnBack().Insert(OnClose);
//!   each frame: m_Runtime.Tick(dt);
//!   close: Unmount, RemoveFromHierarchy host
//!
//! Custom widgets:
//!   ref MyGauge g = new MyGauge();
//!   runtime.Adopt(g);
//!   parent.AddChild(g);
//!
//! HUD: Create(..., ignoreCursor) is required true. Canvas without IGNORE_CURSOR on
//! ALWAYS_TOP steals all mouse clicks from native UI.
//------------------------------------------------------------------------------------------------
class MUI_Runtime
{
	protected Widget m_wHost;
	protected WorkspaceWidget m_Workspace;
	protected ref MUI_Node m_Root;
	protected ref MUI_RenderSurface m_RootSurface;
	protected ref MUI_RenderSurface m_ChromeSurface;
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
	protected string m_sPromptSelect;
	protected string m_sPromptBack;
	protected ref MUI_ThemeData m_Theme;
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
		m_Theme = MUI_ThemeData.CreateUplink();
		m_sPromptSelect = "<action name='MenuSelect' scale='1.35'/>  Select";
		m_sPromptBack = "<action name='MenuBack' scale='1.35'/>  Back";
	}

	//------------------------------------------------------------------------------------------------
	//! Call before BuildUI / Create*. Hot-swap after SetRoot is not supported in 0.2.0.
	void SetTheme(notnull MUI_ThemeData theme)
	{
		m_Theme = theme;
	}

	//------------------------------------------------------------------------------------------------
	MUI_ThemeData GetTheme()
	{
		if (!m_Theme)
			m_Theme = MUI_ThemeData.CreateUplink();
		return m_Theme;
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
		{
			Unmount();
			return false;
		}
		m_RootSurface.FillHost();

		m_ChromeSurface = new MUI_RenderSurface();
		if (!m_ChromeSurface.Create(m_Workspace, m_wHost, 5, !interactive))
		{
			Unmount();
			return false;
		}
		m_ChromeSurface.FillHost();

		m_bInteractive = interactive;
		if (m_bInteractive)
		{
			m_Input.Init(this);
			m_wHost.AddHandler(m_Input);

			if (!m_Edit.Create(m_Workspace, m_wHost))
			{
				Unmount();
				return false;
			}
			m_Edit.BindPointerHandler(m_Input);

			CreatePrompts();
		}

		Widget measureW = m_Workspace.CreateWidget(WidgetType.TextWidgetTypeID, WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR, Color.FromInt(Color.WHITE), 0, m_wHost);
		m_wMeasure = TextWidget.Cast(measureW);
		if (m_wMeasure)
		{
			MUI_ThemeData theme = GetTheme();
			m_wMeasure.SetFont(theme.FONT_REGULAR);
			m_wMeasure.SetExactFontSize(theme.FONT_BODY);
			MUI_TextUtil.SetLiteral(m_wMeasure, " ");
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
		if (m_bInteractive && m_Edit && m_Input)
			m_Edit.UnbindPointerHandler(m_Input);
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
		{
			int oi;
			for (oi = 0; oi < m_aOwned.Count(); oi++)
			{
				if (m_aOwned[oi])
					m_aOwned[oi].DestroyHostWidgets();
			}
			m_aOwned.Clear();
		}
		if (m_Root)
			m_Root.DestroyHostWidgetsTree();

		if (m_ChromeSurface)
			m_ChromeSurface.Destroy();
		m_ChromeSurface = null;

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
	void FocusNode(MUI_Node node)
	{
		if (m_Input)
			m_Input.SetFocused(node);
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
		if (m_bInteractive && m_Edit)
		{
			m_Edit.Tick();
			m_Edit.SyncLayout();
		}
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
	WorkspaceWidget GetWorkspace()
	{
		return m_Workspace;
	}

	//------------------------------------------------------------------------------------------------
	Widget GetHost()
	{
		return m_wHost;
	}

	//------------------------------------------------------------------------------------------------
	//! Host frame for host widgets (blur/image). Prefer clip surface frame when under a ScrollView.
	Widget GetHostForNode(MUI_Node node)
	{
		MUI_Node clipOwner = null;
		MUI_Node walk = node;
		while (walk)
		{
			if (walk.ClipsChildren())
			{
				clipOwner = walk;
				break;
			}
			walk = walk.GetParent();
		}

		if (clipOwner)
		{
			int i;
			for (i = 0; i < m_aClipSurfaces.Count(); i++)
			{
				if (m_aClipSurfaces[i] && m_aClipSurfaces[i].GetClipNode() == clipOwner)
					return m_aClipSurfaces[i].GetFrame();
			}
		}
		return m_wHost;
	}

	//------------------------------------------------------------------------------------------------
	//! DrawX/Y converted into the host widget returned by GetHostForNode (clip frames are local).
	void GetHostLocalPos(MUI_Node node, out float x, out float y)
	{
		x = 0;
		y = 0;
		if (!node)
			return;
		x = node.DrawX();
		y = node.DrawY();
		Widget host = GetHostForNode(node);
		if (!host || host == m_wHost)
			return;

		MUI_Node walk = node.GetParent();
		while (walk)
		{
			if (walk.ClipsChildren())
			{
				x = node.DrawX() - walk.DrawX();
				y = node.DrawY() - walk.DrawY();
				return;
			}
			walk = walk.GetParent();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Paint()
	{
		m_bPaintDirty = false;
		if (!m_Root || !m_RootSurface || !m_ChromeSurface)
			return;

		int i;
		for (i = 0; i < m_aClipSurfaces.Count(); i++)
		{
			if (m_aClipSurfaces[i])
				m_aClipSurfaces[i].SetUsed(false);
		}

		MUI_ThemeData theme = GetTheme();
		m_RootSurface.ApplyThemeFonts(theme);
		m_ChromeSurface.ApplyThemeFonts(theme);
		for (i = 0; i < m_aClipSurfaces.Count(); i++)
		{
			if (m_aClipSurfaces[i])
				m_aClipSurfaces[i].ApplyThemeFonts(theme);
		}

		// Backdrop pass (FxBackdrop) under BlurWidgets.
		m_RootSurface.Begin();
		PaintNode(m_Root, m_RootSurface, 0, 0, 1, true);
		m_RootSurface.Submit();

		// Chrome pass (cards/controls) above BlurWidgets. Card.Paint syncs its blur rect.
		m_ChromeSurface.Begin();
		PaintNode(m_Root, m_ChromeSurface, 0, 0, 1, false);
		m_ChromeSurface.Submit();

		ReapClipSurfaces();

		// Keep host widgets (card blur) in sync even when their owners are hidden.
		if (m_Root)
			m_Root.SyncHostWidgetsTree();
	}

	//------------------------------------------------------------------------------------------------
	//! backdropPass: only paint nodes with PaintsOnBackdropLayer. Chrome pass skips those paints.
	protected void PaintNode(MUI_Node node, notnull MUI_RenderSurface surface, float ox, float oy, float opacity, bool backdropPass)
	{
		if (!node || !node.IsVisible())
			return;

		bool isBackdrop = node.PaintsOnBackdropLayer();
		bool paintSelf = false;
		if (backdropPass)
			paintSelf = isBackdrop;
		else
			paintSelf = !isBackdrop;

		if (paintSelf && surface.HasClipRect())
		{
			MUI_Rect wr = node.GetWorldRect();
			if (surface.IsFullyOutsideClip(wr.m_fX + ox, wr.m_fY + oy + node.GetSlideY(), wr.m_fW, wr.m_fH))
				return;
		}

		node.BeginDraw(ox, oy + node.GetSlideY(), opacity);
		if (paintSelf)
			node.Paint(surface);

		float childOy = oy + node.GetSlideY();
		float childOp = opacity * node.GetIntro() * node.GetOpacity();

		if (paintSelf && node.ClipsChildren())
		{
			MUI_RenderSurface clip = GetOrCreateClipSurface(node);
			if (clip)
			{
				clip.Begin();
				int count = node.GetChildCount();
				int i;
				for (i = 0; i < count; i++)
					PaintNode(node.GetChild(i), clip, ox, childOy, childOp, false);
				clip.Submit();
			}
			return;
		}

		int count = node.GetChildCount();
		int i;
		for (i = 0; i < count; i++)
			PaintNode(node.GetChild(i), surface, ox, childOy, childOp, backdropPass);
	}

	//------------------------------------------------------------------------------------------------
	protected MUI_RenderSurface GetOrCreateClipSurface(notnull MUI_Node node)
	{
		int i;
		for (i = 0; i < m_aClipSurfaces.Count(); i++)
		{
			if (m_aClipSurfaces[i] && m_aClipSurfaces[i].GetClipNode() == node)
			{
				m_aClipSurfaces[i].SetUsed(true);
				return m_aClipSurfaces[i];
			}
		}

		if (!m_Workspace || !m_wHost)
			return null;

		Widget parentW = m_wHost;
		MUI_Node ancestorClip = FindAncestorClip(node);
		if (ancestorClip)
		{
			MUI_RenderSurface parentSurf = FindClipSurface(ancestorClip);
			if (parentSurf && parentSurf.GetFrame())
				parentW = parentSurf.GetFrame();
		}

		ref MUI_RenderSurface surface = new MUI_RenderSurface();
		int z = 10 + m_aClipSurfaces.Count();
		if (!surface.Create(m_Workspace, parentW, z, !m_bInteractive))
			return null;
		surface.SetClipNode(node);
		surface.SetClipParentNode(ancestorClip);
		surface.ApplyThemeFonts(GetTheme());
		surface.SetUsed(true);
		surface.SyncFrameToNode();
		m_aClipSurfaces.Insert(surface);
		return surface;
	}

	//------------------------------------------------------------------------------------------------
	protected MUI_Node FindAncestorClip(notnull MUI_Node node)
	{
		MUI_Node walk = node.GetParent();
		while (walk)
		{
			if (walk.ClipsChildren())
				return walk;
			walk = walk.GetParent();
		}
		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected MUI_RenderSurface FindClipSurface(MUI_Node node)
	{
		if (!node)
			return null;
		int i;
		for (i = 0; i < m_aClipSurfaces.Count(); i++)
		{
			if (m_aClipSurfaces[i] && m_aClipSurfaces[i].GetClipNode() == node)
				return m_aClipSurfaces[i];
		}
		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Destroy clip canvases not visited this paint (hidden / removed ScrollViews leave ghosts otherwise).
	protected void ReapClipSurfaces()
	{
		ref array<ref MUI_RenderSurface> kept = new array<ref MUI_RenderSurface>();
		int i;
		for (i = 0; i < m_aClipSurfaces.Count(); i++)
		{
			MUI_RenderSurface surface = m_aClipSurfaces[i];
			if (surface && surface.WasUsed())
			{
				kept.Insert(surface);
				continue;
			}
			if (surface)
				surface.Destroy();
		}
		m_aClipSurfaces = kept;
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
		return HitTestNode(m_Root, x, y, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected MUI_Node HitTestNode(MUI_Node node, float x, float y, float parentSlide)
	{
		if (!node || !node.IsVisible())
			return null;

		float slide = parentSlide + node.GetSlideY();
		MUI_Rect wr = node.GetWorldRect();
		float nx = wr.m_fX;
		float ny = wr.m_fY + slide;
		if (x < nx)
			return null;
		if (y < ny)
			return null;
		if (x > nx + wr.m_fW)
			return null;
		if (y > ny + wr.m_fH)
			return null;

		int i;
		for (i = node.GetChildCount() - 1; i >= 0; i--)
		{
			MUI_Node child = node.GetChild(i);
			if (!child)
				continue;
			// Clipped parents must not hit-test children that sit outside the viewport.
			if (node.ClipsChildren())
			{
				MUI_Rect cr = child.GetWorldRect();
				float cy = cr.m_fY + slide + child.GetSlideY();
				if (cr.m_fX + cr.m_fW < nx)
					continue;
				if (cr.m_fX > nx + wr.m_fW)
					continue;
				if (cy + cr.m_fH < ny)
					continue;
				if (cy > ny + wr.m_fH)
					continue;
			}
			MUI_Node childHit = HitTestNode(child, x, y, slide);
			if (childHit)
				return childHit;
		}

		if (node.IsInteractive())
			return node;
		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Keep a focused control inside clipping ancestors (ScrollView).
	void EnsureNodeVisible(MUI_Node node)
	{
		if (!node)
			return;

		float yAdj = 0;
		MUI_Node walk = node.GetParent();
		while (walk)
		{
			if (walk.ClipsChildren())
			{
				MUI_Rect nr = node.GetWorldRect();
				MUI_Rect pr = walk.GetWorldRect();
				MUI_Style st = walk.GetStyle();
				float viewTop = pr.m_fY + st.m_fPadT;
				float viewBot = pr.m_fY + pr.m_fH - st.m_fPadB;
				float nodeTop = nr.m_fY + yAdj;
				float nodeBot = nodeTop + nr.m_fH;
				float dy = 0;
				float margin = 4;
				if (nodeTop < viewTop + margin)
					dy = nodeTop - (viewTop + margin);
				else if (nodeBot > viewBot - margin)
					dy = nodeBot - (viewBot - margin);
				if (dy != 0)
				{
					walk.SetScrollY(walk.GetScrollY() + dy);
					yAdj = yAdj - dy;
				}
			}
			walk = walk.GetParent();
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnFocusChanged(MUI_Node node)
	{
		InputManager im = GetGame().GetInputManager();
		bool kbm = true;
		if (im)
			kbm = im.IsUsingMouseAndKeyboard();

		if (node && node.WantsTextInput() && kbm)
		{
			m_Edit.Attach(node);
			return;
		}
		if (m_Edit)
			m_Edit.Detach();
	}

	//------------------------------------------------------------------------------------------------
	void BeginEditing(notnull MUI_Node node)
	{
		if (!node.WantsTextInput())
			return;
		if (m_Edit)
			m_Edit.Attach(node);
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
		if (node.WantsFocus())
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

		StylePrompt(m_wPromptSelect, m_sPromptSelect);
		StylePrompt(m_wPromptBack, m_sPromptBack);
		LayoutPrompts();
	}

	//------------------------------------------------------------------------------------------------
	//! Override Select/Back prompt rich text. Safe to call before or after Mount.
	void SetPromptText(string selectText, string backText)
	{
		m_sPromptSelect = selectText;
		m_sPromptBack = backText;
		if (m_wPromptSelect)
			StylePrompt(m_wPromptSelect, m_sPromptSelect);
		if (m_wPromptBack)
			StylePrompt(m_wPromptBack, m_sPromptBack);
	}

	//------------------------------------------------------------------------------------------------
	protected void StylePrompt(RichTextWidget tw, string text)
	{
		if (!tw)
			return;
		MUI_ThemeData theme = GetTheme();
		tw.SetFont(theme.FONT_BOLD);
		tw.SetExactFontSize(theme.FONT_BODY);
		tw.SetDesiredFontSize(theme.FONT_BODY);
		tw.SetSharpness(0.35);
		tw.SetOutline(1, 0xB0141410);
		tw.SetShadow(2, 0xA0000000, 1, 0, 1);
		tw.SetColor(theme.Text);
		if (text.IsEmpty())
			tw.SetTextFormat("%1", " ");
		else
			tw.SetTextFormat("%1", text);
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
		if (text.IsEmpty())
			return;

		MUI_ThemeData theme = GetTheme();
		if (bold)
			m_wMeasure.SetFont(theme.FONT_BOLD);
		else
			m_wMeasure.SetFont(theme.FONT_REGULAR);
		m_wMeasure.SetExactFontSize(fontSize);
		m_wMeasure.SetDesiredFontSize(fontSize);
		m_wMeasure.SetMinFontSize(fontSize);
		m_wMeasure.SetSharpness(0.35);

		// One-line sample first. GetTextSize/GetNumLines on a reused TextWidget
		// keep the previous string's height, and unwrapped height already includes
		// `\n` so multiplying that by line count leaves blank rows after the text shrinks.
		m_wMeasure.ClearFlags(WidgetFlags.WRAP_TEXT);
		m_wMeasure.SetTextWrapping(false);
		FrameSlot.SetSize(m_wMeasure, 8000, 64);
		MUI_TextUtil.SetLiteral(m_wMeasure, "Ag");
		m_wMeasure.Update();
		float sampleW;
		float lineH;
		m_wMeasure.GetTextSize(sampleW, lineH);
		if (lineH < fontSize)
			lineH = fontSize;

		MUI_TextUtil.SetLiteral(m_wMeasure, text);
		m_wMeasure.Update();
		float unW;
		float unusedH;
		m_wMeasure.GetTextSize(unW, unusedH);
		if (unW < 1)
			unW = sampleW;

		int lines;
		if (wrapWidth <= 0)
			lines = EstimateWrappedLineCount(text, unW, 8000);
		else
			lines = EstimateWrappedLineCount(text, unW, wrapWidth);
		if (lines < 1)
			lines = 1;

		h = lineH * lines;
		if (wrapWidth <= 0)
			w = unW;
		else if (unW < 1)
			w = wrapWidth;
		else if (unW > wrapWidth)
			w = wrapWidth;
		else
			w = unW;
	}

	//------------------------------------------------------------------------------------------------
	protected int EstimateWrappedLineCount(string text, float unwrappedW, float wrapWidth)
	{
		if (text.IsEmpty())
			return 1;
		if (wrapWidth < 8)
			return 1;

		int textLen = text.Length();
		if (textLen < 1)
			return 1;

		float avgChar = unwrappedW / textLen;
		array<string> paragraphs = new array<string>();
		text.Split("\n", paragraphs, false);
		if (paragraphs.IsEmpty())
			paragraphs.Insert(text);

		int total = 0;
		int p;
		for (p = 0; p < paragraphs.Count(); p++)
		{
			if (paragraphs[p].IsEmpty())
				continue;
			total = total + CountWordWrappedLines(paragraphs[p], wrapWidth, avgChar);
		}
		if (total < 1)
			total = 1;
		return total;
	}

	//------------------------------------------------------------------------------------------------
	protected int CountWordWrappedLines(string text, float wrapWidth, float avgChar)
	{
		if (text.IsEmpty())
			return 1;
		if (avgChar < 0.1)
			return 1;

		array<string> words = new array<string>();
		text.Split(" ", words, true);
		if (words.IsEmpty())
			return 1;

		int lines = 1;
		float x = 0;
		int i;
		for (i = 0; i < words.Count(); i++)
		{
			float wordW = avgChar * words[i].Length();
			if (i > 0)
				wordW = wordW + avgChar;
			if (x > 0 && x + wordW > wrapWidth)
			{
				lines = lines + 1;
				x = avgChar * words[i].Length();
			}
			else
				x = x + wordW;
		}
		return lines;
	}

	//------------------------------------------------------------------------------------------------
	protected void Retain(notnull MUI_Node node)
	{
		if (!m_aOwned)
			m_aOwned = new array<ref MUI_Node>();
		m_aOwned.Insert(node);
	}

	//------------------------------------------------------------------------------------------------
	protected void FinishCreate(notnull MUI_Node node, string name)
	{
		Retain(node);
		node.SetName(name);
		node.ApplyTheme(GetTheme());
	}

	//------------------------------------------------------------------------------------------------
	//! Register a consumer-created node (subclass of MUI_Node) with the runtime GC root.
	//! Always `ref` the local, Adopt, then AddChild.
	MUI_Node Adopt(notnull MUI_Node node)
	{
		Retain(node);
		node.ApplyTheme(GetTheme());
		return node;
	}

	//------------------------------------------------------------------------------------------------
	MUI_Panel CreatePanel(string name = "")
	{
		ref MUI_Panel panel = new MUI_Panel();
		FinishCreate(panel, name);
		return panel;
	}

	//------------------------------------------------------------------------------------------------
	MUI_Label CreateLabel(string text, string name = "")
	{
		ref MUI_Label label = new MUI_Label();
		FinishCreate(label, name);
		label.SetText(text);
		return label;
	}

	//------------------------------------------------------------------------------------------------
	MUI_Button CreateButton(string text, string name = "")
	{
		ref MUI_Button button = new MUI_Button();
		FinishCreate(button, name);
		button.SetText(text);
		return button;
	}

	//------------------------------------------------------------------------------------------------
	MUI_Toggle CreateToggle(string text, string name = "")
	{
		ref MUI_Toggle toggle = new MUI_Toggle();
		FinishCreate(toggle, name);
		toggle.SetText(text);
		return toggle;
	}

	//------------------------------------------------------------------------------------------------
	MUI_TextField CreateTextField(string label, string name = "")
	{
		ref MUI_TextField field = new MUI_TextField();
		FinishCreate(field, name);
		field.SetLabel(label);
		return field;
	}

	//------------------------------------------------------------------------------------------------
	MUI_ScrollView CreateScrollView(string name = "")
	{
		ref MUI_ScrollView scroll = new MUI_ScrollView();
		FinishCreate(scroll, name);
		return scroll;
	}

	//------------------------------------------------------------------------------------------------
	MUI_Row CreateRow(string name = "")
	{
		ref MUI_Row row = new MUI_Row();
		FinishCreate(row, name);
		return row;
	}

	//------------------------------------------------------------------------------------------------
	MUI_FxBackdrop CreateFxBackdrop(string name = "")
	{
		ref MUI_FxBackdrop fx = new MUI_FxBackdrop();
		FinishCreate(fx, name);
		return fx;
	}

	//------------------------------------------------------------------------------------------------
	MUI_Card CreateCard(string name = "")
	{
		ref MUI_Card card = new MUI_Card();
		FinishCreate(card, name);
		return card;
	}

	//------------------------------------------------------------------------------------------------
	MUI_LiveHeader CreateLiveHeader(string title, string name = "")
	{
		ref MUI_LiveHeader header = new MUI_LiveHeader();
		FinishCreate(header, name);
		header.SetTitle(title);
		return header;
	}

	//------------------------------------------------------------------------------------------------
	MUI_Hairline CreateHairline(string name = "")
	{
		ref MUI_Hairline line = new MUI_Hairline();
		FinishCreate(line, name);
		return line;
	}

	//------------------------------------------------------------------------------------------------
	MUI_Spacer CreateSpacer(float height = 8, string name = "")
	{
		ref MUI_Spacer spacer = new MUI_Spacer();
		FinishCreate(spacer, name);
		spacer.SetHeight(height);
		return spacer;
	}

	//------------------------------------------------------------------------------------------------
	MUI_Surface CreateSurface(string name = "")
	{
		ref MUI_Surface surface = new MUI_Surface();
		FinishCreate(surface, name);
		return surface;
	}

	//------------------------------------------------------------------------------------------------
	MUI_Header CreateHeader(string title, string name = "")
	{
		ref MUI_Header header = new MUI_Header();
		FinishCreate(header, name);
		header.SetTitle(title);
		return header;
	}

	//------------------------------------------------------------------------------------------------
	MUI_Divider CreateDivider(string name = "")
	{
		ref MUI_Divider line = new MUI_Divider();
		FinishCreate(line, name);
		return line;
	}

	//------------------------------------------------------------------------------------------------
	MUI_Image CreateImage(string name = "")
	{
		ref MUI_Image image = new MUI_Image();
		FinishCreate(image, name);
		return image;
	}

	//------------------------------------------------------------------------------------------------
	MUI_Progress CreateProgress(string name = "")
	{
		ref MUI_Progress progress = new MUI_Progress();
		FinishCreate(progress, name);
		return progress;
	}

	//------------------------------------------------------------------------------------------------
	MUI_Slider CreateSlider(string name = "")
	{
		ref MUI_Slider slider = new MUI_Slider();
		FinishCreate(slider, name);
		return slider;
	}

	//------------------------------------------------------------------------------------------------
	MUI_NumericField CreateNumericField(string label, string name = "")
	{
		ref MUI_NumericField field = new MUI_NumericField();
		FinishCreate(field, name);
		field.SetLabel(label);
		return field;
	}

	//------------------------------------------------------------------------------------------------
	MUI_Tabs CreateTabs(string name = "")
	{
		ref MUI_Tabs tabs = new MUI_Tabs();
		FinishCreate(tabs, name);
		return tabs;
	}

	//------------------------------------------------------------------------------------------------
	MUI_Dropdown CreateDropdown(string name = "")
	{
		ref MUI_Dropdown dropdown = new MUI_Dropdown();
		FinishCreate(dropdown, name);
		return dropdown;
	}
}
