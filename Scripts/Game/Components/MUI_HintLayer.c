//------------------------------------------------------------------------------------------------
//! Fullscreen tutorial overlay. Dims the menu, rings every visible registered control,
//! and lists them in a legend. One overlay per screen — not a step-through tour.
//!
//! Consumer:
//!   MUI_HintLayer hints = runtime.CreateHintLayer("hints");
//!   overlay.AddChild(hints); // last child so it paints on top
//!   hints.AddHint(aiField, "AI scale", "Multiplies spawned AI vs player count.");
//!   helpBtn.GetOnClicked().Insert(OnHelp); // calls hints.Toggle()
//!
//! Layout:
//!   Fill overlay, hidden until Open. Legend card Hug, align right. Back dismisses
//!   this layer (runtime modal) instead of closing the menu.
//!
//! Extend:
//!   Copy stays in the consuming menu. Group dense rows onto a section node.
//!   True cutouts are out of scope — Canvas only adds polygons.
//------------------------------------------------------------------------------------------------
class MUI_Hint
{
	MUI_Node m_Target;
	string m_sTitle;
	string m_sBody;
	int m_iIndex;
	bool m_bRingVisible;

	//------------------------------------------------------------------------------------------------
	void Init(MUI_Node target, string title, string body)
	{
		m_Target = target;
		m_sTitle = title;
		m_sBody = body;
		m_iIndex = 0;
		m_bRingVisible = false;
	}
}

//------------------------------------------------------------------------------------------------
class MUI_HintLayer : MUI_Node
{
	protected ref array<ref MUI_Hint> m_aHints;
	protected ref MUI_Card m_Legend;
	protected ref MUI_Header m_LegendHeader;
	protected ref MUI_Label m_LegendSub;
	protected ref MUI_Label m_Empty;
	protected ref MUI_ScrollView m_LegendList;
	protected ref MUI_Button m_Close;
	protected string m_sVisibleKey;
	protected bool m_bOpen;
	protected bool m_bChromeBuilt;

	//------------------------------------------------------------------------------------------------
	void MUI_HintLayer()
	{
		m_aHints = new array<ref MUI_Hint>();
		m_sVisibleKey = "";
		m_bOpen = false;
		m_bChromeBuilt = false;
		m_bVisible = false;

		m_Style.m_Layout = MUI_LayoutKind.Overlay;
		m_Style.m_WidthMode = MUI_SizeMode.Fill;
		m_Style.m_HeightMode = MUI_SizeMode.Fill;
		m_Style.m_Fill = Color.FromInt(0);
		m_Style.m_fRadius = 0;
		m_Style.m_bBlockHit = true;
		m_Style.m_bInteractive = true;
		SetPadding(24);
	}

	//------------------------------------------------------------------------------------------------
	override void SetRuntime(MUI_Runtime runtime)
	{
		super.SetRuntime(runtime);
		EnsureChrome();
	}

	//------------------------------------------------------------------------------------------------
	override void ApplyTheme(notnull MUI_ThemeData theme)
	{
		if (m_Legend)
			m_Legend.ApplyTheme(theme);
		if (m_Close)
			m_Close.ApplyTheme(theme);
	}

	//------------------------------------------------------------------------------------------------
	override bool WantsFocus()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnClicked()
	{
		Close();
	}

