//------------------------------------------------------------------------------------------------
//! In-addon catalog / consumer pattern demo. Open with:
//!   GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.MUI_SampleMenu);
//! Copy this pattern into YOUR addon — do not add screens to Mikes-UI.
//------------------------------------------------------------------------------------------------
class MUI_SampleDot : MUI_Node
{
	//------------------------------------------------------------------------------------------------
	void MUI_SampleDot()
	{
		m_Style.m_WidthMode = MUI_SizeMode.Exact;
		m_Style.m_HeightMode = MUI_SizeMode.Exact;
		m_Style.m_fWidth = 14;
		m_Style.m_fHeight = 14;
		m_Style.m_fMinWidth = 14;
		m_Style.m_fMinHeight = 14;
		m_Style.m_Fill = Color.FromInt(0);
	}

	//------------------------------------------------------------------------------------------------
	override void PaintForeground(MUI_RenderSurface surface)
	{
		float op = GetDrawOpacity();
		if (op < 0.01)
			return;
		float r = m_World.m_fW * 0.5;
		surface.FillCircle(DrawX() + r, DrawY() + r, r, MUI_ColorUtil.Fade(GetTheme().Live, op));
	}
}

//------------------------------------------------------------------------------------------------
class MUI_SampleMenu : MUI_MenuBase
{
	protected ref MUI_TextField m_NameField;
	protected ref MUI_Toggle m_ReadyToggle;
	protected ref MUI_NumericField m_Numeric;
	protected ref MUI_Slider m_Slider;
	protected ref MUI_Progress m_Progress;
	protected ref MUI_Tabs m_Tabs;
	protected ref MUI_Dropdown m_Dropdown;
	protected ref MUI_Label m_Status;
	protected ref MUI_HintLayer m_Hints;

	//------------------------------------------------------------------------------------------------
	override string GetMUILogTag()
	{
		return "MUI_SampleMenu";
	}

