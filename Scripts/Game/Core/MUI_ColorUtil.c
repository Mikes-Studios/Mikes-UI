//------------------------------------------------------------------------------------------------
//! Color helpers. Enfusion Color stores **linear** RGB.
//! Theme tokens come from FromSRGBA (sRGB authoring → linear storage).
//! Pack for Canvas/TextWidget with PackToInt() (linear → packed sRGB ARGB).
class MUI_ColorUtil
{
	protected static ref Color s_Scratch;
	protected static ref array<ref Color> s_FadePool;
	protected static int s_iFade;

	//------------------------------------------------------------------------------------------------
	protected static Color Scratch()
	{
		if (!s_Scratch)
			s_Scratch = new Color(1, 1, 1, 1);
		return s_Scratch;
	}

	//------------------------------------------------------------------------------------------------
	//! Multiplies alpha only. RGB stays linear.
	static Color Fade(Color src, float opacity)
	{
		if (!src)
		{
			Color s = Scratch();
			s.SetR(0);
			s.SetG(0);
			s.SetB(0);
			s.SetA(0);
			return s;
		}

		if (!s_FadePool)
			s_FadePool = new array<ref Color>();

		ref Color c = src.Copy();
		float a = src.A() * opacity;
		if (a < 0)
			a = 0;
		if (a > 1)
			a = 1;
		c.SetA(a);

		if (s_FadePool.Count() < 96)
		{
			s_FadePool.Insert(c);
		}
		else
		{
			s_FadePool[s_iFade] = c;
			s_iFade = s_iFade + 1;
			if (s_iFade >= 96)
				s_iFade = 0;
		}
		return c;
	}

	//------------------------------------------------------------------------------------------------
	static Color Mix(Color a, Color b, float t, notnull Color dest)
	{
		if (t < 0)
			t = 0;
		if (t > 1)
			t = 1;
		if (!a)
		{
			dest.SetR(0);
			dest.SetG(0);
			dest.SetB(0);
			dest.SetA(0);
			return dest;
		}
		if (!b)
		{
			dest.SetR(a.R());
			dest.SetG(a.G());
			dest.SetB(a.B());
			dest.SetA(a.A());
			return dest;
		}
		dest.SetR(a.R() + (b.R() - a.R()) * t);
		dest.SetG(a.G() + (b.G() - a.G()) * t);
		dest.SetB(a.B() + (b.B() - a.B()) * t);
		dest.SetA(a.A() + (b.A() - a.A()) * t);
		return dest;
	}

	//------------------------------------------------------------------------------------------------
	//! Pack a linear Color for Canvas / SetColorInt (sRGB ARGB int).
	static int Pack(Color src)
	{
		if (!src)
			return 0;
		return src.PackToInt();
	}

	//------------------------------------------------------------------------------------------------
	//! Author sRGB 0-1 floats → packed ARGB. Prefer Theme.FromSRGBA tokens over this.
	static int PackSRGB(float r, float g, float b, float a)
	{
		int ri = Math.Round(Math.Clamp(r, 0, 1) * 255);
		int gi = Math.Round(Math.Clamp(g, 0, 1) * 255);
		int bi = Math.Round(Math.Clamp(b, 0, 1) * 255);
		int ai = Math.Round(Math.Clamp(a, 0, 1) * 255);
		ref Color c = Color.FromSRGBA(ri, gi, bi, ai);
		return c.PackToInt();
	}
}
