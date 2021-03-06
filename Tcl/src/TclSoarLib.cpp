/*
 * TclSoarLib.cpp
 *
 *  Created on: Nov 4, 2013
 *      Author: mazzin
 */

#include "TclSoarLib.h"

#ifdef __cplusplus
extern "C"
#endif
{
    using namespace sml;
    using namespace std;

    TclSoarLib::TclSoarLib(Kernel* myKernel) :
                                                      m_kernel(0),
                                                      m_interp(0)
    {
        m_kernel = myKernel;
        m_interp = Tcl_CreateInterp ();

        Tcl_FindExecutable (0);
    }

    void TclSoarLib::init_TclSoarLib()
    {
        if (!initialize_Tcl_Interpreter() || !initialize_Master())
        {
            exit (EXIT_FAILURE);
            return;
        }
    }

    TclSoarLib::~TclSoarLib()
    {
        if(m_kernel)
        {
            if(m_interp)
            {
                std::string output;
                GlobalEval("smlshutdown", output);
            }
        }

        if (m_interp)
        {
            Tcl_DeleteInterp(m_interp);
        }

        m_kernel = 0;
        m_interp = 0;

    }

    std::string& TclSoarLib::EscapeTclString(const char* in, std::string& out)
    {
        for(const char* c = in; *c != '\0'; ++c)
        {
            if(*c == '\\')
            {
                out += '\\';
            }
            out += *c;
        }
        return out;
    }

    int TclSoarLib::GlobalDirEval(const std::string& command, std::string& result)
    {
        if(!m_interp)
        {
            return TCL_ERROR;
        } else {

            string tcl_cmd_string;

            EscapeTclString(command.c_str(), tcl_cmd_string);
            if(Tcl_Eval(m_interp, (char*) tcl_cmd_string.c_str()) != TCL_OK)
            {
                tcl_cmd_string.erase();
                result = Tcl_GetStringResult(m_interp);
                return TCL_ERROR;
            }
            tcl_cmd_string.erase();
            result = Tcl_GetStringResult(m_interp);
        }
        return TCL_OK;
    }

    int TclSoarLib::GlobalEval(const std::string& command, std::string& result)
    {
        if(!m_interp)
        {
            return TCL_ERROR;
        } else {

            if(Tcl_Eval(m_interp, (char*) command.c_str()) != TCL_OK)
            {
                result = Tcl_GetStringResult(m_interp);
                return TCL_ERROR;
            }
            result = Tcl_GetStringResult(m_interp);
        }
        return TCL_OK;
    }

    bool TclSoarLib::initialize_Tcl_Interpreter()
    {
        /* -- Only needed if compiling with TCL_MEM_DEBUG if you want access to tcl memory command -- */
        //  Tcl_InitMemory(m_interp);

        if (Tcl_Init (m_interp) != TCL_OK) {
            Tcl_DeleteInterp(m_interp);
            return false;
        }

        return true;
    }
    bool TclSoarLib::evaluateDirCommand (const string command)
    {
        string tcl_cmd_string, result_string;

        EscapeTclString(command.c_str(), tcl_cmd_string);
        if (GlobalEval(tcl_cmd_string, result_string) != TCL_OK)
        {
            tcl_cmd_string.erase();
            return false;
        }

        tcl_cmd_string.erase();
        return true;
    }

    bool TclSoarLib::turnOn()
    {
        return (evaluateDirCommand("createCallbackHandlers"));
    }

    bool TclSoarLib::turnOff()
    {
        return (evaluateDirCommand("removeCallbackHandlers"));
    }

    // Test if a path exists and is not a directory.
    bool isFile(const char *path)
    {
#ifdef _WIN32
        DWORD a = GetFileAttributes(path);
        return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
#else
        struct stat st;
        return (stat(path, &st) == 0 && !S_ISDIR(st.st_mode));
#endif
    }
    // Test if a path exists and is not a directory.
    bool isDir(const char *path)
    {
#ifdef _WIN32
        DWORD a = GetFileAttributes(path);
        return a != INVALID_FILE_ATTRIBUTES && !(a & !FILE_ATTRIBUTE_DIRECTORY);
#else
        struct stat st;
        return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
#endif
    }

    bool TclSoarLib::initialize_Master()
    {
        string smlTclDir,  libDir, masterFilePath, result_string;

        if (((GlobalDirEval("pwd", libDir) != TCL_OK) ||
                    (GlobalDirEval("file join [pwd] tcl", smlTclDir) != TCL_OK) ||
                    (GlobalDirEval("file join [pwd] tcl master.tcl", masterFilePath) != TCL_OK)))
        {
            GlobalEval("puts {Error finding tcl scripts.}", result_string);
            return false;
        }
        else
        {
            if (!(isDir(libDir.c_str()) && isDir(smlTclDir.c_str()) && isFile(masterFilePath.c_str())))
            {
                libDir = getenv("SOAR_HOME");
                if (libDir.size() == 0) 
                {
                    GlobalEval("puts {Unable to find tcl scripts under current directory or SOAR_HOME, which is not currently set.}", result_string);
                    return false; 
                }

                smlTclDir = libDir;
                if (smlTclDir.find_last_of("/\\") != smlTclDir.size() - 1)
                {
                    smlTclDir += '/';
                }
                smlTclDir += "tcl";

                /* -- Normalize directory for any cross-platform differences-- */
                string normalizeCmd("file normalize ");
                normalizeCmd += smlTclDir;
                if (GlobalDirEval(normalizeCmd.c_str(), smlTclDir) != TCL_OK)
                    return false;

                masterFilePath = smlTclDir;
                masterFilePath += "/master.tcl";
                if (!(isDir(libDir.c_str()) && isDir(smlTclDir.c_str()) && isFile(masterFilePath.c_str())))
                {
                    GlobalEval("puts {Unable to find tcl scripts under SOAR_HOME}", result_string);
                    return false;
                }
            }
        }

        if (!evaluateDirCommand("source \"" + smlTclDir + "/dirstack.tcl\""))
        { 
            GlobalEval("puts {Unable to find tcl scripts under current directory or SOAR_HOME.}", result_string);
            return false; 
        }
        if (!evaluateDirCommand("pushd \"" + smlTclDir + "\""))
        {
            GlobalEval("puts {Unable to find tcl scripts under current directory or SOAR_HOME.}", result_string);
            return false;
        }

        if (GlobalDirEval("source master.tcl", result_string) != TCL_OK)
        {
            GlobalEval("puts {Unable to find tcl scripts under current directory or SOAR_HOME.}", result_string);
            return false;
        }
        if (GlobalDirEval("initializeMaster", result_string) != TCL_OK)
        {
            GlobalEval("puts {Error initializing master tcl interpreter}", result_string);
            return false;
        }

        if (!evaluateDirCommand("popd"))
        {
            GlobalEval("puts {Error initializing master tcl interpreter}", result_string);
            return false;
        }

        return true;
    }

} // #endif extern "C"