	//------------------------------------------------------------------------------------------------
	override void BuildUI(notnull MUI_Runtime runtime)
	{
		// Optional recolor — call before Create* / Adopt. Hot-swap after SetRoot is not supported.
		// ref MUI_ThemeData theme = MUI_ThemeData.CreateUplink();
		// theme.Accent = Color.FromSRGBA(80, 160, 90, 255);
		// runtime.SetTheme(theme);

		ref MUI_Panel overlay = runtime.CreatePanel("overlay");
		overlay.MakeOverlay();
		overlay.GetStyle().m_Fill = Color.FromInt(0);

		ref MUI_FxBackdrop fx = runtime.CreateFxBackdrop("fx");

		ref MUI_Card card = runtime.CreateCard("card");
		card.SetWidth(580);
		card.SetPadding(28);
		card.SetPaddingTRBL(22, 28, 24, 28);
		card.SetGap(12);
		card.SetAlign(0.5, 0.5);
		card.SetIntro(0.06, 0.55, 46);

		ref MUI_LiveHeader liveHeader = runtime.CreateLiveHeader("SAMPLE CATALOG", "liveHeader");
		liveHeader.SetKicker("VERSION 0.2.0");
		liveHeader.SetIntro(0.16, 0.4, 18);

		ref MUI_Label subtitle = runtime.CreateLabel("Subclass MUI_MenuBase in your addon. Skin factories keep their uplink look.", "subtitle");
		subtitle.SetFontSize(MUI_Theme.FONT_SMALL);
		subtitle.SetMuted(true);

		ref MUI_Hairline lineA = runtime.CreateHairline("lineA");

		ref MUI_ScrollView scroll = runtime.CreateScrollView("scroll");
		scroll.SetViewportHeight(440);
		scroll.SetGap(12);

		ref MUI_Surface surface = runtime.CreateSurface("surface");
		surface.SetFillWidth();
		surface.SetPadding(16);
		surface.SetGap(10);
		surface.SetBlurEnabled(true);

		ref MUI_Header plainHeader = runtime.CreateHeader("PRIMITIVES", "plainHeader");
		plainHeader.SetKicker("SURFACE, HEADER, DIVIDER");

		ref MUI_Divider div = runtime.CreateDivider("div");

		ref MUI_Label primNote = runtime.CreateLabel("CreateSurface honors SetFill. CreateCard keeps uplink chrome.", "primNote");
		primNote.SetFontSize(MUI_Theme.FONT_SMALL);
		primNote.SetMuted(true);

		surface.AddChild(plainHeader);
		surface.AddChild(div);
		surface.AddChild(primNote);

		m_NameField = runtime.CreateTextField("Callsign", "name");
		m_NameField.SetText("Alpha");
		m_NameField.GetOnChanged().Insert(OnSampleChanged);

		m_ReadyToggle = runtime.CreateToggle("Ready for tasking", "ready");
		m_ReadyToggle.SetChecked(true);
		m_ReadyToggle.GetOnChanged().Insert(OnSampleChanged);

		m_Numeric = runtime.CreateNumericField("Numeric field", "num");
		m_Numeric.SetRange(0, 10);
		m_Numeric.SetStep(0.5);
		m_Numeric.SetDecimals(1);
		m_Numeric.SetValue(2.5);
		m_Numeric.GetOnChanged().Insert(OnSampleChanged);

		m_Slider = runtime.CreateSlider("slider");
		m_Slider.SetRange(0, 1);
		m_Slider.SetStep(0.05);
		m_Slider.SetValue(0.65);
		m_Slider.GetOnChanged().Insert(OnSliderChanged);

		m_Progress = runtime.CreateProgress("progress");
		m_Progress.SetValue(0.65);

		m_Tabs = runtime.CreateTabs("tabs");
		m_Tabs.AddTab("Alpha");
		m_Tabs.AddTab("Bravo");
		m_Tabs.AddTab("Charlie");
		m_Tabs.SetIndex(0);
		m_Tabs.GetOnChanged().Insert(OnSampleChanged);

		m_Dropdown = runtime.CreateDropdown("dropdown");
		m_Dropdown.AddItem("Blufor");
		m_Dropdown.AddItem("Opfor");
		m_Dropdown.AddItem("Independent");
		m_Dropdown.SetIndex(0);
		m_Dropdown.GetOnChanged().Insert(OnSampleChanged);

		ref MUI_Image img = runtime.CreateImage("img");
		img.SetWidth(40);
		img.SetHeight(40);

		ref MUI_Row badgeRow = runtime.CreateRow("badgeRow");
		badgeRow.SetGap(10);
		badgeRow.SetHeight(18);

		ref MUI_SampleDot dot = new MUI_SampleDot();
		runtime.Adopt(dot);
		dot.SetName("dot");

		ref MUI_Label badgeLbl = runtime.CreateLabel("Adopt() custom node", "badgeLbl");
		badgeLbl.SetFontSize(MUI_Theme.FONT_SMALL);
		badgeLbl.SetMuted(true);
		badgeLbl.SetFillWidth();
		badgeLbl.SetGrow(1);

		badgeRow.AddChild(dot);
		badgeRow.AddChild(badgeLbl);
		badgeRow.AddChild(img);

		m_Status = runtime.CreateLabel("", "status");
		m_Status.SetFontSize(MUI_Theme.FONT_SMALL);
		m_Status.SetMuted(true);

		scroll.AddChild(surface);
		scroll.AddChild(m_NameField);
		scroll.AddChild(m_ReadyToggle);
		scroll.AddChild(m_Numeric);
		scroll.AddChild(runtime.CreateLabel("Slider + progress", "slLbl"));
		scroll.AddChild(m_Slider);
		scroll.AddChild(m_Progress);
		scroll.AddChild(m_Tabs);
		scroll.AddChild(m_Dropdown);
		scroll.AddChild(badgeRow);
		scroll.AddChild(m_Status);

		ref MUI_Hairline lineB = runtime.CreateHairline("lineB");

		ref MUI_Row buttons = runtime.CreateRow("buttons");
		buttons.SetGap(12);

		ref MUI_Button helpBtn = runtime.CreateButton("Help", "help");
		helpBtn.GetOnClicked().Insert(OnSampleHelp);

		ref MUI_Button okBtn = runtime.CreateButton("Close", "ok");
		okBtn.MakeAccent();
		okBtn.GetOnClicked().Insert(OnMUIBack);

		buttons.AddChild(helpBtn);
		buttons.AddChild(okBtn);

		card.AddChild(liveHeader);
		card.AddChild(subtitle);
		card.AddChild(lineA);
		card.AddChild(scroll);
		card.AddChild(lineB);
		card.AddChild(buttons);

		m_Hints = runtime.CreateHintLayer("hints");
		m_Hints.AddHint(surface, "Primitives", "CreateSurface honors SetFill. CreateCard keeps the uplink chrome.");
		m_Hints.AddHint(m_NameField, "Callsign", "Text field with IME. GetOnChanged fires when SetText commits.");
		m_Hints.AddHint(m_ReadyToggle, "Ready", "Gamepad left/right flips the toggle.");
		m_Hints.AddHint(m_Numeric, "Numeric", "Typed float with SetRange / SetStep. Left/right steps the value.");
		m_Hints.AddHint(m_Slider, "Slider", "Drag or gamepad left/right. This sample mirrors the value into Progress.");
		m_Hints.AddHint(m_Tabs, "Tabs", "Segmented bar. AddTab / SetIndex / GetOnChanged.");
		m_Hints.AddHint(m_Dropdown, "Dropdown", "In-flow list plus an outside-click catcher.");
		m_Hints.AddHint(badgeRow, "Custom node", "runtime.Adopt your MUI_Node subclass, then AddChild.");

		overlay.AddChild(fx);
		overlay.AddChild(card);
		overlay.AddChild(m_Hints);
		runtime.SetRoot(overlay);

		RefreshStatus();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSampleHelp()
	{
		if (m_Hints)
			m_Hints.Toggle();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSliderChanged()
	{
		if (m_Slider && m_Progress)
			m_Progress.SetValue(m_Slider.GetValue());
		RefreshStatus();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSampleChanged()
	{
		RefreshStatus();
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshStatus()
	{
		if (!m_Status)
			return;

		string name = "-";
		if (m_NameField)
			name = m_NameField.GetText();

		string ready = "STANDBY";
		if (m_ReadyToggle && m_ReadyToggle.IsChecked())
			ready = "READY";

		string tab = "-";
		if (m_Tabs)
			tab = m_Tabs.GetTabLabel(m_Tabs.GetIndex());

		string drop = "-";
		if (m_Dropdown)
			drop = m_Dropdown.GetText();

		string num = "-";
		if (m_Numeric)
			num = m_Numeric.GetText();

		m_Status.SetText("UPLINK  " + name + "  |  " + ready + "  |  " + tab + "  |  " + drop + "  |  " + num);
	}
}
