//------------------------------------------------------------------------------------------------
//! Primitive frosted/flat panel with optional BlurWidget. Honors SetFill / SetRadius / SetStroke.
//!
//! Consumer:
//!   MUI_Surface s = runtime.CreateSurface("panel");
//!   s.SetWidth(560); s.SetPadding(24); s.SetFill(theme.Panel);
//!   s.SetBlurEnabled(true); // optional frost behind fill; follows GetDrawOpacity()
//!
//! Layout:
//!   Exact width (you set), Hug height, StackVertical. For uplink chrome use CreateCard.
//------------------------------------------------------------------------------------------------
class MUI_Surface : MUI_Panel
{
	protected static const int BLUR_Z = 1;
	protected static const float DEFAULT_BLUR = 0.72;

	protected BlurWidget m_wBlur;
	protected Widget m_wParentHost;
	protected bool m_bBlurEnabled;
	protected float m_fBlurIntensity;

	//------------------------------------------------------------------------------------------------
	void MUI_Surface()
	{
		m_Style.m_Layout = MUI_LayoutKind.StackVertical;
		m_Style.m_WidthMode = MUI_SizeMode.Exact;
		m_Style.m_HeightMode = MUI_SizeMode.Hug;
		m_Style.m_Fill = MUI_Theme.Panel;
		m_Style.m_fRadius = 12;
		m_Style.m_bBlockHit = true;
		m_Style.m_Stroke = Color.FromInt(0);
		m_Style.m_fBorder = 0;
		m_bBlurEnabled = false;
		m_fBlurIntensity = DEFAULT_BLUR;
	}

	//------------------------------------------------------------------------------------------------
	override void ApplyTheme(notnull MUI_ThemeData theme)
	{
		m_Style.m_Fill = theme.Panel;
	}

	//------------------------------------------------------------------------------------------------
	void SetBlurEnabled(bool enabled)
	{
		m_bBlurEnabled = enabled;
		if (!enabled)
			DestroyHostWidgets();
		InvalidatePaint();
	}

	//------------------------------------------------------------------------------------------------
	void SetBlurIntensity(float intensity)
	{
		if (intensity < 0)
			intensity = 0;
		if (intensity > 1)
			intensity = 1;
		m_fBlurIntensity = intensity;
		InvalidatePaint();
	}

	//------------------------------------------------------------------------------------------------
	override void SetVisible(bool visible)
	{
		super.SetVisible(visible);
		if (!visible && m_wBlur)
		{
			m_wBlur.SetVisible(false);
			m_wBlur.SetIntensity(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void DestroyHostWidgets()
	{
		if (m_wBlur)
		{
			m_wBlur.RemoveFromHierarchy();
			m_wBlur = null;
		}
		m_wParentHost = null;
	}

	//------------------------------------------------------------------------------------------------
	override void SyncHostWidgets()
	{
		if (!m_bBlurEnabled || !IsVisible())
		{
			if (m_wBlur)
				m_wBlur.SetVisible(false);
			return;
		}

		if (!EnsureBlurWidget())
			return;

		float op = GetDrawOpacity();
		float x;
		float y;
		m_Runtime.GetHostLocalPos(this, x, y);
		float w = m_World.m_fW;
		float h = m_World.m_fH;
		if (op < 0.02 || w < 2 || h < 2)
		{
			m_wBlur.SetVisible(false);
			return;
		}

		m_wBlur.SetVisible(true);
		FrameSlot.SetAnchorMin(m_wBlur, 0, 0);
		FrameSlot.SetAnchorMax(m_wBlur, 0, 0);
		FrameSlot.SetPos(m_wBlur, x, y);
		FrameSlot.SetSize(m_wBlur, w, h);
		m_wBlur.SetOpacity(op);
		m_wBlur.SetIntensity(m_fBlurIntensity * op);
		float soft = m_Style.m_fRadius;
		if (soft < 8)
			soft = 8;
		m_wBlur.SetSmoothBorder(soft, soft, soft, soft);
	}

	//------------------------------------------------------------------------------------------------
	protected bool EnsureBlurWidget()
	{
		if (!m_Runtime)
			return false;

		Widget desiredHost = m_Runtime.GetHostForNode(this);
		if (!desiredHost)
			return false;

		if (m_wBlur && m_wParentHost != desiredHost)
			DestroyHostWidgets();

		if (m_wBlur)
			return true;

		WorkspaceWidget workspace = m_Runtime.GetWorkspace();
		if (!workspace)
			return false;

		Widget w = workspace.CreateWidget(WidgetType.BlurWidgetTypeID, WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR | WidgetFlags.INHERIT_CLIPPING, Color.FromInt(Color.WHITE), BLUR_Z, desiredHost);
		m_wBlur = BlurWidget.Cast(w);
		if (!m_wBlur)
			return false;

		m_wParentHost = desiredHost;
		m_wBlur.SetDPIScaleEnabled(true);
		m_wBlur.SetTintBlendModeAdd(false);
		m_wBlur.SetColor(Color.FromSRGBA(12, 16, 18, 255));
		m_wBlur.SetIntensity(m_fBlurIntensity);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Paint(MUI_RenderSurface surface)
	{
		float x = DrawX();
		float y = DrawY();
		float w = m_World.m_fW;
		float h = m_World.m_fH;
		float op = GetDrawOpacity();
		if (op < 0.01)
			return;

		SyncHostWidgets();

		Color fill = ResolveFill();
		if (fill && fill.A() > 0.001)
			surface.FillRect(x, y, w, h, MUI_ColorUtil.Fade(fill, op), m_Style.m_fRadius);

		if (m_Style.m_fBorder > 0 && m_Style.m_Stroke && m_Style.m_Stroke.A() > 0.001)
			surface.StrokeRect(x, y, w, h, MUI_ColorUtil.Fade(m_Style.m_Stroke, op), m_Style.m_fBorder, m_Style.m_fRadius);

		PaintForeground(surface);
	}
}
