//------------------------------------------------------------------------------------------------
//! Scene-graph node. Every control is a MUI_Node. Consumers almost never `new` this
//! directly — use MUI_Runtime factories, or subclass and runtime.Adopt(node).
//!
//! Consumer:
//!   Store every node you keep as `protected ref MUI_Button m_save;` (or local `ref`).
//!   Parent with parent.AddChild(child). Rebuild lists with ClearChildren().
//!   Size: SetWidth/SetHeight (Exact), SetFillWidth/SetFillHeight, SetHugWidth/SetHugHeight.
//!   SetPadding, SetPaddingTRBL, SetGap, SetAlign(ax, ay), SetGrow, SetRadius, SetFill.
//!   Intro: SetIntro(delay, duration, fromY). Paint must use DrawX/DrawY/GetDrawOpacity().
//!
//! Layout:
//!   Default Fill width, Hug height, StackVertical. Overlay children use SetAlign(0.5, 0.5)
//!   to center. Main-axis Fill is measured as Hug then shares leftover via m_fGrow
//!   (Fill + grow 0 counts as flex 1). Do not SetFillWidth on buttons in a row.
//!
//! Extend:
//!   Subclass, set size modes in constructor (parent MUI_Node() runs first, m_Style exists).
//!   Override MeasureIntrinsic (Hug), Paint or PaintForeground, OnClicked, OnTick.
//!   Interactive: m_Style.m_bInteractive = true. Text input: override WantsTextInput,
//!   GetEditText, SetEditTextFromBridge, CommitEdit. D-pad left/right: override HandleNavAxis.
//!   Host widgets (blur, image): override SyncHostWidgets / DestroyHostWidgets.
//!   Overlay catchers that click but must not take d-pad: override WantsFocus to false.
//------------------------------------------------------------------------------------------------
class MUI_Node
{
	string m_sName;
	protected ref MUI_Style m_Style;
	protected ref array<ref MUI_Node> m_aChildren;
	protected MUI_Node m_Parent;
	protected MUI_Runtime m_Runtime;
	protected ref MUI_Rect m_World;
	protected static ref MUI_ThemeData s_FallbackTheme;

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
	//! Theme for paint / ApplyTheme. Falls back to a retained uplink singleton.
	MUI_ThemeData GetTheme()
	{
		if (m_Runtime)
			return m_Runtime.GetTheme();
		if (!s_FallbackTheme)
			s_FallbackTheme = MUI_ThemeData.CreateUplink();
		return s_FallbackTheme;
	}

	//------------------------------------------------------------------------------------------------
	//! Called by Runtime Create*/Adopt. Override to copy theme fills/fonts into m_Style.
	void ApplyTheme(notnull MUI_ThemeData theme)
	{
	}

	//------------------------------------------------------------------------------------------------
	void SetRuntime(MUI_Runtime runtime)
	{
		if (!runtime && m_Runtime)
			DestroyHostWidgetsTree();
		m_Runtime = runtime;
		int count = m_aChildren.Count();
		int i;
		for (i = 0; i < count; i++)
		{
			m_aChildren[i].SetRuntime(runtime);
		}
	}

	//------------------------------------------------------------------------------------------------
	void DestroyHostWidgetsTree()
	{
		DestroyHostWidgets();
		int count = m_aChildren.Count();
		int i;
		for (i = 0; i < count; i++)
		{
			if (m_aChildren[i])
				m_aChildren[i].DestroyHostWidgetsTree();
		}
	}

