//------------------------------------------------------------------------------------------------
class MUI_Node
{
	string m_sName;
	protected ref MUI_Style m_Style;
	protected ref array<ref MUI_Node> m_aChildren;
	protected MUI_Node m_Parent;
	protected MUI_Runtime m_Runtime;
	protected ref MUI_Rect m_World;

	protected float m_fDesiredW;
	protected float m_fDesiredH;
	protected float m_fScrollY;
	protected float m_fContentH;

	protected bool m_bVisible;
	protected bool m_bEnabled;
	protected bool m_bHover;
	protected bool m_bPressed;
	protected bool m_bFocused;

	protected float m_fHoverT;
	protected float m_fPressT;
	protected float m_fFocusT;
	protected float m_fIntro;
	protected float m_fIntroDelay;
	protected float m_fIntroDuration;
	protected float m_fIntroFromY;
	protected float m_fIntroElapsed;
	protected float m_fSlideY;
	protected float m_fDrawOX;
	protected float m_fDrawOY;
	protected float m_fDrawOpacity;
	protected float m_fRipple;

	//------------------------------------------------------------------------------------------------
	void MUI_Node()
	{
		m_Style = new MUI_Style();
		m_aChildren = new array<ref MUI_Node>();
		m_World = new MUI_Rect();
		m_bVisible = true;
		m_bEnabled = true;
		m_fIntro = 1;
		m_fDrawOpacity = 1;
	}

	//------------------------------------------------------------------------------------------------
	MUI_Style GetStyle()
	{
		return m_Style;
	}

	//------------------------------------------------------------------------------------------------
	MUI_Rect GetWorldRect()
	{
		return m_World;
	}

	//------------------------------------------------------------------------------------------------
	MUI_Node GetParent()
	{
		return m_Parent;
	}

	//------------------------------------------------------------------------------------------------
	MUI_Runtime GetRuntime()
	{
		return m_Runtime;
	}

	//------------------------------------------------------------------------------------------------
	void SetRuntime(MUI_Runtime runtime)
	{
		m_Runtime = runtime;
		int count = m_aChildren.Count();
		int i;
		for (i = 0; i < count; i++)
		{
			m_aChildren[i].SetRuntime(runtime);
		}
	}

	//------------------------------------------------------------------------------------------------
	void SetName(string name)
	{
		m_sName = name;
	}

	//------------------------------------------------------------------------------------------------
	void SetVisible(bool visible)
	{
		if (m_bVisible == visible)
			return;
		m_bVisible = visible;
		InvalidateLayout();
	}

	//------------------------------------------------------------------------------------------------
	bool IsVisible()
	{
		return m_bVisible;
	}

	//------------------------------------------------------------------------------------------------
	void SetEnabled(bool enabled)
	{
		m_bEnabled = enabled;
		InvalidatePaint();
	}

