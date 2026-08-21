//------------------------------------------------------------------------------------------------
//! Tessellates rects/circles/arcs/polygons into CanvasWidgetCommand. Internal — call via MUI_RenderSurface.
//! Degenerate polygons: radius is clamped to min(w,h)*0.5-1; tiny fills (w/h < 0.75) are skipped.
//------------------------------------------------------------------------------------------------
class MUI_CanvasRenderer
{
	protected static const int CIRCLE_SEGS = 28;
	protected static const int CORNER_SEGS = 8;

	//------------------------------------------------------------------------------------------------
	static void AddFillRect(notnull array<ref CanvasWidgetCommand> commands, float x, float y, float w, float h, int color, float radius)
	{
		if (w <= 0.75)
			return;
		if (h <= 0.75)
			return;

		ref array<float> verts = BuildRectVertices(x, y, w, h, radius);
		if (!IsSolidPolygon(verts))
			return;

		ref PolygonDrawCommand poly = new PolygonDrawCommand();
		poly.m_iColor = color;
		poly.m_Vertices = verts;
		commands.Insert(poly);
	}

	//------------------------------------------------------------------------------------------------
	static void AddStrokeRect(notnull array<ref CanvasWidgetCommand> commands, float x, float y, float w, float h, int color, float width, float radius)
	{
		if (w <= 0.75)
			return;
		if (h <= 0.75)
			return;
		if (width <= 0)
			return;

		ref array<float> verts = BuildRectVertices(x, y, w, h, radius);
		if (!verts || verts.Count() < 6)
			return;

		ref LineDrawCommand line = new LineDrawCommand();
		line.m_iColor = color;
		line.m_fWidth = width;
		line.m_bShouldEnclose = true;
		line.m_Vertices = verts;
		commands.Insert(line);
	}

	//------------------------------------------------------------------------------------------------
	protected static array<float> BuildRectVertices(float x, float y, float w, float h, float radius)
	{
		ref array<float> verts = new array<float>();
		float maxR = w;
		if (h < maxR)
			maxR = h;
		maxR = maxR * 0.5 - 1.0;
		if (maxR < 0.75)
			radius = 0;
		else if (radius > maxR)
			radius = maxR;

		if (radius <= 0.75)
		{
			verts.Insert(x);
			verts.Insert(y);
			verts.Insert(x + w);
			verts.Insert(y);
			verts.Insert(x + w);
			verts.Insert(y + h);
			verts.Insert(x);
			verts.Insert(y + h);
			return verts;
		}

		AddCorner(verts, x + w - radius, y + radius, radius, 270);
		AddCorner(verts, x + w - radius, y + h - radius, radius, 0);
		AddCorner(verts, x + radius, y + h - radius, radius, 90);
		AddCorner(verts, x + radius, y + radius, radius, 180);
		return verts;
	}

	//------------------------------------------------------------------------------------------------
	static void AddLine(notnull array<ref CanvasWidgetCommand> commands, float x0, float y0, float x1, float y1, int color, float width)
	{
		if (width <= 0)
			return;
		ref LineDrawCommand line = new LineDrawCommand();
		line.m_iColor = color;
		line.m_fWidth = width;
		line.m_bShouldEnclose = false;
		ref array<float> verts = new array<float>();
		verts.Insert(x0);
		verts.Insert(y0);
		verts.Insert(x1);
		verts.Insert(y1);
		line.m_Vertices = verts;
		commands.Insert(line);
	}

	//------------------------------------------------------------------------------------------------
	static void AddFillCircle(notnull array<ref CanvasWidgetCommand> commands, float cx, float cy, float r, int color)
	{
		if (r < 1.25)
			return;

		int segs = CircleSegs(r);
		ref array<float> verts = BuildCircleVertices(cx, cy, r, 0, 360, segs, true);
		if (!IsSolidPolygon(verts))
			return;

		ref PolygonDrawCommand poly = new PolygonDrawCommand();
		poly.m_iColor = color;
		poly.m_Vertices = verts;
		commands.Insert(poly);
	}

	//------------------------------------------------------------------------------------------------
	static void AddStrokeCircle(notnull array<ref CanvasWidgetCommand> commands, float cx, float cy, float r, int color, float width)
	{
		if (r < 1.25)
			return;
		if (width <= 0)
			return;
		ref LineDrawCommand line = new LineDrawCommand();
		line.m_iColor = color;
		line.m_fWidth = width;
		line.m_bShouldEnclose = true;
		line.m_Vertices = BuildCircleVertices(cx, cy, r, 0, 360, CircleSegs(r), false);
		if (!line.m_Vertices || line.m_Vertices.Count() < 6)
			return;
		commands.Insert(line);
	}

