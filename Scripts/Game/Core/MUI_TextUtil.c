//------------------------------------------------------------------------------------------------
//! TextWidget.SetText() treats the value as a string-table ID. Numbers ("1"), empty
//! strings, and ranking prefixes ("#1") spam "Missing string ID" every paint.
//! SetTextFormat still localizes a #param, so stamp NO_LOCALIZATION first.
//! Always set literal UI text through here.
//------------------------------------------------------------------------------------------------
class MUI_TextUtil
{
	//------------------------------------------------------------------------------------------------
	static void SetLiteral(TextWidget tw, string text)
	{
		if (!tw)
			return;
		tw.SetFlags(WidgetFlags.NO_LOCALIZATION);
		// Empty still looks up string-table id "". Use a space; callers that mean
		// "no text" should hide the widget or skip MeasureText/DrawText.
		if (text.IsEmpty())
			tw.SetTextFormat("%1", " ");
		else
			tw.SetTextFormat("%1", text);
	}
}
