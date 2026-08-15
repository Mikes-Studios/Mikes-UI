//------------------------------------------------------------------------------------------------
//! Hosted ImageWidget for textures ({GUID}path.edds). Not canvas-drawn.
//!
//! Consumer:
//!   MUI_Image img = runtime.CreateImage("icon");
//!   img.SetWidth(48); img.SetHeight(48);
//!   img.SetImage("{GUID}UI/Textures/Icons/Icon.edds");
//!
//! Layout:
//!   Exact size (you set). Parents to clip frame when under ScrollView.
//------------------------------------------------------------------------------------------------
class MUI_Image : MUI_Node
{
	protected static const int IMAGE_Z = 6;

	protected ImageWidget m_wImage;
	protected string m_sResource;
	protected Widget m_wParentHost;

	//------------------------------------------------------------------------------------------------
	void MUI_Image()
	{
		m_Style.m_WidthMode = MUI_SizeMode.Exact;
		m_Style.m_HeightMode = MUI_SizeMode.Exact;
		m_Style.m_fWidth = 48;
		m_Style.m_fHeight = 48;
		m_Style.m_fMinWidth = 8;
		m_Style.m_fMinHeight = 8;
		m_Style.m_Fill = Color.FromInt(0);
		m_Style.m_bInteractive = false;
	}

	//------------------------------------------------------------------------------------------------
	void SetImage(string resource)
	{
		m_sResource = resource;
		if (m_wImage && m_sResource != "")
			m_wImage.LoadImageTexture(0, m_sResource);
		InvalidatePaint();
	}

	//------------------------------------------------------------------------------------------------
	string GetImage()
	{
		return m_sResource;
	}

	//------------------------------------------------------------------------------------------------
	override void SetVisible(bool visible)
	{
		super.SetVisible(visible);
		if (!visible && m_wImage)
			m_wImage.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	override void DestroyHostWidgets()
	{
		if (m_wImage)
		{
			m_wImage.RemoveFromHierarchy();
			m_wImage = null;
		}
		m_wParentHost = null;
	}

	//------------------------------------------------------------------------------------------------
	override void SyncHostWidgets()
	{
		if (!IsVisible())
		{
			if (m_wImage)
				m_wImage.SetVisible(false);
			return;
		}

		if (!EnsureImageWidget())
			return;

		float op = GetDrawOpacity();
		float x;
		float y;
		m_Runtime.GetHostLocalPos(this, x, y);
		float w = m_World.m_fW;
		float h = m_World.m_fH;
		if (op < 0.02 || w < 1 || h < 1)
		{
			m_wImage.SetVisible(false);
			return;
		}

		m_wImage.SetVisible(true);
		m_wImage.SetOpacity(op);
		FrameSlot.SetAnchorMin(m_wImage, 0, 0);
		FrameSlot.SetAnchorMax(m_wImage, 0, 0);
		FrameSlot.SetPos(m_wImage, x, y);
		FrameSlot.SetSize(m_wImage, w, h);
	}

	//------------------------------------------------------------------------------------------------
	protected bool EnsureImageWidget()
	{
		if (!m_Runtime)
			return false;

		Widget desiredHost = m_Runtime.GetHostForNode(this);
		if (!desiredHost)
			return false;

		if (m_wImage && m_wParentHost != desiredHost)
			DestroyHostWidgets();

		if (m_wImage)
			return true;

		WorkspaceWidget workspace = m_Runtime.GetWorkspace();
		if (!workspace)
			return false;

		Widget w = workspace.CreateWidget(WidgetType.ImageWidgetTypeID, WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR | WidgetFlags.INHERIT_CLIPPING | WidgetFlags.STRETCH, Color.FromInt(Color.WHITE), IMAGE_Z, desiredHost);
		m_wImage = ImageWidget.Cast(w);
		if (!m_wImage)
			return false;

		m_wParentHost = desiredHost;
		if (m_sResource != "")
			m_wImage.LoadImageTexture(0, m_sResource);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Paint(MUI_RenderSurface surface)
	{
		SyncHostWidgets();
	}
}
