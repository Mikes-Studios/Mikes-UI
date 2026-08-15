//------------------------------------------------------------------------------------------------
//! Resource GUIDs shipped by this addon. Consumers reference these from their own
//! chimeraMenus.conf — do not copy the .layout file into your addon.
//!
//! Consumer:
//!   MenuPreset MyMenu {
//!     Layout "{A1C47E92B6D8305F}UI/Layouts/MUI_BlankMenu.layout"
//!     Class "MyMenu"
//!   }
//!   Or use MUI_Resources.BLANK_MENU_LAYOUT from script when creating widgets.
//------------------------------------------------------------------------------------------------
class MUI_Resources
{
	static const string BLANK_MENU_LAYOUT = "{A1C47E92B6D8305F}UI/Layouts/MUI_BlankMenu.layout";
}
