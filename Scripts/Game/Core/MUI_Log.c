//------------------------------------------------------------------------------------------------
class MUI_Log
{
	protected static const string PREFIX = "[MUI]";

	//------------------------------------------------------------------------------------------------
	static void Info(string message)
	{
		Print(PREFIX + " " + message, LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	static void Warning(string message)
	{
		Print(PREFIX + " " + message, LogLevel.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	static void Error(string message)
	{
		Print(PREFIX + " " + message, LogLevel.ERROR);
	}
}