	//------------------------------------------------------------------------------------------------
	static void AddArc(notnull array<ref CanvasWidgetCommand> commands, float cx, float cy, float r, float startDeg, float sweepDeg, int color, float width)
	{
		if (r < 1.25)
			return;
		if (width <= 0)
			return;
		if (Math.AbsFloat(sweepDeg) < 2)
			return;
		ref LineDrawCommand line = new LineDrawCommand();
		line.m_iColor = color;
		line.m_fWidth = width;
		line.m_bShouldEnclose = false;
		int segs = CircleSegs(r);
		if (Math.AbsFloat(sweepDeg) > 180)
			segs = segs * 2;
		line.m_Vertices = BuildCircleVertices(cx, cy, r, startDeg, sweepDeg, segs, false);
		if (!line.m_Vertices || line.m_Vertices.Count() < 4)
			return;
		commands.Insert(line);
	}

	//------------------------------------------------------------------------------------------------
	static void AddFillPolygon(notnull array<ref CanvasWidgetCommand> commands, array<float> verts, int color)
	{
		if (!IsSolidPolygon(verts))
			return;

		ref PolygonDrawCommand poly = new PolygonDrawCommand();
		poly.m_iColor = color;
		poly.m_Vertices = verts;
		commands.Insert(poly);
	}

	//------------------------------------------------------------------------------------------------
	static void AddPolyline(notnull array<ref CanvasWidgetCommand> commands, array<float> verts, int color, float width, bool enclose)
	{
		if (!verts)
			return;
		if (verts.Count() < 4)
			return;
		if (width <= 0)
			return;

		ref LineDrawCommand line = new LineDrawCommand();
		line.m_iColor = color;
		line.m_fWidth = width;
		line.m_bShouldEnclose = enclose;
		line.m_Vertices = verts;
		commands.Insert(line);
	}

	//------------------------------------------------------------------------------------------------
	protected static int CircleSegs(float r)
	{
		int segs = r * 2.2;
		if (segs < 8)
			segs = 8;
		if (segs > CIRCLE_SEGS)
			segs = CIRCLE_SEGS;
		return segs;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool IsSolidPolygon(array<float> verts)
	{
		if (!verts)
			return false;
		int count = verts.Count();
		if (count < 6)
			return false;
		if ((count % 2) != 0)
			return false;

		float minX = verts[0];
		float maxX = verts[0];
		float minY = verts[1];
		float maxY = verts[1];
		int i;
		for (i = 2; i < count; i = i + 2)
		{
			float px = verts[i];
			float py = verts[i + 1];
			if (px < minX)
				minX = px;
			if (px > maxX)
				maxX = px;
			if (py < minY)
				minY = py;
			if (py > maxY)
				maxY = py;
		}
		if ((maxX - minX) < 0.75)
			return false;
		if ((maxY - minY) < 0.75)
			return false;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static array<float> BuildCircleVertices(float cx, float cy, float r, float startDeg, float sweepDeg, int segs, bool close)
	{
		ref array<float> verts = new array<float>();
		if (segs < 3)
			segs = 3;
		int n = segs;
		if (close)
			n = segs;
		int i;
		for (i = 0; i <= n; i++)
		{
			float t = i;
			if (!close && i == n)
				t = n;
			float deg = startDeg + sweepDeg * (t / segs);
			if (close && i == segs)
				break;
			float rad = deg * Math.DEG2RAD;
			verts.Insert(cx + r * Math.Cos(rad));
			verts.Insert(cy + r * Math.Sin(rad));
		}
		return verts;
	}

	//------------------------------------------------------------------------------------------------
	protected static void AddCorner(notnull array<float> verts, float cx, float cy, float r, float startDeg)
	{
		int i;
		for (i = 0; i <= CORNER_SEGS; i++)
		{
			float deg = startDeg + (90.0 * i / CORNER_SEGS);
			float rad = deg * Math.DEG2RAD;
			float px = cx + r * Math.Cos(rad);
			float py = cy + r * Math.Sin(rad);
			int last = verts.Count();
			if (last >= 2)
			{
				float dx = px - verts[last - 2];
				float dy = py - verts[last - 1];
				if ((dx * dx + dy * dy) < 0.04)
					continue;
			}
			verts.Insert(px);
			verts.Insert(py);
		}
	}
}
