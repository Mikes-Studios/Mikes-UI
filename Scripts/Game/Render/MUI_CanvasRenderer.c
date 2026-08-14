//------------------------------------------------------------------------------------------------
class MUI_CanvasRenderer
{
	protected static const int CIRCLE_SEGS = 16;
	protected static const int CORNER_SEGS = 4;

	//------------------------------------------------------------------------------------------------
	static void AddFillRect(notnull array<ref CanvasWidgetCommand> commands, float x, float y, float w, float h, int color, float radius)
	{
		if (w <= 0.5)
			return;
		if (h <= 0.5)
			return;

		ref PolygonDrawCommand poly = new PolygonDrawCommand();
		poly.m_iColor = color;
		poly.m_Vertices = BuildRectVertices(x, y, w, h, radius);
		if (!poly.m_Vertices || poly.m_Vertices.Count() < 6)
			return;
		commands.Insert(poly);
	}

	//------------------------------------------------------------------------------------------------
	static void AddStrokeRect(notnull array<ref CanvasWidgetCommand> commands, float x, float y, float w, float h, int color, float width, float radius)
	{
		if (w <= 0.5)
			return;
		if (h <= 0.5)
			return;
		if (width <= 0)
			return;

		ref LineDrawCommand line = new LineDrawCommand();
		line.m_iColor = color;
		line.m_fWidth = width;
		line.m_bShouldEnclose = true;
		line.m_Vertices = BuildRectVertices(x, y, w, h, radius);
		if (!line.m_Vertices || line.m_Vertices.Count() < 6)
			return;
		commands.Insert(line);
	}

	//------------------------------------------------------------------------------------------------
	protected static array<float> BuildRectVertices(float x, float y, float w, float h, float radius)
	{
		ref array<float> verts = new array<float>();
		float maxR = w;
		if (h < maxR)
			maxR = h;
		maxR = maxR * 0.5;
		if (radius > maxR)
			radius = maxR;

		if (radius <= 0.5)
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
		if (r <= 0.5)
			return;
		ref PolygonDrawCommand poly = new PolygonDrawCommand();
		poly.m_iColor = color;
		poly.m_Vertices = BuildCircleVertices(cx, cy, r, 0, 360, CIRCLE_SEGS, true);
		if (!poly.m_Vertices || poly.m_Vertices.Count() < 6)
			return;
		commands.Insert(poly);
	}

	//------------------------------------------------------------------------------------------------
	static void AddStrokeCircle(notnull array<ref CanvasWidgetCommand> commands, float cx, float cy, float r, int color, float width)
	{
		if (r <= 0.5)
			return;
		if (width <= 0)
			return;
		ref LineDrawCommand line = new LineDrawCommand();
		line.m_iColor = color;
		line.m_fWidth = width;
		line.m_bShouldEnclose = true;
		line.m_Vertices = BuildCircleVertices(cx, cy, r, 0, 360, CIRCLE_SEGS, false);
		if (!line.m_Vertices || line.m_Vertices.Count() < 6)
			return;
		commands.Insert(line);
	}

	//------------------------------------------------------------------------------------------------
	static void AddArc(notnull array<ref CanvasWidgetCommand> commands, float cx, float cy, float r, float startDeg, float sweepDeg, int color, float width)
	{
		if (r <= 0.5)
			return;
		if (width <= 0)
			return;
		ref LineDrawCommand line = new LineDrawCommand();
		line.m_iColor = color;
		line.m_fWidth = width;
		line.m_bShouldEnclose = false;
		int segs = CIRCLE_SEGS;
		if (Math.AbsFloat(sweepDeg) > 180)
			segs = CIRCLE_SEGS * 2;
		line.m_Vertices = BuildCircleVertices(cx, cy, r, startDeg, sweepDeg, segs, false);
		if (!line.m_Vertices || line.m_Vertices.Count() < 4)
			return;
		commands.Insert(line);
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
			verts.Insert(cx + r * Math.Cos(rad));
			verts.Insert(cy + r * Math.Sin(rad));
		}
	}
}