	//------------------------------------------------------------------------------------------------
	override bool HandleDismiss()
	{
		if (!m_bOpen)
			return false;
		Close();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void AddHint(MUI_Node target, string title, string body)
	{
		if (!target)
			return;
		if (!m_aHints)
			m_aHints = new array<ref MUI_Hint>();

		ref MUI_Hint hint = new MUI_Hint();
		hint.Init(target, title, body);
		m_aHints.Insert(hint);
	}

	//------------------------------------------------------------------------------------------------
	void Open()
	{
		if (m_bOpen)
			return;

		EnsureChrome();
		m_bOpen = true;
		m_sVisibleKey = "";
		SetVisible(true);
		SetIntro(0, 0.22, 0);
		if (m_Legend)
			m_Legend.SetIntro(0.04, 0.32, 22);

		SyncHintState();
		RebuildLegend();

		if (!m_Runtime)
			return;
		if (m_Runtime.IsEditing())
			m_Runtime.StopEditing();
		m_Runtime.SetModal(this);
	}

	//------------------------------------------------------------------------------------------------
	void Close()
	{
		if (!m_bOpen)
			return;

		m_bOpen = false;
		SetVisible(false);
		if (!m_Runtime)
			return;
		if (m_Runtime.GetModal() == this)
			m_Runtime.SetModal(null);
	}

	//------------------------------------------------------------------------------------------------
	void Toggle()
	{
		if (m_bOpen)
			Close();
		else
			Open();
	}

	//------------------------------------------------------------------------------------------------
	bool IsOpen()
	{
		return m_bOpen;
	}

	//------------------------------------------------------------------------------------------------
	override void OnTick(float dt)
	{
		if (!m_bOpen)
			return;

		SyncHintState();
		string key = BuildVisibleKey();
		if (key != m_sVisibleKey)
		{
			m_sVisibleKey = key;
			RebuildLegend();
		}
		InvalidatePaint();
	}

	//------------------------------------------------------------------------------------------------
	override void Paint(MUI_RenderSurface surface)
	{
		float op = GetDrawOpacity();
		if (op < 0.01)
			return;

		MUI_ThemeData theme = GetTheme();
		surface.FillRect(DrawX(), DrawY(), m_World.m_fW, m_World.m_fH, MUI_ColorUtil.Fade(theme.Overlay, op), 0);
		PaintRings(surface);
	}

	//------------------------------------------------------------------------------------------------
	protected void EnsureChrome()
	{
		if (m_bChromeBuilt)
			return;
		if (!m_Runtime)
			return;

		m_Legend = m_Runtime.CreateCard("hintLegend");
		m_Legend.SetWidth(320);
		m_Legend.SetPadding(20);
		m_Legend.SetGap(10);
		m_Legend.SetAlign(1, 0.5);

		m_LegendHeader = m_Runtime.CreateHeader("HINTS", "hintHeader");
		m_LegendHeader.SetKicker("THIS SCREEN");

		m_LegendSub = m_Runtime.CreateLabel("Each number below matches a highlighted setting on this page.", "hintSub");
		m_LegendSub.SetFontSize(m_Runtime.GetTheme().FONT_SMALL);
		m_LegendSub.SetMuted(true);

		m_Empty = m_Runtime.CreateLabel("There is no help for this page yet.", "hintEmpty");
		m_Empty.SetFontSize(m_Runtime.GetTheme().FONT_SMALL);
		m_Empty.SetMuted(true);
		m_Empty.SetVisible(false);

		m_LegendList = m_Runtime.CreateScrollView("hintList");
		m_LegendList.SetMaxViewportHeight(360);
		m_LegendList.SetGap(12);

		m_Close = m_Runtime.CreateButton("Close", "hintClose");
		m_Close.MakeAccent();
		m_Close.GetOnClicked().Insert(OnCloseClicked);

		m_Legend.AddChild(m_LegendHeader);
		m_Legend.AddChild(m_LegendSub);
		m_Legend.AddChild(m_Empty);
		m_Legend.AddChild(m_LegendList);
		m_Legend.AddChild(m_Close);
		AddChild(m_Legend);
		m_bChromeBuilt = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCloseClicked()
	{
		Close();
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsVisibleInTree(MUI_Node node)
	{
		MUI_Node walk = node;
		while (walk)
		{
			if (!walk.IsVisible())
				return false;
			walk = walk.GetParent();
		}
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsRingVisible(MUI_Node target)
	{
		if (!target)
			return false;
		if (!IsVisibleInTree(target))
			return false;

		MUI_Rect r = target.GetWorldRect();
		if (r.m_fW < 1)
			return false;
		if (r.m_fH < 1)
			return false;

		MUI_Node walk = target.GetParent();
		while (walk)
		{
			if (walk.ClipsChildren())
			{
				MUI_Rect c = walk.GetWorldRect();
				MUI_Style st = walk.GetStyle();
				float cx = c.m_fX + st.m_fPadL;
				float cy = c.m_fY + st.m_fPadT;
				float cw = c.m_fW - st.m_fPadL - st.m_fPadR;
				float ch = c.m_fH - st.m_fPadT - st.m_fPadB;
				if (r.m_fX + r.m_fW < cx)
					return false;
				if (r.m_fY + r.m_fH < cy)
					return false;
				if (r.m_fX > cx + cw)
					return false;
				if (r.m_fY > cy + ch)
					return false;
			}
			walk = walk.GetParent();
		}
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void SyncHintState()
	{
		if (!m_aHints)
			return;

		int nextIndex = 1;
		int count = m_aHints.Count();
		int i;
		for (i = 0; i < count; i++)
		{
			MUI_Hint hint = m_aHints[i];
			if (!hint)
				continue;
			if (!hint.m_Target)
			{
				hint.m_iIndex = 0;
				hint.m_bRingVisible = false;
				continue;
			}
			if (!IsVisibleInTree(hint.m_Target))
			{
				hint.m_iIndex = 0;
				hint.m_bRingVisible = false;
				continue;
			}

			hint.m_iIndex = nextIndex;
			nextIndex = nextIndex + 1;
			hint.m_bRingVisible = IsRingVisible(hint.m_Target);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected string BuildVisibleKey()
	{
		string key = "";
		if (!m_aHints)
			return key;

		int count = m_aHints.Count();
		int i;
		for (i = 0; i < count; i++)
		{
			MUI_Hint hint = m_aHints[i];
			if (!hint)
				continue;
			if (hint.m_iIndex < 1)
				continue;
			key = key + i.ToString() + ",";
		}
		return key;
	}

	//------------------------------------------------------------------------------------------------
	protected void RebuildLegend()
	{
		if (!m_Runtime)
			return;
		if (!m_LegendList)
			return;

		m_LegendList.ClearChildren();

		int shown = 0;
		int count = 0;
		if (m_aHints)
			count = m_aHints.Count();
		int i;
		for (i = 0; i < count; i++)
		{
			MUI_Hint hint = m_aHints[i];
			if (!hint)
				continue;
			if (hint.m_iIndex < 1)
				continue;

			shown = shown + 1;
			ref MUI_Panel row = m_Runtime.CreatePanel("hintRow");
			row.GetStyle().m_Fill = Color.FromInt(0);
			row.GetStyle().m_fRadius = 0;
			row.GetStyle().m_fGap = 4;
			row.GetStyle().m_bBlockHit = false;
			row.SetFillWidth();

			string title = hint.m_iIndex.ToString() + "  " + hint.m_sTitle;
			ref MUI_Label titleLbl = m_Runtime.CreateLabel(title, "hintTitle");
			titleLbl.SetBold(true);

			row.AddChild(titleLbl);
			if (!hint.m_sBody.IsEmpty())
			{
				ref MUI_Label bodyLbl = m_Runtime.CreateLabel(hint.m_sBody, "hintBody");
				bodyLbl.SetFontSize(m_Runtime.GetTheme().FONT_SMALL);
				bodyLbl.SetMuted(true);
				row.AddChild(bodyLbl);
			}
			m_LegendList.AddChild(row);
		}

		if (m_Empty)
			m_Empty.SetVisible(shown < 1);
		if (m_LegendList)
			m_LegendList.SetVisible(shown > 0);
		InvalidateLayout();
	}

	//------------------------------------------------------------------------------------------------
	protected void PaintRings(MUI_RenderSurface surface)
	{
		if (!m_aHints)
			return;

		float op = GetDrawOpacity();
		MUI_ThemeData theme = GetTheme();
		int count = m_aHints.Count();
		int i;
		for (i = 0; i < count; i++)
		{
			MUI_Hint hint = m_aHints[i];
			if (!hint)
				continue;
			if (!hint.m_bRingVisible)
				continue;
			MUI_Node target = hint.m_Target;
			if (!target)
				continue;

			MUI_Rect wr = target.GetWorldRect();
			float pad = 5;
			float x = target.DrawX() - pad;
			float y = target.DrawY() - pad;
			float w = wr.m_fW + pad * 2;
			float h = wr.m_fH + pad * 2;
			surface.FillRect(x, y, w, h, MUI_ColorUtil.Fade(theme.Accent, op * 0.14), 8);
			surface.StrokeRect(x, y, w, h, MUI_ColorUtil.Fade(theme.Cyan, op * 0.95), 1.6, 8);

			string badge = hint.m_iIndex.ToString();
			float bw = 22;
			if (hint.m_iIndex > 9)
				bw = 28;
			float bx = x - 4;
			float by = y - 10;
			surface.FillCircle(bx + 11, by + 11, 11, MUI_ColorUtil.Fade(theme.AccentDark, op));
			surface.StrokeCircle(bx + 11, by + 11, 11, MUI_ColorUtil.Fade(theme.Cyan, op), 1.4);
			surface.DrawText(bx, by, bw, 22, badge, theme.FONT_SMALL, MUI_ColorUtil.Fade(theme.Text, op), true, true, true, false);
		}
	}
}
