//------------------------------------------------------------------------------------------------
//! Fullscreen radar / motes / vignette. Paints on the backdrop layer under card blur.
//!
//! Consumer:
//!   MUI_FxBackdrop fx = runtime.CreateFxBackdrop("fx");
//!   overlay.AddChild(fx); overlay.AddChild(card);  // fx first so it sits behind
//!
//! Layout:
//!   Overlay Fill Fill, not interactive, not BlockHit. HUD usually omits this.
//------------------------------------------------------------------------------------------------
class MUI_FxBackdrop : MUI_Node
{
	protected static const int MOTE_COUNT = 22;

	protected ref array<float> m_aX;
	protected ref array<float> m_aY;
	protected ref array<float> m_aV;
	protected ref array<float> m_aS;
	protected ref array<float> m_aPhase;
	protected bool m_bSeeded;

	//------------------------------------------------------------------------------------------------
	void MUI_FxBackdrop()
	{
		m_Style.m_Layout = MUI_LayoutKind.Overlay;
		m_Style.m_WidthMode = MUI_SizeMode.Fill;
		m_Style.m_HeightMode = MUI_SizeMode.Fill;
		m_Style.m_Fill = Color.FromInt(0);
		m_Style.m_fRadius = 0;
		m_Style.m_bBlockHit = false;
		m_Style.m_bInteractive = false;
		m_aX = new array<float>();
		m_aY = new array<float>();
		m_aV = new array<float>();
		m_aS = new array<float>();
		m_aPhase = new array<float>();
	}

	//------------------------------------------------------------------------------------------------
	override bool PaintsOnBackdropLayer()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnTick(float dt)
	{
		if (!m_bSeeded)
			return;
		int i;
		float h = m_World.m_fH;
		float w = m_World.m_fW;
		if (h < 1)
			return;
		for (i = 0; i < MOTE_COUNT; i++)
		{
			m_aY[i] = m_aY[i] - m_aV[i] * dt;
			if (m_aY[i] < -8)
			{
				m_aY[i] = h + 8;
				m_aX[i] = Math.RandomFloat(0, w);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SeedIfNeeded()
	{
		if (m_bSeeded)
			return;
		float w = m_World.m_fW;
		float h = m_World.m_fH;
		if (w < 8 || h < 8)
			return;
		int i;
		for (i = 0; i < MOTE_COUNT; i++)
		{
			m_aX.Insert(Math.RandomFloat(0, w));
			m_aY.Insert(Math.RandomFloat(0, h));
			m_aV.Insert(Math.RandomFloat(8, 28));
			m_aS.Insert(Math.RandomFloat(2.0, 4.0));
			m_aPhase.Insert(Math.RandomFloat(0, Math.PI2));
		}
		m_bSeeded = true;
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

		SeedIfNeeded();

		MUI_ThemeData theme = GetTheme();
		surface.FillRect(x, y, w, h, MUI_ColorUtil.Fade(theme.Overlay, op), 0);

		DrawRadar(surface, x, y, w, h, op, theme);
		DrawMotes(surface, x, y, op, theme);
		DrawVignette(surface, x, y, w, h, op);
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawRadar(MUI_RenderSurface surface, float x, float y, float w, float h, float op, notnull MUI_ThemeData theme)
	{
		float cx = x + w * 0.5;
		float cy = y + h * 0.5;
		float t = GetTime();
		float spin = t * 28;
		Color ring = MUI_ColorUtil.Fade(theme.CyanDim, op * 0.55);
		surface.StrokeCircle(cx, cy, 210, ring, 1.2);
		surface.StrokeCircle(cx, cy, 310, ring, 1);
		surface.StrokeCircle(cx, cy, 410, MUI_ColorUtil.Fade(theme.Accent, op * 0.18), 1.4);
		surface.DrawArc(cx, cy, 310, spin, 70, MUI_ColorUtil.Fade(theme.Cyan, op * 0.55), 2.2);
		surface.DrawArc(cx, cy, 410, -spin * 0.7, 40, MUI_ColorUtil.Fade(theme.Accent, op * 0.45), 2.5);

		int i;
		for (i = 0; i < 12; i++)
		{
			float deg = spin + i * 30;
			float rad = deg * Math.DEG2RAD;
			float inner = 198;
			float outer = 222;
			surface.DrawLine(cx + inner * Math.Cos(rad), cy + inner * Math.Sin(rad), cx + outer * Math.Cos(rad), cy + outer * Math.Sin(rad), ring, 1.5);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawMotes(MUI_RenderSurface surface, float x, float y, float op, notnull MUI_ThemeData theme)
	{
		if (!m_bSeeded)
			return;
		float t = GetTime();
		int i;
		for (i = 0; i < MOTE_COUNT; i++)
		{
			float pulse = 0.45 + 0.55 * Math.Sin(t * 2.2 + m_aPhase[i]);
			if (pulse < 0)
				pulse = 0;
			surface.FillCircle(x + m_aX[i], y + m_aY[i], m_aS[i], MUI_ColorUtil.Fade(theme.Mote, op * pulse));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawVignette(MUI_RenderSurface surface, float x, float y, float w, float h, float op)
	{
		Color edge = MUI_ColorUtil.Fade(Color.Black, op * 0.32);
		surface.FillRect(x, y, w, 70, edge, 0);
		surface.FillRect(x, y + h - 70, w, 70, edge, 0);
		surface.FillRect(x, y, 80, h, edge, 0);
		surface.FillRect(x + w - 80, y, 80, h, edge, 0);
	}
}
