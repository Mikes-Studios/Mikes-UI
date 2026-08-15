//------------------------------------------------------------------------------------------------
//! Layout of children inside a node.
//! Overlay: children stacked in z-order; Fill sizes to inner box; SetAlign(ax, ay) positions.
//! StackVertical / StackHorizontal: gap + grow. Main-axis Fill measures as Hug then flexes.
//------------------------------------------------------------------------------------------------
enum MUI_LayoutKind
{
	Overlay,
	StackVertical,
	StackHorizontal
}

//------------------------------------------------------------------------------------------------
//! Fill = take leftover (cross-axis or overlay) / flex on main axis.
//! Hug = intrinsic or children. Exact = SetWidth / SetHeight.
enum MUI_SizeMode
{
	Fill,
	Hug,
	Exact
}
