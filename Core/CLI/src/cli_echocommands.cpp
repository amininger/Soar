/////////////////////////////////////////////////////////////////
// echo command file.
//
// Author: Douglas Pearson, doug@threepenny.net
// Date  : 2005
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "cli_CLIError.h"

#include "sml_Names.h"
#include "sml_KernelSML.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseEchoCommands(std::vector<std::string>& argv) {

	Options optionsData[] = {
		{'y', "yes",		OPTARG_NONE},
		{'n', "no",			OPTARG_NONE},
		{0, 0, OPTARG_NONE}
	};

	bool echoCommands = true ;
	bool onlyGetValue = true ;

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'y':
				echoCommands = true ;
				onlyGetValue = false ;
				break ;
			case 'n':
				echoCommands = false ;
				onlyGetValue = false ;
				break ;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	if (m_NonOptionArguments)
	{
		SetErrorDetail("Format is 'echo-commands [--yes | --no]") ;
		return SetError(CLIError::kGetOptError) ;
	}

	return DoEchoCommands(onlyGetValue, echoCommands);
}

bool CommandLineInterface::DoEchoCommands(bool onlyGetValue, bool echoCommands) {

	// We only set the value if asked, but we always report the current setting.
	if (!onlyGetValue)
	{
		m_pKernelSML->SetEchoCommands(echoCommands) ;
	}

	echoCommands = m_pKernelSML->GetEchoCommands() ;

	if (m_RawOutput) {
		m_Result << (echoCommands ? "Echoing commands." : "Not echoing commands.") ;
	} else {
		AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, echoCommands ? sml_Names::kTrue : sml_Names::kFalse);
	}

	return true;
}