	//------------------------------------------------------------------------------------------------
	void SyncHostWidgetsTree()
	{
		SyncHostWidgets();
		int count = m_aChildren.Count();
		int i;
		for (i = 0; i < count; i++)
		{
			// Visit hidden nodes too — BlurWidgets must be cleared when cards hide.
			if (m_aChildren[i])
				m_aChildren[i].SyncHostWidgetsTree();
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
	//! Gamepad focus target. Default matches AcceptsClick. Overlay catchers click but do not steal d-pad.
	bool WantsFocus()
	{
		return AcceptsClick();
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
	void SetMaxHeight(float maxHeight)
	{
		m_Style.m_fMaxHeight = maxHeight;
		InvalidateLayout();
	}

	//------------------------------------------------------------------------------------------------
	void SetMinHeight(float minHeight)
	{
		m_Style.m_fMinHeight = minHeight;
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
	void SetPaddingTRBL(float top, float right, float bottom, float left)
	{
		m_Style.SetPaddingTRBL(top, right, bottom, left);
		InvalidateLayout();
	}

	//------------------------------------------------------------------------------------------------
	void SetMinWidth(float minWidth)
	{
		m_Style.m_fMinWidth = minWidth;
		InvalidateLayout();
	}

	//------------------------------------------------------------------------------------------------
	void SetMaxWidth(float maxWidth)
	{
		m_Style.m_fMaxWidth = maxWidth;
		InvalidateLayout();
	}

	//------------------------------------------------------------------------------------------------
	void SetStroke(Color color, float width)
	{
		m_Style.m_Stroke = color;
		m_Style.m_fBorder = width;
		InvalidatePaint();
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
	//! When true, paints on the backdrop surface under BlurWidgets (FxBackdrop).
	bool PaintsOnBackdropLayer()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Host-owned Enfusion widgets (blur, etc.) — sync after layout each frame.
	void SyncHostWidgets()
	{
	}

	//------------------------------------------------------------------------------------------------
	void DestroyHostWidgets()
	{
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
	//! Insert as first child (lower z in Overlay). Used by Dropdown catcher.
	void AddChildFirst(MUI_Node child)
	{
		if (!child)
		{
			MUI_Log.Error("AddChildFirst ignored a null node");
			return;
		}

		if (child.m_Parent)
			child.m_Parent.RemoveChild(child);

		child.m_Parent = this;
		child.SetRuntime(m_Runtime);
		m_aChildren.InsertAt(child, 0);
		InvalidateLayout();
	}

	//------------------------------------------------------------------------------------------------
	void RemoveChild(notnull MUI_Node child)
	{
		int index = m_aChildren.Find(child);
		if (index < 0)
			return;
		// Remove() is unordered (swap-with-last) and would scramble sibling z-order.
		m_aChildren.RemoveOrdered(index);
		child.m_Parent = null;
		InvalidateLayout();
	}

	//------------------------------------------------------------------------------------------------
	void ClearChildren()
	{
		int i;
		for (i = 0; i < m_aChildren.Count(); i++)
		{
			if (m_aChildren[i])
				m_aChildren[i].m_Parent = null;
		}
		m_aChildren.Clear();
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

		PaintFocusRing(surface, x, y, m_World.m_fW, m_World.m_fH);
		PaintForeground(surface);
	}

	//------------------------------------------------------------------------------------------------
	void PaintFocusRing(MUI_RenderSurface surface, float x, float y, float w, float h)
	{
		float t = m_fFocusT;
		if (t < 0.02)
			return;
		float op = GetDrawOpacity() * t;
		float rad = m_Style.m_fRadius + 4;
		MUI_ThemeData theme = GetTheme();
		surface.StrokeRect(x - 5, y - 5, w + 10, h + 10, MUI_ColorUtil.Fade(theme.Cyan, op * 0.9), 2.2, rad);
		surface.StrokeRect(x - 8, y - 8, w + 16, h + 16, MUI_ColorUtil.Fade(theme.Accent, op * 0.4), 1.2, rad + 3);
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float dt)
	{
		float hoverTarget = 0;
		if (m_bHover)
			hoverTarget = 1;
		if (m_bFocused)
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
	void OnDrag(float x, float y)
	{
	}

	//------------------------------------------------------------------------------------------------
	void OnDragEnd(float x, float y)
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

	//------------------------------------------------------------------------------------------------
	//! True if this node should attach the hidden EditBox (IME / console VK).
	bool WantsTextInput()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	string GetEditText()
	{
		return "";
	}

	//------------------------------------------------------------------------------------------------
	void SetEditTextFromBridge(string text)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Called when the EditBox commits (write-mode leave / detach). Default no-op.
	void CommitEdit()
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Consume gamepad left/right/up/down instead of moving focus. Return true if handled.
	//! dirX/dirY are -1, 0, or 1.
	bool HandleNavAxis(int dirX, int dirY)
	{
		return false;
	}
}
