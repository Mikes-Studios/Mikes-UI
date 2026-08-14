//------------------------------------------------------------------------------------------------
class MUI_LayoutEngine
{
	//------------------------------------------------------------------------------------------------
	static void LayoutTree(notnull MUI_Node root, float x, float y, float w, float h)
	{
		Measure(root, w, h, true, true);
		Arrange(root, x, y, w, h);
	}

	//------------------------------------------------------------------------------------------------
	//! stretchW/stretchH: whether Fill on that axis should consume avail (cross-axis or overlay).
	//! Main-axis Fill is measured as Hug so siblings can share leftover during Arrange.
	static void Measure(notnull MUI_Node node, float availW, float availH, bool stretchW, bool stretchH)
	{
		if (!node.IsVisible())
		{
			node.SetDesiredSize(0, 0);
			return;
		}

		MUI_Style style = node.GetStyle();
		float padX = style.m_fPadL + style.m_fPadR;
		float padY = style.m_fPadT + style.m_fPadB;
		float innerAvailW = availW - padX;
		float innerAvailH = availH - padY;
		if (style.m_WidthMode == MUI_SizeMode.Exact)
			innerAvailW = style.m_fWidth - padX;
		if (style.m_HeightMode == MUI_SizeMode.Exact)
			innerAvailH = style.m_fHeight - padY;
		if (innerAvailW < 0)
			innerAvailW = 0;
		if (innerAvailH < 0)
			innerAvailH = 0;

		int count = node.GetChildCount();
		int i;
		MUI_Node child;

		if (style.m_Layout == MUI_LayoutKind.StackHorizontal)
		{
			float used = 0;
			int visibleCount = 0;
			float maxChildH = 0;
			for (i = 0; i < count; i++)
			{
				child = node.GetChild(i);
				if (!child || !child.IsVisible())
					continue;
				visibleCount = visibleCount + 1;
				float childAvailW = innerAvailW;
				float childAvailH = innerAvailH;
				if (child.GetStyle().m_WidthMode == MUI_SizeMode.Exact)
					childAvailW = child.GetStyle().m_fWidth;
				if (child.GetStyle().m_HeightMode == MUI_SizeMode.Exact)
					childAvailH = child.GetStyle().m_fHeight;
				Measure(child, childAvailW, childAvailH, false, true);
				used = used + child.GetDesiredWidth();
				if (child.GetDesiredHeight() > maxChildH)
					maxChildH = child.GetDesiredHeight();
			}
			if (visibleCount > 1)
				used = used + style.m_fGap * (visibleCount - 1);

			node.MeasureIntrinsic(innerAvailW, innerAvailH);
			float desW = used + padX;
			float desH = maxChildH + padY;
			if (node.GetDesiredHeight() + padY > desH)
				desH = node.GetDesiredHeight() + padY;
			ApplySizeModes(node, desW, desH, availW, availH, stretchW, stretchH);
			return;
		}

		if (style.m_Layout == MUI_LayoutKind.StackVertical)
		{
			float used = 0;
			int visibleCount = 0;
			float maxChildW = 0;
			for (i = 0; i < count; i++)
			{
				child = node.GetChild(i);
				if (!child || !child.IsVisible())
					continue;
				visibleCount = visibleCount + 1;
				float childAvailW = innerAvailW;
				float childAvailH = innerAvailH;
				if (child.GetStyle().m_WidthMode == MUI_SizeMode.Exact)
					childAvailW = child.GetStyle().m_fWidth;
				if (child.GetStyle().m_HeightMode == MUI_SizeMode.Exact)
					childAvailH = child.GetStyle().m_fHeight;
				Measure(child, childAvailW, childAvailH, true, false);
				used = used + child.GetDesiredHeight();
				if (child.GetDesiredWidth() > maxChildW)
					maxChildW = child.GetDesiredWidth();
			}
			if (visibleCount > 1)
				used = used + style.m_fGap * (visibleCount - 1);

			node.MeasureIntrinsic(innerAvailW, innerAvailH);
			float desW = maxChildW + padX;
			if (node.GetDesiredWidth() + padX > desW)
				desW = node.GetDesiredWidth() + padX;
			float desH = used + padY;
			if (node.GetDesiredHeight() + padY > desH)
				desH = node.GetDesiredHeight() + padY;
			ApplySizeModes(node, desW, desH, availW, availH, stretchW, stretchH);
			return;
		}

		float maxW = 0;
		float maxH = 0;
		for (i = 0; i < count; i++)
		{
			child = node.GetChild(i);
			if (!child || !child.IsVisible())
				continue;
			Measure(child, innerAvailW, innerAvailH, true, true);
			if (child.GetDesiredWidth() > maxW)
				maxW = child.GetDesiredWidth();
			if (child.GetDesiredHeight() > maxH)
				maxH = child.GetDesiredHeight();
		}
		node.MeasureIntrinsic(innerAvailW, innerAvailH);
		float overlayW = node.GetDesiredWidth() + padX;
		float overlayH = node.GetDesiredHeight() + padY;
		if (maxW + padX > overlayW)
			overlayW = maxW + padX;
		if (maxH + padY > overlayH)
			overlayH = maxH + padY;
		ApplySizeModes(node, overlayW, overlayH, availW, availH, stretchW, stretchH);
	}

