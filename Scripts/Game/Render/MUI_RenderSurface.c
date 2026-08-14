//------------------------------------------------------------------------------------------------
class MUI_RenderSurface
{
	protected Widget m_wFrame;
	protected CanvasWidget m_wCanvas;
	protected Widget m_wTextLayer;
	protected WorkspaceWidget m_Workspace;
	protected ref array<ref CanvasWidgetCommand> m_aCommands;
	protected ref MUI_TextRenderer m_Text;
	protected ref MUI_Rect m_Origin;
	protected MUI_Node m_ClipNode;
	protected bool m_bIgnoreCursor;

	//------------------------------------------------------------------------------------------------
	void MUI_RenderSurface()
	{
		m_aCommands = new array<ref CanvasWidgetCommand>();
		m_Text = new MUI_TextRenderer();
		m_Origin = new MUI_Rect();
	}

	//------------------------------------------------------------------------------------------------
	bool Create(notnull WorkspaceWidget workspace, notnull Widget parent, int zOrder, bool ignoreCursor = false)
	{
		m_Workspace = workspace;
		m_bIgnoreCursor = ignoreCursor;

		int frameFlags = WidgetFlags.VISIBLE | WidgetFlags.CLIPCHILDREN;
		int canvasFlags = WidgetFlags.VISIBLE;
		int textFlags = WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR;
		if (m_bIgnoreCursor)
		{
			frameFlags = frameFlags | WidgetFlags.IGNORE_CURSOR;
			canvasFlags = canvasFlags | WidgetFlags.IGNORE_CURSOR;
		}

		m_wFrame = workspace.CreateWidget(WidgetType.FrameWidgetTypeID, frameFlags, Color.FromInt(Color.WHITE), zOrder, parent);
		if (!m_wFrame)
		{
			MUI_Log.Error("Failed to create render surface frame");
			return false;
		}

		FrameSlot.SetAnchorMin(m_wFrame, 0, 0);
		FrameSlot.SetAnchorMax(m_wFrame, 1, 1);
		FrameSlot.SetOffsets(m_wFrame, 0, 0, 0, 0);

		Widget canvasW = workspace.CreateWidget(WidgetType.CanvasWidgetTypeID, canvasFlags, Color.FromInt(Color.WHITE), 0, m_wFrame);
		m_wCanvas = CanvasWidget.Cast(canvasW);
		if (!m_wCanvas)
		{
			MUI_Log.Error("Failed to create CanvasWidget");
			return false;
		}
		FrameSlot.SetAnchorMin(m_wCanvas, 0, 0);
		FrameSlot.SetAnchorMax(m_wCanvas, 1, 1);
		FrameSlot.SetOffsets(m_wCanvas, 0, 0, 0, 0);

		m_wTextLayer = workspace.CreateWidget(WidgetType.FrameWidgetTypeID, textFlags, Color.FromInt(Color.WHITE), 1, m_wFrame);
		if (!m_wTextLayer)
		{
			MUI_Log.Error("Failed to create text layer");
			return false;
		}
		FrameSlot.SetAnchorMin(m_wTextLayer, 0, 0);
		FrameSlot.SetAnchorMax(m_wTextLayer, 1, 1);
		FrameSlot.SetOffsets(m_wTextLayer, 0, 0, 0, 0);

		m_Text.Init(workspace, m_wTextLayer);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void SetClipNode(MUI_Node node)
	{
		m_ClipNode = node;
	}

	//------------------------------------------------------------------------------------------------
	MUI_Node GetClipNode()
	{
		return m_ClipNode;
	}

	//------------------------------------------------------------------------------------------------
	void SyncFrameToNode()
	{
		if (!m_wFrame || !m_ClipNode)
			return;
		MUI_Rect r = m_ClipNode.GetWorldRect();
		FrameSlot.SetAnchorMin(m_wFrame, 0, 0);
		FrameSlot.SetAnchorMax(m_wFrame, 0, 0);
		FrameSlot.SetPos(m_wFrame, m_ClipNode.DrawX(), m_ClipNode.DrawY());
		FrameSlot.SetSize(m_wFrame, r.m_fW, r.m_fH);
		m_Origin.Set(m_ClipNode.DrawX(), m_ClipNode.DrawY(), r.m_fW, r.m_fH);
	}

	//------------------------------------------------------------------------------------------------
	void FillHost()
	{
		if (!m_wFrame)
			return;
		FrameSlot.SetAnchorMin(m_wFrame, 0, 0);
		FrameSlot.SetAnchorMax(m_wFrame, 1, 1);
		FrameSlot.SetOffsets(m_wFrame, 0, 0, 0, 0);
		m_Origin.Set(0, 0, 0, 0);
	}

	//------------------------------------------------------------------------------------------------
	void Begin()
	{
		m_aCommands.Clear();
		m_Text.Begin();
		if (m_ClipNode)
			SyncFrameToNode();
	}

	//------------------------------------------------------------------------------------------------
	protected float Px(float layout)
	{
		if (!m_Workspace)
			return layout;
		return m_Workspace.DPIScale(layout);
	}

	//------------------------------------------------------------------------------------------------
	void FillRect(float x, float y, float w, float h, Color color, float radius)
	{
		if (!m_wCanvas || !color)
			return;
		MUI_CanvasRenderer.AddFillRect(m_aCommands, Px(x - m_Origin.m_fX), Px(y - m_Origin.m_fY), Px(w), Px(h), color.PackToInt(), Px(radius));
	}

	//------------------------------------------------------------------------------------------------
	void StrokeRect(float x, float y, float w, float h, Color color, float width, float radius)
	{
		if (!m_wCanvas || !color)
			return;
		MUI_CanvasRenderer.AddStrokeRect(m_aCommands, Px(x - m_Origin.m_fX), Px(y - m_Origin.m_fY), Px(w), Px(h), color.PackToInt(), Px(width), Px(radius));
	}

	//------------------------------------------------------------------------------------------------
	void DrawLine(float x0, float y0, float x1, float y1, Color color, float width)
	{
		if (!m_wCanvas || !color)
			return;
		MUI_CanvasRenderer.AddLine(m_aCommands, Px(x0 - m_Origin.m_fX), Px(y0 - m_Origin.m_fY), Px(x1 - m_Origin.m_fX), Px(y1 - m_Origin.m_fY), color.PackToInt(), Px(width));
	}

	//------------------------------------------------------------------------------------------------
	void FillCircle(float cx, float cy, float r, Color color)
	{
		if (!m_wCanvas || !color)
			return;
		MUI_CanvasRenderer.AddFillCircle(m_aCommands, Px(cx - m_Origin.m_fX), Px(cy - m_Origin.m_fY), Px(r), color.PackToInt());
	}

	//------------------------------------------------------------------------------------------------
	void StrokeCircle(float cx, float cy, float r, Color color, float width)
	{
		if (!m_wCanvas || !color)
			return;
		MUI_CanvasRenderer.AddStrokeCircle(m_aCommands, Px(cx - m_Origin.m_fX), Px(cy - m_Origin.m_fY), Px(r), color.PackToInt(), Px(width));
	}

	//------------------------------------------------------------------------------------------------
	void DrawArc(float cx, float cy, float r, float startDeg, float sweepDeg, Color color, float width)
	{
		if (!m_wCanvas || !color)
			return;
		MUI_CanvasRenderer.AddArc(m_aCommands, Px(cx - m_Origin.m_fX), Px(cy - m_Origin.m_fY), Px(r), startDeg, sweepDeg, color.PackToInt(), Px(width));
	}

	//------------------------------------------------------------------------------------------------
	void FillGradientV(float x, float y, float w, float h, Color top, Color bot, int slices)
	{
		if (!m_wCanvas || !top || !bot)
			return;
		if (slices < 2)
			slices = 2;
		float sliceH = h / slices;
		int i;
		ref Color c = new Color(1, 1, 1, 1);
		for (i = 0; i < slices; i++)
		{
			float t = i;
			t = t / (slices - 1);
			c.SetR(top.R() + (bot.R() - top.R()) * t);
			c.SetG(top.G() + (bot.G() - top.G()) * t);
			c.SetB(top.B() + (bot.B() - top.B()) * t);
			c.SetA(top.A() + (bot.A() - top.A()) * t);
			MUI_CanvasRenderer.AddFillRect(m_aCommands, Px(x - m_Origin.m_fX), Px(y + sliceH * i - m_Origin.m_fY), Px(w), Px(sliceH + 0.5), c.PackToInt(), 0);
		}
	}

	//------------------------------------------------------------------------------------------------
	void DrawText(float x, float y, float w, float h, string text, int fontSize, Color color, bool bold, bool center, bool vCenter, bool wrap)
	{
		if (!color)
			return;
		m_Text.Add(x - m_Origin.m_fX, y - m_Origin.m_fY, w, h, text, fontSize, color.PackToInt(), bold, center, vCenter, wrap);
	}

	//------------------------------------------------------------------------------------------------
	void Submit()
	{
		if (m_wCanvas)
			m_wCanvas.SetDrawCommands(m_aCommands);
		m_Text.Submit();
	}

	//------------------------------------------------------------------------------------------------
	CanvasWidget GetCanvas()
	{
		return m_wCanvas;
	}

	//------------------------------------------------------------------------------------------------
	Widget GetFrame()
	{
		return m_wFrame;
	}

	//------------------------------------------------------------------------------------------------
	void Destroy()
	{
		m_Text.Destroy();
		if (m_wFrame)
			m_wFrame.RemoveFromHierarchy();
		m_wFrame = null;
		m_wCanvas = null;
		m_wTextLayer = null;
		m_Workspace = null;
		m_ClipNode = null;
		m_aCommands.Clear();
	}
}
