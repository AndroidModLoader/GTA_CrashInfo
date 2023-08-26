#include <mod/amlmod.h>
#include <yyjson/src/yyjson.h>
#include <fstream>
#include <unistd.h>

MYMOD(net.rusjj.crashinfo.gta, CrashInfo: GTA, 1.0, RusJJ)

std::ofstream g_pCrashInfoLog;
yyjson_doc* g_pCrashDataDoc = NULL;
//char g_szCrashData[16384];

extern "C" void OnModLoad()
{
    char path[320];
    sprintf(path, "%s/crash.data.json", aml->GetAndroidDataPath());
    if(access(path, F_OK) != 0) return; // Failed to load!

    //FILE* crashdataFile = fopen(path, "r");
    //if(crashdataFile)
    //{
    //    g_bCrashData = false;
    //    return;
    //}
    //int len = fread(g_szCrashData, 1, sizeof(g_szCrashData), crashdataFile);
    
    g_pCrashDataDoc = yyjson_read_file(path, 0, NULL, NULL);
}

extern "C" void OnGameCrash(const char* szLibName, int sig, int code, uintptr_t libaddr, mcontext_t* mcontext)
{
    #ifdef AML32
        uintptr_t PC = mcontext->arm_pc;
    #else
        uintptr_t PC = mcontext->pc;
    #endif
    uintptr_t CrashedPCOffset = PC - libaddr;

    char path[320];
    sprintf(path, "%s/gta_crashinfo.txt", aml->GetAndroidDataRootPath());
    g_pCrashInfoLog.open(path, std::ios::out | std::ios::trunc);

    #ifdef AML32
        char crashedHexAddr[16];
        sprintf(crashedHexAddr, "0x%X", libaddr);
    #else
        char crashedHexAddr[24];
        sprintf(crashedHexAddr, "0x%lX", libaddr);
    #endif

    // Something is wrong!
    if(!g_pCrashInfoLog.is_open()) return;

    g_pCrashInfoLog << "Crash has been happened in library " << szLibName << " at offset address 0x" << std::hex << std::uppercase << CrashedPCOffset << std::endl;
    g_pCrashInfoLog << std::dec << std::nouppercase; // not necessary?
    g_pCrashInfoLog << "An overall reason of the crash: ";
    switch(sig)
    {
    case SIGABRT:
        g_pCrashInfoLog << "Because an application got killed by someone";
        break;
    case SIGBUS:
        g_pCrashInfoLog << "Not enough memory, invalid execution address";
        break;
    case SIGFPE:
        g_pCrashInfoLog << "An error somewhere in the code, often - dividing by zero";
        break;
    case SIGSEGV:
        g_pCrashInfoLog << "An application tried to access the memory address that is unaccessible, protected or just wrong";
        break;
    case SIGILL:
        g_pCrashInfoLog << "Corrupted application stack, wrong call address or no privileges to do something";
        break;
    case SIGSTKFLT:
        g_pCrashInfoLog << "Stack fault on coprocessor";
        break;
    case SIGTRAP:
        g_pCrashInfoLog << "It`s a trap! Somewhere in the application is called \"it`s a trap! stop me!\"";
        break;
    }
    g_pCrashInfoLog << std::endl << std::endl;

    if(!szLibName || !szLibName[0])
    {
        g_pCrashInfoLog << "A crash happened in a random memory chunk! Sadly, no one is able to help you!";
        return;
    }

    if(g_pCrashDataDoc)
    {
        yyjson_val* pRoot = yyjson_doc_get_root(g_pCrashDataDoc);
        if(pRoot)
        {
            yyjson_val* pLibraryRoot = yyjson_obj_get(pRoot, szLibName);
            if(pLibraryRoot)
            {
                yyjson_val* crashData = yyjson_obj_get(pLibraryRoot, crashedHexAddr);
                if(!crashData) crashData = yyjson_obj_get(pLibraryRoot, "default");
                if(crashData)
                {
                    g_pCrashInfoLog << "Here is what i have for that crash:\n" << yyjson_get_str(crashData) << std::endl;
                }
                else
                {
                    goto no_info_moment;
                }
            }
            else
            {
              no_info_moment:
                g_pCrashInfoLog << "File crash.data.json has no info for library " << szLibName << "!\n";
                g_pCrashInfoLog << "Ask the author about this or report this crash to someone!" << std::endl;
            }
        }
        yyjson_doc_free(g_pCrashDataDoc);
    }
    else
    {
        g_pCrashInfoLog << "File crash.data.json has no info for library " << szLibName << "!\n";
        g_pCrashInfoLog << "Ask the author about this or report this crash to someone!\n";
        g_pCrashInfoLog << "Another reason of the missing crashdata may be the corrupted file." << std::endl;
    }
}