	//------------------------------------------------------------------------------------------------
	protected static void ApplySizeModes(notnull MUI_Node node, float hugW, float hugH, float availW, float availH, bool stretchW, bool stretchH)
	{
		MUI_Style style = node.GetStyle();
		float w = hugW;
		float h = hugH;

		if (style.m_WidthMode == MUI_SizeMode.Exact)
			w = style.m_fWidth;
		else if (stretchW)
		{
			if (style.m_WidthMode == MUI_SizeMode.Fill)
				w = availW;
		}

		if (style.m_HeightMode == MUI_SizeMode.Exact)
			h = style.m_fHeight;
		else if (stretchH)
		{
			if (style.m_HeightMode == MUI_SizeMode.Fill)
				h = availH;
		}

		if (w < style.m_fMinWidth)
			w = style.m_fMinWidth;
		if (h < style.m_fMinHeight)
			h = style.m_fMinHeight;
		if (w > style.m_fMaxWidth)
			w = style.m_fMaxWidth;
		if (h > style.m_fMaxHeight)
			h = style.m_fMaxHeight;
		if (w < 0)
			w = 0;
		if (h < 0)
			h = 0;

		node.SetDesiredSize(w, h);
	}

	//------------------------------------------------------------------------------------------------
	static void Arrange(notnull MUI_Node node, float x, float y, float w, float h)
	{
		node.SetWorld(x, y, w, h);
		if (!node.IsVisible())
			return;

		MUI_Style style = node.GetStyle();
		float innerX = x + style.m_fPadL;
		float innerY = y + style.m_fPadT;
		float innerW = w - style.m_fPadL - style.m_fPadR;
		float innerH = h - style.m_fPadT - style.m_fPadB;
		if (innerW < 0)
			innerW = 0;
		if (innerH < 0)
			innerH = 0;

		int count = node.GetChildCount();
		int i;
		MUI_Node child;

		if (style.m_Layout == MUI_LayoutKind.StackVertical)
		{
			int visibleCount = CountVisible(node);
			float content = 0;
			float totalFlex = 0;
			int visIndex = 0;
			for (i = 0; i < count; i++)
			{
				child = node.GetChild(i);
				if (!child || !child.IsVisible())
					continue;
				content = content + child.GetDesiredHeight();
				totalFlex = totalFlex + MainAxisFlex(child, false);
				visIndex = visIndex + 1;
				if (visIndex < visibleCount)
					content = content + style.m_fGap;
			}
			node.SetContentHeight(content);
			node.ClampScroll(innerH);

			float extra = innerH - content;
			if (extra < 0)
				extra = 0;

			float cursor = innerY - node.GetScrollY();
			visIndex = 0;
			for (i = 0; i < count; i++)
			{
				child = node.GetChild(i);
				if (!child || !child.IsVisible())
					continue;
				float cw = child.GetDesiredWidth();
				float ch = child.GetDesiredHeight();
				if (child.GetStyle().m_WidthMode == MUI_SizeMode.Fill)
					cw = innerW;
				float flex = MainAxisFlex(child, false);
				if (totalFlex > 0 && flex > 0)
					ch = ch + extra * (flex / totalFlex);
				float cx = innerX + (innerW - cw) * child.GetStyle().m_fAlignX;
				Arrange(child, cx, cursor, cw, ch);
				cursor = cursor + ch;
				visIndex = visIndex + 1;
				if (visIndex < visibleCount)
					cursor = cursor + style.m_fGap;
			}
			return;
		}

		if (style.m_Layout == MUI_LayoutKind.StackHorizontal)
		{
			int visibleCount = CountVisible(node);
			float totalFlex = 0;
			float used = 0;
			for (i = 0; i < count; i++)
			{
				child = node.GetChild(i);
				if (!child || !child.IsVisible())
					continue;
				used = used + child.GetDesiredWidth();
				totalFlex = totalFlex + MainAxisFlex(child, true);
			}
			if (visibleCount > 1)
				used = used + style.m_fGap * (visibleCount - 1);
			float extra = innerW - used;
			if (extra < 0)
				extra = 0;

			float cursor = innerX;
			int visIndex = 0;
			for (i = 0; i < count; i++)
			{
				child = node.GetChild(i);
				if (!child || !child.IsVisible())
					continue;
				float cw = child.GetDesiredWidth();
				float flex = MainAxisFlex(child, true);
				if (totalFlex > 0 && flex > 0)
					cw = cw + extra * (flex / totalFlex);
				float ch = child.GetDesiredHeight();
				if (child.GetStyle().m_HeightMode == MUI_SizeMode.Fill)
					ch = innerH;
				float cy = innerY + (innerH - ch) * child.GetStyle().m_fAlignY;
				Arrange(child, cursor, cy, cw, ch);
				cursor = cursor + cw;
				visIndex = visIndex + 1;
				if (visIndex < visibleCount)
					cursor = cursor + style.m_fGap;
			}
			return;
		}

		for (i = 0; i < count; i++)
		{
			child = node.GetChild(i);
			if (!child || !child.IsVisible())
				continue;
			float cw = child.GetDesiredWidth();
			float ch = child.GetDesiredHeight();
			if (child.GetStyle().m_WidthMode == MUI_SizeMode.Fill)
				cw = innerW;
			if (child.GetStyle().m_HeightMode == MUI_SizeMode.Fill)
				ch = innerH;
			float cx = innerX + (innerW - cw) * child.GetStyle().m_fAlignX;
			float cy = innerY + (innerH - ch) * child.GetStyle().m_fAlignY;
			Arrange(child, cx, cy, cw, ch);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static float MainAxisFlex(notnull MUI_Node child, bool horizontal)
	{
		MUI_Style style = child.GetStyle();
		float grow = style.m_fGrow;
		if (horizontal)
		{
			if (style.m_WidthMode == MUI_SizeMode.Fill && grow <= 0)
				return 1;
			return grow;
		}
		if (style.m_HeightMode == MUI_SizeMode.Fill && grow <= 0)
			return 1;
		return grow;
	}

	//------------------------------------------------------------------------------------------------
	protected static int CountVisible(notnull MUI_Node node)
	{
		int n = 0;
		int count = node.GetChildCount();
		int i;
		for (i = 0; i < count; i++)
		{
			MUI_Node child = node.GetChild(i);
			if (child && child.IsVisible())
				n = n + 1;
		}
		return n;
	}
}
