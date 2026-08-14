//------------------------------------------------------------------------------------------------
class MUI_Card : MUI_Panel
{
	protected static const int BLUR_Z = 1;
	protected static const float DEFAULT_BLUR = 0.72;

	protected BlurWidget m_wBlur;
	protected bool m_bBlurEnabled;
	protected float m_fBlurIntensity;

	//------------------------------------------------------------------------------------------------
	void MUI_Card()
	{
		m_Style.m_Layout = MUI_LayoutKind.StackVertical;
		m_Style.m_WidthMode = MUI_SizeMode.Exact;
		m_Style.m_HeightMode = MUI_SizeMode.Hug;
		m_Style.m_Fill = MUI_Theme.DeepFrost;
		m_Style.m_fRadius = 18;
		m_Style.m_bBlockHit = true;
		m_Style.m_Stroke = Color.FromInt(0);
		m_Style.m_fBorder = 0;
		m_bBlurEnabled = true;
		m_fBlurIntensity = DEFAULT_BLUR;
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
		// Blur is a host Enfusion widget; Paint is skipped when hidden, so hide it here.
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
		float x = DrawX();
		float y = DrawY();
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
		m_wBlur.SetIntensity(m_fBlurIntensity * op);
		float soft = m_Style.m_fRadius;
		if (soft < 8)
			soft = 8;
		m_wBlur.SetSmoothBorder(soft, soft, soft, soft);
	}

	//------------------------------------------------------------------------------------------------
	protected bool EnsureBlurWidget()
	{
		if (m_wBlur)
			return true;
		if (!m_Runtime)
			return false;

		WorkspaceWidget workspace = m_Runtime.GetWorkspace();
		Widget host = m_Runtime.GetHost();
		if (!workspace || !host)
			return false;

		Widget w = workspace.CreateWidget(WidgetType.BlurWidgetTypeID, WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR, Color.FromInt(Color.WHITE), BLUR_Z, host);
		m_wBlur = BlurWidget.Cast(w);
		if (!m_wBlur)
			return false;

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

		float glow = 0.12 + 0.08 * MUI_Ease.Pulse(GetTime(), 0.7);
		surface.FillRect(x - 10, y - 10, w + 20, h + 20, MUI_ColorUtil.Fade(MUI_Theme.Glow, op * glow), 24);
		surface.FillRect(x - 4, y - 4, w + 8, h + 8, MUI_ColorUtil.Fade(MUI_Theme.Cyan, op * 0.06), 20);

		Color fill = MUI_Theme.DeepFrost;
		if (!m_bBlurEnabled)
			fill = MUI_Theme.Deep;
		surface.FillRect(x, y, w, h, MUI_ColorUtil.Fade(fill, op), 18);
		surface.FillGradientV(x, y, w, 64, MUI_ColorUtil.Fade(MUI_Theme.Header, op * 0.85), MUI_ColorUtil.Fade(fill, op), 8);

		surface.FillRect(x, y, w, 3, MUI_ColorUtil.Fade(MUI_Theme.Accent, op), 0);
		surface.FillRect(x, y + 3, w, 1, MUI_ColorUtil.Fade(MUI_Theme.Cyan, op * 0.7), 0);

		float sweep = MUI_Ease.Fract(GetTime() * 0.22);
		float sx = x + sweep * (w + 120) - 80;
		surface.FillRect(sx, y, 70, 4, MUI_ColorUtil.Fade(MUI_Theme.Sheen, op), 0);

		surface.StrokeRect(x, y, w, h, MUI_ColorUtil.Fade(MUI_Theme.Border, op * 0.9), 1.4, 18);

		DrawBracket(surface, x + 10, y + 10, 18, 18, 1, 1, op);
		DrawBracket(surface, x + w - 10, y + 10, 18, 18, -1, 1, op);
		DrawBracket(surface, x + 10, y + h - 10, 18, 18, 1, -1, op);
		DrawBracket(surface, x + w - 10, y + h - 10, 18, 18, -1, -1, op);

		PaintForeground(surface);
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawBracket(MUI_RenderSurface surface, float x, float y, float arm, float thick, float dirX, float dirY, float op)
	{
		Color c = MUI_ColorUtil.Fade(MUI_Theme.Accent, op);
		surface.DrawLine(x, y, x + arm * dirX, y, c, 2.2);
		surface.DrawLine(x, y, x, y + arm * dirY, c, 2.2);
		surface.FillRect(x - 1, y - 1, 3, 3, MUI_ColorUtil.Fade(MUI_Theme.Cyan, op), 0);
	}
}
