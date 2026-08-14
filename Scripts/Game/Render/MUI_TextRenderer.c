//------------------------------------------------------------------------------------------------
class MUI_TextItem
{
	float m_fX;
	float m_fY;
	float m_fW;
	float m_fH;
	string m_sText;
	int m_iFontSize;
	int m_iColor;
	bool m_bBold;
	bool m_bCenter;
	bool m_bVCenter;
	bool m_bWrap;
}

//------------------------------------------------------------------------------------------------
class MUI_TextRenderer
{
	protected ref array<ref MUI_TextItem> m_aItems;
	protected ref array<TextWidget> m_aPool;
	protected Widget m_wLayer;
	protected WorkspaceWidget m_Workspace;

	//------------------------------------------------------------------------------------------------
	void MUI_TextRenderer()
	{
		m_aItems = new array<ref MUI_TextItem>();
		m_aPool = new array<TextWidget>();
	}

	//------------------------------------------------------------------------------------------------
	void Init(notnull WorkspaceWidget workspace, notnull Widget layer)
	{
		m_Workspace = workspace;
		m_wLayer = layer;
	}

	//------------------------------------------------------------------------------------------------
	void Begin()
	{
		m_aItems.Clear();
	}

	//------------------------------------------------------------------------------------------------
	void Add(float x, float y, float w, float h, string text, int fontSize, int color, bool bold, bool center, bool vCenter, bool wrap)
	{
		if (text == "")
			return;
		ref MUI_TextItem item = new MUI_TextItem();
		item.m_fX = x;
		item.m_fY = y;
		item.m_fW = w;
		item.m_fH = h;
		item.m_sText = text;
		item.m_iFontSize = fontSize;
		item.m_iColor = color;
		item.m_bBold = bold;
		item.m_bCenter = center;
		item.m_bVCenter = vCenter;
		item.m_bWrap = wrap;
		m_aItems.Insert(item);
	}

	//------------------------------------------------------------------------------------------------
	void Submit()
	{
		if (!m_Workspace || !m_wLayer)
			return;

		int needed = m_aItems.Count();
		EnsurePool(needed);

		int i;
		for (i = 0; i < m_aPool.Count(); i++)
		{
			TextWidget tw = m_aPool[i];
			if (i >= needed)
			{
				tw.SetVisible(false);
				continue;
			}

			MUI_TextItem item = m_aItems[i];
			tw.SetVisible(true);
			tw.SetText(item.m_sText);
			tw.SetDesiredFontSize(item.m_iFontSize);
			tw.SetMinFontSize(item.m_iFontSize);
			tw.SetExactFontSize(item.m_iFontSize);
			tw.SetSharpness(0.35);
			tw.SetOutline(1, 0xB0141410);
			tw.SetShadow(2, 0xA0000000, 1, 0, 1);
			tw.SetColorInt(item.m_iColor);
			tw.SetTextWrapping(item.m_bWrap);

			int flags = WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR | WidgetFlags.INHERIT_CLIPPING;
			if (item.m_bCenter)
				flags = flags | WidgetFlags.CENTER;
			if (item.m_bVCenter)
				flags = flags | WidgetFlags.VCENTER;
			if (item.m_bWrap)
				flags = flags | WidgetFlags.WRAP_TEXT;
			tw.ClearFlags(WidgetFlags.CENTER | WidgetFlags.VCENTER | WidgetFlags.WRAP_TEXT);
			tw.SetFlags(flags);

			if (item.m_bBold)
				tw.SetFont(MUI_Theme.FONT_BOLD);
			else
				tw.SetFont(MUI_Theme.FONT_REGULAR);
			tw.SetBold(false);

			FrameSlot.SetAnchorMin(tw, 0, 0);
			FrameSlot.SetAnchorMax(tw, 0, 0);
			FrameSlot.SetPos(tw, Math.Round(item.m_fX), Math.Round(item.m_fY));
			FrameSlot.SetSize(tw, item.m_fW, item.m_fH);
			FrameSlot.SetSizeToContent(tw, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void EnsurePool(int needed)
	{
		while (m_aPool.Count() < needed)
		{
			Widget w = m_Workspace.CreateWidget(WidgetType.TextWidgetTypeID, WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR | WidgetFlags.INHERIT_CLIPPING, Color.FromInt(Color.WHITE), 0, m_wLayer);
			TextWidget tw = TextWidget.Cast(w);
			if (!tw)
			{
				MUI_Log.Error("Failed to create TextWidget for pool");
				return;
			}
			tw.SetFont(MUI_Theme.FONT_REGULAR);
			tw.SetSharpness(0.35);
			tw.SetOutline(1, 0xB0141410);
			tw.SetShadow(2, 0xA0000000, 1, 0, 1);
			m_aPool.Insert(tw);
		}
	}

	//------------------------------------------------------------------------------------------------
	void Destroy()
	{
		int i;
		for (i = 0; i < m_aPool.Count(); i++)
		{
			if (m_aPool[i])
				m_aPool[i].RemoveFromHierarchy();
		}
		m_aPool.Clear();
		m_aItems.Clear();
		m_wLayer = null;
		m_Workspace = null;
	}
}