	//------------------------------------------------------------------------------------------------
	bool IsEnabled()
	{
		if (!m_bEnabled)
			return false;
		if (m_Parent && !m_Parent.IsEnabled())
			return false;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void SetHover(bool hover)
	{
		if (m_bHover == hover)
			return;
		m_bHover = hover;
		InvalidatePaint();
	}

	//------------------------------------------------------------------------------------------------
	bool IsHover()
	{
		return m_bHover;
	}

	//------------------------------------------------------------------------------------------------
	void SetPressed(bool pressed)
	{
		if (m_bPressed == pressed)
			return;
		m_bPressed = pressed;
		InvalidatePaint();
	}

	//------------------------------------------------------------------------------------------------
	bool IsPressed()
	{
		return m_bPressed;
	}

	//------------------------------------------------------------------------------------------------
	void SetFocused(bool focused)
	{
		if (m_bFocused == focused)
			return;
		m_bFocused = focused;
		InvalidatePaint();
	}

	//------------------------------------------------------------------------------------------------
	bool IsFocused()
	{
		return m_bFocused;
	}

	//------------------------------------------------------------------------------------------------
	bool IsInteractive()
	{
		if (!m_bVisible)
			return false;
		if (!IsEnabled())
			return false;
		if (m_Style.m_bInteractive)
			return true;
		if (m_Style.m_bBlockHit)
			return true;
		return false;
	}

	//------------------------------------------------------------------------------------------------
	bool AcceptsClick()
	{
		if (!m_bVisible)
			return false;
		if (!IsEnabled())
			return false;
		return m_Style.m_bInteractive;
	}

	//------------------------------------------------------------------------------------------------
	void SetLayout(MUI_LayoutKind layout)
	{
		m_Style.m_Layout = layout;
		InvalidateLayout();
	}

	//------------------------------------------------------------------------------------------------
	void SetWidth(float width)
	{
		m_Style.m_WidthMode = MUI_SizeMode.Exact;
		m_Style.m_fWidth = width;
		InvalidateLayout();
	}

	//------------------------------------------------------------------------------------------------
	void SetHeight(float height)
	{
		m_Style.m_HeightMode = MUI_SizeMode.Exact;
		m_Style.m_fHeight = height;
		InvalidateLayout();
	}

	//------------------------------------------------------------------------------------------------
	void SetFillWidth()
	{
		m_Style.m_WidthMode = MUI_SizeMode.Fill;
		InvalidateLayout();
	}

	//------------------------------------------------------------------------------------------------
	void SetFillHeight()
	{
		m_Style.m_HeightMode = MUI_SizeMode.Fill;
		InvalidateLayout();
	}

	//------------------------------------------------------------------------------------------------
	void SetHugWidth()
	{
		m_Style.m_WidthMode = MUI_SizeMode.Hug;
		InvalidateLayout();
	}

	//------------------------------------------------------------------------------------------------
	void SetHugHeight()
	{
		m_Style.m_HeightMode = MUI_SizeMode.Hug;
		InvalidateLayout();
	}

	//------------------------------------------------------------------------------------------------
	void SetPadding(float value)
	{
		m_Style.SetPadding(value);
		InvalidateLayout();
	}

	//------------------------------------------------------------------------------------------------
	void SetGap(float gap)
	{
		m_Style.m_fGap = gap;
		InvalidateLayout();
	}

	//------------------------------------------------------------------------------------------------
	void SetRadius(float radius)
	{
		m_Style.m_fRadius = radius;
		InvalidatePaint();
	}

	//------------------------------------------------------------------------------------------------
	void SetAlign(float x, float y)
	{
		m_Style.m_fAlignX = x;
		m_Style.m_fAlignY = y;
		InvalidateLayout();
	}

	//------------------------------------------------------------------------------------------------
	void SetGrow(float grow)
	{
		m_Style.m_fGrow = grow;
		InvalidateLayout();
	}

	//------------------------------------------------------------------------------------------------
	void SetFill(Color color)
	{
		m_Style.m_Fill = color;
		InvalidatePaint();
	}

	//------------------------------------------------------------------------------------------------
	void SetClipChildren(bool clip)
	{
		m_Style.m_bClipChildren = clip;
		InvalidateLayout();
	}

	//------------------------------------------------------------------------------------------------
	bool ClipsChildren()
	{
		return m_Style.m_bClipChildren;
	}

	//------------------------------------------------------------------------------------------------
	void SetScrollY(float scrollY)
	{
		if (m_fScrollY == scrollY)
			return;
		m_fScrollY = scrollY;
		InvalidateLayout();
	}

	//------------------------------------------------------------------------------------------------
	void SetContentHeight(float height)
	{
		m_fContentH = height;
	}

	//------------------------------------------------------------------------------------------------
	void ClampScroll(float viewportH)
	{
		float maxScroll = m_fContentH - viewportH;
		if (maxScroll < 0)
			maxScroll = 0;
		if (m_fScrollY < 0)
			m_fScrollY = 0;
		if (m_fScrollY > maxScroll)
			m_fScrollY = maxScroll;
	}

	//------------------------------------------------------------------------------------------------
	float GetScrollY()
	{
		return m_fScrollY;
	}

	//------------------------------------------------------------------------------------------------
	float GetContentHeight()
	{
		return m_fContentH;
	}

	//------------------------------------------------------------------------------------------------
	void AddChild(MUI_Node child)
	{
		if (!child)
		{
			MUI_Log.Error("AddChild ignored a null node");
			return;
		}

		if (child.m_Parent)
			child.m_Parent.RemoveChild(child);

		child.m_Parent = this;
		child.SetRuntime(m_Runtime);
		m_aChildren.Insert(child);
		InvalidateLayout();
	}

	//------------------------------------------------------------------------------------------------
	void RemoveChild(notnull MUI_Node child)
	{
		int index = m_aChildren.Find(child);
		if (index < 0)
			return;
		m_aChildren.Remove(index);
		child.m_Parent = null;
		InvalidateLayout();
	}

	//------------------------------------------------------------------------------------------------
	int GetChildCount()
	{
		return m_aChildren.Count();
	}

	//------------------------------------------------------------------------------------------------
	MUI_Node GetChild(int index)
	{
		if (index < 0)
			return null;
		if (index >= m_aChildren.Count())
			return null;
		return m_aChildren[index];
	}

	//------------------------------------------------------------------------------------------------
	void InvalidateLayout()
	{
		if (m_Runtime)
			m_Runtime.MarkLayoutDirty();
	}

	//------------------------------------------------------------------------------------------------
	void InvalidatePaint()
	{
		if (m_Runtime)
			m_Runtime.MarkPaintDirty();
	}

	//------------------------------------------------------------------------------------------------
	void SetDesiredSize(float w, float h)
	{
		m_fDesiredW = w;
		m_fDesiredH = h;
	}

	//------------------------------------------------------------------------------------------------
	float GetDesiredWidth()
	{
		return m_fDesiredW;
	}

	//------------------------------------------------------------------------------------------------
	float GetDesiredHeight()
	{
		return m_fDesiredH;
	}

	//------------------------------------------------------------------------------------------------
	void SetWorld(float x, float y, float w, float h)
	{
		m_World.Set(x, y, w, h);
	}

	//------------------------------------------------------------------------------------------------
	//! Override to provide intrinsic hug size (labels, etc).
	void MeasureIntrinsic(float availW, float availH)
	{
		m_fDesiredW = 0;
		m_fDesiredH = 0;
	}

	//------------------------------------------------------------------------------------------------
	Color ResolveFill()
	{
		if (!m_bEnabled && m_Style.m_FillDisabled && m_Style.m_FillDisabled.A() > 0.001)
			return m_Style.m_FillDisabled;
		if (m_bPressed && m_Style.m_FillPress && m_Style.m_FillPress.A() > 0.001)
			return m_Style.m_FillPress;
		if (m_bHover && m_Style.m_FillHover && m_Style.m_FillHover.A() > 0.001)
			return m_Style.m_FillHover;
		return m_Style.m_Fill;
	}

	//------------------------------------------------------------------------------------------------
	void Paint(MUI_RenderSurface surface)
	{
		float x = DrawX();
		float y = DrawY();
		float op = GetDrawOpacity();
		if (op < 0.01)
			return;

		Color fill = ResolveFill();
		if (fill && fill.A() > 0.001)
			surface.FillRect(x, y, m_World.m_fW, m_World.m_fH, MUI_ColorUtil.Fade(fill, op), m_Style.m_fRadius);

		if (m_Style.m_fBorder > 0 && m_Style.m_Stroke && m_Style.m_Stroke.A() > 0.001)
			surface.StrokeRect(x, y, m_World.m_fW, m_World.m_fH, MUI_ColorUtil.Fade(m_Style.m_Stroke, op), m_Style.m_fBorder, m_Style.m_fRadius);

		PaintForeground(surface);
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float dt)
	{
		float hoverTarget = 0;
		if (m_bHover)
			hoverTarget = 1;
		float pressTarget = 0;
		if (m_bPressed)
			pressTarget = 1;
		float focusTarget = 0;
		if (m_bFocused)
			focusTarget = 1;

		m_fHoverT = MUI_Ease.Approach(m_fHoverT, hoverTarget, dt, 14);
		m_fPressT = MUI_Ease.Approach(m_fPressT, pressTarget, dt, 22);
		m_fFocusT = MUI_Ease.Approach(m_fFocusT, focusTarget, dt, 12);

		if (m_fRipple > 0 && m_fRipple < 1)
		{
			m_fRipple = m_fRipple + dt * 2.4;
			if (m_fRipple > 1)
				m_fRipple = 1;
		}

		if (m_fIntroDuration > 0)
		{
			m_fIntroElapsed = m_fIntroElapsed + dt;
			float t = (m_fIntroElapsed - m_fIntroDelay) / m_fIntroDuration;
			if (t < 0)
				t = 0;
			if (t > 1)
				t = 1;
			if (m_fIntroFromY > 24)
				m_fIntro = MUI_Ease.BackOut(t);
			else
				m_fIntro = MUI_Ease.CubicOut(t);
			m_fSlideY = (1.0 - m_fIntro) * m_fIntroFromY;
		}

		OnTick(dt);

		int count = m_aChildren.Count();
		int i;
		for (i = 0; i < count; i++)
		{
			if (m_aChildren[i])
				m_aChildren[i].Tick(dt);
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnTick(float dt)
	{
	}

	//------------------------------------------------------------------------------------------------
	void SetIntro(float delay, float duration, float fromY)
	{
		m_fIntroDelay = delay;
		m_fIntroDuration = duration;
		m_fIntroFromY = fromY;
		m_fIntroElapsed = 0;
		m_fIntro = 0;
		m_fSlideY = fromY;
	}

	//------------------------------------------------------------------------------------------------
	void BeginDraw(float ox, float oy, float parentOpacity)
	{
		m_fDrawOX = ox;
		m_fDrawOY = oy;
		m_fDrawOpacity = parentOpacity * m_fIntro;
		if (m_fDrawOpacity < 0)
			m_fDrawOpacity = 0;
		if (m_fDrawOpacity > 1)
			m_fDrawOpacity = 1;
	}

	//------------------------------------------------------------------------------------------------
	float DrawX()
	{
		return m_World.m_fX + m_fDrawOX;
	}

	//------------------------------------------------------------------------------------------------
	float DrawY()
	{
		return m_World.m_fY + m_fDrawOY;
	}

	//------------------------------------------------------------------------------------------------
	float GetDrawOpacity()
	{
		return m_fDrawOpacity;
	}

	//------------------------------------------------------------------------------------------------
	float GetIntro()
	{
		return m_fIntro;
	}

	//------------------------------------------------------------------------------------------------
	float GetSlideY()
	{
		return m_fSlideY;
	}

	//------------------------------------------------------------------------------------------------
	float GetHoverT()
	{
		return m_fHoverT;
	}

	//------------------------------------------------------------------------------------------------
	float GetPressT()
	{
		return m_fPressT;
	}

	//------------------------------------------------------------------------------------------------
	float GetFocusT()
	{
		return m_fFocusT;
	}

	//------------------------------------------------------------------------------------------------
	float GetRipple()
	{
		return m_fRipple;
	}

	//------------------------------------------------------------------------------------------------
	void PlayRipple()
	{
		m_fRipple = 0.02;
	}

	//------------------------------------------------------------------------------------------------
	float GetTime()
	{
		if (!m_Runtime)
			return 0;
		return m_Runtime.GetTime();
	}

	//------------------------------------------------------------------------------------------------
	void PaintForeground(MUI_RenderSurface surface)
	{
	}

	//------------------------------------------------------------------------------------------------
	void OnClicked()
	{
	}

	//------------------------------------------------------------------------------------------------
	void OnMouseWheel(int wheel)
	{
	}

	//------------------------------------------------------------------------------------------------
	bool HandleActivate()
	{
		if (!AcceptsClick())
			return false;
		PlayRipple();
		OnClicked();
		return true;
	}
}
