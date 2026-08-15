//------------------------------------------------------------------------------------------------
//! Generic rectangle / stack container. Root of most menus.
//!
//! Consumer:
//!   MUI_Panel overlay = runtime.CreatePanel("overlay");
//!   overlay.MakeOverlay();              // dimmed fullscreen, blocks hits (menus)
//!   overlay.MakePassThroughOverlay();   // transparent, no hit block (HUD)
//!
//! Layout:
//!   Default StackVertical, Fill width, Hug height, Panel fill, radius 10, BlockHit.
//------------------------------------------------------------------------------------------------
class MUI_Panel : MUI_Node
{
	//------------------------------------------------------------------------------------------------
	void MUI_Panel()
	{
		m_Style.m_Layout = MUI_LayoutKind.StackVertical;
		m_Style.m_WidthMode = MUI_SizeMode.Fill;
		m_Style.m_HeightMode = MUI_SizeMode.Hug;
		m_Style.m_Fill = MUI_Theme.Panel;
		m_Style.m_fRadius = 10;
		m_Style.m_bBlockHit = true;
	}

	//------------------------------------------------------------------------------------------------
	override void ApplyTheme(notnull MUI_ThemeData theme)
	{
		m_Style.m_Fill = theme.Panel;
	}

	//------------------------------------------------------------------------------------------------
	void MakeOverlay()
	{
		m_Style.m_Layout = MUI_LayoutKind.Overlay;
		m_Style.m_WidthMode = MUI_SizeMode.Fill;
		m_Style.m_HeightMode = MUI_SizeMode.Fill;
		m_Style.m_Fill = GetTheme().Overlay;
		m_Style.m_fRadius = 0;
		m_Style.m_bBlockHit = true;
		InvalidateLayout();
	}

	//------------------------------------------------------------------------------------------------
	//! Transparent fullscreen overlay that does not dim the world or eat hits (HUD toasts).
	void MakePassThroughOverlay()
	{
		m_Style.m_Layout = MUI_LayoutKind.Overlay;
		m_Style.m_WidthMode = MUI_SizeMode.Fill;
		m_Style.m_HeightMode = MUI_SizeMode.Fill;
		m_Style.m_Fill = Color.FromInt(0);
		m_Style.m_fRadius = 0;
		m_Style.m_bBlockHit = false;
		InvalidateLayout();
	}
}
